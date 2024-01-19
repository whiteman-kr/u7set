#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"
#include "SoftwareCfgGenerator.h"
#include "DeviceHelper.h"

namespace Builder
{
	class AcquiredDiagObject
	{
	public:
		Hardware::DeviceType deviceType = Hardware::DeviceType::DeviceTypeCount;	// means - not initialized
		QString equipmentID;
		Hash parent = 0;					// calcHash(parent.equipmentID)
	};

	class AcquiredDiagSignal : public AcquiredDiagObject
	{
	public:
		AcquiredDiagSignal() : deviceType(Hardware::DeviceType::DiagSignal) {}

		E::DiagLevel diagLevel = E::DiagLevel::Message;
		QString diagSignalTypeID;
		QString validitySignalID;
		int valueSizeBit = 0;
		int discreteContainerSize = 0;
		bool logChanges = false;
		bool archive = false;
		bool reserved = false;
		E::ApertureType apertureType = E::ApertureType::AbsValue;
		double coarseAperture = 0;
		double fineAperture = 0;
		Address16 absAddr;					// signal data address from beginning of module diag data offset in RUP diag packet
											// calculate as controller.DiagDataOffset + diagSignal.DataOffset
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

		bool writeDiagSignalTypesXml();
		bool writeDiagDataSourcesXml();

	private:

	};
}
