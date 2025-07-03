#include "rc_subscription_handler.hpp"
#include "messagerouting/subscription_callbacks.hpp"
#include <iostream>

void ran_control_callback(void* data) {
    std::cout << "🔁 E2SM-RC Callback triggered (ran_control_callback)." << std::endl;
    // TODO: ajouter le décodage de ActionDefinition ici
}

void register_rc_subscription_callback(int action_id, void (*callback)(void*)) {
    subscription_callbacks[action_id] = callback;
    std::cout << "📌 E2SM-RC callback registered for action ID: " << action_id << std::endl;
}
