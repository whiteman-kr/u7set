#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"
#include <QDynamicPropertyChangeEvent>
#include <QJSEngine>
#include <QQmlEngine>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QMetaProperty>
#include <QXmlStreamReader>


namespace Hardware
{
	const wchar_t* DeviceObjectExtensions[] =
		{
			L".hrt",		// Root
			L".hsm",		// System
			L".hrk",		// Rack
			L".hcs",		// Chassis
			L".hmd",		// Module
			L".hcr",		// Controller
			L".hws",		// Workstation
			L".hsw",		// Software
			L".hds",		// Diagnostics Signal
		};

	Factory<Hardware::DeviceObject> DeviceObjectFactory;

	void Init()
	{
		qDebug() << "Hardware::Init";

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

		QMetaType::registerConverter<QString, Hardware::DeviceModule::FamilyType>([] (QString str){ return Hardware::DeviceModule::FamilyType(str.toInt()); });
		QMetaType::registerConverter<QString, Hardware::DeviceSignal::SignalType>([] (QString str){ return Hardware::DeviceSignal::SignalType(str.toInt()); });
		QMetaType::registerConverter<QString, Hardware::DeviceSignal::SignalFunction>([] (QString str){ return Hardware::DeviceSignal::SignalFunction(str.toInt()); });
		QMetaType::registerConverter<QString, Hardware::DeviceSignal::ByteOrder>([] (QString str){ return Hardware::DeviceSignal::ByteOrder(str.toInt()); });
		QMetaType::registerConverter<QString, Hardware::DeviceSignal::DataFormat>([] (QString str){ return Hardware::DeviceSignal::DataFormat(str.toInt()); });

		QMetaType::registerConverter<Hardware::DeviceModule::FamilyType, QString>([] (Hardware::DeviceModule::FamilyType type){ return QString::number(int(type)); });
		QMetaType::registerConverter<Hardware::DeviceSignal::SignalType, QString>([] (Hardware::DeviceSignal::SignalType type){ return QString::number(int(type)); });
		QMetaType::registerConverter<Hardware::DeviceSignal::SignalFunction, QString>([] (Hardware::DeviceSignal::SignalFunction type){ return QString::number(int(type)); });
		QMetaType::registerConverter<Hardware::DeviceSignal::ByteOrder, QString>([] (Hardware::DeviceSignal::ByteOrder type){ return QString::number(int(type)); });
		QMetaType::registerConverter<Hardware::DeviceSignal::DataFormat, QString>([] (Hardware::DeviceSignal::DataFormat type){ return QString::number(int(type)); });

		QMetaType::registerConverter<int, Hardware::DeviceModule::FamilyType>(IntToEnum<Hardware::DeviceModule::FamilyType>);
		QMetaType::registerConverter<int, Hardware::DeviceSignal::SignalType>(IntToEnum<Hardware::DeviceSignal::SignalType>);
		QMetaType::registerConverter<int, Hardware::DeviceSignal::SignalFunction>(IntToEnum<Hardware::DeviceSignal::SignalFunction>);
		QMetaType::registerConverter<int, Hardware::DeviceSignal::ByteOrder>(IntToEnum<Hardware::DeviceSignal::ByteOrder>);
		QMetaType::registerConverter<int, Hardware::DeviceSignal::DataFormat>(IntToEnum<Hardware::DeviceSignal::DataFormat>);
	}

	void Shutdwon()
	{
		qDebug() << "Hardware::Shutdown";
	}

	//
	//
	// DynamicProperty
	//
	//
	DynamicProperty::DynamicProperty()
	{
	}

	DynamicProperty::DynamicProperty(const QString& name, const QVariant& min, const QVariant& max, const QVariant& defaultVal, const QVariant& value) :
		m_name(name),
		m_c_str_name(name.toStdString().c_str()),
		m_min(min),
		m_max(max),
		m_default(defaultVal),
		m_value(value)
	{
	}

