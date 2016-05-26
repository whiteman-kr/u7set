#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"
#include <utility>
#include <QJSEngine>
#include <QQmlEngine>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QMetaProperty>
#include <QXmlStreamReader>
#include <QFile>
#include <QMetaProperty>


namespace Hardware
{
	const wchar_t* DeviceObjectExtensions[] =
		{
			L".hrt",		// Root
			L".hsm",		// System
			L".hrk",		// Rack
			L".hcs",		// Chassis
			L".hmd",		// Module
			L".hws",		// Workstation
			L".hsw",		// Software
			L".hcr",		// Controller
			L".hds",		// Signal
		};

	const wchar_t* DeviceTypeNames[] =
		{
			L"Root",		// Root
			L"System",		// System
			L"Rack",		// Rack
			L"Chassis",		// Chassis
			L"Module",		// Module
			L"Workstation",	// Workstation
			L"Software",	// Software
			L"Controller",	// Controller
			L"Signal",		// Signal
		};

	Factory<Hardware::DeviceObject> DeviceObjectFactory;

	void Init()
	{
#ifdef VFRAME30LIB_LIBRARY
		qDebug() << "Hardware::Init" << " VFrame30 instance";
#else
		qDebug() << "Hardware::Init" << " not VFrame30 instance";
#endif

		static bool firstRun = false;
		if (firstRun)
		{
			assert(false);
		}
		firstRun = true;

		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRoot>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceSystem>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRack>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceChassis>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceModule>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceController>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceSignal>();
		Hardware::DeviceObjectFactory.Register<Hardware::Workstation>();
		Hardware::DeviceObjectFactory.Register<Hardware::Software>();

	}


	void Shutdwon()
	{
#ifdef VFRAME30LIB_LIBRARY
		qDebug() << "Hardware::Shutdown" << " VFrame30 instance";
#else
		qDebug() << "Hardware::Shutdown" << " not VFrame30 instance";
#endif

		DeviceObject::PrintRefCounter("DeviceObject");

		return;
	}

	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(bool preset /*= false*/) :
		m_preset(preset)
	{
static const QString fileIdCaption("FileID");								// Perfomance optimization
static const QString uuidCaption("Uuid");
static const QString equipmentIdTemplateCaption("EquipmentIDTemplate");
static const QString equipmentIdCaption("EquipmentID");
static const QString captionCaption("Caption");
static const QString childRestrictionCaption("ChildRestriction");
static const QString placeCaption("Place");
static const QString specPropCaption("SpecificProperties");
static const QString presetCaption("Preset");
static const QString presetRootCaption("PresetRoot");
static const QString presetNameCaption("PresetName");
static const QString presetObjectUuidCaption("PresetObjectUuid");

		ADD_PROPERTY_GETTER(int, fileIdCaption, true, DeviceObject::fileId);
		ADD_PROPERTY_GETTER(QUuid, uuidCaption, true, DeviceObject::uuid);
		ADD_PROPERTY_GETTER_SETTER(QString, equipmentIdTemplateCaption, true, DeviceObject::equipmentIdTemplate, DeviceObject::setEquipmentIdTemplate);

		auto equipmentIdProp = ADD_PROPERTY_GETTER(QString, equipmentIdCaption, true, DeviceObject::equipmentId);
		equipmentIdProp->setReadOnly(true);

		auto captionProp = ADD_PROPERTY_GETTER_SETTER(QString, captionCaption, true, DeviceObject::caption, DeviceObject::setCaption);

		auto childRestrProp = ADD_PROPERTY_GETTER_SETTER(QString, childRestrictionCaption, true, DeviceObject::childRestriction, DeviceObject::setChildRestriction);
		childRestrProp->setExpert(true);

		ADD_PROPERTY_GETTER_SETTER(int, placeCaption, true, DeviceObject::place, DeviceObject::setPlace);

		auto specificProp = ADD_PROPERTY_GETTER_SETTER(QString, specPropCaption, true, DeviceObject::specificProperties, DeviceObject::setSpecificProperties);
		specificProp->setExpert(true);

		auto presetProp = ADD_PROPERTY_GETTER(bool, presetCaption, true, DeviceObject::preset);
		presetProp->setExpert(true);

		auto presetRootProp = ADD_PROPERTY_GETTER(bool, presetRootCaption, true, DeviceObject::presetRoot);
		presetRootProp->setExpert(true);

		if (preset == true)
		{
			auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, presetNameCaption, true, DeviceObject::presetName, DeviceObject::setPresetName);
			presetNameProp->setExpert(true);
		}

		auto presetObjectUuidProp = ADD_PROPERTY_GETTER(QUuid, presetObjectUuidCaption, true, DeviceObject::presetObjectUuid);
		presetObjectUuidProp->setExpert(true);

