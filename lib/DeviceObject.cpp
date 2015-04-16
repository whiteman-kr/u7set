#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"
#include <QDynamicPropertyChangeEvent>
#include <QJSEngine>
#include <QQmlEngine>
#include <QXmlStreamWriter>


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
			L".hds",		// Diagnostics Signal
			L".hws",		// Workstation
			L".hsw",		// Software
		};

	Factory<Hardware::DeviceObject> DeviceObjectFactory;

	void Init()
	{
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
	}


	//
	//
	// Subsystem
	//
	//
	Subsystem::Subsystem():
		m_index(-1)
	{

	}

	Subsystem::Subsystem(int index, const QString& strId, const QString& caption):
		m_index(index),
		m_strId(strId),
		m_caption(caption)
	{

	}

	bool Subsystem::save(QXmlStreamWriter& writer)
	{
		writer.writeAttribute("Index", QString::number(index()));
		writer.writeAttribute("StrID", strId());
		writer.writeAttribute("Caption", caption());
		return true;
	}


	bool Subsystem::load(QXmlStreamReader& reader)
	{
		if (reader.attributes().hasAttribute("Index"))
		{
			setIndex(reader.attributes().value("Index").toInt());
		}
		else
		{
			reader.raiseError(QObject::tr("Subsystem - No Index found"));
		}

		if (reader.attributes().hasAttribute("StrID"))
		{
			setStrId(reader.attributes().value("StrID").toString());
		}
		else
		{
			reader.raiseError(QObject::tr("Subsystem - No StrID found"));
		}

		if (reader.attributes().hasAttribute("Caption"))
		{
			setCaption(reader.attributes().value("Caption").toString());
		}
		else
		{
			reader.raiseError(QObject::tr("Subsystem - No Caption found"));
		}

		QXmlStreamReader::TokenType endToken = reader.readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return true;
	}


	const QString& Subsystem::strId() const
	{
		return m_strId;
	}

	void Subsystem::setStrId(const QString& value)
	{
		m_strId = value;
	}

	const QString& Subsystem::caption() const
	{
		return m_caption;
	}

	void Subsystem::setCaption(const QString& value)
	{
		m_caption = value;
	}

	int Subsystem::index() const
	{
		return m_index;
	}

	void Subsystem::setIndex(int value)
	{
		m_index = value;
	}

	//
	//
	// SubsystemStorage
	//
	//
	SubsystemStorage::SubsystemStorage()
	{

	}

	void SubsystemStorage::add(std::shared_ptr<Subsystem> subsystem)
	{
		m_subsystems.push_back(subsystem);
	}

	int SubsystemStorage::count() const
	{
		return static_cast<int>(m_subsystems.size());
	}

	std::shared_ptr<Subsystem> SubsystemStorage::get(int index) const
	{
		if (index < 0 || index >= count())
		{
			assert(false);
			return std::make_shared<Subsystem>();
		}
		return m_subsystems[index];
	}

	void SubsystemStorage::clear()
	{
		m_subsystems.clear();
	}

	bool SubsystemStorage::load(const QByteArray& data, QString& errorCode)
	{
		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			return !reader.hasError();
		}

		if (reader.name() != "Subsystems")
		{
			reader.raiseError(QObject::tr("The file is not an Subsystems file."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if (reader.name() == "Subsystem")
			{
				std::shared_ptr<Hardware::Subsystem> s = std::make_shared<Hardware::Subsystem>();

				if (s->load(reader) == true)
				{
					m_subsystems.push_back(s);
				}
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				errorCode = reader.errorString();
				reader.skipCurrentElement();
			}
		}
		return !reader.hasError();
	}

	bool SubsystemStorage::save(QByteArray& data)
	{
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);
		writer.writeStartDocument();

		writer.writeStartElement("Subsystems");
		for (auto s : m_subsystems)
		{
			writer.writeStartElement("Subsystem");
			s->save(writer);
			writer.writeEndElement();
		}
		writer.writeEndElement();

		writer.writeEndDocument();
		return true;
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

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

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

			if (deviceobject.has_presetroot() == true)
			{
				m_presetRoot = deviceobject.presetroot();
			}
			else
			{
				assert(deviceobject.has_presetroot() == true);
			}

			if (deviceobject.has_presetname() == true)
			{
				Proto::Read(deviceobject.presetname(), &m_presetName);
			}
			else
			{
				assert(deviceobject.has_presetname());
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

    DeviceType DeviceObject::deviceType() const
	{
		assert(false);
		return DeviceType::Root;
	}

	int DeviceObject::jsDeviceType() const
	{
		return static_cast<int>(deviceType());
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
		auto fr = std::find_if(m_children.begin(), m_children.end(),
							   [child](const std::shared_ptr<DeviceObject>& v)
		{
			return v.get() == child;
		});

		if (fr == m_children.end())
		{
			return -1;
		}

		return std::distance(m_children.begin(), fr);
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

	void DeviceObject::sortChildrenByPlace()
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				return o1->m_place < o2->m_place;
			});

		return;
	}

	const QString& DeviceObject::strId() const
	{
		return m_strId;
	}

	void DeviceObject::setStrId(const QString& value)
	{
		m_strId = value;
	}

	const QString& DeviceObject::caption() const
	{
		return m_caption;
	}

	void DeviceObject::setCaption(const QString& value)
	{
		m_caption = value;
	}

	DbFileInfo& DeviceObject::fileInfo()
	{
		return m_fileInfo;
	}

	const DbFileInfo& DeviceObject::fileInfo() const
	{
		return m_fileInfo;
	}

	void DeviceObject::setFileInfo(const DbFileInfo& value)
	{
		m_fileInfo = value;
	}

	const QString& DeviceObject::childRestriction() const
	{
		return m_childRestriction;
	}

	void DeviceObject::setChildRestriction(const QString& value)
	{
		m_childRestriction = value;
	}

	const QString& DeviceObject::dynamicProperties() const
	{
		return m_dynamicPropertiesStruct;
	}

	void DeviceObject::setDynamicProperties(const QString& value)
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
		m_place = value;
	}

	bool DeviceObject::preset() const
	{
		return m_preset;
	}

	bool DeviceObject::presetRoot() const
	{
		assert(m_preset == true);
		return m_presetRoot;
	}

	void DeviceObject::setPresetRoot(bool value)
	{
		m_presetRoot = value;
	}

	const QString& DeviceObject::presetName() const
	{
		assert(m_preset == true);
		return m_presetName;
	}

	void DeviceObject::setPresetName(const QString& value)
	{
		m_presetName = value;
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

		moduleMessage->set_confindex(m_confIndex);
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

		m_type =  static_cast<Hardware::DeviceModule::ModuleType>(moduleMessage.type());

		m_confIndex = moduleMessage.confindex();
		m_subSysID = moduleMessage.subsysid().c_str();
		m_confType = moduleMessage.conftype().c_str();

		return true;
	}

	DeviceType DeviceModule::deviceType() const
	{
		return m_deviceType;
	}


	Hardware::DeviceModule::ModuleType DeviceModule::type() const
	{
		return m_type;
	}

	void DeviceModule::setType(Hardware::DeviceModule::ModuleType value)
	{
		m_type = value;
	}

	int DeviceModule::confIndex() const
	{
		return m_confIndex;
	}

	void DeviceModule::setConfIndex(int value)
	{
		m_confIndex = value;
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
		Proto::DeviceSignal* signalMessage =
				message->mutable_deviceobject()->mutable_signal();

		signalMessage->set_type(static_cast<int>(m_type));

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

		m_type = static_cast<SignalType>(signalMessage.type());

		return true;
	}

	DeviceType DeviceSignal::deviceType() const
	{
		return m_deviceType;
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
}

