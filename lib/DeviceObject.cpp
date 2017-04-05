#include "../lib/DeviceObject.h"
#include "../lib/ProtoSerialization.h"
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
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

namespace Hardware
{
	const std::array<QString, 9> DeviceObjectExtensions =
		{
			".hrt",			// Root
			".hsm",			// System
			".hrk",			// Rack
			".hcs",			// Chassis
			".hmd",			// Module
			".hws",			// Workstation
			".hsw",			// Software
			".hcr",			// Controller
			".hds",			// Signal
		};

	extern const std::array<QString, 9> DeviceTypeNames =
		{
			"Root",			// Root
			"System",		// System
			"Rack",			// Rack
			"Chassis",		// Chassis
			"Module",		// Module
			"Workstation",	// Workstation
			"Software",		// Software
			"Controller",	// Controller
			"Signal",		// Signal
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
	// PropertyNames
	//
	//
	const QString PropertyNames::fileId = "FileID";
	const QString PropertyNames::uuid = "Uuid";
	const QString PropertyNames::equipmentIdTemplate = "EquipmentIDTemplate";
	const QString PropertyNames::equipmentId = "EquipmentID";
	const QString PropertyNames::caption = "Caption";
	const QString PropertyNames::childRestriction = "ChildRestriction";
	const QString PropertyNames::place = "Place";
	const QString PropertyNames::specificProperties = "SpecificProperties";
	const QString PropertyNames::preset = "Preset";
	const QString PropertyNames::presetRoot = "PresetRoot";
	const QString PropertyNames::presetName = "PresetName";
	const QString PropertyNames::presetObjectUuid = "PresetObjectUuid";

	const QString PropertyNames::lmDescriptionFile = "LmDescriptionFile";
	const QString PropertyNames::lmNumber = "LMNumber";
	const QString PropertyNames::lmSubsystemChannel = "SubsystemChannel";
	const QString PropertyNames::lmSubsystemID = "SubsystemID";

	const QString PropertyNames::type = "Type";
	const QString PropertyNames::function = "Function";
	const QString PropertyNames::byteOrder = "ByteOrder";
	const QString PropertyNames::format = "Format";
	const QString PropertyNames::memoryArea = "MemoryArea";
	const QString PropertyNames::size = "Size";

	const QString PropertyNames::validityOffset = "ValidityOffset";
	const QString PropertyNames::validityBit = "ValidityBit";
	const QString PropertyNames::valueOffset = "ValueOffset";
	const QString PropertyNames::valueBit = "ValueBit";

	const QString PropertyNames::appSignalLowAdc = "LowAdc";
	const QString PropertyNames::appSignalHighAdc = "HighAdc";
	const QString PropertyNames::appSignalLowEngUnits = "LowEngUnits";
	const QString PropertyNames::appSignalHighEngUnits = "HighEngUnits";
	const QString PropertyNames::appSignalDataFormat = "DataFormat";

	const QString PropertyNames::categoryAnalogAppSignal = "AnalogAppSignal";

	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(bool preset /*= false*/) :
		m_preset(preset)
	{
		auto fileIdProp = ADD_PROPERTY_GETTER(int, PropertyNames::fileId, true, DeviceObject::fileId);
		fileIdProp->setExpert(true);

		auto uuidProp = ADD_PROPERTY_GETTER(QUuid, PropertyNames::uuid, true, DeviceObject::uuid);
		uuidProp->setExpert(true);

		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::equipmentIdTemplate, true, DeviceObject::equipmentIdTemplate, DeviceObject::setEquipmentIdTemplate);

		auto equipmentIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::equipmentId, true, DeviceObject::equipmentId);
		equipmentIdProp->setReadOnly(true);

