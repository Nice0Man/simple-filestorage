#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <chrono>
#include <optional>

namespace fileserver {
namespace utils {

/**
 * @brief LRU Cache для JWT токенов с TTL
 * 
 * Thread-safe кэш с автоматическим удалением устаревших записей
 */
template<typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity, std::chrono::seconds ttl = std::chrono::seconds{300})
        : capacity_(capacity), ttl_(ttl) {}
    
    /**
     * @brief Добавить или обновить значение в кэше
     */
    void put(const std::string& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto it = cache_map_.find(key);
        
        if (it != cache_map_.end()) {
            // Обновляем существующее значение
            items_list_.erase(it->second);
            cache_map_.erase(it);
        } else if (cache_map_.size() >= capacity_) {
            // Удаляем самый старый элемент
            evictOldest();
        }
        
        // Добавляем новый элемент в начало
        items_list_.push_front({key, value, now});
        cache_map_[key] = items_list_.begin();
        
        // Статистика
        stats_.total_puts++;
    }
    
    /**
     * @brief Получить значение из кэша
     */
    std::optional<Value> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            stats_.misses++;
            return std::nullopt;
        }
        
        // Проверяем TTL
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second->timestamp);
        
        if (age > ttl_) {
            // Значение устарело
            items_list_.erase(it->second);
            cache_map_.erase(it);
            stats_.misses++;
            stats_.expired++;
            return std::nullopt;
        }
        
        // Перемещаем элемент в начало (recently used)
        auto value = it->second->value;
        items_list_.splice(items_list_.begin(), items_list_, it->second);
        
        stats_.hits++;
        return value;
    }
    
    /**
     * @brief Удалить значение из кэша
     */
    void erase(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            items_list_.erase(it->second);
            cache_map_.erase(it);
            stats_.evictions++;
        }
    }
    
    /**
     * @brief Очистить весь кэш
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_map_.clear();
        items_list_.clear();
    }
    
    /**
     * @brief Получить размер кэша
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_map_.size();
    }
    
    /**
     * @brief Статистика кэша
     */
    struct Stats {
        uint64_t hits = 0;
        uint64_t misses = 0;
        uint64_t evictions = 0;
        uint64_t expired = 0;
        uint64_t total_puts = 0;
        
        double hitRate() const {
            auto total = hits + misses;
            return total > 0 ? static_cast<double>(hits) / total : 0.0;
        }
    };
    
    Stats getStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }
    
    /**
     * @brief Очистить устаревшие записи
     */
    void cleanExpired() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto it = items_list_.rbegin();
        
        while (it != items_list_.rend()) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->timestamp);
            if (age > ttl_) {
                cache_map_.erase(it->key);
                it = decltype(it)(items_list_.erase(std::next(it).base()));
                stats_.expired++;
            } else {
                break; // Список отсортирован по времени
            }
        }
    }
    
private:
    struct CacheItem {
        std::string key;
        Value value;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    void evictOldest() {
        if (!items_list_.empty()) {
            auto& oldest = items_list_.back();
            cache_map_.erase(oldest.key);
            items_list_.pop_back();
            stats_.evictions++;
        }
    }
    
    size_t capacity_;
    std::chrono::seconds ttl_;
    mutable std::mutex mutex_;
    
    std::list<CacheItem> items_list_;
    std::unordered_map<std::string, typename std::list<CacheItem>::iterator> cache_map_;
    
    Stats stats_;
};

/**
 * @brief Кэшированные данные пользователя для токена
 */
struct CachedUserData {
    std::string user_id;
    std::string username;
    std::string role;
    bool is_active;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_login;
};

} // namespace utils
} // namespace fileserver