	void DynamicProperty::saveValue(::Proto::Property* protoProperty) const
	{
		assert(protoProperty);

		protoProperty->set_name(m_name.toStdString());

		QString value;

		switch (m_value.type()) {
			case QVariant::Bool:
				value = m_value.toBool() ? "t" : "f";
				break;
			case QVariant::Int:
				value.setNum(m_value.toInt());
				break;
			case QVariant::UInt:
				value.setNum(m_value.toUInt());
				break;
			case QVariant::String:
				value = m_value.toString();
				break;
			case QVariant::Double:
				value.setNum(m_value.toDouble());
				break;
			default:
				assert(false);
		}

		protoProperty->set_value(value.toStdString());
		return;
	}

	bool DynamicProperty::loadValue(const ::Proto::Property& protoProperty)
	{
		if (QString(protoProperty.name().c_str()) != m_name)
		{
			assert(QString(protoProperty.name().c_str()) == m_name);
			return false;
		}

		bool ok = false;
		QString sv(protoProperty.value().c_str());

		switch (m_value.type()) {
			case QVariant::Bool:
				{
					m_value = sv == "t" ? true : false;
					ok = true;
				}
				break;
			case QVariant::Int:
				{
					qint32 i = sv.toInt(&ok);
					setValue(QVariant(i));
				}
				break;
			case QVariant::UInt:
				{
					quint32 i = sv.toUInt(&ok);
					setValue(QVariant(i));
				}
				break;
			case QVariant::String:
				{
					m_value = sv;
					ok = true;
				}
				break;
			case QVariant::Double:
				{
					double i = sv.toDouble(&ok);
					setValue(QVariant(i));
				}
				break;
			default:
				assert(false);
		}

		return ok;
	}

	QString DynamicProperty::name() const
	{
		return m_name;
	}

	const char* DynamicProperty::name_c_str() const
	{
		return m_c_str_name.constData();
	}

	void DynamicProperty::setName(const QString& value)
	{
		m_name = value;
		m_c_str_name = QByteArray(value.toStdString().c_str());
	}

	QVariant DynamicProperty::min() const
	{
		return m_min;
	}

	QVariant DynamicProperty::max() const
	{
		return m_max;
	}

	QVariant DynamicProperty::defaultValue() const
	{
		return m_default;
	}

	QVariant DynamicProperty::value() const
	{
		return m_value;
	}

