#pragma once

#include <unordered_map>
#include <functional>
#include <array>
#include "DbStruct.h"
#include "QUuid"
#include "../lib/DebugInstCounter.h"
#include "../lib/PropertyObject.h"
#include "../lib/Factory.h"
#include "../lib/Types.h"
#include "../lib/ModuleFirmware.h"
#include "../Proto/ProtoSerialization.h"

class DbController;

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
		DeviceObject() = delete;
		explicit DeviceObject(DeviceType deviceType, bool preset = false) noexcept;

	public:
		virtual ~DeviceObject();

		std::shared_ptr<const DeviceObject> sharedPtr() const;
		std::shared_ptr<DeviceObject> sharedPtr();

		// Serialization
		//
	public:
		friend Proto::ObjectSerialization<DeviceObject>;	// for call CreateObject from Proto::ObjectSerialization

		static std::shared_ptr<DeviceObject> fromDbFile(const DbFile& file);
		static std::vector<std::shared_ptr<DeviceObject>> fromDbFiles(std::vector<std::shared_ptr<DbFile>> files);

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
		static std::shared_ptr<DeviceObject> CreateObject(const Proto::Envelope& message);

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
		std::vector<std::shared_ptr<DeviceAppSignal>> getAllSignals() const;

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Get all signals, including signals from child items
		//
		void getAllSignalsRecursive(std::vector<std::shared_ptr<DeviceAppSignal>>* deviceSignals) const;

		// Properties, etc
		//
	public:
		bool hasParent() const noexcept;

		std::shared_ptr<DeviceObject> parent() noexcept;
		const std::shared_ptr<DeviceObject> parent() const noexcept;

		DeviceType deviceType() const noexcept;

		bool isRoot() const noexcept;
		bool isSystem() const noexcept;
		bool isRack() const noexcept;
		bool isChassis() const noexcept;
		bool isModule() const noexcept;
		bool isController() const noexcept;
		bool isWorkstation() const noexcept;
		bool isSoftware() const noexcept;
		bool isAppSignal() const noexcept;

		std::shared_ptr<const Hardware::DeviceRoot> toRoot() const noexcept;
		std::shared_ptr<Hardware::DeviceRoot> toRoot() noexcept;

		std::shared_ptr<const Hardware::DeviceSystem> toSystem() const noexcept;
		std::shared_ptr<Hardware::DeviceSystem> toSystem() noexcept;

		std::shared_ptr<const Hardware::DeviceRack> toRack() const noexcept;
		std::shared_ptr<Hardware::DeviceRack> toRack() noexcept;

		std::shared_ptr<const Hardware::DeviceChassis> toChassis() const noexcept;
		std::shared_ptr<Hardware::DeviceChassis> toChassis() noexcept;

		std::shared_ptr<const Hardware::DeviceModule> toModule() const noexcept;
		std::shared_ptr<Hardware::DeviceModule> toModule() noexcept;

		std::shared_ptr<const Hardware::DeviceController> toController() const noexcept;
		std::shared_ptr<Hardware::DeviceController> toController() noexcept;

		std::shared_ptr<const Hardware::DeviceAppSignal> toAppSignal() const noexcept;
		std::shared_ptr<Hardware::DeviceAppSignal> toAppSignal() noexcept;

		std::shared_ptr<const Hardware::Workstation> toWorkstation() const noexcept;
		std::shared_ptr<Hardware::Workstation> toWorkstation() noexcept;

		std::shared_ptr<const Hardware::Software> toSoftware() const noexcept;
		std::shared_ptr<Hardware::Software> toSoftware() noexcept;

	private:
		template <typename DT>
		std::shared_ptr<const DT> toType() const noexcept
		{
			std::shared_ptr<const DT> result = std::dynamic_pointer_cast<const DT>(shared_from_this());
			return result;
		}

		template <typename DT>
		std::shared_ptr<DT> toType() noexcept
		{
			std::shared_ptr<DT> result = std::dynamic_pointer_cast<DT>(shared_from_this());
			return result;
		}

	public:
		const Hardware::DeviceController* getParentController() const;
		const Hardware::DeviceModule* getParentModule() const;
		const Hardware::DeviceChassis* getParentChassis() const;
		const Hardware::DeviceRack* getParentRack() const;
		const Hardware::DeviceSystem* getParentSystem() const;
		const Hardware::DeviceRoot* getParentRoot() const;

	public:
		QString fileExtension() const;
		static QString fileExtension(DeviceType device);

		void setExpertToProperty(const QString& property, bool expert);

		// Children care
		//
		int childrenCount() const;
		int childIndex(const std::shared_ptr<const DeviceObject>& child) const;

		const std::shared_ptr<DeviceObject>& child(int index) const;
		std::shared_ptr<DeviceObject> child(const QUuid& uuid) const;
		std::shared_ptr<DeviceObject> childByPresetUuid(const QUuid& presetObjectUuid) const;
		std::shared_ptr<DeviceObject> childByEquipmentId(const QString& id);

		bool canAddChild(const DeviceType childType) const;

		void addChild(std::shared_ptr<DeviceObject> child);
		void deleteChild(std::shared_ptr<DeviceObject> child);
		void deleteAllChildren();

		bool checkChild(std::shared_ptr<DeviceObject> child, QString* errorMessage);

		void sortByPlace(Qt::SortOrder order);
		void sortByEquipmentId(Qt::SortOrder order);
		void sortByCaption(Qt::SortOrder order);
		void sortByType(Qt::SortOrder order);
		void sortByState(Qt::SortOrder order);
		void sortByUser(Qt::SortOrder order, const std::map<int, QString>& users);

