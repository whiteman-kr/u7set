#pragma once

#include "../lib/ConstStrings.h"
#include "../Builder/IssueLogger.h"
#include "../Builder/Context.h"

#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/LanControllerInfo.h>

class LanControllerInfoHelper
{
public:
	static bool getInfo(const Hardware::DeviceModule& lm,
							E::LanControllerType lanControllerType,
							int lanControllerNo,
							const Builder::Context& context,
							bool ignoreTuningData,
							LanControllerInfo* lanControllerInfo,
							Builder::IssueLogger* log);

	static bool getInfo(const Hardware::DeviceModule& lm,
							E::LanControllerType lanControllerType,
							const Builder::Context& context,
							bool ignoreTuningData,
							LanControllersInfo* lanControllersInfo,
							Builder::IssueLogger* log);

	static QString getLanControllerSuffix(int controllerNo);

public:
	static const QString LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR;
};


