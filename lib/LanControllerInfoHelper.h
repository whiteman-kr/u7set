#pragma once

#include "../CommonLib/Types.h"
#include "../HardwareLib/DeviceObject.h"
#include "ConstStrings.h"
#include "../Builder/IssueLogger.h"
#include "../Builder/Context.h"
#include "LanControllerInfo.h"

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

	static QString getLanControllerSuffix(int controllerNo);

public:
	static const QString LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR;

private:
	static Hardware::DeviceController* getLanControllerBySuffix(const Hardware::DeviceModule& lm, const QString& suffix);
};


