#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>

/**
 * @brief Thread-safe EventBus template class for event-driven communication.
 * 
 * Allows subscribing to events, publishing events to all subscribers,
 * and safely unsubscribing with optional waiting for handler completion.
 * 
 * @tparam EventType The type of event to handle
 */
template <typename EventType>
class EventBus {
public:
    using Handler = std::function<void(const EventType&)>;
    using SubscriptionId = uint64_t;

    EventBus() : next_id_(1) {}

    // Non-copyable, non-movable to ensure thread safety
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    ~EventBus() {
        // Wait for all active handlers to complete before destruction
        std::unique_lock<std::mutex> lock(active_mutex_);
        active_cv_.wait(lock, [this] { return active_handlers_.empty(); });
    }

    /**
     * @brief Subscribe a handler to receive events.
     * 
     * @param handler The function to call when an event is published
     * @return SubscriptionId A unique identifier for this subscription
     */
    SubscriptionId Subscribe(Handler handler) {
        std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
        SubscriptionId id = next_id_++;
        handlers_[id] = std::move(handler);
        return id;
    }

    /**
     * @brief Unsubscribe a handler from receiving events.
     * 
     * Note: If the handler is currently executing, this method returns
     * immediately without waiting. Use UnsubscribeAndWait for waiting behavior.
     * 
     * @param id The subscription ID returned from Subscribe
     * @return true if the handler was found and removed, false otherwise
     */
    bool Unsubscribe(SubscriptionId id) {
        std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
        return handlers_.erase(id) > 0;
    }

    /**
     * @brief Unsubscribe a handler and wait for its completion if executing.
     * 
     * This method guarantees that after it returns, the handler is no longer
     * subscribed and is not currently executing.
     * 
     * @param id The subscription ID returned from Subscribe
     * @return true if the handler was found and removed, false otherwise
     */
    bool UnsubscribeAndWait(SubscriptionId id) {
        {
            std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
            handlers_.erase(id);
        }

        // Wait for the handler to complete if it's currently executing
        std::unique_lock<std::mutex> lock(active_mutex_);
        active_cv_.wait(lock, [this, id] {
            return active_handlers_.find(id) == active_handlers_.end();
        });

        return true;
    }

    /**
     * @brief Publish an event to all subscribed handlers.
     * 
     * All handlers are called synchronously in the calling thread.
     * Handlers are called in an unspecified order.
     * 
     * @param event The event to publish
     */
    void Publish(const EventType& event) {
        // Take a snapshot of handlers under the lock
        std::vector<std::pair<SubscriptionId, Handler>> handlers_snapshot;
        {
            std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
            handlers_snapshot.reserve(handlers_.size());
            for (const auto& [id, handler] : handlers_) {
                handlers_snapshot.emplace_back(id, handler);
            }
        }

        // Call handlers outside the lock to prevent deadlocks
        for (const auto& [id, handler] : handlers_snapshot) {
            // Mark handler as active
            {
                std::lock_guard<std::mutex> lock(active_mutex_);
                active_handlers_.insert(id);
            }

            // Check if handler is still subscribed before calling
            bool is_subscribed = false;
            {
                std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
                is_subscribed = handlers_.find(id) != handlers_.end();
            }

            if (is_subscribed) {
                try {
                    handler(event);
                } catch (...) {
                    // Mark handler as inactive before re-throwing
                    {
                        std::lock_guard<std::mutex> lock(active_mutex_);
                        active_handlers_.erase(id);
                    }
                    active_cv_.notify_all();
                    throw;
                }
            }

            // Mark handler as inactive
            {
                std::lock_guard<std::mutex> lock(active_mutex_);
                active_handlers_.erase(id);
            }
            active_cv_.notify_all();
        }
    }

    /**
     * @brief Get the number of currently subscribed handlers.
     * 
     * @return size_t Number of subscribers
     */
    size_t SubscriberCount() const {
        std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
        return handlers_.size();
    }

private:
    mutable std::shared_mutex handlers_mutex_;  // Protects handlers_ map
    std::unordered_map<SubscriptionId, Handler> handlers_;
    std::atomic<SubscriptionId> next_id_;

    std::mutex active_mutex_;  // Protects active_handlers_ set
    std::condition_variable active_cv_;  // Signals when handlers complete
    std::unordered_set<SubscriptionId> active_handlers_;  // Currently executing handlers
};