		captionProp->setUpdateFromPreset(true);
		childRestrProp->setUpdateFromPreset(true);
		specificProp->setUpdateFromPreset(true);
	}

	DeviceObject::~DeviceObject()
	{
	}

	DeviceObject* DeviceObject::fromDbFile(const DbFile& file)
	{
		DeviceObject* object = DeviceObject::Create(file.data());

		if (object == nullptr)
		{
			assert(object != nullptr);
			return nullptr;
		}

		object->setFileInfo(file);
		return object;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);

		Proto::DeviceObject* mutableDeviceObject = message->mutable_deviceobject();

		Proto::Write(mutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(mutableDeviceObject->mutable_equipmentid(), m_equipmentId);
		Proto::Write(mutableDeviceObject->mutable_caption(), m_caption);

		mutableDeviceObject->set_place(m_place);

		if (m_childRestriction.isEmpty() == false)
		{
			Proto::Write(mutableDeviceObject->mutable_childrestriction(), m_childRestriction);
		}

		if (m_specificPropertiesStruct.isEmpty() == false)
		{
			mutableDeviceObject->set_specific_properties_struct(m_specificPropertiesStruct.toStdString());
		}

		// Save specific properties' values
		//
		std::vector<std::shared_ptr<Property>> props = this->properties();

		for (auto p : props)
		{
			if (p->specific() == true)
			{
				::Proto::Property* protoProp = mutableDeviceObject->mutable_properties()->Add();
				p->saveValue(protoProp);
			}
		}

		// --
		//
		if (m_preset == true)
		{
			mutableDeviceObject->set_preset(m_preset);

			mutableDeviceObject->set_presetroot(m_presetRoot);
			Proto::Write(mutableDeviceObject->mutable_presetname(), m_presetName);
			Proto::Write(mutableDeviceObject->mutable_presetobjectuuid(), m_presetObjectUuid);
		}

		return true;
	}

	bool DeviceObject::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		const Proto::DeviceObject& deviceobject = message.deviceobject();

		m_uuid = Proto::Read(deviceobject.uuid());
		assert(m_uuid.isNull() == false);

		Proto::Read(deviceobject.equipmentid(), &m_equipmentId);
		Proto::Read(deviceobject.caption(), &m_caption);
		m_place = deviceobject.place();

		if (deviceobject.has_childrestriction() == true)
		{
			Proto::Read(deviceobject.childrestriction(), &m_childRestriction);
		}
		else
		{
			m_childRestriction.clear();
		}

		if (deviceobject.has_specific_properties_struct() == true)
		{
			m_specificPropertiesStruct = QString::fromStdString(deviceobject.specific_properties_struct());
			parseSpecificPropertiesStruct();
		}
		else
		{
			m_specificPropertiesStruct.clear();
		}

		// Load specific properties' values. They are already exists after calling parseSpecificPropertiesStruct()
		//
		std::vector<std::shared_ptr<Property>> specificProps = this->properties();

		for (const ::Proto::Property& p :  deviceobject.properties())
		{
			auto it = std::find_if(specificProps.begin(), specificProps.end(),
				[p](std::shared_ptr<Property> dp)
				{
					return dp->caption().toStdString() == p.name();
				});

			if (it == specificProps.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in" << m_equipmentId;
			}
			else
			{
				std::shared_ptr<Property> property = *it;

				assert(property->specific() == true);	// it's suppose to be specific property;

				bool loadOk = property->loadValue(p);

				Q_UNUSED(loadOk);
				assert(loadOk);
			}

		}

		// --
		//

		if (deviceobject.has_preset() == true && deviceobject.preset() == true)
		{
			m_preset = deviceobject.preset();

			if (m_preset == true && propertyExists("PresetName") == false)
			{
static const QString presetNameCaption("PresetName");	// Optimization
				auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, presetNameCaption, true, DeviceObject::presetName, DeviceObject::setPresetName);
				presetNameProp->setExpert(true);
			}

			assert(deviceobject.has_presetroot() == true);
			if (deviceobject.has_presetroot() == true)
			{
				m_presetRoot = deviceobject.presetroot();
			}

			assert(deviceobject.has_presetname());
			if (deviceobject.has_presetname() == true)
			{
				Proto::Read(deviceobject.presetname(), &m_presetName);
			}

			assert(deviceobject.has_presetobjectuuid());
			if (deviceobject.has_presetobjectuuid() == true)
			{
				m_presetObjectUuid = Proto::Read(deviceobject.presetobjectuuid());
			}

		}

		return true;
	}

	DeviceObject* DeviceObject::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		DeviceObject* pDeviceObject = DeviceObjectFactory.Create(classNameHash);

		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return nullptr;
		}

		pDeviceObject->LoadData(message);

		return pDeviceObject;
	}

	void DeviceObject::expandEquipmentId()
	{
		if (parent() != nullptr)
		{
			m_equipmentId.replace(QString("$(PARENT)"), parent()->equipmentIdTemplate(), Qt::CaseInsensitive);
		}

		m_equipmentId.replace(QString("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);

		for (int i = 0; i < childrenCount(); i++)
		{
			child(i)->expandEquipmentId();
		}

		return;
	}

	// Get all signals, including signals from child items
	//
	std::vector<std::shared_ptr<DeviceSignal>> DeviceObject::getAllSignals() const
	{
		std::vector<std::shared_ptr<DeviceSignal>> deviceSignals;
		deviceSignals.reserve(128);

		getAllSignalsRecursive(&deviceSignals);

		return deviceSignals;
	}

	bool DeviceObject::event(QEvent* /*e*/)
	{
		// Event was not recognized
		//
		return false;
	}

	// Parse m_specificProperties and create PropertyObject meta system specific properies
	//
	void DeviceObject::parseSpecificPropertiesStruct()
	{
		// Save all specific properties values
		//
		auto oldProperties = this->properties();

		oldProperties.erase(std::remove_if(oldProperties.begin(), oldProperties.end(),
			[](std::shared_ptr<Property> p)
			{
				return p->specific();
			}), oldProperties.end());

		// Delete all previous object's specific properties
		//
		this->removeSpecificProperties();

		// Parse struct (rows, divided by semicolon) and create new properties
		//

		/*
		Example:

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset
		1;          IP;		Server;		string;		0;			0;			192.168.75.254;     0           false
		1;          Port;	Server;		uint32_t;	1;			65535;		2345;               0           false

		version:            record version
		name:               property name
		category:           category name
		type:               property type, can by one of
							qint32  (4 bytes signed integral),
							quint32 (4 bytes unsigned integer)
							bool (true, false),
							double,
							E::Channel,
							string
		min:                property minimum value (ignored for bool, string)
		max:                property maximim value (ignored for bool, string)
		default:            can be any value of the specified type
		precision:          property precision
		updateFromPreset:   property will be updated from preset
		*/

		QStringList rows = m_specificPropertiesStruct.split(QChar::LineFeed, QString::SkipEmptyParts);

		for (const QString& r : rows)
		{
			if (r.isEmpty() == true)
			{
				continue;
			}

			QStringList columns = r.split(';');


			QString strVersion(columns[0]);
			bool ok = false;
			int version = strVersion.toInt(&ok);
			if (ok == false || version < 1)
			{
				qDebug() << Q_FUNC_INFO << " SpecificProperties: version should be greater than 0: " << strVersion;
				continue;
			}

			if (version > 2)
			{
				qDebug() << Q_FUNC_INFO << " SpecificProperties: Unsupported property version: " << version;
				continue;
			}

			if (version == 1)
			{
				parseSpecificPropertieyStructV1(columns);
			}

			if (version == 2)
			{
				parseSpecificPropertieyStructV2(columns);
			}

		}

		// Set to parsed properties old value
		//
		auto newProperties = properties();

		for (std::shared_ptr<Property> p : oldProperties)
		{
			auto it = std::find_if(newProperties.begin(), newProperties.end(),
				[p](std::shared_ptr<Property> np)
				{
					  return np->caption() == p->caption();
				}
				);

			if (it != newProperties.end() && (*it)->value().type() == p->value().type())
			{
				setPropertyValue(p->caption(), p->value());
			}
			else
			{
				continue;
				// default value already was set
			}
		}

		return;
	}


	void DeviceObject::parseSpecificPropertieyStructV1(const QStringList &columns)
	{
		if (columns.count() != 9)
		{
			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 1!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset";
			return;
		}

		QString name(columns[1]);
		QString category(columns[2]);
		QStringRef type(&columns[3]);
		QStringRef min(&columns[4]);
		QStringRef max(&columns[5]);
		QStringRef defaultValue(&columns[6]);
		QStringRef strPrecision(&columns[7]);
		QString strUpdateFromPreset(columns[8]);

		int precision = strPrecision.toInt();

		bool updateFromPreset = false;
		if (strUpdateFromPreset.toUpper() == "TRUE")
		{
			updateFromPreset = true;
		}

		if (name.isEmpty() || name.size() > 1024)
		{
			qDebug() << Q_FUNC_INFO << " SpecificProperties: filed name must have size  from 1 to 1024, name: " << name;
			return;
		}

		if (type != "qint32" &&
				type != "quint32" &&
				type != "bool" &&
				type != "double" &&
				type != "E::Channel" &&
				type != "string")
		{
			qDebug() << Q_FUNC_INFO << " SpecificProperties: wrong filed tyep: " << type;
			return;
		}


		if (type == "qint32")
		{
			// Min
			//
			bool ok = false;
			qint32 minInt = min.toInt(&ok);
			if (ok == false)
			{
				minInt = std::numeric_limits<qint32>::min();
			}

			// Max
			//
			qint32 maxInt = max.toInt(&ok);
			if (ok == false)
			{
				maxInt = std::numeric_limits<qint32>::max();
			}

			// Default Value
			//
			qint32 defaultInt = defaultValue.toInt();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minInt), QVariant(maxInt));
			newProperty->setValue(QVariant(defaultInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}

		if (type == "quint32")
		{
			// Min
			//
			bool ok = false;
			quint32 minUInt = min.toUInt(&ok);
			if (ok == false)
			{
				minUInt = std::numeric_limits<quint32>::min();
			}

			// Max
			//
			quint32 maxUInt = max.toUInt(&ok);
			if (ok == false)
			{
				maxUInt = std::numeric_limits<quint32>::max();
			}

			// Default Value
			//
			quint32 defaultUInt = defaultValue.toUInt();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minUInt), QVariant(maxUInt));
			newProperty->setValue(QVariant(defaultUInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}

		if (type == "double")
		{
			// Min
			//
			bool ok = false;
			double minDouble = min.toDouble(&ok);
			if (ok == false)
			{
				minDouble = std::numeric_limits<double>::min();
			}

			// Max
			//
			double maxDouble = max.toDouble(&ok);
			if (ok == false)
			{
				maxDouble = std::numeric_limits<double>::max();
			}

			// Default Value
			//
			double defaultDouble = defaultValue.toDouble();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minDouble), QVariant(maxDouble));
			newProperty->setValue(QVariant(defaultDouble));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}

		if (type == "bool")
		{
			// Default Value
			//
			bool defaultBool = defaultValue.compare("true", Qt::CaseInsensitive) == 0;

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(QVariant(defaultBool));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}

		if (type == "E::Channel")
		{
			// Default Value
			//
			QString defaultString = defaultValue.toString();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);
			newProperty->setValue(QVariant::fromValue(E::Channel::A));

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(defaultString.toStdString().c_str());
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}

		if (type == "string")
		{
			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(QVariant(defaultValue.toString()));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}
	}

	void DeviceObject::parseSpecificPropertieyStructV2(const QStringList &columns)
	{
		if (columns.count() != 11)
		{
			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 2!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description";
			return;
		}
		QString name(columns[1]);
		QString category(columns[2]);
		QStringRef type(&columns[3]);
		QStringRef min(&columns[4]);
		QStringRef max(&columns[5]);
		QStringRef defaultValue(&columns[6]);
		QStringRef strPrecision(&columns[7]);
		QString strUpdateFromPreset(columns[8]);
		QString strExpert(columns[9]);
		QString strDescription(columns[10]);

		int precision = strPrecision.toInt();

		bool updateFromPreset = false;
		if (strUpdateFromPreset.toUpper() == "TRUE")
		{
			updateFromPreset = true;
		}

		bool expert = false;
		if (strExpert.toUpper() == "TRUE")
		{
			expert = true;
		}

		if (name.isEmpty() || name.size() > 1024)
		{
			qDebug() << Q_FUNC_INFO << " SpecificProperties: filed name must have size  from 1 to 1024, name: " << name;
			return;
		}

		if (type != "qint32" &&
				type != "quint32" &&
				type != "bool" &&
				type != "double" &&
				type != "E::Channel" &&
				type != "string")
		{
			qDebug() << Q_FUNC_INFO << " SpecificProperties: wrong filed tyep: " << type;
			return;
		}


		if (type == "qint32")
		{
			// Min
			//
			bool ok = false;
			qint32 minInt = min.toInt(&ok);
			if (ok == false)
			{
				minInt = std::numeric_limits<qint32>::min();
			}

			// Max
			//
			qint32 maxInt = max.toInt(&ok);
			if (ok == false)
			{
				maxInt = std::numeric_limits<qint32>::max();
			}

			// Default Value
			//
			qint32 defaultInt = defaultValue.toInt();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minInt), QVariant(maxInt));
			newProperty->setValue(QVariant(defaultInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}

		if (type == "quint32")
		{
			// Min
			//
			bool ok = false;
			quint32 minUInt = min.toUInt(&ok);
			if (ok == false)
			{
				minUInt = std::numeric_limits<quint32>::min();
			}

			// Max
			//
			quint32 maxUInt = max.toUInt(&ok);
			if (ok == false)
			{
				maxUInt = std::numeric_limits<quint32>::max();
			}

			// Default Value
			//
			quint32 defaultUInt = defaultValue.toUInt();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minUInt), QVariant(maxUInt));
			newProperty->setValue(QVariant(defaultUInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}

		if (type == "double")
		{
			// Min
			//
			bool ok = false;
			double minDouble = min.toDouble(&ok);
			if (ok == false)
			{
				minDouble = std::numeric_limits<double>::min();
			}

			// Max
			//
			double maxDouble = max.toDouble(&ok);
			if (ok == false)
			{
				maxDouble = std::numeric_limits<double>::max();
			}

			// Default Value
			//
			double defaultDouble = defaultValue.toDouble();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setLimits(QVariant(minDouble), QVariant(maxDouble));
			newProperty->setValue(QVariant(defaultDouble));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}

		if (type == "bool")
		{
			// Default Value
			//
			bool defaultBool = defaultValue.compare("true", Qt::CaseInsensitive) == 0;

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(QVariant(defaultBool));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}

		if (type == "E::Channel")
		{
			// Default Value
			//
			QString defaultString = defaultValue.toString();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);
			newProperty->setValue(QVariant::fromValue(E::Channel::A));

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(defaultString.toStdString().c_str());
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}

		if (type == "string")
		{
			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty<QVariant>(name, true);

			newProperty->setSpecific(true);
			newProperty->setCategory(category);
			newProperty->setValue(QVariant(defaultValue.toString()));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}
	}

	// Get all signals, including signals from child items
	//
	void DeviceObject::getAllSignalsRecursive(std::vector<std::shared_ptr<DeviceSignal>>* deviceSignals) const
	{
		if (deviceSignals == nullptr)
		{
			assert(deviceSignals);
			return;
		}

		for (const std::shared_ptr<DeviceObject>& child : m_children)
		{
			if (child->deviceType() == DeviceType::Signal)
			{
				deviceSignals->push_back(std::dynamic_pointer_cast<DeviceSignal>(child));
				assert(dynamic_cast<DeviceSignal*>(deviceSignals->back().get()) != nullptr);
			}
			else
			{
				child->getAllSignalsRecursive(deviceSignals);
			}
		}

		return;
	}

	DeviceObject* DeviceObject::parent()
	{
		return m_parent;
	}

	const DeviceObject* DeviceObject::parent() const
	{
		return m_parent;
	}

	QObject* DeviceObject::jsParent() const
	{
		QObject* c = m_parent;
		QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
		return c;
	}

	int DeviceObject::jsPropertyInt(QString name) const
	{
		const std::shared_ptr<Property> p = propertyByCaption(name);
		if (p == nullptr)
		{
			assert(false);
			return 0;
		}

		QVariant v = p->value();
		if (v.isValid() == false)
		{
			assert(v.isValid());
			return 0;
		}

		return v.toInt();
	}

	quint32 DeviceObject::jsPropertyIP(QString name) const
	{
		const std::shared_ptr<Property> p = propertyByCaption(name);
		if (p == nullptr)
		{
			assert(false);
			return 0;
		}

		QVariant v = p->value();
		if (v.isValid() == false)
		{
			assert(v.isValid());
			return 0;
		}

		QString s = v.toString();
		QStringList l = s.split(".");
		if (l.size() != 4)
		{
			return 0;
		}

		quint32 result = 0;
		for (int i = 0; i < 4; i++)
		{
			bool ok = false;
			quint8 b = l[i].toInt(&ok);

			if (ok == false)
			{
				return 0;
			}

			result |= b;
			if (i < 3)
			{
				result <<= 8;
			}
		}

		return result;
	}

	DeviceType DeviceObject::deviceType() const
	{
		assert(false);
		return DeviceType::Root;
	}

	int DeviceObject::jsDeviceType() const
	{
		return static_cast<int>(deviceType());
	}

	bool DeviceObject::isRoot() const
	{
		return deviceType() == DeviceType::Root;
	}

	bool DeviceObject::isSystem() const
	{
		return deviceType() == DeviceType::System;
	}

	bool DeviceObject::isRack() const
	{
		return deviceType() == DeviceType::Rack;
	}

	bool DeviceObject::isChassis() const
	{
		return deviceType() == DeviceType::Chassis;
	}

	bool DeviceObject::isModule() const
	{
		return deviceType() == DeviceType::Module;
	}

	bool DeviceObject::isController() const
	{
		return deviceType() == DeviceType::Controller;
	}

	bool DeviceObject::isWorkstation() const
	{
		return deviceType() == DeviceType::Workstation;
	}

	bool DeviceObject::isSoftware() const
	{
		return deviceType() == DeviceType::Software;
	}

	bool DeviceObject::isSignal() const
	{
		return deviceType() == DeviceType::Signal;
	}

	const Hardware::DeviceSystem* DeviceObject::toSystem() const
	{
		const Hardware::DeviceSystem* d = dynamic_cast<const Hardware::DeviceSystem*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::DeviceSystem* DeviceObject::toSystem()
	{
		Hardware::DeviceSystem* d = dynamic_cast<Hardware::DeviceSystem*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::DeviceRack* DeviceObject::toRack() const
	{
		const Hardware::DeviceRack* d = dynamic_cast<const Hardware::DeviceRack*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::DeviceRack* DeviceObject::toRack()
	{
		Hardware::DeviceRack* d = dynamic_cast<Hardware::DeviceRack*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::DeviceChassis* DeviceObject::toChassis() const
	{
		const Hardware::DeviceChassis* d = dynamic_cast<const Hardware::DeviceChassis*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::DeviceChassis* DeviceObject::toChassis()
	{
		Hardware::DeviceChassis* d = dynamic_cast<Hardware::DeviceChassis*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::DeviceModule* DeviceObject::toModule() const
	{
		const Hardware::DeviceModule* d = dynamic_cast<const Hardware::DeviceModule*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::DeviceModule* DeviceObject::toModule()
	{
		Hardware::DeviceModule* d = dynamic_cast<Hardware::DeviceModule*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::DeviceController* DeviceObject::toController() const
	{
		const Hardware::DeviceController* d = dynamic_cast<const Hardware::DeviceController*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::DeviceController* DeviceObject::toController()
	{
		Hardware::DeviceController* d = dynamic_cast<Hardware::DeviceController*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::Software* DeviceObject::toSoftware() const
	{
		const Hardware::Software* d = dynamic_cast<const Hardware::Software*>(this);
		assert(d != nullptr);

		return d;
	}

	Hardware::Software* DeviceObject::toSoftware()
	{
		Hardware::Software* d = dynamic_cast<Hardware::Software*>(this);
		assert(d != nullptr);

		return d;
	}

	const Hardware::DeviceController* DeviceObject::getParentController() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject->isController())
			{
				return deviceObject->toController();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceModule* DeviceObject::getParentModule() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject->isModule())
			{
				return deviceObject->toModule();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceChassis* DeviceObject::getParentChassis() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject->isChassis())
			{
				return deviceObject->toChassis();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceRack* DeviceObject::getParentRack() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject->isRack())
			{
				return deviceObject->toRack();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceSystem* DeviceObject::getParentSystem() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject->isSystem())
			{
				return deviceObject->toSystem();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	QString DeviceObject::fileExtension() const
	{
		size_t index = static_cast<size_t>(deviceType());
		assert(index < sizeof(Hardware::DeviceObjectExtensions) / sizeof(Hardware::DeviceObjectExtensions[0]));

		QString result = QString::fromWCharArray(Hardware::DeviceObjectExtensions[index]);
		return result;
	}

	QString DeviceObject::fileExtension(DeviceType device)
	{
		QString result = QString::fromWCharArray(Hardware::DeviceObjectExtensions[static_cast<int>(device)]);
		return result;
	}

	int DeviceObject::childrenCount() const
	{
		return static_cast<int>(m_children.size());
	}

	DeviceObject* DeviceObject::child(int index) const
	{
		return m_children.at(index).get();
	}

	DeviceObject* DeviceObject::child(QUuid uuid) const
	{
		for (std::shared_ptr<Hardware::DeviceObject> child : m_children)
		{
			if (child->uuid() == uuid)
			{
				return child.get();
			}
		}

		return nullptr;
	}

	QObject* DeviceObject::jsChild(int index) const
	{
		QObject* c = m_children.at(index).get();
		QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
		return c;
	}

	int DeviceObject::childIndex(DeviceObject* child) const
	{
		// Manual search for an index is 1.6 times faster than std::find
		//
		size_t childCount = m_children.size();
		for (size_t i = 0; i < childCount; i++)
		{
			if (m_children[i].get() == child)
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	std::shared_ptr<DeviceObject> DeviceObject::childSharedPtr(int index)
	{
		std::shared_ptr<DeviceObject> sp = m_children.at(index);
		return sp;
	}

	std::shared_ptr<DeviceObject> DeviceObject::childSharedPtr(QUuid uuid)
	{
		for (std::shared_ptr<Hardware::DeviceObject> child : m_children)
		{
			if (child->uuid() == uuid)
			{
				return child;
			}
		}

		return std::shared_ptr<DeviceObject>();
	}

	std::shared_ptr<DeviceObject> DeviceObject::childSharedPtrByPresetUuid(QUuid presetObjectUuid)
	{
		for (std::shared_ptr<Hardware::DeviceObject> child : m_children)
		{
			if (child->presetObjectUuid() == presetObjectUuid)
			{
				return child;
			}
		}

		return std::shared_ptr<DeviceObject>();
	}

	void DeviceObject::addChild(std::shared_ptr<DeviceObject> child)
	{
		if (child->deviceType() == DeviceType::Software &&
			deviceType() != DeviceType::Workstation &&
			deviceType() != DeviceType::Root)
		{
			assert(false);
			return;
		}

		if (deviceType() >= child->deviceType())
		{
			assert(deviceType() < child->deviceType());
			return;
		}

		if (child->deviceType() == DeviceType::Workstation && deviceType() > DeviceType::Chassis)
		{
			assert(false);
			return;
		}

//		if (child->deviceType() == DeviceType::Software &&
//			deviceType() != DeviceType::Workstation &&
//			deviceType() != DeviceType::Root)
//		{
//			assert(false);
//			return;
//		}

		child->m_parent = this;
		m_children.push_back(child);
	}

	void DeviceObject::deleteChild(DeviceObject* child)
	{
		auto found = std::find_if(m_children.begin(), m_children.end(), [child](decltype(m_children)::const_reference c)
			{
				return c.get() == child;
			});

		if (found == m_children.end())
		{
			assert(found != m_children.end());
			return;
		}

		m_children.erase(found);
		return;
	}

	void DeviceObject::deleteAllChildren()
	{
		m_children.clear();
	}

	bool DeviceObject::checkChild(DeviceObject* child, QString* errorMessage)
	{
		if (child == nullptr ||
			errorMessage == nullptr)
		{
			assert(child);
			assert(errorMessage);
			return false;
		}

		// Check device level
		//
		if (deviceType() > child->deviceType())
		{
			*errorMessage = tr("Childer device level must be lower that perents.");
			return false;
		}

		// Assume that an empty script is true. It will allow to save memory for modules, controllers...
		//
		if (m_childRestriction.isEmpty() == true)
		{
			return true;
		}

		// Create a copy of child, will be deleted by jsEngine on destroying.
		// It is because QJSEngine::newQObject makes value with JavaEngine ownership
		// and it cannot be changed (((
		// In QQmlEngine (derived from QJSEngine) it can be changed by calling QQmlEngine::setObjectOwnership(???, QQmlEngine::CppOwnership);
		// also, QScriptEngine::QtOwnership can help.
		// Hopefully in future Qt releases it might be changed, that ownership can be chnged from QJSEngine or QJSValue.
		//

		//QByteArray data;
		//child->Save(data);

		//DeviceObject* childCopy = DeviceObject::Create(data);
		//data.clear();

		// Run m_childRestriction script
		//
		QJSEngine jsEngine;

		//QJSValue arg = jsEngine.newQObject(childCopy);
		QJSValue arg = jsEngine.newQObject(child);
		QQmlEngine::setObjectOwnership(child, QQmlEngine::CppOwnership);

		QJSValue function = jsEngine.evaluate(m_childRestriction);
		QJSValue result = function.call(QJSValueList() << arg);

		if (result.isError() == true)
		{
			*errorMessage = tr("Script error:").arg(result.toString());
			return false;
		}

		bool boolResult = result.toBool();
		if (boolResult == false)
		{
			*errorMessage = tr("DeviceObject is not allowed.");
		}

		return boolResult;
	}

	void DeviceObject::sortByPlace(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_place < ref2->m_place)
				{
					return true;
				}
				else
				{
					if (ref1->m_place == ref2->m_place)
					{
						return ref1->m_equipmentId < ref2->m_equipmentId;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	void DeviceObject::sortByEquipmentId(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_equipmentId < ref2->m_equipmentId)
				{
					return true;
				}
				else
				{
					if (ref1->m_equipmentId == ref2->m_equipmentId)
					{
						return ref1->m_place < ref2->m_place;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	void DeviceObject::sortByCaption(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_caption < ref2->m_caption)
				{
					return true;
				}
				else
				{
					if (ref1->m_caption == ref2->m_caption)
					{
						return ref1->m_place < ref2->m_place;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	void DeviceObject::sortByState(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_fileInfo.state() < ref2->m_fileInfo.state())
				{
					return true;
				}
				else
				{
					if (ref1->m_fileInfo.state() == ref2->m_fileInfo.state())
					{
						return ref1->m_equipmentId < ref2->m_equipmentId;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	void DeviceObject::sortByUser(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_fileInfo.userId() < ref2->m_fileInfo.userId())
				{
					return true;
				}
				else
				{
					if (ref1->m_fileInfo.userId() == ref2->m_fileInfo.userId())
					{
						return ref1->m_equipmentId < ref2->m_equipmentId;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	std::vector<DeviceObject*> DeviceObject::findChildObjectsByMask(const QString& mask)
	{
		std::vector<DeviceObject*> list;
		if (mask.isEmpty() == false)
		{
			findChildObjectsByMask(mask, list);
		}
		return list;
	}

	void DeviceObject::findChildObjectsByMask(const QString& mask, std::vector<DeviceObject*>& list)
	{
		if (mask.isEmpty() == true)
		{
			return;
		}

		for (auto c : m_children)
		{
			if (CUtils::processDiagSignalMask(mask, c->equipmentIdTemplate()) == true)
			{
				list.push_back(c.get());
			}
			c->findChildObjectsByMask(mask, list);
		}
	}

	QObject* DeviceObject::jsFindChildObjectByMask(const QString& mask)
	{
		if (mask.isEmpty() == true)
		{
			return nullptr;
		}

		std::vector<DeviceObject*> list = findChildObjectsByMask(mask);
		if (list.empty() == true)
		{
			return nullptr;
		}

		QObject* c = list.at(0);
		QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
		return c;
	}

	int DeviceObject::fileId() const
	{
		return fileInfo().fileId();
	}

	QUuid DeviceObject::uuid() const
	{
		return m_uuid;
	}

	void DeviceObject::setUuid(QUuid value)
	{
		if (m_uuid != value)
		{
			m_uuid = value;
		}
	}

	QString DeviceObject::equipmentIdTemplate() const
	{
		return m_equipmentId;
	}

	void DeviceObject::setEquipmentIdTemplate(QString value)
	{
		if (m_equipmentId != value)
		{
			m_equipmentId = value;
		}
	}

	QString DeviceObject::equipmentId() const
	{
		std::list<std::pair<const DeviceObject*, QString>> devices;		// 1st: device, 2nd: it's equipmentId

		const DeviceObject* d = this;
		while (d != nullptr)
		{
			devices.push_front(std::make_pair(d, d->equipmentIdTemplate()));

			d = d->parent();
		}

		QString parentId = "";
		for (std::pair<const DeviceObject*, QString>& dp : devices)
		{
			const DeviceObject* device = dp.first;
			QString equipId = dp.second;

			if (device->parent() != nullptr)
			{
				equipId.replace(QString("$(PARENT)"), parentId, Qt::CaseInsensitive);
			}

			equipId.replace(QString("$(PLACE)"), QString::number(device->place()).rightJustified(2, '0'), Qt::CaseInsensitive);

			parentId = equipId;
		}

		QString thisId = parentId;
		return thisId;
	}

	QString DeviceObject::caption() const
	{
		return m_caption;
	}

	void DeviceObject::setCaption(QString value)
	{
		if (m_caption != value)
		{
			m_caption = value;
		}
	}

	DbFileInfo& DeviceObject::fileInfo()
	{
		//m_fileInfo.setDetails(details());		// fileInfo() is called to often
		return m_fileInfo;
	}

	const DbFileInfo& DeviceObject::fileInfo() const
	{
		//const_cast<DeviceObject*>(this)->m_fileInfo.setDetails(details());	// fileInfo() is called to often
		return m_fileInfo;
	}

	void DeviceObject::setFileInfo(const DbFileInfo& value)
	{
		m_fileInfo = value;
		m_fileInfo.setDetails(details());
	}

	QString DeviceObject::childRestriction() const
	{
		return m_childRestriction;
	}

	void DeviceObject::setChildRestriction(QString value)
	{
		m_childRestriction = value;
	}

	QString DeviceObject::specificProperties() const
	{
		return m_specificPropertiesStruct;
	}

	void DeviceObject::setSpecificProperties(QString value)
	{
		if (m_specificPropertiesStruct != value)
		{
			m_specificPropertiesStruct = value;
			parseSpecificPropertiesStruct();
		}
	}

	int DeviceObject::place() const
	{
		return m_place;
	}

	int DeviceObject::jsPlace() const
	{
		return place();

	}

	void DeviceObject::setPlace(int value)
	{
		if (m_place != value)
		{
			m_place = value;
		}
	}

	// JSON short description, uuid, equipmentId, caption, place, etc
	//
	QString DeviceObject::details() const
	{
		QString json = QString(
R"DELIM({
	"Uuid" : "%1",
	"EquipmentID" : "%2",
	"Caption" : "%3",
	"Place" : %4,
	"Type" : "%5"
})DELIM")
			.arg(uuid().toString())
			.arg(equipmentIdTemplate())
			.arg(caption())
			.arg(place())
			.arg(fileExtension());

		return json;
	}

	bool DeviceObject::preset() const
	{
		return m_preset;
	}

	bool DeviceObject::presetRoot() const
	{
		return m_presetRoot;
	}

	void DeviceObject::setPresetRoot(bool value)
	{
		m_presetRoot = value;
	}

	QString DeviceObject::presetName() const
	{
		return m_presetName;
	}

	void DeviceObject::setPresetName(QString value)
	{
		m_presetName = value;
	}

	QUuid DeviceObject::presetObjectUuid() const
	{
		return m_presetObjectUuid;
	}

	void DeviceObject::setPresetObjectUuid(QUuid value)
	{
		m_presetObjectUuid = value;
	}

	//
	//
	// DeviceRoot
	//
	//
	DeviceRoot::DeviceRoot(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		qDebug() << "DeviceRoot::DeviceRoot";
	}

	DeviceRoot::~DeviceRoot()
	{
		qDebug() << "DeviceRoot::~DeviceRoot";
	}

	DeviceType DeviceRoot::deviceType() const
	{
		return m_deviceType;
	}


	//
	//
	// DeviceSystem
	//
	//
	DeviceSystem::DeviceSystem(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		qDebug() << "DeviceRoot::DeviceSystem";
	}

	DeviceSystem::~DeviceSystem()
	{
		qDebug() << "DeviceRoot::~DeviceSystem";
	}

	bool DeviceSystem::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceSystem* systemMessage =
				message->mutable_deviceobject()->mutable_system();

		Q_UNUSED(systemMessage);
		//systemMessage->set_startxdocpt(m_startXDocPt);
		//systemMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceSystem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_system() == false)
		{
			assert(message.deviceobject().has_system());
			return false;
		}

		const Proto::DeviceSystem& systemMessage = message.deviceobject().system();

		Q_UNUSED(systemMessage);
		//m_startXDocPt = systemMessage.startxdocpt();
		//m_startYDocPt = systemMessage.startydocpt();

		return true;
	}

	DeviceType DeviceSystem::deviceType() const
	{
		return m_deviceType;
	}


	//
	//
	// DeviceRack
	//
	//
	DeviceRack::DeviceRack(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceRack::~DeviceRack()
	{
	}

	bool DeviceRack::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceRack* rackMessage =
				message->mutable_deviceobject()->mutable_rack();

		Q_UNUSED(rackMessage);
		//rackMessage->set_startxdocpt(m_startXDocPt);
		//rackMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceRack::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_rack() == false)
		{
			assert(message.deviceobject().has_rack());
			return false;
		}

		const Proto::DeviceRack& rackMessage = message.deviceobject().rack();

		Q_UNUSED(rackMessage);
		//x = rackMessage.startxdocpt();
		//y = rackMessage.startydocpt();

		return true;
	}

	DeviceType DeviceRack::deviceType() const
	{
		return m_deviceType;
	}

	//
	//
	// DeviceChassis
	//
	//
	DeviceChassis::DeviceChassis(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, DeviceChassis::type, DeviceChassis::setType)
		typeProp->setUpdateFromPreset(true);
	}

	DeviceChassis::~DeviceChassis()
	{
	}

	bool DeviceChassis::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceChassis* chassisMessage = message->mutable_deviceobject()->mutable_chassis();

		chassisMessage->set_type(m_type);

		return true;
	}

	bool DeviceChassis::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_chassis() == false)
		{
			assert(message.deviceobject().has_chassis());
			return false;
		}

		const Proto::DeviceChassis& chassisMessage = message.deviceobject().chassis();

		m_type =  chassisMessage.type();

		return true;
	}

	DeviceType DeviceChassis::deviceType() const
	{
		return m_deviceType;
	}

	int DeviceChassis::type() const
	{
		return m_type;
	}

	void DeviceChassis::setType(int value)
	{
		m_type = value;
	}

	//
	//
	// DeviceModule
	//
	//
	DeviceModule::DeviceModule(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		auto familyTypeProp = ADD_PROPERTY_GETTER_SETTER(DeviceModule::FamilyType, "ModuleFamily", true, DeviceModule::moduleFamily, DeviceModule::setModuleFamily)
		familyTypeProp->setExpert(true);

		auto moduleVersionProp = ADD_PROPERTY_GETTER_SETTER(int, "ModuleVersion", true, DeviceModule::moduleVersion, DeviceModule::setModuleVersion)
		moduleVersionProp->setExpert(true);

		familyTypeProp->setUpdateFromPreset(true);
		moduleVersionProp->setUpdateFromPreset(true);
	}

	DeviceModule::~DeviceModule()
	{
	}

	bool DeviceModule::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceModule* moduleMessage = message->mutable_deviceobject()->mutable_module();

		moduleMessage->set_type(static_cast<int>(m_type));

		return true;
	}

	bool DeviceModule::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_module() == false)
		{
			assert(message.deviceobject().has_module());
			return false;
		}

		const Proto::DeviceModule& moduleMessage = message.deviceobject().module();

		m_type =  static_cast<decltype(m_type)>(moduleMessage.type());

		return true;
	}

	DeviceType DeviceModule::deviceType() const
	{
		return m_deviceType;
	}


	DeviceModule::FamilyType DeviceModule::moduleFamily() const
	{
		return static_cast<DeviceModule::FamilyType>(m_type & 0xFF00);
	}

	void DeviceModule::setModuleFamily(DeviceModule::FamilyType value)
	{
		decltype(m_type) tmp = static_cast<decltype(m_type)>(value);

		assert((tmp & 0x00FF) == 0);

		tmp &= 0xFF00;

		m_type = (m_type & 0x00FF) | tmp;
	}

	int DeviceModule::moduleVersion() const
	{
		return static_cast<int>(m_type) & 0xFF;
	}

	void DeviceModule::setModuleVersion(int value)
	{
		decltype(m_type) tmp = static_cast<decltype(m_type)>(value);

		assert((tmp & 0xFF00) == 0);

		m_type = (m_type & 0xFF00) | tmp;
	}

	int DeviceModule::moduleType() const
	{
		return m_type;
	}

	bool DeviceModule::isIOModule() const
	{
		return isInputModule() || isOutputModule();
	}

	bool DeviceModule::isInputModule() const
	{
		FamilyType family = moduleFamily();

		return	family == FamilyType::AIM ||
				family == FamilyType::DIM ||
				family == FamilyType::AIFM;
	}

	bool DeviceModule::isOutputModule() const
	{
		FamilyType family = moduleFamily();

		return	family == FamilyType::AOM ||
				family == FamilyType::DOM;
	}

	bool DeviceModule::isLM() const
	{
		return	moduleFamily() == FamilyType::LM;
	}

	//
	//
	// DeviceController
	//
	//
	DeviceController::DeviceController(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceController::~DeviceController()
	{
	}

	bool DeviceController::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceController* controllerMessage =
				message->mutable_deviceobject()->mutable_controller();

		Q_UNUSED(controllerMessage);
		//controllerMessage->set_startxdocpt(m_startXDocPt);
		//controllerMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceController::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_controller() == false)
		{
			assert(message.deviceobject().has_controller());
			return false;
		}

		const Proto::DeviceController& controllerMessage = message.deviceobject().controller();

		Q_UNUSED(controllerMessage);
		//x = controllerMessage.startxdocpt();
		//y = controllerMessage.startydocpt();

		return true;
	}

	DeviceType DeviceController::deviceType() const
	{
		return m_deviceType;
	}

	//
	//
	// DeviceDiagSignal
	//
	//
	DeviceSignal::DeviceSignal(bool preset /*= false*/) :
		DeviceObject(preset)
	{
static const QString typeCaption("Type");				// Optimization
static const QString functionCaption("Function");
static const QString byteOrderCaption("ByteOrder");
static const QString formatCaption("Format");
static const QString memoryAreaCaption("MemoryArea");
static const QString sizeCaption("Size");
static const QString validityOffsetCaption("ValidityOffset");
static const QString validityBitCaption("ValidityBit");
static const QString valueOffsetCaption("ValueOffset");
static const QString valueBitCaption("ValueBit");

		auto typeProp = ADD_PROPERTY_GETTER_SETTER(E::SignalType, typeCaption, true, DeviceSignal::type, DeviceSignal::setType)
		auto functionProp = ADD_PROPERTY_GETTER_SETTER(E::SignalFunction, functionCaption, true, DeviceSignal::function, DeviceSignal::setFunction)
		auto byteOrderProp = ADD_PROPERTY_GETTER_SETTER(E::ByteOrder, byteOrderCaption, true, DeviceSignal::byteOrder, DeviceSignal::setByteOrder)
		auto formatProp = ADD_PROPERTY_GETTER_SETTER(E::DataFormat, formatCaption, true, DeviceSignal::format, DeviceSignal::setFormat)
		auto memoryAreaProp = ADD_PROPERTY_GETTER_SETTER(E::MemoryArea, memoryAreaCaption, true, DeviceSignal::memoryArea, DeviceSignal::setMemoryArea)

		auto sizeProp = ADD_PROPERTY_GETTER_SETTER(int, sizeCaption, true, DeviceSignal::size, DeviceSignal::setSize)
		auto validityOffsetProp = ADD_PROPERTY_GETTER_SETTER(int, validityOffsetCaption, true, DeviceSignal::validityOffset, DeviceSignal::setValidityOffset)
		auto valididtyBitProp = ADD_PROPERTY_GETTER_SETTER(int, validityBitCaption, true, DeviceSignal::validityBit, DeviceSignal::setValidityBit)

		auto valueOffsetProp = ADD_PROPERTY_GETTER_SETTER(int, valueOffsetCaption, true, DeviceSignal::valueOffset, DeviceSignal::setValueOffset)
		auto valueBitProp = ADD_PROPERTY_GETTER_SETTER(int, valueBitCaption, true, DeviceSignal::valueBit, DeviceSignal::setValueBit)

		typeProp->setUpdateFromPreset(true);
		typeProp->setExpert(preset);

		functionProp->setUpdateFromPreset(true);
		functionProp->setExpert(preset);

		byteOrderProp->setUpdateFromPreset(true);
		byteOrderProp->setExpert(preset);

		formatProp->setUpdateFromPreset(true);
		formatProp->setExpert(preset);

		memoryAreaProp->setUpdateFromPreset(true);
		memoryAreaProp->setExpert(preset);

		sizeProp->setUpdateFromPreset(true);
		sizeProp->setExpert(preset);

		validityOffsetProp->setUpdateFromPreset(true);
		validityOffsetProp->setExpert(preset);

		valididtyBitProp->setUpdateFromPreset(true);
		valididtyBitProp->setExpert(preset);

		valueOffsetProp->setUpdateFromPreset(true);
		valueOffsetProp->setExpert(preset);

		valueBitProp->setUpdateFromPreset(true);
		valueBitProp->setExpert(preset);
	}

	DeviceSignal::~DeviceSignal()
	{
	}

	bool DeviceSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceSignal* signalMessage = message->mutable_deviceobject()->mutable_signal();

		signalMessage->set_type(static_cast<int>(m_type));
		signalMessage->set_function(static_cast<int>(m_function));

		signalMessage->set_byteorder(static_cast<int>(m_byteOrder));
		signalMessage->set_format(static_cast<int>(m_format));
		signalMessage->set_memoryarea(static_cast<int>(m_memoryArea));

		signalMessage->set_size(static_cast<int>(m_size));

		signalMessage->set_validityoffset(static_cast<int>(m_validityOffset));
		signalMessage->set_validitybit(static_cast<int>(m_validityBit));

		signalMessage->set_valueoffset(static_cast<int>(m_valueOffset));
		signalMessage->set_valuebit(static_cast<int>(m_valueBit));

		return true;
	}

	bool DeviceSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_signal() == false)
		{
			assert(message.deviceobject().has_signal());
			return false;
		}

		const Proto::DeviceSignal& signalMessage = message.deviceobject().signal();

		if (signalMessage.has_obsoletetype() == true)
		{
			assert(signalMessage.has_type() == false);
			assert(signalMessage.has_function() == false);

			Obsolete::SignalType obsoleteType = static_cast<Obsolete::SignalType>(signalMessage.obsoletetype());

			switch (obsoleteType)
			{
				case Obsolete::SignalType::DiagDiscrete:
					m_type = E::SignalType::Discrete;
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::DiagAnalog:
					m_type = E::SignalType::Analog;
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::InputDiscrete:
					m_type = E::SignalType::Discrete;
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::InputAnalog:
					m_type = E::SignalType::Analog;
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::OutputDiscrete:
					m_type = E::SignalType::Discrete;
					m_function = E::SignalFunction::Output;
					break;
				case Obsolete::SignalType::OutputAnalog:
					m_type = E::SignalType::Analog;
					m_function = E::SignalFunction::Output;
					break;
				default:
					assert(false);
			}
		}
		else
		{
			m_type = static_cast<E::SignalType>(signalMessage.type());
			m_function = static_cast<E::SignalFunction>(signalMessage.function());
		}

		m_byteOrder = static_cast<E::ByteOrder>(signalMessage.byteorder());
		m_format = static_cast<E::DataFormat>(signalMessage.format());
		m_memoryArea = static_cast<E::MemoryArea>(signalMessage.memoryarea());

		m_size = signalMessage.size();

		m_validityOffset = signalMessage.validityoffset();
		m_validityBit = signalMessage.validitybit();

		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();

		if (m_preset == true)
		{
			auto prop = propertyByCaption("Type");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("Function");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("ByteOrder");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("Format");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("Size");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("ValidityOffset");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("ValidityBit");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("ValueOffset");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}

			prop = propertyByCaption("ValueBit");
			if (prop != nullptr)
			{
				prop->setExpert(true);
			}
		}

		return true;
	}

	DeviceType DeviceSignal::deviceType() const
	{
		return DeviceSignal::m_deviceType;
	}

	quint32 DeviceSignal::valueToMantExp1616(double value)
	{
		if (value == 0)
			return 0;

		//value = 2;

		double m = 0;
		int p = 1;

		m = frexp (value, &p);

		p+= 30;

		if (abs((int)m) < 0x3fffffff)
		{
			while (abs((int)m) < 0x3fffffff)
			{
				m *= 2;
				p--;
			}

			if ((int)m == -0x40000000)
			{
				m *= 2;
				p--;
			}
		}
		else
		{
			while (abs((int)m) > 0x20000000)
			{
				m /= 2;
				p++;
			}
		}

		if (p < -256 || p > 255)
		{
			return 0;
		}

		quint16 _m16 = (int)m >> 16;
		quint16 _p16 = p;

		quint32 result = (_m16 << 16) | _p16;
		return result;
	}

	E::SignalType DeviceSignal::type() const
	{
		return m_type;
	}

	int DeviceSignal::jsType() const
	{
		return static_cast<int>(type());
	}

	void DeviceSignal::setType(E::SignalType value)
	{
		m_type = value;
	}

	E::SignalFunction DeviceSignal::function() const
	{
		return m_function;
	}

	int DeviceSignal::jsFunction() const
	{
		return static_cast<int>(function());
	}

	void DeviceSignal::setFunction(E::SignalFunction value)
	{
		m_function = value;
	}

	E::ByteOrder DeviceSignal::byteOrder() const
	{
		return m_byteOrder;
	}

	void DeviceSignal::setByteOrder(E::ByteOrder value)
	{
		m_byteOrder = value;
	}

	E::DataFormat DeviceSignal::format() const
	{
		return m_format;
	}

	void DeviceSignal::setFormat(E::DataFormat value)
	{
		m_format = value;
	}

	E::MemoryArea DeviceSignal::memoryArea() const
	{
		return m_memoryArea;
	}

	void DeviceSignal::setMemoryArea(E::MemoryArea value)
	{
		m_memoryArea = value;
	}

	int DeviceSignal::size() const
	{
		return m_size;
	}

	void DeviceSignal::setSize(int value)
	{
		m_size = value;
	}

	int DeviceSignal::validityOffset() const
	{
		return m_validityOffset;
	}

	void DeviceSignal::setValidityOffset(int value)
	{
		m_validityOffset = value;
	}

	int DeviceSignal::validityBit() const
	{
		return m_validityBit;
	}

	void DeviceSignal::setValidityBit(int value)
	{
		m_validityBit = value;
	}

	int DeviceSignal::valueOffset() const
	{
		return m_valueOffset;
	}

	void DeviceSignal::setValueOffset(int value)
	{
		m_valueOffset = value;
	}

	int DeviceSignal::valueBit() const
	{
		return m_valueBit;
	}

	void DeviceSignal::setValueBit(int value)
	{
		m_valueBit = value;
	}

	bool DeviceSignal::isInputSignal() const
	{
		return m_function == E::SignalFunction::Input;
	}

	bool DeviceSignal::isOutputSignal() const
	{
		return m_function == E::SignalFunction::Output;
	}

	bool DeviceSignal::isDiagSignal() const
	{
		return m_function == E::SignalFunction::Diagnostics;
	}

	bool DeviceSignal::isValiditySignal() const
	{
		return m_function == E::SignalFunction::Validity;
	}

	bool DeviceSignal::isAnalogSignal() const
	{
		return	m_type == E::SignalType::Analog;
	}

	bool DeviceSignal::isDiscreteSignal() const
	{
		return	m_type == E::SignalType::Discrete;
	}


	//
	//
	// Workstation
	//
	//
	Workstation::Workstation(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, Workstation::type, Workstation::setType)

		typeProp->setUpdateFromPreset(true);
	}

	Workstation::~Workstation()
	{
	}

	bool Workstation::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::Workstation* workstationMessage = message->mutable_deviceobject()->mutable_workstation();

		workstationMessage->set_type(m_type);

		return true;
	}

	bool Workstation::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_workstation() == false)
		{
			assert(message.deviceobject().has_workstation());
			return false;
		}

		const Proto::Workstation& workstationMessage = message.deviceobject().workstation();

		m_type =  workstationMessage.type();

		return true;
	}

	DeviceType Workstation::deviceType() const
	{
		return m_deviceType;
	}


	int Workstation::type() const
	{
		return m_type;
	}

	void Workstation::setType(int value)
	{
		m_type = value;
	}



	//
	//
	// Software
	//
	//
	Software::Software(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(E::SoftwareType, "Type", true, Software::type, Software::setType);

		typeProp->setUpdateFromPreset(true);
	}

	Software::~Software()
	{
	}

	bool Software::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::Software* softwareMessage = message->mutable_deviceobject()->mutable_software();

		softwareMessage->set_type(static_cast<int>(m_type));

		return true;
	}

	bool Software::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_software() == false)
		{
			assert(message.deviceobject().has_software());
			return false;
		}

		const Proto::Software& softwareMessage = message.deviceobject().software();

		m_type =  static_cast<E::SoftwareType>(softwareMessage.type());

		return true;
	}

	DeviceType Software::deviceType() const
	{
		return m_deviceType;
	}


	E::SoftwareType Software::type() const
	{
		return m_type;
	}

	void Software::setType(E::SoftwareType value)
	{
		m_type = value;
	}


	//
	//
	// EquipmentSet
	//
	//
	EquipmentSet::EquipmentSet(std::shared_ptr<DeviceObject> root)
	{
		set(root);
	}

	void EquipmentSet::set(std::shared_ptr<DeviceObject> root)
	{
		if (root == nullptr)
		{
			assert(root);
			return;
		}

		m_root = root;

		// fill map for fast access
		//
		m_deviceTable.clear();

		m_deviceTable.insert(m_root->equipmentIdTemplate(), m_root);
		addDeviceChildrenToHashTable(m_root);

//		for (auto it : m_deviceTable)
//		{
//			qDebug() << it->equipmentIdTemplate();
//		}

		return;
	}

	DeviceObject* EquipmentSet::deviceObject(const QString& equipmentId)
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it.value().get();
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<DeviceObject> EquipmentSet::deviceObjectSharedPointer(const QString& equipmentId)
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it.value();
		}
		else
		{
			return std::shared_ptr<DeviceObject>();
		}
	}

	DeviceRoot* EquipmentSet::root()
	{
		return dynamic_cast<DeviceRoot*>(m_root.get());
	}

	const DeviceRoot* EquipmentSet::root() const
	{
		return dynamic_cast<const DeviceRoot*>(m_root.get());
	}

	void EquipmentSet::addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent)
	{
		for (int i = 0; i < parent->childrenCount(); i++)
		{
			std::shared_ptr<DeviceObject> child = parent->childSharedPtr(i);
			m_deviceTable.insert(child->equipmentIdTemplate(), child);

			addDeviceChildrenToHashTable(child);
		}

		return;
	}

	void equipmentWalker(DeviceObject* currentDevice, std::function<void (DeviceObject*)> processBeforeChildren, std::function<void (DeviceObject*)> processAfterChildren)
	{
		if (currentDevice == nullptr)
		{
			assert(currentDevice != nullptr);

			QString msg = QString(QObject::tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__);

			qDebug() << msg;
			return;
		}

		if (processBeforeChildren != nullptr)
		{
			processBeforeChildren(currentDevice);
		}

		int childrenCount = currentDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = currentDevice->child(i);

			equipmentWalker(device, processBeforeChildren, processAfterChildren);
		}

		if (processAfterChildren != nullptr)
		{
			processAfterChildren(currentDevice);
		}
	}

	void equipmentWalker(DeviceObject* currentDevice, std::function<void (DeviceObject*)> processBeforeChildren)
	{
		equipmentWalker(currentDevice, processBeforeChildren, nullptr);
	}

	void SerializeEquipmentFromXml(const QString& filePath, std::shared_ptr<Hardware::DeviceRoot>& deviceRoot)
	{
		QXmlStreamReader equipmentReader;
		QFile file(filePath);

		Hardware::DeviceObject* pCurrentDevice = nullptr;

		if (file.open(QIODevice::ReadOnly))
		{
			equipmentReader.setDevice(&file);

			while (!equipmentReader.atEnd())
			{
				QXmlStreamReader::TokenType token = equipmentReader.readNext();

				switch (token)
				{
				case QXmlStreamReader::StartElement:
				{
					const QXmlStreamAttributes& attr = equipmentReader.attributes();
					const QString classNameHash = attr.value("classNameHash").toString();
					if (classNameHash.isEmpty())
					{
						qDebug() << "Attribute classNameHash of DeviceObject not found";
						continue;
					}
					bool ok = false;
					quint32 hash = classNameHash.toUInt(&ok, 16);
					if (!ok)
					{
						qDebug() << QString("Could not interpret hash %s").arg(classNameHash);
						continue;
					}
					std::shared_ptr<Hardware::DeviceObject> pDeviceObject(Hardware::DeviceObjectFactory.Create(hash));
					if (pDeviceObject == nullptr)
					{
						qDebug() << QString("Unknown element %s found").arg(equipmentReader.name().toString());
						continue;
					}

					if (typeid(*pDeviceObject) == typeid(Hardware::DeviceRoot))
					{
						pCurrentDevice = pDeviceObject.get();
						deviceRoot = std::dynamic_pointer_cast<Hardware::DeviceRoot>(pDeviceObject);
						continue;
					}

					if (pCurrentDevice == nullptr)
					{
						qDebug() << "DeviceRoot should be the root xml element";
						return;
					}

					pDeviceObject->setSpecificProperties(attr.value("SpecificProperties").toString());

					for (auto p : pDeviceObject->properties())
					{
						if (p->readOnly() || p->caption() == "SpecificProperties")
						{
							continue;
						}
						QVariant tmp = QVariant::fromValue(attr.value(p->caption()).toString());
						bool result = tmp.convert(p->value().userType());
						if (result == false)
						{
							assert(tmp.canConvert(p->value().userType()));
						}
						else
						{
							p->setValue(tmp);
						}
					}

					pCurrentDevice->addChild(pDeviceObject);
					pCurrentDevice = pDeviceObject.get();
					break;
				}
				case QXmlStreamReader::EndElement:
					if (pCurrentDevice != nullptr && typeid(*pCurrentDevice) != typeid(Hardware::DeviceRoot))
					{
						if (pCurrentDevice->parent() == nullptr)
						{
							assert(false);
							break;
						}
						pCurrentDevice = pCurrentDevice->parent();
					}
					else
					{
						return;	// Closing root element, nothing to read left
					}
					break;
				default:
					continue;
				}
			}
			if (equipmentReader.hasError())
			{
				qDebug() << "Parse equipment.xml error";
			}
		}
	}

}

