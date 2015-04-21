#pragma once
#include <unordered_map>
#include <QJSValue>
#include "DbStruct.h"
#include "QUuid"
#include "../include/Factory.h"
#include "../include/Types.h"
#include "../include/ProtoSerialization.h"
#include "../include/ModuleConfiguration.h"

namespace Hardware
{
	extern const wchar_t* DeviceObjectExtensions[];

	void Init();
	void Shutdwon();

	class DeviceObject;

	// Device type, for defining hierrarche, don't save these data to file, can be changed (new level) later
	//
	enum class DeviceType
	{
		Root,
		System,
		Rack,
		Chassis,
		Module,
		Controller,

		Workstation,
		Software,

		Signal,

		DeviceTypeCount
	};


	// Software Module Type Identifiers
	//
	enum class SoftwareType
	{
		Monitor = 9000,
		ConfigurationService = 9001,
		DataAcquisitionService = 9002,
		DataArchivingService = 9003,
	};

	//
	//
	// Subsystem
	//
	//
	class Subsystem : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(int Index READ index WRITE setIndex)
		Q_PROPERTY(QString StrID READ strId WRITE setStrId)
		Q_PROPERTY(QString Caption READ caption WRITE setCaption)

	public:
		Subsystem();
		Subsystem(int index, const QString& strId, const QString& caption);

		bool save(QXmlStreamWriter& writer);
		bool load(QXmlStreamReader& reader);

		// Properties
		//
	public:
		const QString& strId() const;
		void setStrId(const QString& value);

		const QString& caption() const;
		void setCaption(const QString& value);

		int index() const;
		void setIndex(int value);

