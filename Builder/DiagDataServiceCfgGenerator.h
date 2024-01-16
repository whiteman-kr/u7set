#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"
#include "SoftwareCfgGenerator.h"
#include "DeviceHelper.h"

namespace Builder
{
	class AcquiredDiagSignal
	{
	public:
	};

	class AcquiredDiagObject
	{
	public:
		Hardware::DeviceType deviceType;
		int place = -1;
		QString equipmentID;

		AcquiredDiagObject children;		// ordered by place ascending
	};


	class DiagDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:

	public:
		DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software);
		~DiagDataServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeRunScriptFile(const QString& profile, const DiagDataServiceSettings& settings, E::OS os);

		bool writeDiagDataSourcesXml();

	private:

	};
}
