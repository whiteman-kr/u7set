#pragma once

#include "SoftwareCfgGenerator.h"
#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{
	class Context;

	class DiagnosticsCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		DiagnosticsCfgGenerator(Context* context, Hardware::Software* software);
		~DiagnosticsCfgGenerator() = default;

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	protected:
		bool initSchemaTags();
		bool writeSchemasByTags();
		bool writeAppSignals();
		bool writeBehaviorFile();
		bool writeDiagnosticsLogo();

	private:
		VFrame30::SchemaDetailsSet m_detailsSet;
		QStringList m_schemaTagList;		// Generated in writeDiagnosticsSettings
	};
}