	void DynamicProperty::setValue(QVariant v)
	{
		assert(v.type() == m_default.type());
		assert(v.type() == m_min.type());
		assert(v.type() == m_max.type());

		if (v.type() == QVariant::Int ||
			v.type() == QVariant::UInt ||
			v.type() == QVariant::Double)
		{
				if (v < m_min)
				{
					v = m_min;
				}

				if (v > m_max)
				{
					v = m_max;
				}
		}

		m_value = v;
	}

	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(bool preset /*= false*/) :
		m_preset(preset)
	{
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

		Proto::DeviceObject* pMutableDeviceObject = message->mutable_deviceobject();

		Proto::Write(pMutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(pMutableDeviceObject->mutable_strid(), m_strId);
		Proto::Write(pMutableDeviceObject->mutable_caption(), m_caption);

		pMutableDeviceObject->set_place(m_place);

		if (m_childRestriction.isEmpty() == false)
		{
			Proto::Write(pMutableDeviceObject->mutable_childrestriction(), m_childRestriction);
		}

		if (m_dynamicPropertiesStruct.isEmpty() == false)
		{
			pMutableDeviceObject->set_dynamic_properties_struct(m_dynamicPropertiesStruct.toStdString());
		}

		// Save dynamic properties' values
		//
		for (const DynamicProperty& p : m_dynamicProperties)
		{
			::Proto::Property* protoProp = pMutableDeviceObject->mutable_properties()->Add();
			p.saveValue(protoProp);
		}

		// --
		//
		if (m_preset == true)
		{
			pMutableDeviceObject->set_preset(m_preset);

			pMutableDeviceObject->set_presetroot(m_presetRoot);
			Proto::Write(pMutableDeviceObject->mutable_presetname(), m_presetName);
			Proto::Write(pMutableDeviceObject->mutable_presetobjectuuid(), m_presetObjectUuid);
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

		Proto::Read(deviceobject.strid(), &m_strId);
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

		if (deviceobject.has_dynamic_properties_struct() == true)
		{
			m_dynamicPropertiesStruct = QString::fromStdString(deviceobject.dynamic_properties_struct());
			parseDynamicPropertiesStruct();
		}
		else
		{
			m_dynamicPropertiesStruct.clear();
		}

		// Load dynamic properties' values. They are already exists after calling parseDynamicPropertiesStruct()
		//
		for (const ::Proto::Property& p :  deviceobject.properties())
		{
			auto it = m_dynamicProperties.find(p.name().c_str());

			if (it == m_dynamicProperties.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in m_strId";
			}
			else
			{
				bool loadOk = it->loadValue(p);

				Q_UNUSED(loadOk);
				assert(loadOk);

				m_avoidEventRecursion = true;
				this->setProperty((*it).name_c_str(), (*it).value());
				m_avoidEventRecursion = false;
			}

		}

		// --
		//

		if (deviceobject.has_preset() == true && deviceobject.preset() == true)
		{
			m_preset = deviceobject.preset();

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

	void DeviceObject::expandStrId()
	{
		if (parent() != nullptr)
		{
			m_strId.replace(QString("$(PARENT)"), parent()->strId(), Qt::CaseInsensitive);
		}

		m_strId.replace(QString("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);

		for (int i = 0; i < childrenCount(); i++)
		{
			child(i)->expandStrId();
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

	bool DeviceObject::event(QEvent* e)
	{
		if (e->type() == QEvent::DynamicPropertyChange && m_avoidEventRecursion == false)
		{
			// Configuration property was changed
			//
			QDynamicPropertyChangeEvent* d = dynamic_cast<QDynamicPropertyChangeEvent*>(e);
			assert(d != nullptr);

			QString propertyName = d->propertyName();
			QVariant value = this->property(propertyName.toStdString().c_str());

			if (value.isValid() == true)
			{
				auto it = m_dynamicProperties.find(propertyName);

				if (it == m_dynamicProperties.end())
				{
					// can't find property,
					// probably it is adding it to the qt meta system now?
					//
				}
				else
				{
					m_avoidEventRecursion = true;
					(*it).setValue(value);
					this->setProperty((*it).name_c_str(), (*it).value());
					m_avoidEventRecursion = false;
				}
			}

			// Accept event
			//
			return true;
		}

		// Event was not recognized
		//
		return false;
	}

	// Parse m_dynamicProperties and create Qt meta system dynamic properies
	//
	void DeviceObject::parseDynamicPropertiesStruct()
	{
		// Delete all previous object's dynamic properties
		// Don't worry about old values, the are stored in m_dynamicProperties
		//
		QList<QByteArray> dynamicProps = dynamicPropertyNames();

		m_avoidEventRecursion = true;
		for (const QByteArray& p : dynamicProps)
		{
			setProperty(QString(p).toStdString().c_str(), QVariant());
		}
		m_avoidEventRecursion = false;

		// Parse struct (rows, divided by semicolon) and create new properties
		//

		/*
		 name;			type;		min;		max;		default

		 Example:
		 Server\IP;		string;		0;			0;			192.168.75.254
		 Server\Port;	uint32_t;	1;			65535;		2345



		 name: property name, can be devided by symbol '\'
		 type: property type, can by one of
					qint32  (4 bytes signed integral),
					quint32 (4 bytes unsigned integer)
					bool (true, false),
					double,
					string
		 min: property minimum value (ignored for bool, string)
		 max: property maximim value (ignored for bool, string)
		 default: can be any value of the specified type

		 */

		QHash<QString, DynamicProperty> parsedProperties;

		QStringList rows = m_dynamicPropertiesStruct.split(QChar::LineFeed, QString::SkipEmptyParts);

		for (const QString& r : rows)
		{
			if (r.isEmpty() == true)
			{
				continue;
			}

			QStringList columns = r.split(';');

			if (columns.count() != 5)
			{
				qDebug() << Q_FUNC_INFO << " Wrong proprty struct: " << r;
				qDebug() << Q_FUNC_INFO << " Expected: name;type;min;max;default";
				continue;
			}

			QString name(columns[0]);
			QStringRef type(&columns[1]);
			QStringRef min(&columns[2]);
			QStringRef max(&columns[3]);
			QStringRef defaultValue(&columns[4]);

			if (name.isEmpty() || name.size() > 1024)
			{
				qDebug() << Q_FUNC_INFO << " DynamicProperties: filed name must have size  from 1 to 1024, name: " << name;
				continue;
			}

			if (type != "qint32" &&
				type != "quint32" &&
				type != "bool" &&
				type != "double" &&
				type != "string")
			{
				qDebug() << Q_FUNC_INFO << " DynamicProperties: wrong filed tyep: " << type;
				continue;
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

				DynamicProperty dp(name, QVariant(minInt), QVariant(maxInt), QVariant(defaultInt), QVariant(defaultInt));
				parsedProperties.insert(name, dp);
				continue;
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

				DynamicProperty dp(name, QVariant(minUInt), QVariant(maxUInt), QVariant(defaultUInt), QVariant(defaultUInt));
				parsedProperties.insert(name, dp);

				continue;
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

				DynamicProperty dp(name, QVariant(minDouble), QVariant(maxDouble), QVariant(defaultDouble), QVariant(defaultDouble));
				parsedProperties.insert(name, dp);

				continue;
			}

			if (type == "bool")
			{
				// Default Value
				//
				bool defaultBool = defaultValue.compare("true", Qt::CaseInsensitive) == 0;
				DynamicProperty dp(name, QVariant(false), QVariant(true), QVariant(defaultBool), QVariant(defaultBool));
				parsedProperties.insert(name, dp);
				continue;
			}

			if (type == "string")
			{
				DynamicProperty dp(name, QVariant(""), QVariant(""), QVariant(defaultValue.toString()), QVariant(defaultValue.toString()));
				parsedProperties.insert(name, dp);
				continue;
			}

			assert(false);
		}

		// Set to parsed properties old value
		//
		for (DynamicProperty& p : parsedProperties)
		{
			auto it = m_dynamicProperties.find(p.name());

			if (it != m_dynamicProperties.end() && (*it).value().type() == p.value().type())
			{
				p.setValue((*it).value());
			}
			else
			{
				p.setValue(p.defaultValue());		// Completely new property
			}
		}

		// Add all properties to QObject meta system
		//
		m_dynamicProperties.clear();

		m_avoidEventRecursion = true;
		for (DynamicProperty& p : parsedProperties)
		{
			this->setProperty(p.name_c_str(), p.value());
		}
		m_avoidEventRecursion = false;

		m_dynamicProperties.swap(parsedProperties);
		return;
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

    QObject* DeviceObject::jsParent() const
    {
        QObject* c = m_parent;
        QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
        return c;
    }

	int DeviceObject::jsPropertyInt(QString name) const
	{
		QVariant v = property(name.toStdString().c_str());
		if (v.isValid() == false)
		{
			assert(v.isValid());
			return 0;
		}

		return v.toInt();
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

		if (child->deviceType() == DeviceType::Software &&
			deviceType() != DeviceType::Workstation &&
			deviceType() != DeviceType::Root)
		{
			assert(false);
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
						return ref1->m_strId < ref2->m_strId;
					}
					else
					{
						return false;
					}
				}
			});

		return;
	}

	void DeviceObject::sortByStrId(Qt::SortOrder order)
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[order](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				const std::shared_ptr<DeviceObject>& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
				const std::shared_ptr<DeviceObject>& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

				if (ref1->m_strId < ref2->m_strId)
				{
					return true;
				}
				else
				{
					if (ref1->m_strId == ref2->m_strId)
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
						return ref1->m_strId < ref2->m_strId;
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
						return ref1->m_strId < ref2->m_strId;
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
		findChildObjectsByMask(mask, list);
		return list;
	}

	void DeviceObject::findChildObjectsByMask(const QString& mask, std::vector<DeviceObject*>& list)
	{
		for (auto c : m_children)
		{
			if (CUtils::processDiagSignalMask(mask, c->strId()) == true)
			{
				list.push_back(c.get());
			}
			c->findChildObjectsByMask(mask, list);
		}
	}

	QObject* DeviceObject::jsFindChildObjectByMask(const QString& mask)
	{
		std::vector<DeviceObject*> list = findChildObjectsByMask(mask);
		if (list.empty() == true)
		{
			return nullptr;
		}

		QObject* c = list.at(0);
		QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
		return c;
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

	QString DeviceObject::strId() const
	{
		return m_strId;
	}

	void DeviceObject::setStrId(QString value)
	{
		if (m_strId != value)
		{
			m_strId = value;
		}
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

	QString DeviceObject::dynamicProperties() const
	{
		return m_dynamicPropertiesStruct;
	}

	void DeviceObject::setDynamicProperties(QString value)
	{
		if (m_dynamicPropertiesStruct != value)
		{
			m_dynamicPropertiesStruct = value;
			parseDynamicPropertiesStruct();
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

	// JSON short description, uuid, strId, caption, place, etc
	//
	QString DeviceObject::details() const
	{
		QString json = QString(
R"DELIM({
	"Uuid" : "%1",
	"StrID" : "%2",
	"Caption" : "%3",
	"Place" : %4,
	"Type" : "%5"
})DELIM")
			.arg(uuid().toString())
			.arg(strId())
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
	}

	DeviceRoot::~DeviceRoot()
	{
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
	}

	DeviceSystem::~DeviceSystem()
	{
		qDebug() << Q_FUNC_INFO;
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

		moduleMessage->set_channel(m_channel);
		moduleMessage->set_subsysid(m_subSysID.toStdString());
		moduleMessage->set_conftype(m_confType.toStdString());

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

		m_channel = moduleMessage.channel();
		m_subSysID = moduleMessage.subsysid().c_str();
		m_confType = moduleMessage.conftype().c_str();

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

	int DeviceModule::channel() const
	{
		return m_channel;
	}

	void DeviceModule::setChannel(int value)
	{
		m_channel = value;
	}

	QString DeviceModule::subSysID() const
	{
		return m_subSysID;
	}

	void DeviceModule::setSubSysID(const QString& value)
	{
		m_subSysID = value;
	}

	QString DeviceModule::confType() const
	{
		return m_confType;
	}

	void DeviceModule::setConfType(const QString& value)
	{
		m_confType = value;
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
					m_type = SignalType::Discrete;
					m_function = SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::DiagAnalog:
					m_type = SignalType::Analog;
					m_function = SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::InputDiscrete:
					m_type = SignalType::Discrete;
					m_function = SignalFunction::Input;
					break;
				case Obsolete::SignalType::InputAnalog:
					m_type = SignalType::Analog;
					m_function = SignalFunction::Input;
					break;
				case Obsolete::SignalType::OutputDiscrete:
					m_type = SignalType::Discrete;
					m_function = SignalFunction::Output;
					break;
				case Obsolete::SignalType::OutputAnalog:
					m_type = SignalType::Analog;
					m_function = SignalFunction::Output;
					break;
				default:
					assert(false);
			}
		}
		else
		{
			m_type = static_cast<SignalType>(signalMessage.type());
			m_function = static_cast<SignalFunction>(signalMessage.function());
		}

		m_byteOrder = static_cast<ByteOrder>(signalMessage.byteorder());
		m_format = static_cast<DataFormat>(signalMessage.format());

		m_size = signalMessage.size();

		m_validityOffset = signalMessage.validityoffset();
		m_validityBit = signalMessage.validitybit();

		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();

		return true;
	}

	DeviceType DeviceSignal::deviceType() const
	{
		return DeviceSignal::m_deviceType;
	}

	DeviceSignal::SignalType DeviceSignal::type() const
	{
		return m_type;
	}

    int DeviceSignal::jsType() const
    {
        return static_cast<int>(type());
    }

    void DeviceSignal::setType(DeviceSignal::SignalType value)
	{
		m_type = value;
	}

	DeviceSignal::SignalFunction DeviceSignal::function() const
	{
		return m_function;
	}

	int DeviceSignal::jsFunction() const
	{
		return static_cast<int>(function());
	}

	void DeviceSignal::setFunction(DeviceSignal::SignalFunction value)
	{
		m_function = value;
	}

	DeviceSignal::ByteOrder DeviceSignal::byteOrder() const
	{
		return m_byteOrder;
	}

	void DeviceSignal::setByteOrder(DeviceSignal::ByteOrder value)
	{
		m_byteOrder = value;
	}

	DeviceSignal::DataFormat DeviceSignal::format() const
	{
		return m_format;
	}

	void DeviceSignal::setFormat(DeviceSignal::DataFormat value)
	{
		m_format = value;
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
		return m_function == SignalFunction::Input;
	}

	bool DeviceSignal::isOutputSignal() const
	{
		return m_function == SignalFunction::Output;
	}

	bool DeviceSignal::isDiagSignal() const
	{
		return m_function == SignalFunction::Diagnostics;
	}

	bool DeviceSignal::isValiditySignal() const
	{
		return m_function == SignalFunction::Validity;
	}

	bool DeviceSignal::isAnalogSignal() const
	{
		return	m_type == SignalType::Analog;
	}

	bool DeviceSignal::isDiscreteSignal() const
	{
		return	m_type == SignalType::Discrete;
	}


	//
	//
	// Workstation
	//
	//
	Workstation::Workstation(bool preset /*= false*/) :
		DeviceObject(preset)
	{

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

		softwareMessage->set_type(m_type);

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

		m_type =  softwareMessage.type();

		return true;
	}

	DeviceType Software::deviceType() const
	{
		return m_deviceType;
	}


	int Software::type() const
	{
		return m_type;
	}

	void Software::setType(int value)
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

		m_deviceTable.insert(m_root->strId(), m_root);
		addDeviceChildrenToHashTable(m_root);

		for (auto it : m_deviceTable)
		{
			qDebug() << it->strId();
		}

		return;
	}

	DeviceObject* EquipmentSet::deviceObject(const QString& strId)
	{
		auto it = m_deviceTable.find(strId);

		if (it != m_deviceTable.end())
		{
			return it.value().get();
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<DeviceObject> EquipmentSet::deviceObjectSharedPointer(const QString& strId)
	{
		auto it = m_deviceTable.find(strId);

		if (it != m_deviceTable.end())
		{
			return it.value();
		}
		else
		{
			return std::shared_ptr<DeviceObject>();
		}
	}


	void EquipmentSet::addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent)
	{
		for (int i = 0; i < parent->childrenCount(); i++)
		{
			std::shared_ptr<DeviceObject> child = parent->childSharedPtr(i);
			m_deviceTable.insert(child->strId(), child);

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

	void SerializeEquipmentFromXml(std::shared_ptr<Hardware::DeviceRoot>& deviceRoot)
	{
		QXmlStreamReader equipmentReader;
		QFile file("equipment.xml");

		Hardware::DeviceObject* pCurrentDevice;

		if (file.open(QIODevice::ReadOnly))
		{
			equipmentReader.setDevice(&file);
			Hardware::Init();

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

					const QMetaObject* metaObject = pDeviceObject->metaObject();
					for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
					{
						const QMetaProperty& property = metaObject->property(i);
						if (property.isValid())
						{
							const char* name = property.name();
							QVariant tmp = QVariant::fromValue(attr.value(name).toString());
							assert(tmp.convert(pDeviceObject->property(name).userType()));
							pDeviceObject->setProperty(name, tmp);
						}
					}

					pDeviceObject->setStrId(attr.value("StrID").toString());
					pDeviceObject->setCaption(attr.value("Caption").toString());
					pDeviceObject->setChildRestriction(attr.value("ChildRestriction").toString());
					pDeviceObject->setPlace(attr.value("Place").toInt());
					pDeviceObject->setDynamicProperties(attr.value("DynamicProperties").toString());

					pCurrentDevice->addChild(pDeviceObject);
					pCurrentDevice = pDeviceObject.get();
					break;
				}
				case QXmlStreamReader::EndElement:
					if (typeid(*pCurrentDevice) != typeid(Hardware::DeviceRoot))
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

