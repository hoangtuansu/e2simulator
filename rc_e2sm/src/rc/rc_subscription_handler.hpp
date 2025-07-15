#pragma once

// Fonction callback pour traiter un message E2SM-RC
void ran_control_callback(void* data);

// Fonction pour enregistrer le callback
void register_rc_subscription_callback(int action_id, void (*callback)(void*));
