#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CellConfiguration.h"
#include "asn_application.h"
#include "xer_encoder.h"

int main() {
    CellConfiguration_t config;
    memset(&config, 0, sizeof(config));
    xer_fprint(stdout, &asn_DEF_CellConfiguration, &config);
    return 0;
}
