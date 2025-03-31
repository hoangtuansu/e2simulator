// encode_kpm_indication.cpp corrigé pour e2sim avec structures ASN.1

#include "E2SM-KPM-IndicationMessage.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "MeasurementInfoItem.h"
#include "MeasurementInfoList.h"
#include "MeasurementData.h"
#include "MeasurementDataItem.h"
#include "MeasurementRecord.h"
#include "MeasurementRecordItem.h"
#include "MeasurementType.h"
#include "asn_application.h"
#include "asn_internal.h"
#include "aper_encoder.h"
#include "OCTET_STRING.h"
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

bool encode_kpm_indication(const string& kpi_name, double value1, int64_t value2, vector<unsigned char>& buffer) {
    auto ind_msg = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    auto format1 = (E2SM_KPM_IndicationMessage_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));

    // ---------- MeasurementInfoItem ----------
    auto measInfoItem = (MeasurementInfoItem_t*)calloc(1, sizeof(MeasurementInfoItem_t));
    measInfoItem->measType.present = MeasurementType_PR_name;
    OCTET_STRING_fromString(&measInfoItem->measType.choice.name, kpi_name.c_str());
    ASN_SEQUENCE_ADD(&format1->measInfoList.list, measInfoItem);

    // ---------- MeasurementData ----------
    auto measItem = (MeasurementDataItem_t*)calloc(1, sizeof(MeasurementDataItem_t));

    auto mri1 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri1->present = MeasurementRecordItem_PR_real;
    mri1->choice.real = value1;

    auto mri2 = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));
    mri2->present = MeasurementRecordItem_PR_integer;
    asn_long2INTEGER(&mri2->choice.integer, value2);

    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri1);
    ASN_SEQUENCE_ADD(&measItem->measRecord.list, mri2);

    ASN_SEQUENCE_ADD(&format1->measData.list, measItem);

    // ---------- Assembly ----------
    ind_msg->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;
    ind_msg->indicationMessage_formats.choice.indicationMessage_Format1 = *format1;

    // ---------- Encoding ----------
    uint8_t temp_buf[8192];
    asn_enc_rval_t enc_ret = aper_encode_to_buffer(&asn_DEF_E2SM_KPM_IndicationMessage, nullptr, ind_msg, temp_buf, sizeof(temp_buf));
    if (enc_ret.encoded == -1) return false;

    buffer.assign(temp_buf, temp_buf + (enc_ret.encoded + 7) / 8);
    return true;
}