	private:
		int m_index;
		QString m_strId;
		QString m_caption;

	};

	//
	//
	// SubsystemStorage
	//
	//
	class SubsystemStorage
	{
	public:

		SubsystemStorage();

		void add(std::shared_ptr<Subsystem> subsystem);
		int count() const;
		std::shared_ptr<Subsystem> get(int index) const;
		void clear();

		bool load(const QByteArray& data, QString &errorCode);
		bool save(QByteArray& data);

	private:
		std::vector<std::shared_ptr<Subsystem>> m_subsystems;

	};

	//
	//
	// DynamicProperty
	//
	//
	class DynamicProperty
	{
	public:
		DynamicProperty();
		DynamicProperty(const QString& name,
						const QVariant& min,
						const QVariant& max,
						const QVariant& defaultVal,
						const QVariant& value);

		void saveValue(::Proto::Property* protoProperty) const;
		bool loadValue(const ::Proto::Property& protoProperty);

		// Properties
		//
	public:
		QString name() const;
		const char* name_c_str() const;
		void setName(const QString& value);

		QVariant min() const;
		QVariant max() const;
		QVariant defaultValue() const;

		QVariant value() const;
		void setValue(QVariant v);

		// Data
		//
	private:
		QString m_name;
		QByteArray m_c_str_name;

		QVariant m_min;
		QVariant m_max;
		QVariant m_default;

		QVariant m_value;
	};

	//
	//
	// DeviceObject
	//
	//
	class DeviceObject :
		public QObject,
		public Proto::ObjectSerialization<DeviceObject>
	{
		Q_OBJECT

		Q_PROPERTY(QString StrID READ strId WRITE setStrId)
		Q_PROPERTY(QString Caption READ caption WRITE setCaption)
		Q_PROPERTY(QString ChildRestriction READ childRestriction WRITE setChildRestriction)
		Q_PROPERTY(int Place READ place WRITE setPlace)
		Q_PROPERTY(QString DynamicProperties READ dynamicProperties WRITE setDynamicProperties)

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
		void expandStrId();

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Parse m_dynamicProperties and create Qt meta system dynamic properies
		void parseDynamicPropertiesStruct();


		// Properties, etc
		//
	public:
		DeviceObject* parent();
        Q_INVOKABLE QObject* jsParent() const;

		virtual DeviceType deviceType() const;
		Q_INVOKABLE int jsDeviceType() const;

		QString fileExtension() const;
		static QString fileExtension(DeviceType device);

		// Children care
		//
		Q_INVOKABLE int childrenCount() const;

		DeviceObject* child(int index) const;
		Q_INVOKABLE QObject* jsChild(int index) const;

		int childIndex(DeviceObject* child) const;

		std::shared_ptr<DeviceObject> childSharedPtr(int index);

		void addChild(std::shared_ptr<DeviceObject> child);
		void deleteChild(DeviceObject* child);
		void deleteAllChildren();

		bool checkChild(DeviceObject* child, QString* errorMessage);
		void sortChildrenByPlace();

		std::vector<DeviceObject*> findChildObjectsByMask(const QString& mask);
		void findChildObjectsByMask(const QString& mask, std::vector<DeviceObject*>& list);
		Q_INVOKABLE QObject* jsFindChildObjectByMask(const QString& mask);



		// Props
		//
	public:
		const QString& strId() const;
		void setStrId(const QString& value);

		const QString& caption() const;
		void setCaption(const QString& value);

		DbFileInfo& fileInfo();
		const DbFileInfo& fileInfo() const;
		void setFileInfo(const DbFileInfo& value);

		const QString& childRestriction() const;
		void setChildRestriction(const QString& value);

		const QString& dynamicProperties() const;
		void setDynamicProperties(const QString& value);

		int place() const;
		Q_INVOKABLE int jsPlace() const;
		void setPlace(int value);

		// Preset
		//
		bool preset() const;

		bool presetRoot() const;
		void setPresetRoot(bool value);

		const QString& presetName() const;
		void setPresetName(const QString& value);

		// Data
		//
	protected:
		DeviceObject* m_parent = nullptr;
		std::vector<std::shared_ptr<DeviceObject>> m_children;

		QUuid m_uuid;
		QString m_strId;
		QString m_caption;

		DbFileInfo m_fileInfo;

		QString m_childRestriction;			// Restriction script for child items
		QString m_dynamicPropertiesStruct;	// Desctription of the Object's dynamic properties

		int m_place = 0;

		// Preset Data
		//
		bool m_preset = false;				// It is preset or part of it
		bool m_presetRoot = false;			// This object is preset root
		QString m_presetName;				// PresetName, if it is preset
		//QUuid m_presetId;

	private:
		bool m_avoidEventRecursion = false;
		QHash<QString, DynamicProperty> m_dynamicProperties;
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

		Q_PROPERTY(int Type READ type WRITE setType)

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
		Q_ENUMS(FamilyType)

		Q_PROPERTY(FamilyType ModuleFamily READ moduleFamily WRITE setModuleFamily)
		Q_PROPERTY(int ModuleVersion READ moduleVersion WRITE setModuleVersion)

		Q_PROPERTY(int Channel READ channel WRITE setChannel)
		Q_PROPERTY(QString SubsysID READ subSysID WRITE setSubSysID)
		Q_PROPERTY(QString ConfType READ confType WRITE setConfType)

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

		int channel() const;
		void setChannel(int value);

		QString subSysID() const;
		void setSubSysID(const QString& value);

		QString confType() const;
		void setConfType(const QString& value);

		bool isIOModule() const;

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Module;

		uint16_t m_type = 0;	// high byte is family type, low byte is module version

		int m_channel = 0;		// 1 - base channel, 0 - means channel not set or not required
		QString m_subSysID;
		QString m_confType;
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

		Q_PROPERTY(SignalType Type READ type WRITE setType)
		Q_PROPERTY(ByteOrder ByteOrder READ byteOrder WRITE setByteOrder)
		Q_PROPERTY(DataFormat Format READ format WRITE setFormat)

		Q_PROPERTY(int Size READ size WRITE setSize)

		Q_PROPERTY(int ValidityOffset READ validityOffset WRITE setValidityOffset)
		Q_PROPERTY(int ValidityBit READ validityBit WRITE setValidityBit)

		Q_PROPERTY(int ValueOffset READ valueOffset WRITE setValueOffset)
		Q_PROPERTY(int ValueBit READ valueBit WRITE setValueBit)

		Q_ENUMS(SignalType)
		Q_ENUMS(ByteOrder)
		Q_ENUMS(DataFormat)

		// SignalType
		//
	public:
		enum SignalType
		{
			DiagDiscrete,
			DiagAnalog,
			InputDiscrete,
			InputAnalog,
			OutputDiscrete,
			OutputAnalog,
		};

		enum DataFormat
		{
			UnsignedInt = 0,
			SignedInt = 1,
		};


		enum ByteOrder
		{
			LittleEdndian = 0,		// little endian
			BigEndian = 1			// big endian
		};

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

		// Properties
		//
	public:
        DeviceSignal::SignalType type() const;
        Q_INVOKABLE int jsType() const;
        void setType(DeviceSignal::SignalType value);

		ByteOrder byteOrder() const;
		void setByteOrder(ByteOrder value);

		DataFormat format() const;
		void setFormat(DataFormat value);

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

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Signal;

		SignalType m_type = SignalType::DiagDiscrete;
		ByteOrder m_byteOrder = ByteOrder::LittleEdndian;
		DataFormat m_format = DataFormat::UnsignedInt;

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

		Q_PROPERTY(int Type READ type WRITE setType)

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

		Q_PROPERTY(int Type READ type WRITE setType)

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
		int type() const;
		void setType(int value);

		// Data
		//
	private:
		static const DeviceType m_deviceType = DeviceType::Software;

		int m_type = 0;
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

		DeviceObject* deviceObject(const QString& strId);
		std::shared_ptr<DeviceObject> deviceObjectSharedPointer(const QString& strId);

	private:
		void addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent);

	private:
		std::shared_ptr<DeviceObject> m_root;
		QHash<QString, std::shared_ptr<DeviceObject>> m_deviceTable;
	};

	extern Factory<Hardware::DeviceObject> DeviceObjectFactory;
}
