#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <array>
#include "../lib/DebugInstCounter.h"
#include "../lib/PropertyObject.h"
#include "../lib/Factory.h"
#include "../CommonLib/Types.h"
#include "../Proto/ProtoSerialization.h"

class DbFileInfo;

namespace Hardware
{
	extern const std::array<QString, 10> DeviceObjectExtensions;
	extern const std::array<QString, 10> DeviceTypeNames;


	void init();
	void shutdown();

	class DeviceObject;
	class DeviceRoot;
	class DeviceSystem;
	class DeviceRack;
	class DeviceChassis;
	class DeviceModule;
	class DeviceController;
	class Workstation;
	class Software;
	class DeviceAppSignal;
	class DeviceDiagSignal;


	// Device type, for defining hierrarche, don't save these data to file, can be changed (new level) later
	// If you add or change order in this enum, DO NOT FORGET TO CHANGE !!!!DeviceObjectExtensions!!!!
	//
	enum class DeviceType
	{
		Root,
		System,
		Rack,
		Chassis,
		Module,

		Workstation,
		Software,

		Controller,

		AppSignal,
		DiagSignal,

		DeviceTypeCount
	};

	// Property names
	//
	class PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		static const QString fileId;
		static const QString uuid;
		static const QString equipmentIdTemplate;
		static const QString equipmentId;
		static const QString caption;
		static const QString childRestriction;
		static const QString place;
		static const QString specificProperties;
		static const QString signalSpecificProperties;
		static const QString preset;
		static const QString presetRoot;
		static const QString presetName;
		static const QString presetObjectUuid;

		static const QString lmDescriptionFile;
		static const QString lmNumber;
		static const QString lmSubsystemChannel;
		static const QString lmSubsystemID;

		static const QString type;
		static const QString function;
		static const QString byteOrder;
		static const QString format;
		static const QString memoryArea;
		static const QString size;

		static const QString valueOffset;
		static const QString valueBit;
		static const QString validitySignalId;

		static const QString appSignalLowAdc;
		static const QString appSignalHighAdc;
		static const QString appSignalLowEngUnits;
		static const QString appSignalHighEngUnits;
		static const QString appSignalDataFormat;
		static const QString appSignalBusTypeId;

