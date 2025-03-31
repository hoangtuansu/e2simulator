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
#include "E2SM-KPM-IndicationHeader.h"
#include "E2SM-KPM-IndicationHeader-Format1.h"
#include "MeasurementData.h"
#include "MeasurementRecordItem.h"
#include "MeasurementInfoList.h"
#include "MeasurementInfoItem.h"
#include "E2SM-KPM-EventTriggerDefinition.h"
#include "E2SM-KPM-EventTriggerDefinition-Format1.h"
//#include "RIC-EventTriggerStyle-Type.h"
#include "RIC-EventTriggerStyle-Item.h"
#include "asn_application.h"
#include "asn_internal.h"
#include "E2AP-PDU.h"
#include "RICindication.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

bool encode_kpm_indication(const string& kpmType, double value1, int64_t value2, std::vector<unsigned char>& buffer) {
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;

    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));

    MeasurementInfoItem_t* measInfoItem = (MeasurementInfoItem_t*)calloc(1, sizeof(MeasurementInfoItem_t));
    OCTET_STRING_fromString(&measInfoItem->measType.choice.measName, kpmType.c_str());
    measInfoItem->measType.present = MeasurementType_PR_measName;
    ASN_SEQUENCE_ADD(&format1->measInfoList.list, measInfoItem);

    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));

    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = value1;
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);

    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;
    if (asn_long2INTEGER(&(mri2->choice.integer), value2) != 0) return false;
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);

    ASN_SEQUENCE_ADD(&format1->measData.list, measItem);

    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = format1;

    // Encodage
    asn_enc_rval_t enc_ret = aper_encode_to_vector(&asn_DEF_E2SM_KPM_IndicationMessage, NULL, ind_msg, buffer);
    ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
    return (enc_ret.encoded != -1);
}
