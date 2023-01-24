#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{

class Context;

class TestSuiteCfgGenerator : public SoftwareCfgGenerator
{
public:
	TestSuiteCfgGenerator(Context* context, Hardware::Software* software);
	~TestSuiteCfgGenerator();

	virtual bool createSettingsProfile(const QString& profile) override;
	virtual bool generateConfigurationStep1() override;

protected:
	bool initTuningSources();

	bool writeTuningSignals();

private:
	QStringList m_tuningSources;
};

}

