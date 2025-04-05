#include "kpm_callbacks.hpp"

// Définition des variables globales utilisées dans kpm_callbacks.cpp
int global_sock = 1;                     // remplace par ton socket réel si nécessaire
long global_ran_function_id = 1;        // ID de fonction RAN utilisée dans E2AP
uint16_t global_ran_node_id = 101;      // ID du nœud RAN fictif

int main() {
    generate_and_send_kpm_report();
    return 0;
}
