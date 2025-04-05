#ifndef BUILD_E2AP_INDICATION_HPP
#define BUILD_E2AP_INDICATION_HPP

#include <vector>
#include <cstdint>

extern "C" {
#include "E2AP-PDU.h"
}

/**
 * @brief Construit et encode un message E2AP RICindication contenant un message KPM encodé.
 * 
 * @param kpm_encoded Le buffer contenant le message KPM encodé.
 * @param e2ap_output Le buffer de sortie pour le message E2AP encodé.
 * @param request_id ID de la requête RIC.
 * @param ran_func_id ID de la fonction RAN.
 * @return true si l'encodage a réussi.
 * @return false en cas d'erreur.
 */
bool build_e2ap_indication(const std::vector<uint8_t>& kpm_encoded,
                           std::vector<uint8_t>& e2ap_output,
                           int request_id = 1,
                           int ran_func_id = 1);

#endif // BUILD_E2AP_INDICATION_HPP
