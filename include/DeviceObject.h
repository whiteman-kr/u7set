#pragma once

#include <unordered_map>
#include <functional>
#include <QJSValue>
#include "DbStruct.h"
#include "QUuid"
#include "../include/DebugInstCounter.h"
#include "../include/PropertyObject.h"
#include "../include/Factory.h"
#include "../include/Types.h"
#include "../include/ProtoSerialization.h"
#include "../include/ModuleConfiguration.h"

class DbController;

namespace Hardware
{
	extern const wchar_t* DeviceObjectExtensions[];
	extern const wchar_t* DeviceTypeNames[];

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

		static DeviceObject* fromDbFile(const DbFile& file);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this function only while serialization, as when object is created is not fully initialized
		// and must be read before use
		//
		static DeviceObject* CreateObject(const Proto::Envelope& message);

		// Public methods
		//
	public:
		// Expand EquipmentIDTemplate for this and for all children
		//
		void expandEquipmentId();

		// Get all signals, including signals from child items
		std::vector<std::shared_ptr<DeviceSignal>> getAllSignals() const;

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Parse m_specificProperties and create Qt meta system specific properies
		void parseSpecificPropertiesStruct();

		void parseSpecificPropertieyStructV1(const QStringList& columns);
		void parseSpecificPropertieyStructV2(const QStringList& columns);

		// Get all signals, including signals from child items
		void getAllSignalsRecursive(std::vector<std::shared_ptr<DeviceSignal>>* deviceSignals) const;

		// Properties, etc
		//
	public:
		DeviceObject* parent();
		const DeviceObject* parent() const;
		Q_INVOKABLE QObject* jsParent() const;

		Q_INVOKABLE int jsPropertyInt(QString name) const;
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

		const Hardware::Software* toSoftware() const;
		Hardware::Software* toSoftware();

		const Hardware::DeviceController* getParentController() const;
		const Hardware::DeviceModule* getParentModule() const;
		const Hardware::DeviceChassis* getParentChassis() const;
		const Hardware::DeviceRack* getParentRack() const;
		const Hardware::DeviceSystem* getParentSystem() const;

		QString fileExtension() const;
		static QString fileExtension(DeviceType device);

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

		void addChild(std::shared_ptr<DeviceObject> child);
		void deleteChild(DeviceObject* child);
		void deleteAllChildren();

		bool checkChild(DeviceObject* child, QString* errorMessage);

		void sortByPlace(Qt::SortOrder order);
		void sortByEquipmentId(Qt::SortOrder order);
		void sortByCaption(Qt::SortOrder order);
		void sortByState(Qt::SortOrder order);
		void sortByUser(Qt::SortOrder order);

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
											// In preset edit mode this field has the same valie with m_uuid

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
		virtual bool SaveData(Proto::Envelope* message) const override;
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
		virtual bool SaveData(Proto::Envelope* message) const override;
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	public:
		virtual DeviceType deviceType() const override;

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
		enum FamilyType		// WARNING!!! Only high byte can be used as it is a part of the type
		{					// (high byte is module family, low byte is module version)
			OTHER = 0x0000,
			LM = 0x0100,
			AIM = 0x0200,
			AOM = 0x0300,
			DIM = 0x0400,
			DOM = 0x0500,
			AIFM = 0x0600,
			OCM = 0x0700
		};
		Q_ENUM(FamilyType)

	public:
		explicit DeviceModule(bool preset = false);
		virtual ~DeviceModule();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	public:
		virtual DeviceType deviceType() const override;

		// Public Methods
		//
	public:

		// Properties
		//
	public:
		FamilyType moduleFamily() const;
		void setModuleFamily(FamilyType value);

		int moduleVersion() const;
		void setModuleVersion(int value);

		int moduleType() const;

		bool isIOModule() const;
		bool isInputModule() const;
		bool isOutputModule() const;
		bool isLM() const;

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Module;

		uint16_t m_type = 0;	// high byte is family type, low byte is module version
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
		virtual bool SaveData(Proto::Envelope* message) const override;
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

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

		int size() const;
		void setSize(int value);

		int validityOffset() const;
		void setValidityOffset(int value);

		int validityBit() const;
		void setValidityBit(int value);

		int valueOffset() const;
		void setValueOffset(int value);

		int valueBit() const;
		void setValueBit(int value);

		bool isInputSignal() const;
		bool isOutputSignal() const;
		bool isDiagSignal() const;
		bool isValiditySignal() const;

		bool isAnalogSignal() const;
		bool isDiscreteSignal() const;

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Signal;

		E::SignalType m_type = E::SignalType::Discrete;
		E::SignalFunction m_function = E::SignalFunction::Input;

		E::ByteOrder m_byteOrder = E::ByteOrder::LittleEndian;
		E::DataFormat m_format = E::DataFormat::UnsignedInt;

		int m_size = 0;
		int m_validityOffset = -1;
		int m_validityBit = -1;
		int m_valueOffset = 0;
		int m_valueBit = 0;

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
		virtual bool SaveData(Proto::Envelope* message) const override;
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
		virtual bool SaveData(Proto::Envelope* message) const override;
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

	public:
		void set(std::shared_ptr<DeviceObject> root);

		DeviceObject* deviceObject(const QString& equipmentId);
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

