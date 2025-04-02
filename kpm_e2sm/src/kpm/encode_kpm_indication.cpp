#include "encode_kpm_indication.hpp"

#include "E2SM-KPM-IndicationMessage.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "MeasurementInfoList.h" // ✅ Ajout obligatoire
#include "MeasurementData.h"     // ✅ Ajout obligatoire
#include "MeasurementDataItem.h"
#include "MeasurementRecordItem.h"
#include "MeasurementInfoItem.h"
#include "MeasurementLabel.h"
#include "MeasurementType.h"
#include "asn_application.h"
#include "asn_internal.h"
#include "OCTET_STRING.h"
#include "INTEGER.h"
#include "asn_SEQUENCE_OF.h"
#include "aper_encoder.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

bool encode_kpm_indication(const std::string& kpi_name, double value1, int64_t value2, std::vector<unsigned char>& buffer)
{
    E2SM_KPM_IndicationMessage_t* ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    if (!ind_msg) return false;

    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;

    E2SM_KPM_IndicationMessage_Format1_t* format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
    if (!format1) {
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }

    // ✅ Correction : Allocation avec sizeof(struct complet) et cast correct
    format1->measInfoList = (MeasurementInfoList_t*)calloc(1, sizeof(MeasurementInfoList_t));
    format1->measData = (MeasurementData_t*)calloc(1, sizeof(MeasurementData_t));

    if (!format1->measInfoList || !format1->measData) {
        if (format1->measInfoList) free(format1->measInfoList);
        if (format1->measData) free(format1->measData);
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        free(format1);
        return false;
    }

    // ✅ Allocation et initialisation correcte MeasurementInfoItem
    MeasurementInfoItem_t* measInfoItem = (MeasurementInfoItem_t*)calloc(1, sizeof(MeasurementInfoItem_t));
    measInfoItem->measType.present = MeasurementType_PR_measName;
    OCTET_STRING_fromString(&measInfoItem->measType.choice.measName, kpi_name.c_str());
    ASN_SEQUENCE_ADD(&format1->measInfoList->list, measInfoItem);

    // ✅ Allocation et initialisation correcte MeasurementDataItem
    MeasurementDataItem_t* measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));

    MeasurementRecordItem_t* mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = value1;

    MeasurementRecordItem_t* mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;

    asn_long2INTEGER(&mri2->choice.integer, value2);

    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);
    ASN_SEQUENCE_ADD(&format1->measData->list, measItem);

    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = format1;

    // ✅ Encodage APER correctement réalisé
    uint8_t* encoded_buf = nullptr;
    ssize_t encoded_size = aper_encode_to_new_buffer(&asn_DEF_E2SM_KPM_IndicationMessage, nullptr, ind_msg, (void**)&encoded_buf);

    if (encoded_size <= 0 || !encoded_buf) {
        std::cerr << "Échec de l'encodage APER" << std::endl;
        ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);
        return false;
    }

    buffer.assign(encoded_buf, encoded_buf + encoded_size);

    free(encoded_buf);
    ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, ind_msg);

    return true;
}
