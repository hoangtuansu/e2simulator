#include "rc_handler.hpp"
#include <stdio.h>
#include <stdint.h>
#include "E2Sim.hpp"

extern "C" {
#include "E2SM-RC-EventTriggerDefinition.h"
#include "asn_application.h"
#include "asn_internal.h"
}

void handle_subscription_rc(uint8_t* buf, size_t len) {
    printf("📨 Début du décodage de l'EventTrigger RC\n");

    E2SM_RC_EventTriggerDefinition_t *event_trigger = nullptr;

    asn_dec_rval_t rval = asn_decode(
        NULL,
        ATS_ALIGNED_BASIC_PER,
        &asn_DEF_E2SM_RC_EventTriggerDefinition,
        (void**)&event_trigger,
        buf,
        len
    );

    if (rval.code != RC_OK) {
        printf("❌ Erreur de décodage de l’EventTriggerDefinition RC\n");
        return;
    }

    // Exemple minimal : affichage du type d'événement s’il est défini
    if (event_trigger->ric_eventTriggerDefinition_formats.present ==
        E2SM_RC_EventTriggerDefinition__ric_eventTriggerDefinition_formats_PR_eventTrigger_Format1) {
        printf("✔️ Format1 reçu dans EventTriggerDefinition\n");
    }

    ASN_STRUCT_FREE(asn_DEF_E2SM_RC_EventTriggerDefinition, event_trigger);
}
