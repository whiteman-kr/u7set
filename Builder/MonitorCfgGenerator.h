#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class Context;

	class MonitorCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		MonitorCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	protected:
		bool initSchemaTags();
		bool initTuningSources();

		bool writeSchemasByTags();

		// Generate tuning signals file
		//
		bool writeTuningSignals();
		bool writeMonitorBehavior();
		bool writeMonitorLogo();

	private:
		QStringList m_tuningSources;

		VFrame30::SchemaDetailsSet m_detailsSet;
		QStringList m_schemaTagList; // Generated in writeMonitorSettings
	};
} // namespace Builder