//		std::vector<std::shared_ptr<DeviceObject>> findChildObjectsByMask(const QString& mask);
//		void findChildObjectsByMask(const QString& mask, std::vector<std::shared_ptr<DeviceObject>>& result);

		static QString replaceEngeneeringToEngineering(const QString& data);

		// Props
		//
	public:
		int fileId() const;

		QUuid uuid() const;
		void setUuid(QUuid value);

		QString equipmentIdTemplate() const;
		void setEquipmentIdTemplate(QString value);

		QString equipmentId() const;

		QString caption() const;
		void setCaption(QString value);

		DbFileInfo& fileInfo();
		const DbFileInfo& fileInfo() const;
		void setFileInfo(const DbFileInfo& value);

		QString childRestriction() const;
		void setChildRestriction(QString value);

		QString specificProperties() const;
		void setSpecificProperties(QString value);

		int place() const;
		void setPlace(int value);

		QString details() const;		// JSON short description, uuid, equipmentId, caption, place, etc

		// Preset
		//
		bool preset() const;

		bool presetRoot() const;
		void setPresetRoot(bool value);

		QString presetName() const;
		void setPresetName(QString value);

		QUuid presetObjectUuid() const;
		void setPresetObjectUuid(QUuid value);

		// Data
		//
	protected:
		const DeviceType m_deviceType = DeviceType::Root;

		std::weak_ptr<DeviceObject> m_parent;
		std::vector<std::shared_ptr<DeviceObject>> m_children;

		QUuid m_uuid;
		QString m_equipmentId;
		QString m_caption;

		DbFileInfo m_fileInfo;

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
		explicit DeviceRoot(bool preset = false);
		virtual ~DeviceRoot();
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
		explicit DeviceSystem(bool preset = false);
		virtual ~DeviceSystem();

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
		explicit DeviceRack(bool preset = false);
		virtual ~DeviceRack();

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
		explicit DeviceChassis(bool preset = false);
		virtual ~DeviceChassis();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	public:
		std::shared_ptr<DeviceModule> findLogicModule();

		// Properties
		//
	public:
		int type() const;
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
		explicit DeviceModule(bool preset = false);
		virtual ~DeviceModule();

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
		FamilyType moduleFamily() const;
		void setModuleFamily(FamilyType value);

		int customModuleFamily() const;
		void setCustomModuleFamily(int value);

		int moduleVersion() const;
		void setModuleVersion(int value);

		QString configurationScript() const;
		void setConfigurationScript(const QString& value);

		QString rawDataDescription() const;
		void setRawDataDescription(const QString& value);
		bool hasRawData() const;

		int moduleType() const;

		bool isIOModule() const;
		bool isInputModule() const;
		bool isOutputModule() const;
		bool isLogicModule() const;
		bool isFSCConfigurationModule() const;
		bool isOptoModule() const;
		bool isBvb() const;

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
		explicit DeviceController(bool preset = false);
		virtual ~DeviceController();

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
		explicit DeviceAppSignal(bool preset = false) noexcept;
		virtual ~DeviceAppSignal();

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
		E::SignalType signalType() const;
		void setSignalType(E::SignalType value);

		E::SignalFunction function() const;
		void setFunction(E::SignalFunction value);

		E::ByteOrder byteOrder() const;
		void setByteOrder(E::ByteOrder value);

		E::DataFormat format() const;
		void setFormat(E::DataFormat value);

		E::MemoryArea memoryArea() const;
		void setMemoryArea(E::MemoryArea value);

		int size() const;
		void setSize(int value);

		int valueOffset() const;
		void setValueOffset(int value);

		int valueBit() const;
		void setValueBit(int value);

		QString validitySignalId() const;
		void setValiditySignalId(QString value);

		bool isInputSignal() const;
		bool isOutputSignal() const;
		bool isDiagSignal() const;
		bool isValiditySignal() const;

		bool isAnalogSignal() const;
		bool isDiscreteSignal() const;

		int appSignalLowAdc() const;
		void setAppSignalLowAdc(int value);

		int appSignalHighAdc() const;
		void setAppSignalHighAdc(int value);

		double appSignalLowEngUnits() const;
		void setAppSignalLowEngUnits(double value);

		double appSignalHighEngUnits() const;
		void setAppSignalHighEngUnits(double value);

		E::AnalogAppSignalFormat appSignalDataFormat() const;
		void setAppSignalDataFormat(E::AnalogAppSignalFormat value);

		QString appSignalBusTypeId() const;
		void setAppSignalBusTypeId(QString value);

		QString signalSpecPropsStruct() const;
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

		QString m_signalSpecPropsStruct;		// Description of the specific properties to be generated by Signal
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
		explicit Workstation(bool preset = false);
		virtual ~Workstation();

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
		int type() const;
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
		explicit Software(bool preset = false);
		virtual ~Software();

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
		E::SoftwareType softwareType() const;
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

		std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId);
		const std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId) const;

		std::shared_ptr<DeviceObject> deviceObjectSharedPointer(const QString& equipmentId);

		std::shared_ptr<DeviceRoot> root();
		const std::shared_ptr<DeviceRoot> root() const;

	private:
		void addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent);

	private:
		std::shared_ptr<DeviceRoot> m_root;
		QHash<QString, std::shared_ptr<DeviceObject>> m_deviceTable;
	};

