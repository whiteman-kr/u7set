#pragma once

#include "SoftwareCfgGenerator.h"
#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{
	class Context;

	class MonitorCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		MonitorCfgGenerator(Context* context, Hardware::Software* software);
		~MonitorCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	protected:
		bool initSchemaTags();
		
		bool createAppEquipmentList(QStringList* equipmentList);
		bool createTuningEquipmentList(QStringList* equipmentList);

		bool writeSchemasByTags();

		// Generate tuning signals file
		//
		bool writeMonitorBehavior();
		bool writeMonitorLogo();

	private:
		VFrame30::SchemaDetailsSet m_detailsSet;
		QStringList m_schemaTagList;		// Generated in writeMonitorSettings
	};
}

