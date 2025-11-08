#include "database/connection_pool.h"
#include <stdexcept>
#include <sstream>
#include <thread>
#include <algorithm>

namespace fileserver {
namespace database {

// Connection implementation
Connection::Connection(PGconn* conn) : conn_(conn) {}

Connection::~Connection() {
    if (conn_) {
        PQfinish(conn_);
    }
}

Connection::Connection(Connection&& other) noexcept : conn_(other.conn_) {
    other.conn_ = nullptr;
}

Connection& Connection::operator=(Connection&& other) noexcept {
    if (this != &other) {
        if (conn_) {
            PQfinish(conn_);
        }
        conn_ = other.conn_;
        other.conn_ = nullptr;
    }
    return *this;
}

bool Connection::isValid() const {
    return conn_ && PQstatus(conn_) == CONNECTION_OK;
}

void Connection::reset() {
    if (conn_) {
        PQreset(conn_);
    }
}

// ConnectionPool implementation
ConnectionPool::ConnectionPool(const Config& config) 
    : config_(config), shutdown_requested_(false) {
    initialize();
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

void ConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create minimum number of connections
    for (int i = 0; i < config_.min_connections; ++i) {
        auto conn = createConnection();
        if (conn && conn->isValid()) {
            available_connections_.push(std::move(conn));
            stats_.total_connections++;
        } else {
            stats_.failed_connections++;
        }
    }
    
    if (available_connections_.empty()) {
        throw std::runtime_error("Failed to create any database connections");
    }
}

std::string ConnectionPool::buildConnectionString() const {
    std::ostringstream oss;
    oss << "host=" << config_.host
        << " port=" << config_.port
        << " dbname=" << config_.database
        << " user=" << config_.username
        << " password=" << config_.password
        << " connect_timeout=" << config_.connection_timeout.count();
    
    if (config_.enable_ssl) {
        oss << " sslmode=require";
    } else {
        oss << " sslmode=disable";
    }
    
    return oss.str();
}

std::unique_ptr<Connection> ConnectionPool::createConnection() {
    std::string conn_str = buildConnectionString();
    PGconn* pg_conn = PQconnectdb(conn_str.c_str());
    
    if (!pg_conn) {
        return nullptr;
    }
    
    auto conn = std::make_unique<Connection>(pg_conn);
    if (!conn->isValid()) {
        return nullptr;
    }
    
    // Set connection parameters
    PQexec(pg_conn, "SET client_encoding = 'UTF8'");
    PQexec(pg_conn, "SET timezone = 'UTC'");
    PQexec(pg_conn, "SET statement_timeout = '30s'");
    
    return conn;
}

std::unique_ptr<Connection> ConnectionPool::getConnection(std::chrono::seconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    stats_.total_requests++;
    
    // Wait for available connection or timeout
    auto deadline = std::chrono::steady_clock::now() + timeout;
    
    while (available_connections_.empty() && !shutdown_requested_) {
        if (stats_.total_connections < config_.max_connections) {
            // Try to create new connection
            lock.unlock();
            auto new_conn = createConnection();
            lock.lock();
            
            if (new_conn && new_conn->isValid()) {
                stats_.total_connections++;
                stats_.active_connections++;
                return new_conn;
            } else {
                stats_.failed_connections++;
            }
        }
        
        // Wait for connection to become available
        if (condition_.wait_until(lock, deadline) == std::cv_status::timeout) {
            stats_.failed_requests++;
            return nullptr;
        }
    }
    
    if (shutdown_requested_ || available_connections_.empty()) {
        stats_.failed_requests++;
        return nullptr;
    }
    
    auto conn = std::move(available_connections_.front());
    available_connections_.pop();
    
    // Verify connection is still valid
    if (!conn->isValid()) {
        conn->reset();
        if (!conn->isValid()) {
            // Connection is dead, try to create a new one
            lock.unlock();
            auto new_conn = createConnection();
            lock.lock();
            
            if (new_conn && new_conn->isValid()) {
                stats_.active_connections++;
                return new_conn;
            } else {
                stats_.failed_connections++;
                stats_.failed_requests++;
                return nullptr;
            }
        }
    }
    
    stats_.active_connections++;
    stats_.idle_connections = available_connections_.size();
    
    return conn;
}

void ConnectionPool::returnConnection(std::unique_ptr<Connection> conn) {
    if (!conn) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (shutdown_requested_) {
        return;
    }
    
    stats_.active_connections--;
    
    if (conn->isValid()) {
        available_connections_.push(std::move(conn));
        stats_.idle_connections = available_connections_.size();
        condition_.notify_one();
    } else {
        stats_.total_connections--;
        stats_.failed_connections++;
    }
}

ConnectionPool::Stats ConnectionPool::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool ConnectionPool::healthCheck() {
    auto conn = getConnection(std::chrono::seconds{5});
    if (!conn) {
        return false;
    }
    
    PGresult* result = PQexec(conn->get(), "SELECT 1");
    bool is_healthy = result && PQresultStatus(result) == PGRES_TUPLES_OK;
    
    if (result) {
        PQclear(result);
    }
    
    returnConnection(std::move(conn));
    return is_healthy;
}

void ConnectionPool::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    shutdown_requested_ = true;
    
    // Clear all connections
    while (!available_connections_.empty()) {
        available_connections_.pop();
    }
    
    condition_.notify_all();
}

void ConnectionPool::cleanupConnections() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove invalid connections
    std::queue<std::unique_ptr<Connection>> valid_connections;
    
    while (!available_connections_.empty()) {
        auto conn = std::move(available_connections_.front());
        available_connections_.pop();
        
        if (conn->isValid()) {
            valid_connections.push(std::move(conn));
        } else {
            stats_.total_connections--;
            stats_.failed_connections++;
        }
    }
    
    available_connections_ = std::move(valid_connections);
    stats_.idle_connections = available_connections_.size();
}

// PooledConnection implementation
PooledConnection::PooledConnection(std::unique_ptr<Connection> conn, ConnectionPool* pool)
    : conn_(std::move(conn)), pool_(pool) {}

PooledConnection::~PooledConnection() {
    if (conn_ && pool_) {
        pool_->returnConnection(std::move(conn_));
    }
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : conn_(std::move(other.conn_)), pool_(other.pool_) {
    other.pool_ = nullptr;
}

PooledConnection& PooledConnection::operator=(PooledConnection&& other) noexcept {
    if (this != &other) {
        if (conn_ && pool_) {
            pool_->returnConnection(std::move(conn_));
        }
        conn_ = std::move(other.conn_);
        pool_ = other.pool_;
        other.pool_ = nullptr;
    }
    return *this;
}

} // namespace database
} // namespace fileserver
