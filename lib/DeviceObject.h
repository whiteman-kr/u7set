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
#include "../lib/ProtoSerialization.h"
#include "../lib/ModuleFirmware.h"

class DbController;

namespace Hardware
{
	extern const std::array<QString, 9> DeviceObjectExtensions;
	extern const std::array<QString, 9> DeviceTypeNames;

	void Init();
	void Shutdwon();

	class DeviceObject;
	class DeviceRoot;
	class DeviceSystem;
	class DeviceRack;
	class DeviceChassis;
	class DeviceModule;
	class DeviceController;
	class Workstation;
	class Software;
	class DeviceSignal;

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

		Signal,

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

		static const QString categoryAnalogAppSignal;
	};

	// Forward declarations
	//
	class DeviceSignal;

	//
	//
	// DeviceObject
	//
	//
	class DeviceObject :
		public PropertyObject,
		public Proto::ObjectSerialization<DeviceObject>,
		public DebugInstCounter<DeviceObject>
	{
		Q_OBJECT

	protected:
		explicit DeviceObject(bool preset = false);

	public:
		virtual ~DeviceObject();

		// Serialization
		//
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
		std::vector<std::shared_ptr<DeviceSignal>> getAllSignals() const;

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Parse m_specificProperties and create Qt meta system specific properies
		void parseSpecificPropertiesStruct();

		void parseSpecificPropertiesStructV1(const QStringList& columns);
		void parseSpecificPropertiesStructV2(const QStringList& columns);
		void parseSpecificPropertiesStructV3(const QStringList& columns);

		// Get all signals, including signals from child items
		void getAllSignalsRecursive(std::vector<std::shared_ptr<DeviceSignal>>* deviceSignals) const;

		// Properties, etc
		//
	public:
		DeviceObject* parent();
		const DeviceObject* parent() const;
		Q_INVOKABLE QObject* jsParent() const;

		Q_INVOKABLE int jsPropertyInt(QString name) const;
		Q_INVOKABLE bool jsPropertyBool(QString name) const;
		Q_INVOKABLE QString jsPropertyString(QString name) const;
		Q_INVOKABLE quint32 jsPropertyIP(QString name) const;

		virtual DeviceType deviceType() const;
		Q_INVOKABLE int jsDeviceType() const;

		Q_INVOKABLE bool isRoot() const;
		Q_INVOKABLE bool isSystem() const;
		Q_INVOKABLE bool isRack() const;
		Q_INVOKABLE bool isChassis() const;
		Q_INVOKABLE bool isModule() const;
		Q_INVOKABLE bool isController() const;
		Q_INVOKABLE bool isWorkstation() const;
		Q_INVOKABLE bool isSoftware() const;
		Q_INVOKABLE bool isSignal() const;

		const Hardware::DeviceRoot* toRoot() const;
		Hardware::DeviceRoot* toRoot();

		const Hardware::DeviceSystem* toSystem() const;
		Hardware::DeviceSystem* toSystem();

		const Hardware::DeviceRack* toRack() const;
		Hardware::DeviceRack* toRack();

		const Hardware::DeviceChassis* toChassis() const;
		Hardware::DeviceChassis* toChassis();

		const Hardware::DeviceModule* toModule() const;
		Hardware::DeviceModule* toModule();

		const Hardware::DeviceController* toController() const;
		Hardware::DeviceController* toController();

		const Hardware::DeviceSignal* toSignal() const;
		Hardware::DeviceSignal* toSignal();

		const Hardware::Software* toSoftware() const;
		Hardware::Software* toSoftware();

		const Hardware::DeviceController* getParentController() const;
		const Hardware::DeviceModule* getParentModule() const;
		const Hardware::DeviceChassis* getParentChassis() const;
		const Hardware::DeviceRack* getParentRack() const;
		const Hardware::DeviceSystem* getParentSystem() const;
		const Hardware::DeviceRoot* getParentRoot() const;

		QString fileExtension() const;
		static QString fileExtension(DeviceType device);

		void setExpertToProperty(const QString& property, bool expert);

		// Children care
		//
		Q_INVOKABLE int childrenCount() const;

		DeviceObject* child(int index) const;
		DeviceObject* child(QUuid uuid) const;
		Q_INVOKABLE QObject* jsChild(int index) const;

		int childIndex(DeviceObject* child) const;

		std::shared_ptr<DeviceObject> childSharedPtr(int index);
		std::shared_ptr<DeviceObject> childSharedPtr(QUuid uuid);
		std::shared_ptr<DeviceObject> childSharedPtrByPresetUuid(QUuid presetObjectUuid);

		bool canAddChild(DeviceObject* child) const;

		void addChild(std::shared_ptr<DeviceObject> child);
		void deleteChild(DeviceObject* child);
		void deleteAllChildren();

        bool checkChild(DeviceObject *child, QString* errorMessage);

		void sortByPlace(Qt::SortOrder order);
		void sortByEquipmentId(Qt::SortOrder order);
		void sortByCaption(Qt::SortOrder order);
		void sortByState(Qt::SortOrder order);
		void sortByUser(Qt::SortOrder order, const std::map<int, QString>& users);

		std::vector<DeviceObject*> findChildObjectsByMask(const QString& mask);
		void findChildObjectsByMask(const QString& mask, std::vector<DeviceObject*>& list);
		Q_INVOKABLE QObject* jsFindChildObjectByMask(const QString& mask);

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
		Q_INVOKABLE int jsPlace() const;
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
		DeviceObject* m_parent = nullptr;
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

	public:
		virtual DeviceType deviceType() const override;

	private:
		static const DeviceType m_deviceType = DeviceType::Root;
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

	public:
		virtual DeviceType deviceType() const override;

	private:
		static const DeviceType m_deviceType = DeviceType::System;
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

	public:
		virtual DeviceType deviceType() const override;

	private:
		static const DeviceType m_deviceType = DeviceType::Rack;
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
		virtual DeviceType deviceType() const override;

		std::shared_ptr<DeviceModule> getLogicModuleSharedPointer();

		// Properties
		//
	public:
		int type() const;
		void setType(int value);

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Chassis;

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
			MPS17 = 0x5100,
			BVK4 = 0x5300,
			BP336 = 0x5500,
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

	public:
		virtual DeviceType deviceType() const override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		Q_INVOKABLE int jsModuleFamily() const;

		FamilyType moduleFamily() const;
		void setModuleFamily(FamilyType value);

		int customModuleFamily() const;
		void setCustomModuleFamily(int value);

		Q_INVOKABLE int moduleVersion() const;
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
		static const DeviceType m_deviceType = DeviceType::Module;

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

	public:
		virtual DeviceType deviceType() const override;

	private:
		static const DeviceType m_deviceType = DeviceType::Controller;
	};


	//
	//
	// DeviceDiagSignal
	//
	//
	class DeviceSignal : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceSignal(bool preset = false);
		virtual ~DeviceSignal();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Expand EquipmentIDTemplate, ValiditySignalId for this and for all children
		//
	public:
		virtual void expandEquipmentId() override;

	public:
		virtual DeviceType deviceType() const override;

		Q_INVOKABLE quint32 valueToMantExp1616(double value);

		// Properties
		//
	public:
		E::SignalType type() const;
		Q_INVOKABLE int jsType() const;
		void setType(E::SignalType value);

		E::SignalFunction function() const;
		Q_INVOKABLE int jsFunction() const;
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
		void setValiditySignalId(const QString& value);

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

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Signal;

		E::SignalType m_type = E::SignalType::Discrete;
		E::SignalFunction m_function = E::SignalFunction::Input;

		E::ByteOrder m_byteOrder = E::ByteOrder::LittleEndian;
		E::DataFormat m_format = E::DataFormat::UnsignedInt;

		E::MemoryArea m_memoryArea = E::MemoryArea::ApplicationData;

		int m_size = 0;
		int m_valueOffset = 0;
		int m_valueBit = 0;

		QString m_validitySignalId;

		// Signals for creating Analog AppSignals
		//
		int m_appSignalLowAdc = 0;
		int m_appSignalHighAdc = 65535;
		double m_appSignalLowEngUnits = 0.0;
		double m_appSignalHighEngUnits = 100.0;
		E::AnalogAppSignalFormat m_appSignalDataFormat = E::AnalogAppSignalFormat::Float32;
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

	public:
		virtual DeviceType deviceType() const override;

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
		static const DeviceType m_deviceType = DeviceType::Workstation;

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

	public:
		virtual DeviceType deviceType() const override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		E::SoftwareType type() const;
		void setType(E::SoftwareType value);

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Software;

		E::SoftwareType m_type = E::SoftwareType::Monitor;
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
		EquipmentSet(std::shared_ptr<DeviceObject> root);
		~EquipmentSet();

	public:
		void set(std::shared_ptr<DeviceObject> root);

		DeviceObject* deviceObject(const QString& equipmentId);
		const DeviceObject* deviceObject(const QString& equipmentId) const;

		std::shared_ptr<DeviceObject> deviceObjectSharedPointer(const QString& equipmentId);

		DeviceRoot* root();
		const DeviceRoot* root() const;

	private:
		void addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent);

	private:
		std::shared_ptr<DeviceObject> m_root;
		QHash<QString, std::shared_ptr<DeviceObject>> m_deviceTable;
	};


	extern Factory<Hardware::DeviceObject> DeviceObjectFactory;

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
}

