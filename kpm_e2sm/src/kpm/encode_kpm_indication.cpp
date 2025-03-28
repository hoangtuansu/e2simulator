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

#include <iostream>
#include <vector>
#include <cstring>

extern "C" {
#include "E2SM-KPM-IndicationMessage.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "MeasurementData.h"
#include "MeasurementDataItem.h"
#include "MeasurementRecord.h"
#include "MeasurementRecordItem.h"
#include "asn_application.h"
#include "asn_internal.h"
}

bool encode_kpm_indication(
    const std::string& slice_type,
    double rx_brate_uplink_Mbps,
    int ul_n_samples,
    std::vector<uint8_t>& encoded_output
) {
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    if (!ind_msg) return false;

    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;
    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
    if (!format1) return false;

    format1->measData = (MeasurementData_t*)calloc(1, sizeof(MeasurementData_t));

    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));
    measItem->measRecord.numberOfItems = 2;
    ASN_SEQUENCE_ADD(&format1->measData->list, measItem);

    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem__present_real;
    mri1->choice.real = rx_brate_uplink_Mbps;
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);

    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem__present_integer;
    mri2->choice.integer = ul_n_samples;
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);

    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = *format1;

    uint8_t buffer[2048];
    asn_enc_rval_t er = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER,
        &asn_DEF_E2SM_KPM_IndicationMessage, ind_msg, buffer, sizeof(buffer));

    if (er.encoded > 0) {
        encoded_output.assign(buffer, buffer + er.encoded);
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return true;
    } else {
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }
}



