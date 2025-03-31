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
#include "MeasurementInfoItem.h"
#include "MeasurementRecordItem.h"
#include "MeasurementData.h"
#include "MeasurementDataItem.h"
#include "asn_application.h"
#include "asn_internal.h"
#include <vector>
#include <iostream>
#include <cstdlib>  // For calloc

using namespace std;

bool encode_kpm_indication(const string& measurement_name, double value1, int64_t value2, vector<unsigned char>& buffer) {
    // Allocation du message d'indication principal
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    if (!ind_msg) return false;

    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;

    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
    if (!format1) return false;

    // Données de mesure
    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));
    if (!measItem) return false;

    // Mesure 1 (valeur float)
    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = value1;

    // Mesure 2 (valeur entiere)
    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;
    ASN_STRUCT_RESET(asn_DEF_INTEGER, &mri2->choice.integer);
    if (asn_long2INTEGER(&mri2->choice.integer, value2) != 0) return false;

    // Ajout dans le record
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);

    // Ajout dans MeasurementData
    ASN_SEQUENCE_ADD(&format1->measData.list, measItem);

    // Assignation finale
    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = *format1;

    // Encodage ASN.1
    asn_enc_rval_t ec = der_encode_to_buffer(&asn_DEF_E2SM_KPM_IndicationMessage, ind_msg, buffer.data(), buffer.size());

    return ec.encoded > 0;
}
