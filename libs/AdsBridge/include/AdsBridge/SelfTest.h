#ifndef ADSB_TEST_DATA_H
#define ADSB_TEST_DATA_H

#include <string.h>

#include "Common.h"

#ifdef __cplusplus
extern "C"
{
#endif

	// Self-test functions.
	//
	bool AdsTestAdsConnectionStatus(size_t structSize, const struct AdsConnectionStatus* testValue);
	bool AdsTestMatsAppSignalParam(size_t structSize, const struct MatsAppSignalParam* testValue);
	bool AdsTestMatsAppSignalState(size_t structSize, const struct MatsAppSignalState* testValue);

	inline bool AdsSelfTest()
	{
		struct AdsConnectionStatus adsTestConnectionStatus;
		struct MatsAppSignalParam matsTestAppSignalParam;
		struct MatsAppSignalState matsTestAppSignalState;

		memset(&adsTestConnectionStatus, 0, sizeof(adsTestConnectionStatus));
		adsTestConnectionStatus.id = 2ull;
		adsTestConnectionStatus.status = true;
		adsTestConnectionStatus.setConnectionResult = ADS_SET_CONNECTION_RESULT_WRONG_CLIENT_HOST_NAME;
		adsTestConnectionStatus.connectionType = (char*)0x223344998899AABBull;
		adsTestConnectionStatus.port = 7654;
		adsTestConnectionStatus.address = (char*)0xBBAADDFF7711AA99ull;
		adsTestConnectionStatus.adsEquipmentId = (char*)0x77223399BBAAEEDDull;
		adsTestConnectionStatus.received = 1234567ull;
		adsTestConnectionStatus.sent = 7654321ull;
		adsTestConnectionStatus.requestCount = 123ull;
		adsTestConnectionStatus.replyCount = 456ull;

		memset(&matsTestAppSignalParam, 0, sizeof(matsTestAppSignalParam));
		matsTestAppSignalParam.hash = 0x123456789abcdef0ull;
		matsTestAppSignalParam.appSignalId = (char*)0x223344998899AABBull;
		matsTestAppSignalParam.customSignalId = (char*)0xBBAADDFF7711AA99ull;
		matsTestAppSignalParam.caption = (char*)0x77223399BBAAEEDDull;
		matsTestAppSignalParam.equipmentId = (char*)0x2233D1998899AA9Aull;
		matsTestAppSignalParam.lmEquipmentId = (char*)0x3233D199889AAA8Bull;
		matsTestAppSignalParam.units = (char*)0xBBAADDFF7711AA99ull;
		matsTestAppSignalParam.tags = (char*)0x223344998899AABBull;
		matsTestAppSignalParam.channel = MATS_CHANNEL_D;
		matsTestAppSignalParam.inOutType = MATS_SIGNAL_INTERNAL;
		matsTestAppSignalParam.type = MATS_SIGNAL_DISCRETE;
		matsTestAppSignalParam.decimalPlaces = 5;
		matsTestAppSignalParam.lowValidRange = -100.0;
		matsTestAppSignalParam.highValidRange = 100.0;
		matsTestAppSignalParam.tuning = true;

		memset(&matsTestAppSignalState, 0, sizeof(matsTestAppSignalState));
		matsTestAppSignalState.hash = 0x123456789abcdef0ull;
		matsTestAppSignalState.plantTime = 0x123456789abcdef0ull;
		matsTestAppSignalState.serverTime = 0x123456789abcdef0ull;
		matsTestAppSignalState.value = 123.456;
		matsTestAppSignalState.flags = MATS_FLAG_VALID | MATS_FLAG_TUNING_DEFAULT;

		return AdsTestAdsConnectionStatus(sizeof adsTestConnectionStatus, &adsTestConnectionStatus) &&
			   AdsTestMatsAppSignalParam(sizeof matsTestAppSignalParam, &matsTestAppSignalParam) &&
			   AdsTestMatsAppSignalState(sizeof matsTestAppSignalState, &matsTestAppSignalState);
	}

#ifdef __cplusplus
} // extern "C"
#endif

#endif