		auto captionProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::caption, true, DeviceObject::caption, DeviceObject::setCaption);

		auto childRestrProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::childRestriction, true, DeviceObject::childRestriction, DeviceObject::setChildRestriction);
		childRestrProp->setExpert(true);

		ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::place, true, DeviceObject::place, DeviceObject::setPlace);

		auto specificProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::specificProperties, true, DeviceObject::specificProperties, DeviceObject::setSpecificProperties);
		specificProp->setExpert(true);

		auto presetProp = ADD_PROPERTY_GETTER(bool, PropertyNames::preset, true, DeviceObject::preset);
		presetProp->setExpert(true);

		auto presetRootProp = ADD_PROPERTY_GETTER(bool, PropertyNames::presetRoot, true, DeviceObject::presetRoot);
		presetRootProp->setExpert(true);

		if (preset == true)
		{
			auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetName, true, DeviceObject::presetName, DeviceObject::setPresetName);
			presetNameProp->setExpert(true);
		}

		auto presetObjectUuidProp = ADD_PROPERTY_GETTER(QUuid, PropertyNames::presetObjectUuid, true, DeviceObject::presetObjectUuid);
		presetObjectUuidProp->setExpert(true);

		captionProp->setUpdateFromPreset(true);
		childRestrProp->setUpdateFromPreset(true);
		specificProp->setUpdateFromPreset(true);
	}

	DeviceObject::~DeviceObject()
	{
	}

	std::shared_ptr<DeviceObject> DeviceObject::fromDbFile(const DbFile& file)
	{
		std::shared_ptr<DeviceObject> object = DeviceObject::Create(file.data());

		if (object == nullptr)
		{
			assert(object != nullptr);
			return nullptr;
		}

		object->setFileInfo(file);
		return object;
	}

	std::vector<std::shared_ptr<DeviceObject>> DeviceObject::fromDbFiles(std::vector<std::shared_ptr<DbFile>> files)
	{
		std::vector<std::shared_ptr<DeviceObject>> result;
		result.reserve(files.size());

		for (const std::shared_ptr<DbFile>& f : files)
		{
			std::shared_ptr<DeviceObject> object = fromDbFile(*f.get());
			result.push_back(object);
		}

		return result;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message) const
	{
		bool ok = SaveData(message, false);
		return ok;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);

		Proto::DeviceObject* mutableDeviceObject = message->mutable_deviceobject();

		Proto::Write(mutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(mutableDeviceObject->mutable_equipmentid(), m_equipmentId);
		Proto::Write(mutableDeviceObject->mutable_caption(), m_caption);

		mutableDeviceObject->set_place(m_place);
		mutableDeviceObject->set_childcounthint(static_cast<int>(m_children.size()));

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
				Proto::saveProperty(protoProp, p);
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

		// Save children if it is necessary (can be in serialization for the clipboard)
		//
		if (saveTree == true)
		{
			for (std::shared_ptr<DeviceObject> child : m_children)
			{
				::Proto::Envelope* childMessage = mutableDeviceObject->add_children();
				assert(childMessage);

				child->SaveData(childMessage, saveTree);
			}
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

		size_t childCountHint = deviceobject.childcounthint();
		if (childCountHint != 0)
		{
			m_children.reserve(childCountHint);
		}

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

				bool loadOk = Proto::loadProperty(p, property);

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
				auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetName, true, DeviceObject::presetName, DeviceObject::setPresetName);
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

		// Load children if all tree was saved
		//
//		m_children.clear();
//		m_children.reserve(deviceobject.children_size());

//		for (int childIndex = 0; childIndex < deviceobject.children_size(); childIndex++)
//		{
//			const ::Proto::Envelope& childMessage = deviceobject.children(childIndex);

//			std::shared_ptr<DeviceObject> child(DeviceObject::Create(childMessage));

//			if (child == nullptr)
//			{
//				assert(child);
//				continue;
//			}

//			m_children.push_back(child);
//		}

		if (this->isRack() == true && deviceobject.children_size() > 0)
		{
			// Multithread reading
			//
			std::vector<QFuture<std::shared_ptr<DeviceObject>>> threadFuncs;
			threadFuncs.reserve(deviceobject.children_size());

			for (int childIndex = 0; childIndex < deviceobject.children_size(); childIndex++)
			{
				const ::Proto::Envelope& childMessage = deviceobject.children(childIndex);

				QFuture<std::shared_ptr<DeviceObject>> f = QtConcurrent::run(DeviceObject::CreateObject, childMessage);

				threadFuncs.push_back(f);
			}

			for (QFuture<std::shared_ptr<DeviceObject>>& f : threadFuncs)
			{
				std::shared_ptr<DeviceObject> child = f.result();

				if (child == nullptr)
				{
					assert(child);
					continue;
				}

				m_children.push_back(child);
			}
		}
		else
		{
			for (int childIndex = 0; childIndex < deviceobject.children_size(); childIndex++)
			{
				const ::Proto::Envelope& childMessage = deviceobject.children(childIndex);

				std::shared_ptr<DeviceObject> child(DeviceObject::Create(childMessage));

				if (child == nullptr)
				{
					assert(child);
					continue;
				}

				m_children.push_back(child);
			}
		}

		return true;
	}

	std::shared_ptr<DeviceObject> DeviceObject::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<DeviceObject> deviceObject = DeviceObjectFactory.Create(classNameHash);

		if (deviceObject.get() == nullptr)
		{
			assert(deviceObject);
			return deviceObject;
		}

		deviceObject->LoadData(message);

		return deviceObject;
	}

	bool DeviceObject::SaveObjectTree(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		try
		{
			bool ok = this->SaveData(message, true);
			return ok;
		}
		catch (...)
		{
			assert(false);
			return false;
		}
	}

	void DeviceObject::expandEquipmentId()
	{
		// The same procedure is done in expandEquipmentId, keep it in mind if add any new macroses
		//

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
				return p->specific() == false;
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

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible
		3;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true

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

			if (ok == false)
			{
				qDebug() << Q_FUNC_INFO << " SpecificProperties: failed to parse specific prop version filed: " << r;
				continue;
			}

			switch (version)
			{
			case 1:
				parseSpecificPropertiesStructV1(columns);
				break;
			case 2:
				parseSpecificPropertiesStructV2(columns);
				break;
			case 3:
				parseSpecificPropertiesStructV3(columns);
				break;
			default:
				assert(false);
				qDebug() << "Object " << this->equipmentId() << " has spec prop with unsuported version: " << r;
			}
		}

		// Set to parsed properties old value
		//
		std::vector<std::shared_ptr<Property>> newProperties = properties();

		for (std::shared_ptr<Property> p : oldProperties)
		{
			auto it = std::find_if(newProperties.begin(), newProperties.end(),
				[p](std::shared_ptr<Property> np)
				{
					  return np->caption() == p->caption();
				}
				);

			if (it != newProperties.end() &&
				(*it)->value().type() == p->value().type() &&
				p != (*it))
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


	void DeviceObject::parseSpecificPropertiesStructV1(const QStringList& columns)
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);
			newProperty->setValue(QVariant::fromValue(E::Channel::A));

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setValue(QVariant(defaultValue.toString()));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);

			return;
		}
	}

	void DeviceObject::parseSpecificPropertiesStructV2(const QStringList &columns)
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);
			newProperty->setValue(QVariant::fromValue(E::Channel::A));

			newProperty->setSpecific(true);
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setValue(QVariant(defaultValue.toString()));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);

			return;
		}
	}

	void DeviceObject::parseSpecificPropertiesStructV3(const QStringList &columns)
	{
		if (columns.count() != 12)
		{
			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 3!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible";
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
		QString strVisible(columns[11]);

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

		bool visible = false;
		if (strVisible.toUpper() == "TRUE")
		{
			visible = true;
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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setLimits(QVariant(minInt), QVariant(maxInt));
			newProperty->setValue(QVariant(defaultInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setLimits(QVariant(minUInt), QVariant(maxUInt));
			newProperty->setValue(QVariant(defaultUInt));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

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
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setLimits(QVariant(minDouble), QVariant(maxDouble));
			newProperty->setValue(QVariant(defaultDouble));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

			return;
		}

		if (type == "bool")
		{
			// Default Value
			//
			bool defaultBool = defaultValue.compare("true", Qt::CaseInsensitive) == 0;

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setValue(QVariant(defaultBool));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

			return;
		}

		if (type == "E::Channel")
		{
			// Default Value
			//
			QString defaultString = defaultValue.toString();

			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty(name, category, true);
			newProperty->setValue(QVariant::fromValue(E::Channel::A));

			newProperty->setSpecific(true);
			newProperty->setValue(defaultString.toStdString().c_str());
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

			return;
		}

		if (type == "string")
		{
			// Add property with default value, if present old value, it will be set later
			//
			auto newProperty = addProperty(name, category, true);

			newProperty->setSpecific(true);
			newProperty->setValue(QVariant(defaultValue.toString()));
			newProperty->setReadOnly(false);
			newProperty->setPrecision(precision);
			newProperty->setUpdateFromPreset(updateFromPreset);
			newProperty->setExpert(expert);
			newProperty->setDescription(strDescription);
			newProperty->setVisible(visible);

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

	bool DeviceObject::jsPropertyBool(QString name) const
	{
		const std::shared_ptr<Property> p = propertyByCaption(name);
		if (p == nullptr)
		{
			assert(false);
			return false;
		}

		QVariant v = p->value();
		if (v.isValid() == false)
		{
			assert(v.isValid());
			return false;
		}

		return v.toBool();
	}

	QString DeviceObject::jsPropertyString(QString name) const
	{
		const std::shared_ptr<Property> p = propertyByCaption(name);
		if (p == nullptr)
		{
			assert(false);
			return QString();
		}

		QVariant v = p->value();
		if (v.isValid() == false)
		{
			assert(v.isValid());
			return QString();
		}

		return v.toString();
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

	const Hardware::DeviceRoot* DeviceObject::toRoot() const
	{
		const Hardware::DeviceRoot* r = dynamic_cast<const Hardware::DeviceRoot*>(this);
		assert(r != nullptr);

		return r;
	}

	Hardware::DeviceRoot* DeviceObject::toRoot()
	{
		Hardware::DeviceRoot* r = dynamic_cast<Hardware::DeviceRoot*>(this);
		assert(r != nullptr);

		return r;
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

			if (deviceObject == nullptr)
			{
				break;
			}

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

			if (deviceObject == nullptr)
			{
				break;
			}

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

			if (deviceObject == nullptr)
			{
				break;
			}

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

			if (deviceObject == nullptr)
			{
				break;
			}

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

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isSystem())
			{
				return deviceObject->toSystem();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceRoot* DeviceObject::getParentRoot() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isRoot())
			{
				return deviceObject->toRoot();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	QString DeviceObject::fileExtension() const
	{
		size_t index = static_cast<size_t>(deviceType());
		assert(index < sizeof(Hardware::DeviceObjectExtensions) / sizeof(Hardware::DeviceObjectExtensions[0]));

		QString result = Hardware::DeviceObjectExtensions[index];
		return result;
	}

	QString DeviceObject::fileExtension(DeviceType device)
	{
		QString result = Hardware::DeviceObjectExtensions[static_cast<int>(device)];
		return result;
	}

	void DeviceObject::setExpertToProperty(const QString& property, bool expert)
	{
		std::shared_ptr<Property> prop = propertyByCaption(property);

		if (prop != nullptr)
		{
			prop->setExpert(expert);
		}
		else
		{
			assert(prop);
		}

		return;
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

	bool DeviceObject::canAddChild(DeviceObject* child) const
	{
		if (child->deviceType() == DeviceType::Software &&
			deviceType() != DeviceType::Workstation &&
			deviceType() != DeviceType::Root)
		{
			return false;
		}

		if (deviceType() >= child->deviceType())
		{
			return false;
		}

		if (child->deviceType() == DeviceType::Workstation &&
			deviceType() > DeviceType::Chassis)
		{
			return false;
		}

		return true;
	}

	void DeviceObject::addChild(std::shared_ptr<DeviceObject> child)
	{
		if (child.get() == nullptr)
		{
			assert(child);
			return;
		}

		if (canAddChild(child.get()) == false)
		{
			assert(canAddChild(child.get()) == true);
			return;
		}

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

	void DeviceObject::sortByUser(Qt::SortOrder order, const std::map<int, QString>& users)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order, &users](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				auto uit1 = users.find(ref1->m_fileInfo.userId());
				QString u1 =  uit1 == users.end() ? "Unknows" : uit1->second;

				auto uit2 = users.find(ref2->m_fileInfo.userId());
				QString u2 =  uit2 == users.end() ? "Unknows" : uit2->second;

				if (u1 < u2)
				{
					return true;
				}
				else
				{
					if (u1 == u2)
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
			m_equipmentId.replace(QRegExp("[^a-zA-Z0-9#$_()]"), "#");
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

		// The same procedure is done in expandEquipmentId, keep it in mind if add any new macroses
		//
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

			m_caption.replace(QChar::LineFeed, " ");
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
		QString captionEscaped = caption();
		captionEscaped.replace("'", "''");
		captionEscaped.replace("\"", "\\\"");

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
			.arg(captionEscaped)
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
		qDebug() << "DeviceRoot::DeviceRoot ThreadId: " << QThread::currentThreadId();
	}

	DeviceRoot::~DeviceRoot()
	{
		qDebug() << "DeviceRoot::~DeviceRoot ThreadId: " << QThread::currentThreadId();
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

	bool DeviceSystem::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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

	bool DeviceRack::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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
		typeProp->setExpert(true);
	}

	DeviceChassis::~DeviceChassis()
	{
	}

	bool DeviceChassis::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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

	std::shared_ptr<DeviceModule> DeviceChassis::getLogicModuleSharedPointer()
	{
		int count = childrenCount();

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<DeviceObject> device = childSharedPtr(i);

			if (device == nullptr)
			{
				assert(false);
				continue;
			}

			if (device->isModule())
			{
				std::shared_ptr<DeviceModule> module = std::dynamic_pointer_cast<DeviceModule>(device);

				if (module == nullptr)
				{
					assert(false);
					continue;
				}

				if (module->isLogicModule())
				{
					return module;
				}
			}
		}

		return nullptr;
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

		auto configScriptProp = ADD_PROPERTY_GETTER_SETTER(QString, "ConfigurationScript", true, DeviceModule::configurationScript, DeviceModule::setConfigurationScript)
		configScriptProp->setExpert(true);

		auto rawDataDescrProp = ADD_PROPERTY_GETTER_SETTER(QString, "RawDataDescription", true, DeviceModule::rawDataDescription, DeviceModule::setRawDataDescription)
		rawDataDescrProp->setExpert(true);

		familyTypeProp->setUpdateFromPreset(true);
		moduleVersionProp->setUpdateFromPreset(true);
		configScriptProp->setUpdateFromPreset(true);
		rawDataDescrProp->setUpdateFromPreset(true);
	}

	DeviceModule::~DeviceModule()
	{
	}

	bool DeviceModule::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceModule* moduleMessage = message->mutable_deviceobject()->mutable_module();

		moduleMessage->set_moduletype(static_cast<int>(m_type));
		moduleMessage->set_configurationscript(m_configurationScript.toStdString());
		moduleMessage->set_rawdatadescription(m_rawDataDescription.toStdString());

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

		const Proto::DeviceModule& modulemessage = message.deviceobject().module();

		if (modulemessage.has_moduletype())
		{
			m_type =  static_cast<decltype(m_type)>(modulemessage.moduletype());
		}
		else
		{
			m_type =  static_cast<decltype(m_type)>(modulemessage.typeobsolete());

			if ((m_type & 0xff00) == 0x0100
				|| (m_type & 0xff00) == 0x0200
				|| (m_type & 0xff00) == 0x0300
				|| (m_type & 0xff00) == 0x0400
				|| (m_type & 0xff00) == 0x0500
				|| (m_type & 0xff00) == 0x0600
				|| (m_type & 0xff00) == 0x0700
				)
			{
				m_type |= 0x1000;	// Module family 01..07 changed to 11..17, this is for compatibitity
			}
		}

		m_configurationScript = QString::fromStdString(modulemessage.configurationscript());
		m_rawDataDescription = QString::fromStdString(modulemessage.rawdatadescription());

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

	int DeviceModule::jsModuleFamily() const
	{
		return static_cast<int>(m_type & 0xFF00);
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

	QString DeviceModule::configurationScript() const
	{
		return m_configurationScript;
	}

	void DeviceModule::setConfigurationScript(const QString& value)
	{
		m_configurationScript = value;
	}

	QString DeviceModule::rawDataDescription() const
	{
		return m_rawDataDescription;
	}

	void DeviceModule::setRawDataDescription(const QString& value)
	{
		m_rawDataDescription = value;
	}

	bool DeviceModule::hasRawData() const
	{
		return m_rawDataDescription.isEmpty() != true;
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
				family == FamilyType::AIFM ||
				family == FamilyType::MPS17;
				//family == FamilyType::BVK4;
	}

	bool DeviceModule::isOutputModule() const
	{
		FamilyType family = moduleFamily();

		return	family == FamilyType::AOM ||
				family == FamilyType::DOM;
	}

	bool DeviceModule::isLogicModule() const
	{
		return moduleFamily() == FamilyType::LM;
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

	bool DeviceController::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(E::SignalType, PropertyNames::type, true, DeviceSignal::type, DeviceSignal::setType)
		auto functionProp = ADD_PROPERTY_GETTER_SETTER(E::SignalFunction, PropertyNames::function, true, DeviceSignal::function, DeviceSignal::setFunction)
		auto byteOrderProp = ADD_PROPERTY_GETTER_SETTER(E::ByteOrder, PropertyNames::byteOrder, true, DeviceSignal::byteOrder, DeviceSignal::setByteOrder)
		auto formatProp = ADD_PROPERTY_GETTER_SETTER(E::DataFormat, PropertyNames::format, true, DeviceSignal::format, DeviceSignal::setFormat)
		auto memoryAreaProp = ADD_PROPERTY_GETTER_SETTER(E::MemoryArea, PropertyNames::memoryArea, true, DeviceSignal::memoryArea, DeviceSignal::setMemoryArea)

		auto sizeProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::size, true, DeviceSignal::size, DeviceSignal::setSize)
		auto validityOffsetProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::validityOffset, true, DeviceSignal::validityOffset, DeviceSignal::setValidityOffset)
		auto valididtyBitProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::validityBit, true, DeviceSignal::validityBit, DeviceSignal::setValidityBit)

		auto valueOffsetProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::valueOffset, true, DeviceSignal::valueOffset, DeviceSignal::setValueOffset)
		auto valueBitProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::valueBit, true, DeviceSignal::valueBit, DeviceSignal::setValueBit)

		auto appSignalLowAdcProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::appSignalLowAdc, true, DeviceSignal::appSignalLowAdc, DeviceSignal::setAppSignalLowAdc)
		auto appSignalHighAdcProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::appSignalHighAdc, true, DeviceSignal::appSignalHighAdc, DeviceSignal::setAppSignalHighAdc)

		auto appSignalLowEngUnitsProp = ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::appSignalLowEngUnits, true, DeviceSignal::appSignalLowEngUnits, DeviceSignal::setAppSignalLowEngUnits)
		auto appSignalHighEngUnitsProp = ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::appSignalHighEngUnits, true, DeviceSignal::appSignalHighEngUnits, DeviceSignal::setAppSignalHighEngUnits)

		auto appSignalDataFormatProp = ADD_PROPERTY_GETTER_SETTER(E::AnalogAppSignalFormat, PropertyNames::appSignalDataFormat, true, DeviceSignal::appSignalDataFormat, DeviceSignal::setAppSignalDataFormat)

		appSignalLowAdcProp->setCategory(PropertyNames::categoryAnalogAppSignal);
		appSignalHighAdcProp->setCategory(PropertyNames::categoryAnalogAppSignal);
		appSignalLowEngUnitsProp->setCategory(PropertyNames::categoryAnalogAppSignal);
		appSignalHighEngUnitsProp->setCategory(PropertyNames::categoryAnalogAppSignal);
		appSignalDataFormatProp->setCategory(PropertyNames::categoryAnalogAppSignal);

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

		appSignalLowAdcProp->setUpdateFromPreset(true);
		appSignalLowAdcProp->setExpert(preset);

		appSignalHighAdcProp->setUpdateFromPreset(true);
		appSignalHighAdcProp->setExpert(preset);

		appSignalLowEngUnitsProp->setUpdateFromPreset(true);
		appSignalLowEngUnitsProp->setExpert(preset);

		appSignalHighEngUnitsProp->setUpdateFromPreset(true);
		appSignalHighEngUnitsProp->setExpert(preset);

		appSignalDataFormatProp->setUpdateFromPreset(true);
		appSignalDataFormatProp->setExpert(preset);

		// Show/Hide analog signal properties
		//
		setType(type());

		return;
	}

	DeviceSignal::~DeviceSignal()
	{
	}

	bool DeviceSignal::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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

		signalMessage->set_appsignallowadc(m_appSignalLowAdc);
		signalMessage->set_appsignalhighadc(m_appSignalHighAdc);

		signalMessage->set_appsignallowengunits(m_appSignalLowEngUnits);
		signalMessage->set_appsignalhighengunits(m_appSignalHighEngUnits);

		signalMessage->set_appsignaldataformat(static_cast<int>(m_appSignalDataFormat));

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
					setType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::DiagAnalog:
					setType(E::SignalType::Analog);
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::InputDiscrete:
					setType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::InputAnalog:
					setType(E::SignalType::Analog);
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::OutputDiscrete:
					setType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Output;
					break;
				case Obsolete::SignalType::OutputAnalog:
					setType(E::SignalType::Analog);
					m_function = E::SignalFunction::Output;
					break;
				default:
					assert(false);
			}
		}
		else
		{
			setType(static_cast<E::SignalType>(signalMessage.type()));				// Show hide some props
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

		m_appSignalLowAdc = signalMessage.appsignallowadc();
		m_appSignalHighAdc = signalMessage.appsignalhighadc();

		m_appSignalLowEngUnits = signalMessage.appsignallowengunits();
		m_appSignalHighEngUnits =  signalMessage.appsignalhighengunits();

		m_appSignalDataFormat = static_cast<E::AnalogAppSignalFormat>(signalMessage.appsignaldataformat());

		if (m_preset == true)
		{
			setExpertToProperty(PropertyNames::type, true);
			setExpertToProperty(PropertyNames::function, true);
			setExpertToProperty(PropertyNames::byteOrder, true);
			setExpertToProperty(PropertyNames::format, true);
			setExpertToProperty(PropertyNames::memoryArea, true);
			setExpertToProperty(PropertyNames::size, true);
			setExpertToProperty(PropertyNames::validityOffset, true);
			setExpertToProperty(PropertyNames::validityBit, true);
			setExpertToProperty(PropertyNames::valueOffset, true);
			setExpertToProperty(PropertyNames::valueBit, true);
			setExpertToProperty(PropertyNames::appSignalLowAdc, true);
			setExpertToProperty(PropertyNames::appSignalHighAdc, true);
			setExpertToProperty(PropertyNames::appSignalLowEngUnits, true);
			setExpertToProperty(PropertyNames::appSignalHighEngUnits, true);
			setExpertToProperty(PropertyNames::appSignalDataFormat, true);
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

		if (function() == E::SignalFunction::Input ||
			function() == E::SignalFunction::Output)
		{
			bool appSignalProps = false;

			switch (m_type)
			{
			case E::SignalType::Analog:
				appSignalProps = true;
				break;
			case E::SignalType::Discrete:
				appSignalProps = false;
				break;
			default:
				assert(false);
			}

			auto p1 = propertyByCaption(PropertyNames::appSignalLowAdc);
			auto p2 = propertyByCaption(PropertyNames::appSignalHighAdc);
			auto p3 = propertyByCaption(PropertyNames::appSignalLowEngUnits);
			auto p4 = propertyByCaption(PropertyNames::appSignalHighEngUnits);
			auto p5 = propertyByCaption(PropertyNames::appSignalDataFormat);

			assert(p1);
			assert(p2);
			assert(p3);
			assert(p4);
			assert(p5);

			p1->setVisible(appSignalProps);
			p2->setVisible(appSignalProps);
			p3->setVisible(appSignalProps);
			p4->setVisible(appSignalProps);
			p5->setVisible(appSignalProps);
		}
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
		return m_type == E::SignalType::Analog;
	}

	bool DeviceSignal::isDiscreteSignal() const
	{
		return m_type == E::SignalType::Discrete;
	}

	int DeviceSignal::appSignalLowAdc() const
	{
		return m_appSignalLowAdc;
	}

	void DeviceSignal::setAppSignalLowAdc(int value)
	{
		m_appSignalLowAdc = value;
	}

	int DeviceSignal::appSignalHighAdc() const
	{
		return m_appSignalHighAdc;
	}

	void DeviceSignal::setAppSignalHighAdc(int value)
	{
		m_appSignalHighAdc = value;
	}

	double DeviceSignal::appSignalLowEngUnits() const
	{
		return m_appSignalLowEngUnits;
	}

	void DeviceSignal::setAppSignalLowEngUnits(double value)
	{
		m_appSignalLowEngUnits = value;
	}

	double DeviceSignal::appSignalHighEngUnits() const
	{
		return m_appSignalHighEngUnits;
	}

	void DeviceSignal::setAppSignalHighEngUnits(double value)
	{
		m_appSignalHighEngUnits = value;
	}

	E::AnalogAppSignalFormat DeviceSignal::appSignalDataFormat() const
	{
		return m_appSignalDataFormat;
	}

	void DeviceSignal::setAppSignalDataFormat(E::AnalogAppSignalFormat value)
	{
		m_appSignalDataFormat = value;
	}

	//
	//
	// Workstation
	//
	//
	Workstation::Workstation(bool preset /*= false*/) :
		DeviceObject(preset)
	{
		//auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, Workstation::type, Workstation::setType)
		//typeProp->setUpdateFromPreset(true);
	}

	Workstation::~Workstation()
	{
	}

	bool Workstation::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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

	bool Software::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
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

	EquipmentSet::~EquipmentSet()
	{
		// Release m_root in separate thrtead, if it is posiible
		//
		m_deviceTable.clear();	// Clear it first because it also holds m_root

		if (m_root.use_count() == 1)
		{
			std::shared_ptr<Hardware::DeviceObject> equipmentSharedPointer = m_root;
			m_root.reset();

			QtConcurrent::run(
				[](std::shared_ptr<Hardware::DeviceObject> deviceObject)
				{
					deviceObject.reset();
				},
				equipmentSharedPointer);
		}

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
