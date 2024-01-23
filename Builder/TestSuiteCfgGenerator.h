#pragma once

#include "SoftwareCfgGenerator.h"
#include "../OnlineLib//SoftwareSettings.h"

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
	bool writeTestScripts();
	bool writeReportTemplates();

	bool checkScriptFileTags(std::shared_ptr<DbFile>& file, const QStringList& scriptTags);

private:
	QStringList m_tuningSources;
};

}