		static const QString categoryAppSignal;
	};

	//
	//
	// DeviceObject
	//
	//
	class DeviceObject :
		public PropertyObject,
		public Proto::ObjectSerialization<DeviceObject>,
		public DebugInstCounter<DeviceObject>,
		public std::enable_shared_from_this<DeviceObject>
	{
		Q_OBJECT

	protected:
		explicit DeviceObject(DeviceType deviceType, bool preset = false, QObject* parent = nullptr) noexcept;

	public:
		DeviceObject() = delete;
		virtual ~DeviceObject() = default;

		DeviceObject(const DeviceObject&) = delete;

	public:
		void dump(bool dumpProps, bool dumpTree, QString* out = nullptr, int nesting = 0) const;

		[[nodiscard]] std::shared_ptr<const DeviceObject> sharedPtr() const;
		[[nodiscard]] std::shared_ptr<DeviceObject> sharedPtr();

		// Serialization
		//
		friend Proto::ObjectSerialization<DeviceObject>;	// for call CreateObject from Proto::ObjectSerialization
	protected:
		// Implementing Proto::ObjectSerialization<DeviceObject>::SaveData, LoadData
		//
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this function only while serialization, as when object is created is not fully initialized
		// and must be read before use
		//
		[[nodiscard]] static std::shared_ptr<DeviceObject> CreateObject(const Proto::Envelope& message);

		// Public methods
		//
	public:
		// Save object with ALL children
		//
		bool SaveObjectTree(Proto::Envelope* message) const;

		// Expand EquipmentIDTemplate for this and for all children
		//
		virtual void expandEquipmentId();

		// Get all signals, including signals from child items
		//
		[[nodiscard]] std::vector<std::shared_ptr<DeviceAppSignal>> getAllAppSignals() const;

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Get all signals, including signals from child items
		//
		void getAllAppSignalsRecursive(std::vector<std::shared_ptr<DeviceAppSignal>>* deviceSignals) const;

		// Properties, etc
		//
	public:
		[[nodiscard]] bool hasParent() const noexcept;

		[[nodiscard]] std::shared_ptr<DeviceObject> parent() noexcept;
		[[nodiscard]] const std::shared_ptr<DeviceObject> parent() const noexcept;

		[[nodiscard]] DeviceType deviceType() const noexcept;

		[[nodiscard]] bool isRoot() const noexcept;
		[[nodiscard]] bool isSystem() const noexcept;
		[[nodiscard]] bool isRack() const noexcept;
		[[nodiscard]] bool isChassis() const noexcept;
		[[nodiscard]] bool isModule() const noexcept;
		[[nodiscard]] bool isController() const noexcept;
		[[nodiscard]] bool isWorkstation() const noexcept;
		[[nodiscard]] bool isSoftware() const noexcept;
		[[nodiscard]] bool isAppSignal() const noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceRoot> toRoot() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceRoot> toRoot() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceSystem> toSystem() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceSystem> toSystem() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceRack> toRack() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceRack> toRack() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceChassis> toChassis() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceChassis> toChassis() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceModule> toModule() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceModule> toModule() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceController> toController() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceController> toController() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceAppSignal> toAppSignal() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceAppSignal> toAppSignal() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::Workstation> toWorkstation() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::Workstation> toWorkstation() noexcept;

		[[nodiscard]] std::shared_ptr<const Hardware::Software> toSoftware() const noexcept;
		[[nodiscard]] std::shared_ptr<Hardware::Software> toSoftware() noexcept;

	private:
		template <typename DT>
		[[nodiscard]] std::shared_ptr<const DT> toType() const noexcept
		{
			std::shared_ptr<const DT> result = std::dynamic_pointer_cast<const DT>(shared_from_this());
			return result;
		}

		template <typename DT>
		[[nodiscard]] std::shared_ptr<DT> toType() noexcept
		{
			std::shared_ptr<DT> result = std::dynamic_pointer_cast<DT>(shared_from_this());
			return result;
		}

	public:
		[[nodiscard]] const Hardware::DeviceController* getParentController() const;
		[[nodiscard]] const Hardware::DeviceModule* getParentModule() const;
		[[nodiscard]] const Hardware::DeviceChassis* getParentChassis() const;
		[[nodiscard]] const Hardware::DeviceRack* getParentRack() const;
		[[nodiscard]] const Hardware::DeviceSystem* getParentSystem() const;
		[[nodiscard]] const Hardware::DeviceRoot* getParentRoot() const;

	public:
		[[nodiscard]] QString fileExtension() const;
		[[nodiscard]] static QString fileExtension(DeviceType device);

		void setExpertToProperty(const QString& property, bool expert);

		// Children care
		//
		[[nodiscard]] int childrenCount() const;
		[[nodiscard]] int childIndex(const std::shared_ptr<const DeviceObject>& child) const;

		[[nodiscard]] const std::vector<std::shared_ptr<DeviceObject>>& children() const;

		[[nodiscard]] const std::shared_ptr<DeviceObject>& child(int index) const;
		[[nodiscard]] std::shared_ptr<DeviceObject> child(const QUuid& uuid) const;
		[[nodiscard]] std::shared_ptr<DeviceObject> childByPresetUuid(const QUuid& presetObjectUuid) const;
		[[nodiscard]] std::shared_ptr<DeviceObject> childByEquipmentId(const QString& id);

		[[nodiscard]] bool canAddChild(const DeviceType childType) const;

		void addChild(const std::shared_ptr<DeviceObject>& child);
		void deleteChild(std::shared_ptr<DeviceObject> child);
		void deleteAllChildren();

		[[nodiscard]] bool checkChild(std::shared_ptr<DeviceObject> child, QString* errorMessage);

		static QString replaceEngeneeringToEngineering(const QString& data);

		// Props
		//
	public:
		[[nodiscard]] QUuid uuid() const;
		void setUuid(QUuid value);

		[[nodiscard]] QString equipmentIdTemplate() const;
		void setEquipmentIdTemplate(const QString& value);

		[[nodiscard]] QString equipmentId() const;			// This unwinds equipmentIdTemplate

		[[nodiscard]] QString caption() const;
		void setCaption(QString value);

		[[nodiscard]] QString childRestriction() const;
		void setChildRestriction(QString value);

		[[nodiscard]] QString specificProperties() const;
		void setSpecificProperties(QString value);

		[[nodiscard]] int place() const;
		void setPlace(int value);

		[[nodiscard]] QString details() const;		// JSON short description, uuid, equipmentId, caption, place, etc

		// Preset
		//
		[[nodiscard]] bool preset() const;

		[[nodiscard]] bool presetRoot() const;
		void setPresetRoot(bool value);

		[[nodiscard]] QString presetName() const;
		void setPresetName(QString value);

		[[nodiscard]] QUuid presetObjectUuid() const;
		void setPresetObjectUuid(QUuid value);

		[[nodiscard]] DbFileInfo* data();
		[[nodiscard]] const DbFileInfo* data() const;
		void setData(std::shared_ptr<DbFileInfo> data);

		// Data
		//
	protected:
		const DeviceType m_deviceType = DeviceType::Root;

		std::weak_ptr<DeviceObject> m_parent;
		std::vector<std::shared_ptr<DeviceObject>> m_children;

		QUuid m_uuid;
		QString m_equipmentId;
		QString m_caption;

		QString m_childRestriction;			// Restriction script for child items
		QString m_specificPropertiesStruct;	// Desctription of the Object's specific properties

		int m_place = -1;

		// Preset Data
		//
		bool m_preset = false;				// It is preset or part of it
		bool m_presetRoot = false;			// This object is preset root
		QString m_presetName;				// PresetName, if it is preset
		QUuid m_presetObjectUuid;			// In configuration this field has uuid of the PRESET object from which it was constructed
											// In preset edit mode this field has the same value with m_uuid
	private:
		std::shared_ptr<DbFileInfo> m_data;	// Application-specific value associated with the specified item (DbFileInfo)
	};


	//
	//
	// DeviceRoot
	//
	//
	class DeviceRoot : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceRoot(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceRoot() = default;
	};


	//
	//
	// DeviceSystem
	//
	//
	class DeviceSystem : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceSystem(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceSystem() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};


	//
	//
	// DeviceRack
	//
	//
	class DeviceRack : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceRack(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceRack() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};


	//
	//
	// DeviceChassis
	//
	//
	class DeviceChassis : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceChassis(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceChassis() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	public:
		[[nodiscard]] std::shared_ptr<DeviceModule> findLogicModule();

		// Properties
		//
	public:
		[[nodiscard]] int type() const;
		void setType(int value);

		// Data
		//
	private:
		int m_type = 0;
	};


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
			BVB = 0x5600
		};
		Q_ENUM(FamilyType)

	public:
		explicit DeviceModule(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceModule() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
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

		// Data
		//
	private:
		uint16_t m_type = 0;	// high byte is family type, low byte is module version

		uint16_t m_customModuleFamily = 0;

		QString m_configurationScript;
		QString m_rawDataDescription;
	};


	//
	//
	// DeviceController
	//
	//
	class DeviceController : public DeviceObject
	{
		Q_OBJECT
	public:
		explicit DeviceController(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceController() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};


	//
	//
	// DeviceAppSignal
	//
	//
	class DeviceAppSignal : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceAppSignal(bool preset = false, QObject* parent = nullptr) noexcept;
		virtual ~DeviceAppSignal() = default;

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Expand EquipmentIDTemplate, ValiditySignalId for this and for all children
		//
	public:
		virtual void expandEquipmentId() override;

		// Properties
		//
	public:
		[[nodiscard]] E::SignalType signalType() const;
		void setSignalType(E::SignalType value);

		[[nodiscard]] E::SignalFunction function() const;
		void setFunction(E::SignalFunction value);

		[[nodiscard]] E::ByteOrder byteOrder() const;
		void setByteOrder(E::ByteOrder value);

		[[nodiscard]] E::DataFormat format() const;
		void setFormat(E::DataFormat value);

		[[nodiscard]] E::MemoryArea memoryArea() const;
		void setMemoryArea(E::MemoryArea value);

		[[nodiscard]] int size() const;
		void setSize(int value);

		[[nodiscard]] int valueOffset() const;
		void setValueOffset(int value);

		[[nodiscard]] int valueBit() const;
		void setValueBit(int value);

		[[nodiscard]] QString validitySignalId() const;
		void setValiditySignalId(QString value);

		[[nodiscard]] bool isInputSignal() const;
		[[nodiscard]] bool isOutputSignal() const;
		[[nodiscard]] bool isDiagSignal() const;
		[[nodiscard]] bool isValiditySignal() const;

		[[nodiscard]] bool isAnalogSignal() const;
		[[nodiscard]] bool isDiscreteSignal() const;

		[[nodiscard]] int appSignalLowAdc() const;
		void setAppSignalLowAdc(int value);

		[[nodiscard]] int appSignalHighAdc() const;
		void setAppSignalHighAdc(int value);

		[[nodiscard]] double appSignalLowEngUnits() const;
		void setAppSignalLowEngUnits(double value);

		[[nodiscard]] double appSignalHighEngUnits() const;
		void setAppSignalHighEngUnits(double value);

		[[nodiscard]] E::AnalogAppSignalFormat appSignalDataFormat() const;
		void setAppSignalDataFormat(E::AnalogAppSignalFormat value);

		[[nodiscard]] QString appSignalBusTypeId() const;
		void setAppSignalBusTypeId(QString value);

		[[nodiscard]] QString signalSpecPropsStruct() const;
		void setSignalSpecPropsStruct(QString value);

		// Data
		//
	private:
		E::SignalType m_signalType = E::SignalType::Discrete;
		E::SignalFunction m_function = E::SignalFunction::Input;

		E::ByteOrder m_byteOrder = E::ByteOrder::LittleEndian;
		E::DataFormat m_format = E::DataFormat::UnsignedInt;

		E::MemoryArea m_memoryArea = E::MemoryArea::ApplicationData;

		int m_size = 0;
		int m_valueOffset = 0;
		int m_valueBit = 0;

		QString m_validitySignalId;

		// Properties for creating Analog AppSignal
		//
		int m_appSignalLowAdc = 0;
		int m_appSignalHighAdc = 65535;
		double m_appSignalLowEngUnits = 0.0;
		double m_appSignalHighEngUnits = 100.0;
		E::AnalogAppSignalFormat m_appSignalDataFormat = E::AnalogAppSignalFormat::Float32;

		// Properties for creating Bus AppSignal
		//
		QString m_appSignalBusTypeId;

		QString m_signalSpecPropsStruct;		// Description of the specific properties to be generated by AppSignal
	};

	//
	//
	// Workstation
	//
	//
	class Workstation : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit Workstation(bool preset = false, QObject* parent = nullptr);
		virtual ~Workstation() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		[[nodiscard]] int type() const;
		void setType(int value);

		// Data
		//
	private:
		int m_type = 0;
	};


	//
	//
	// Software
	//
	//
	class Software : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit Software(bool preset = false, QObject* parent = nullptr);
		virtual ~Software() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		[[nodiscard]] E::SoftwareType softwareType() const;
		void setSoftwareType(E::SoftwareType value);

		// Data
		//
	private:
		E::SoftwareType m_softwareType = E::SoftwareType::Monitor;
	};

	//
	//
	// EquipmentSet
	//
	//
	class EquipmentSet
	{
	public:
		EquipmentSet() = default;
		EquipmentSet(std::shared_ptr<DeviceRoot> root);
		~EquipmentSet();

	public:
		void set(std::shared_ptr<DeviceRoot> root);

		[[nodiscard]] std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId);
		[[nodiscard]] const std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId) const;

		[[nodiscard]] std::shared_ptr<DeviceRoot> root();
		[[nodiscard]] const std::shared_ptr<DeviceRoot> root() const;

		[[nodiscard]] std::vector<std::shared_ptr<DeviceObject>> devices();

		void dump(bool dumpProps, QDebug d) const;

	private:
		void addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent);

	private:
		std::shared_ptr<DeviceRoot> m_root;
		QHash<QString, std::shared_ptr<DeviceObject>> m_deviceTable;
	};

	//
	// Factory
	//
	extern Factory<DeviceObject> DeviceObjectFactory;

	namespace Obsolete
	{
		enum SignalType
		{
			DiagDiscrete,
			DiagAnalog,
			InputDiscrete,
			InputAnalog,
			OutputDiscrete,
			OutputAnalog,
		};
	}

	// Walk through equipment tree
	//
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren, std::function<void(Hardware::DeviceObject* device)> processAfterChildren);
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren);

	void SerializeEquipmentFromXml(const QString &filePath, std::shared_ptr<DeviceRoot>& deviceRoot);

	QString expandDeviceSignalTemplate(	const Hardware::DeviceObject& startDeviceObject,
										const QString& templateStr,
										QString* errMsg);

	QString expandDeviceObjectMacro(const Hardware::DeviceObject& startDeviceObject,
									const QString& macroStr,
									QString* errMsg);

	const Hardware::DeviceObject* getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
															  const QString& parentObjectType,
															  QString* errMsg);
}

