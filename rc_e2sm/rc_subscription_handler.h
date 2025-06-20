#ifndef RC_SUBSCRIPTION_HANDLER_H
#define RC_SUBSCRIPTION_HANDLER_H

#include <stdint.h>
#include <stddef.h>

// Déclare la fonction handler RC que tu as définie dans le .cpp
void handle_subscription_rc(uint8_t* event_trigger_buf, size_t event_trigger_len);

#endif // RC_SUBSCRIPTION_HANDLER_H
