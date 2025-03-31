#include "encode_kpm_indication.hpp"

#include "E2SM-KPM-IndicationMessage.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "MeasurementDataItem.h"
#include "MeasurementRecordItem.h"
#include "MeasurementInfoItem.h"
#include "MeasurementLabel.h"
#include "MeasurementType.h"
#include "MeasurementTypeName.h"

#include "asn_application.h"
#include "asn_internal.h"
#include "aper_encoder.h"
#include "OCTET_STRING.h"
#include "INTEGER.h"
#include "asn_SEQUENCE_OF.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

bool encode_kpm_indication(const std::string& kpi_name, double value1, int64_t value2, std::vector<unsigned char>& buffer)
{
    // Message principal
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    if (!ind_msg) return false;
    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;

    // Format1
    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
    if (!format1) {
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }

    // Measurement Info
    MeasurementInfoItem_t* measInfoItem = (MeasurementInfoItem_t*)calloc(1, sizeof(MeasurementInfoItem_t));
    if (!measInfoItem) {
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        free(format1);
        return false;
    }
    measInfoItem->measType.present = MeasurementType_PR_measName;
    OCTET_STRING_fromString(&measInfoItem->measType.choice.measName, kpi_name.c_str());

    ASN_SEQUENCE_ADD(&format1->measInfoList->list, measInfoItem);  // CORRIGÉ

    // Measurement Data
    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));
    if (!measItem) {
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }

    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = value1;

    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;
    long val = static_cast<long>(value2);  // CORRIGÉ
    asn_long2INTEGER(&mri2->choice.integer, val);

    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);
    ASN_SEQUENCE_ADD(&format1->measData.list, measItem);

    // Assignation
    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = format1;

    // Encodage
    uint8_t* encoded_buf = nullptr;
    ssize_t encoded_len = aper_encode_to_new_buffer(&asn_DEF_E2SM_KPM_IndicationMessage, nullptr, ind_msg, (void**)&encoded_buf);  // CORRIGÉ

    if (encoded_len <= 0 || !encoded_buf) {
        std::cerr << "Failed to encode KPM indication" << std::endl;
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }

    buffer.assign(encoded_buf, encoded_buf + encoded_len);
    free(encoded_buf);
    ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
    return true;
}