//	//
//	// Scripting wrappers
//	//
//	class ScriptDeviceObject : public QObject
//	{
//		Q_OBJECT

//		//Q_PROPERTY(QString equipmentID READ equipmentId)

//	public:
//		ScriptDeviceObject() = delete;
//		ScriptDeviceObject(const ScriptDeviceObject& src);
//		explicit ScriptDeviceObject(std::shared_ptr<DeviceObject> deviceObject);

//		ScriptDeviceObject& operator=(const ScriptDeviceObject& src);

//	public slots:
//		QJSValue parent() const;

//		int propertyInt(QString name) const;
//		bool propertyBool(QString name) const;
//		QString propertyString(QString name) const;
//		quint32 propertyIP(QString name) const;

//		int deviceType() const;

//		bool isRoot() const;
//		bool isSystem() const;
//		bool isRack() const;
//		bool isChassis() const;
//		bool isModule() const;
//		bool isController() const;
//		bool isWorkstation() const;
//		bool isSoftware() const;
//		bool isAppSignal() const;

//		int childrenCount() const;

//		QJSValue child(int index) const;
//		QJSValue childByMask(QString mask);

//		int place() const;

//		// Module specific
//		//
//		int moduleFamily() const;
//		int moduleVersion() const;

//		// AppSignal specific
//		//
//		quint32 valueToMantExp1616(double value);
//		//int jsType() const;
//		//int jsFunction() const;

//	private:
//		std::shared_ptr<DeviceObject> m_deviceObject;
//	};


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
	// Walk through equipment tree
	//
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren, std::function<void(Hardware::DeviceObject* device)> processAfterChildren);
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren);

	void SerializeEquipmentFromXml(const QString &filePath, std::shared_ptr<DeviceRoot>& deviceRoot);
}

