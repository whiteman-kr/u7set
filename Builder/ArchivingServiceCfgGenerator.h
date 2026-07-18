#pragma once

#include "SoftwareCfgGenerator.h"
#include <ArchSignal.pb.h>

namespace Builder
{
	class ArchivingServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ArchivingServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	private:
		bool writeArchSignalsFile();
		bool writeArchInfoV3File();

		bool copyArchSignal(const AppSignal* s, Proto::ArchSignal* ps) const;

		bool writeRunScriptFile(const QString& profile, const ArchivingServiceSettings& settings, E::OS os);
	};
}
