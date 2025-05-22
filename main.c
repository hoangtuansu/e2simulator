#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AMF-UE-NGAP-ID.h"
#include "xer_encoder.h"
#include "xer_decoder.h"

int main() {
    // 1. Créer un pointeur vers la structure
    AMF_UE_NGAP_ID_t *id = malloc(sizeof(AMF_UE_NGAP_ID_t));
    if (!id) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    // 2. Affecter une valeur
    *id = 12345;

    // 3. Encoder en XER (format XML)
    printf("Encodage XER de AMF_UE_NGAP_ID (valeur = %ld):\n", *id);
    if (xer_fprint(stdout, &asn_DEF_AMF_UE_NGAP_ID, id) != 0) {
        fprintf(stderr, "Erreur encodage XER\n");
        free(id);
        return EXIT_FAILURE;
    }

    // 4. Libérer la structure
    free(id);

    return EXIT_SUCCESS;
}
