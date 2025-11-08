#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>
#include <libpq-fe.h>

namespace fileserver {
namespace database {

/**
 * @brief PostgreSQL connection wrapper with RAII
 */
class Connection {
public:
    explicit Connection(PGconn* conn);
    ~Connection();
    
    // Non-copyable, movable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;
    
    PGconn* get() const { return conn_; }
    bool isValid() const;
    void reset();
    
private:
    PGconn* conn_;
};

/**
 * @brief Thread-safe PostgreSQL connection pool
 * 
 * Implements connection pooling with automatic connection management,
 * health checks, and configurable pool size.
 */
class ConnectionPool {
public:
    struct Config {
        std::string host = "localhost";
        int port = 5432;
        std::string database = "fileserver";
        std::string username = "fileserver";
        std::string password = "fileserver123";
        int min_connections = 5;
        int max_connections = 20;
        std::chrono::seconds connection_timeout{30};
        std::chrono::seconds idle_timeout{300};
        bool enable_ssl = true;
    };
    
    explicit ConnectionPool(const Config& config);
    ~ConnectionPool();
    
    // Non-copyable, non-movable
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    /**
     * @brief Get a connection from the pool
     * @param timeout Maximum time to wait for a connection
     * @return Unique pointer to connection, nullptr if timeout
     */
    std::unique_ptr<Connection> getConnection(
        std::chrono::seconds timeout = std::chrono::seconds{10}
    );
    
    /**
     * @brief Return a connection to the pool
     * @param conn Connection to return
     */
    void returnConnection(std::unique_ptr<Connection> conn);
    
    /**
     * @brief Get pool statistics
     */
    struct Stats {
        int total_connections;
        int active_connections;
        int idle_connections;
        int failed_connections;
        int total_requests;
        int failed_requests;
    };
    
    Stats getStats() const;
    
    /**
     * @brief Health check - verify pool is working
     */
    bool healthCheck();
    
    /**
     * @brief Close all connections and shutdown pool
     */
    void shutdown();
    
private:
    Config config_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::unique_ptr<Connection>> available_connections_;
    
    // Statistics
    mutable Stats stats_{};
    
    bool shutdown_requested_;
    
    /**
     * @brief Create a new database connection
     */
    std::unique_ptr<Connection> createConnection();
    
    /**
     * @brief Build PostgreSQL connection string
     */
    std::string buildConnectionString() const;
    
    /**
     * @brief Initialize the connection pool
     */
    void initialize();
    
    /**
     * @brief Clean up expired connections
     */
    void cleanupConnections();
};

/**
 * @brief RAII wrapper for automatic connection return to pool
 */
class PooledConnection {
public:
    PooledConnection(std::unique_ptr<Connection> conn, ConnectionPool* pool);
    ~PooledConnection();
    
    // Non-copyable, movable
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;
    PooledConnection(PooledConnection&& other) noexcept;
    PooledConnection& operator=(PooledConnection&& other) noexcept;
    
    Connection* operator->() const { return conn_.get(); }
    Connection& operator*() const { return *conn_; }
    Connection* get() const { return conn_.get(); }
    
    bool isValid() const { return conn_ && conn_->isValid(); }
    
private:
    std::unique_ptr<Connection> conn_;
    ConnectionPool* pool_;
};

} // namespace database
} // namespace fileserver
