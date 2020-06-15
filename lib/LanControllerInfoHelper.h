#pragma once

#include "Types.h"
#include "DeviceObject.h"
#include "../Builder/IssueLogger.h"
#include "LanControllerInfo.h"

class LanControllerInfoHelper
{
public:
	static bool getInfo(	const Hardware::DeviceModule& lm,
							int lanControllerNo,
							E::LanControllerType lanControllerType,
							LanControllerInfo* lanControllerInfo,
							Builder::IssueLogger* log);

	static bool isProvideTuning(E::LanControllerType lanControllerType);
	static bool isProvideAppData(E::LanControllerType lanControllerType);
	static bool isProvideDiagData(E::LanControllerType lanControllerType);

	static QString getLanControllerSuffix(int controllerNo);

public:
	static const QString PROP_TUNING_ENABLE;
	static const QString PROP_TUNING_IP;
	static const QString PROP_TUNING_PORT;
	static const QString PROP_TUNING_SERVICE_ID;

	static const QString PROP_APP_DATA_ENABLE;
	static const QString PROP_APP_DATA_IP;
	static const QString PROP_APP_DATA_PORT;
	static const QString PROP_APP_DATA_SERVICE_ID;
	static const QString PROP_LM_APP_DATA_UID;
	static const QString PROP_LM_APP_DATA_SIZE;

	static const QString PROP_DIAG_DATA_ENABLE;
	static const QString PROP_DIAG_DATA_IP;
	static const QString PROP_DIAG_DATA_PORT;
	static const QString PROP_DIAG_DATA_SERVICE_ID;
	static const QString PROP_LM_DIAG_DATA_UID;
	static const QString PROP_LM_DIAG_DATA_SIZE;

	static const QString LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR;

private:
	static Hardware::DeviceController* getLanControllerBySuffix(const Hardware::DeviceModule& lm, const QString& suffix);
};


