#pragma once

#include "Types.h"
#include "DeviceObject.h"
#include "ConstStrings.h"
#include "../Builder/IssueLogger.h"
#include "LanControllerInfo.h"

class LanControllerInfoHelper
{
public:
	static bool getInfo(	const Hardware::DeviceModule& lm,
							int lanControllerNo,
							E::LanControllerType lanControllerType,
							LanControllerInfo* lanControllerInfo,
							const Hardware::EquipmentSet& equipmentSet,
							Builder::IssueLogger* log);

	static bool isProvideTuning(E::LanControllerType lanControllerType);
	static bool isProvideAppData(E::LanControllerType lanControllerType);
	static bool isProvideDiagData(E::LanControllerType lanControllerType);

	static QString getLanControllerSuffix(int controllerNo);

public:
	static const QString LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR;

	static const QString IP_NULL;

private:
	static Hardware::DeviceController* getLanControllerBySuffix(const Hardware::DeviceModule& lm, const QString& suffix);
};


