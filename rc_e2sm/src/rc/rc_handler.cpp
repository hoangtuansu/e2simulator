#include "rc_handler.hpp"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
    #include "asn_application.h"
    #include "asn_internal.h"

    // ASN.1 headers générés
    #include "E2SM-RC-EventTrigger.h"
    #include "E2SM-RC-EventTrigger-Format1.h"
    #include "E2SM-RC-EventTrigger-Format2.h"
}

void handle_subscription_rc(uint8_t* buf, size_t len) {
    printf("📨 Début du décodage de l'EventTrigger RC\n");

    E2SM_RC_EventTrigger_t *event_trigger = nullptr;

    asn_dec_rval_t rval = asn_decode(
        NULL,
        ATS_ALIGNED_BASIC_PER,
        &asn_DEF_E2SM_RC_EventTrigger,
        (void**)&event_trigger,
        buf,
        len
    );

    if (rval.code != RC_OK || !event_trigger) {
        printf("❌ Erreur de décodage de l'EventTriggerDefinition RC\n");
        return;
    }

    switch (event_trigger->ric_eventTrigger_formats.present) {
        case E2SM_RC_EventTrigger__ric_eventTrigger_formats_PR_eventTrigger_Format1:
            printf("✔️ Format 1 détecté dans EventTrigger\n");
            break;

        case E2SM_RC_EventTrigger__ric_eventTrigger_formats_PR_eventTrigger_Format2:
            printf("✔️ Format 2 détecté dans EventTrigger\n");
            break;

        default:
            printf("⚠️ Format inconnu dans EventTrigger\n");
            break;
    }

    ASN_STRUCT_FREE(asn_DEF_E2SM_RC_EventTrigger, event_trigger);
}
