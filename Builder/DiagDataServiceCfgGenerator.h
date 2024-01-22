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
		AcquiredDiagObject(std::shared_ptr<const Hardware::DeviceObject> dev,
						   Hardware::DeviceType devType,
						   const QString& equipID);
		std::shared_ptr<const Hardware::DeviceObject> device;

		Hardware::DeviceType deviceType = Hardware::DeviceType::DeviceTypeCount;	// means - not initialized
		QString equipmentID;
		Hash parentHash = 0;					// calcHash(parent.equipmentID)
	};

	class AcquiredDiagSignal: public AcquiredDiagObject
	{
	public:
		AcquiredDiagSignal(std::shared_ptr<const Hardware::DiagSignal> ds);

		E::DiagLevel diagLevel = E::DiagLevel::Message;
		QString diagSignalTypeID;
		bool isReflection = false;
		QString reflectedSignalID;
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

		bool findAcquiredDiagSignals(const Hardware::DeviceModule* lm);
		bool findAcquiredParentObjects();

	private:
		std::vector<std::shared_ptr<const Hardware::DiagSignal>> m_acquiredDiagSignals;
		std::map<Hash, AcquiredDiagObject> m_acquiredDiagObjects;	// calcHash(diagObject->equipmentID) => acquiredDiagObject

	};
}
