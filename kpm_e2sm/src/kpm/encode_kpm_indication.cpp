/*****************************************************************************
#                                                                            *
# Copyright 2020 AT&T Intellectual Property                                  *
#                                                                            *
# Licensed under the Apache License, Version 2.0 (the "License");            *
# you may not use this file except in compliance with the License.           *
# You may obtain a copy of the License at                                    *
#                                                                            *
#      http://www.apache.org/licenses/LICENSE-2.0                            *
#                                                                            *
# Unless required by applicable law or agreed to in writing, software        *
# distributed under the License is distributed on an "AS IS" BASIS,          *
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
# See the License for the specific language governing permissions and        *
# limitations under the License.                                             *
#                                                                            *
******************************************************************************/
#include "encode_kpm_indication.hpp"

#include "E2SM-KPM-IndicationMessage.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "MeasurementData.h"
#include "MeasurementRecord.h"
#include "MeasurementRecordItem.h"

#include <iostream>
#include <vector>
#include <cstdlib>   // Pour calloc, free
#include <cstring>   // Pour memcpy

extern "C" {
#include "asn_application.h"
#include "asn_internal.h"
}

using namespace std;

bool encode_kpm_indication(const string& metricName, double metricValue, int timestamp, std::vector<unsigned char>& buffer)
{
    // Créer le message d’indication principal
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    if (!ind_msg) return false;

    ind_msg->present = E2SM_KPM_IndicationMessage_PR_indicationMessage_formats;
    ind_msg->choice.indicationMessage_formats.present = E2SM_KPM_IndicationMessage_Format_PR_indicationMessage_Format1;

    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
    if (!format1) return false;

    // Créer les données de mesure
    MeasurementData_t* measData = (MeasurementData_t*)calloc(1, sizeof(MeasurementData_t));
    if (!measData) return false;

    // Créer un élément de mesure
    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));
    if (!measItem) return false;

    // Créer des valeurs de mesure
    MeasurementRecord_t* measRecord = &measItem->measRecord;

    // Élément 1 : valeur réelle
    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = metricValue;

    ASN_SEQUENCE_ADD(&measRecord->list, mri1);

    // Élément 2 : valeur entière
    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;
    mri2->choice.integer = timestamp;

    ASN_SEQUENCE_ADD(&measRecord->list, mri2);

    ASN_SEQUENCE_ADD(&measData->list, measItem);

    format1->measData = *measData;  // Copie du contenu (pas pointeur)
    ind_msg->choice.indicationMessage_formats.choice.indicationMessage_Format1 = *format1;

    // Encoder
    asn_enc_rval_t ec;
    unsigned char temp_buf[4096];
    ec = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationMessage, ind_msg, temp_buf, sizeof(temp_buf));
    if (ec.encoded == -1) {
        std::cerr << "ASN.1 encode failed." << std::endl;
        return false;
    }

    buffer.assign(temp_buf, temp_buf + ec.encoded);

    // Libération mémoire
    ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
    free(format1);
    free(measData);

    return true;
}



