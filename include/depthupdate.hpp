#pragma once

#include <boost/asio/strand.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <boost/json.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <shared_mutex>

using tcp = boost::asio::ip::tcp;

constexpr auto binanceApiHost = "fapi.binance.com";

class ThreadSafeHashMap {
public:
    ThreadSafeHashMap(const ThreadSafeHashMap&) = delete;
    ThreadSafeHashMap& operator=(const ThreadSafeHashMap&) = delete;

    static ThreadSafeHashMap& getInstance() {
        static ThreadSafeHashMap instance;
        return instance;
    }
    void insert(const std::string& key, const uint64_t& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        hashmap_[key] = value;
    }

    bool get(const std::string& key, uint64_t& value) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = hashmap_.find(key);
        if (it == hashmap_.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    bool remove(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = hashmap_.find(key);
        if (it == hashmap_.end()) {
            return false;
        }
        hashmap_.erase(it);
        return true;
    }

    bool contains(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return hashmap_.find(key) != hashmap_.end();
    }

private:
    ThreadSafeHashMap() {}
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, uint64_t> hashmap_;
};

void update_depth(std::string const& ticker);