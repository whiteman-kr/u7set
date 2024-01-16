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
		// deviceType == Hardware::DeviceType::DiagSignal;
		//
		QString equipmentID;
		int place = -1;

		QString diagSignalTypeID;
	};

	/*class AcquiredDiagObject
	{
	public:
		Hardware::DeviceType deviceType;
		QString equipmentID;
		int place = -1;

		AcquiredDiagObject children;		// ordered by place ascending
	};*/

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

		bool writeDiagSignalTypesXml();
		bool writeDiagDataSourcesXml();

	private:

	};
}
