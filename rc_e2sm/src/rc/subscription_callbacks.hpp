#ifndef SUBSCRIPTION_CALLBACKS_HPP
#define SUBSCRIPTION_CALLBACKS_HPP

#include <functional>
#include <unordered_map>
#include <string>

typedef std::function<void(void* subscription_data)> SubscriptionCallback;

class SubscriptionCallbackRegistry {
public:
    void register_callback(const std::string& ran_function, SubscriptionCallback cb) {
        callbacks[ran_function] = cb;
    }

    SubscriptionCallback get_callback(const std::string& ran_function) {
        if (callbacks.find(ran_function) != callbacks.end()) {
            return callbacks[ran_function];
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, SubscriptionCallback> callbacks;
};

#endif // SUBSCRIPTION_CALLBACKS_HPP
