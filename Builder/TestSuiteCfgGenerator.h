#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class Context;

	class TestSuiteCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TestSuiteCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	protected:
		bool createTuningEquipmentList(QStringList* equipmentList);

		bool writeTestScripts();
		bool writeReportTemplates();

		bool checkScriptFileTags(std::shared_ptr<DbFile>& file, const QStringList& scriptTags);
	};

} // namespace Builder
