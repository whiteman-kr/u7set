#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceModule
	//
	//
	class DeviceModule : public DeviceObject
	{
		Q_OBJECT

	public:
		enum FamilyType		// WARNING!!! Only high byte can be used as a part of the type
		{					// (high byte is a module family, low byte is a module version)
			OTHER = 0x0000,
			LM = 0x1100,
			AIM = 0x1200,
			AOM = 0x1300,
			DIM = 0x1400,
			DOM = 0x1500,
			AIFM = 0x1600,
			OCM = 0x1700,
			WAIM = 0x1800,
			TIM = 0x1900,
			RIM = 0x1A00,
			FIM = 0x1B00,
			MPS = 0x5100,
			BVK4 = 0x5300,	// obsolete, for compatibility
			BP336 = 0x5500,	// obsolete, for compatibility
			BVB = 0x5600,
			BUIM = 0x5700,
			VDU = 0x1C00
		};
		Q_ENUM(FamilyType)

	public:
		explicit DeviceModule(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceModule() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		[[nodiscard]] FamilyType moduleFamily() const;
		void setModuleFamily(FamilyType value);

		[[nodiscard]] int customModuleFamily() const;
		void setCustomModuleFamily(int value);

		[[nodiscard]] int moduleVersion() const;
		void setModuleVersion(int value);

		[[nodiscard]] QString configurationScript() const;
		void setConfigurationScript(const QString& value);

		[[nodiscard]] QString rawDataDescription() const;
		void setRawDataDescription(const QString& value);
		[[nodiscard]] bool hasRawData() const;

		[[nodiscard]] int moduleType() const;

		[[nodiscard]] bool isIOModule() const;
		[[nodiscard]] bool isInputModule() const;
		[[nodiscard]] bool isOutputModule() const;
		[[nodiscard]] bool isLogicModule() const;
		[[nodiscard]] bool isFSCConfigurationModule() const;
		[[nodiscard]] bool isOptoModule() const;
		[[nodiscard]] bool isBvb() const;
		[[nodiscard]] bool isVdu() const;

		// Data
		//
	private:
		uint16_t m_type = 0;	// high byte is family type, low byte is module version

		uint16_t m_customModuleFamily = 0;

		QString m_configurationScript;
		QString m_rawDataDescription;
	};
} // namespace Hardware
