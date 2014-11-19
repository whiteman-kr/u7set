
#include "../include/ModuleConfiguration.h"

namespace Hardware
{

	// ----------------------------------------------------------------------------
	//
	//						ModuleConfigurationValue
	//
	// ----------------------------------------------------------------------------
	ModuleConfigurationValue::ModuleConfigurationValue()
	{
	}

	ModuleConfigurationValue::~ModuleConfigurationValue()
	{
	}

	void ModuleConfigurationValue::readValue(QXmlStreamReader& reader)
	{
		assert(reader.name() == "value");

		bool ok = false;

		// Reading "Value" attributes and data
		//
		QXmlStreamAttributes attr = reader.attributes();

		if (attr.hasAttribute("name"))
		{
			m_name = attr.value("name").toString();
		}

		if (attr.hasAttribute("type"))
		{
			m_type = attr.value("type").toString();
		}

		if (attr.hasAttribute("offset"))
		{
			m_offset = attr.value("offset").toInt(&ok);
		}

		if (attr.hasAttribute("bit"))
		{
			m_bit = attr.value("bit").toInt(&ok);
		}

		if (attr.hasAttribute("size"))
		{
			m_boolSize = attr.value("size").toInt(&ok);
		}

		if (attr.hasAttribute("user"))
		{
			m_userProperty = attr.value("user").compare("true", Qt::CaseInsensitive) == 0;
		}
		else
		{
			m_userProperty = false;
		}

		if (attr.hasAttribute("default"))
		{
			m_defaultValue = attr.value("default").toString();
		}

		m_value = m_defaultValue;

		// Finishing
		//
		reader.readElementText();

		if (reader.isEndElement())
		{
			reader.readNext();
		}

		return;
	}

	const QString& ModuleConfigurationValue::name() const
	{
		return m_name;
	}

	void ModuleConfigurationValue::setName(const QString& name)
	{
		m_name = name;
	}

	const QString& ModuleConfigurationValue::type() const
	{
		return m_type;
	}

	void ModuleConfigurationValue::setType(const QString& type)
	{
		m_type = type;
	}

	int ModuleConfigurationValue::offset() const
	{
		return m_offset;
	}

	void ModuleConfigurationValue::setOffset(int offset)
	{
		m_offset = offset;
	}

	int ModuleConfigurationValue::bit() const
	{
		return m_bit;
	}

	void ModuleConfigurationValue::setBit(int bit)
	{
		m_bit = bit;
	}

	int ModuleConfigurationValue::boolSize() const
	{
		return m_boolSize;
	}

	void ModuleConfigurationValue::setBoolSize(int boolSize)
	{
		m_boolSize = boolSize;
	}

	bool ModuleConfigurationValue::userProperty() const
	{
		return m_userProperty;
	}

	void ModuleConfigurationValue::setUserProperty(bool value)
	{
		m_userProperty = value;
	}

	const QString& ModuleConfigurationValue::defaultValue() const
	{
		return m_defaultValue;
	}

	void ModuleConfigurationValue::setDefaultValue(const QString& defaultValue)
	{
		m_defaultValue = defaultValue;
	}

	const QString& ModuleConfigurationValue::value() const
	{
		return m_value;
	}

	void ModuleConfigurationValue::setValue(const QString& value)
	{
		m_value = value;
	}


	// ----------------------------------------------------------------------------
	//
	//						ModulleConfigurationStruct
	//
	// ----------------------------------------------------------------------------
	ModuleConfigurationStruct::ModuleConfigurationStruct()
	{
		m_values.reserve(32);
	}

	ModuleConfigurationStruct::ModuleConfigurationStruct(const QString& name, int size, bool be) :
		m_name(name),
		m_dataSize(0),
		m_size(size),
		m_be(be)
	{
		m_values.reserve(32);
	}

	void ModuleConfigurationStruct::readStruct(QXmlStreamReader& reader, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage != nullptr);
			return;
		}

		QXmlStreamAttributes attr = reader.attributes();

		if (attr.hasAttribute("name"))
		{
			setName(attr.value("name").toString());
		}

		bool ok = false;
		if (attr.hasAttribute("size"))
		{
			setSize(attr.value("size").toInt(&ok));
		}

		if (attr.hasAttribute("byteorder"))
		{
			setBe(attr.value("byteorder").toString() == "be");
		}

		// --
		//
		reader.readNext();

		while (reader.atEnd() == false)
		{
			if (reader.isEndElement() == true)
			{
				reader.readNext();
				break;
			}

			if (reader.isStartElement() == true)
			{
				if (reader.name() == "value")
				{
					ModuleConfigurationValue value;
					value.readValue(reader);
					m_values.append(value);
				}
				else
				{
					ModuleConfiguration::skipUnknownElement(&reader, errorMessage);
				}
			}
			else
			{
				reader.readNext();
			}
		}

		return;
	}

	const QString& ModuleConfigurationStruct::name() const
	{
		return m_name;
	}

	void ModuleConfigurationStruct::setName(const QString& name)
	{
		m_name = name;
	}

	int ModuleConfigurationStruct::size() const
	{
		return m_size;
	}

	void ModuleConfigurationStruct::setSize(int size)
	{
		m_size = size;
	}

	int ModuleConfigurationStruct::dataSize() const
	{
		return m_dataSize;
	}

	void ModuleConfigurationStruct::setDataSize(int dataSize)
	{
		m_dataSize = dataSize;
	}

	int ModuleConfigurationStruct::actualSize() const
	{
		if (m_size == 0)
		{
			return m_dataSize;
		}
		else
		{
			return m_size;
		}
	}

	const QVector<ModuleConfigurationValue>& ModuleConfigurationStruct::values() const
	{
		return m_values;
	}

	bool ModuleConfigurationStruct::be() const
	{
		return m_be;
	}

	void ModuleConfigurationStruct::setBe(bool be)
	{
		m_be = be;
	}

	// ----------------------------------------------------------------------------
	//
	//						ModuleConfigurationVariable
	//
	// ----------------------------------------------------------------------------
	ModuleConfigurationVariable::ModuleConfigurationVariable()
	{
	}

	ModuleConfigurationVariable::~ModuleConfigurationVariable()
	{
	}

	void ModuleConfigurationVariable::readVariable(QXmlStreamReader& reader)
	{
		Q_ASSERT(reader.name() == "variable");

		QXmlStreamAttributes attr = reader.attributes();

		if (attr.hasAttribute("name"))
		{
			setName(attr.value("name").toString());
		}

		if (attr.hasAttribute("type"))
		{
			setType(attr.value("type").toString());
		}

		if (attr.hasAttribute("frameIndex"))
		{
			setFrameIndex(attr.value("frameIndex").toInt());
		}

		reader.readElementText();

		if (reader.isEndElement() == true)
		{
			reader.readNext();
		}

		return;
	}

	const QString& ModuleConfigurationVariable::name() const
	{
		return m_name;
	}

	void ModuleConfigurationVariable::setName(const QString& name)
	{
		m_name = name;
	}

	const QString& ModuleConfigurationVariable::type() const
	{
		return m_type;
	}

	void ModuleConfigurationVariable::setType(const QString& type)
	{
		m_type = type;
	}

	int ModuleConfigurationVariable::frameIndex() const
	{
		return m_frameIndex;
	}

	void ModuleConfigurationVariable::setFrameIndex(int frameIndex)
	{
		m_frameIndex = frameIndex;
	}

	const std::shared_ptr<ModuleConfigurationStruct>& ModuleConfigurationVariable::data() const
	{
		return m_data;
	}

	void ModuleConfigurationVariable::setData(const std::shared_ptr<ModuleConfigurationStruct>& data)
	{
		m_data = data;
	}


	// ----------------------------------------------------------------------------
	//
	//						ModuleConfiguration
	//
	// ----------------------------------------------------------------------------

	ModuleConfiguration::ModuleConfiguration()
	{
		m_structures.reserve(64);
		m_userProperties.reserve(256);
	}

	ModuleConfiguration::~ModuleConfiguration()
	{
	}

	bool ModuleConfiguration::load(const Proto::ModuleConfiguration& message)
	{
		m_hasConfiguration = message.has_struct_description();
		if (m_hasConfiguration == false)
		{
			return true;
		}

		m_xmlStructDesctription = message.struct_description();

		bool result = readStructure(m_xmlStructDesctription.data());

		for (const ::Proto::ModuleConfigurationValue& pv : message.values())
		{
			assert(pv.name().empty() == false);

			QString name = QString::fromStdString(pv.name());
			QString value = QString::fromStdString(pv.value());

			bool contains = m_userProperties.contains(name);
			if (contains == true)
			{
				m_userProperties[name].setValue(value);
			}
		}

		return result;
	}

	void ModuleConfiguration::save(Proto::ModuleConfiguration* message) const
	{
		if (m_hasConfiguration == false)
		{
			// There is no modlue configuration
			return;
		}

		message->mutable_struct_description()->assign(m_xmlStructDesctription);

		for (const ModuleConfigurationValue& v : m_userProperties)
		{
			::Proto::ModuleConfigurationValue* pv = message->add_values();
			pv->set_name(v.name().toStdString());
			pv->set_value(v.value().toStdString());
		}

		return;
	}

	void ModuleConfiguration::addUserPropertiesToObject(QObject* object) const
	{
		if (object == nullptr)
		{
			assert(object != nullptr);
			return;
		}

		// Delete all previous dynamic properties
		//
		QList<QByteArray> dynamicProperties(object->dynamicPropertyNames());

		for (const QByteArray& ba : dynamicProperties)
		{
			QString name(ba);
			object->setProperty(name.toStdString().c_str(), QVariant());
		}

		// Set new user properties
		//
		for (const ModuleConfigurationValue& up : m_userProperties)
		{
			object->setProperty(up.name().toStdString().c_str(), QVariant(up.value()));
		}

		return;
	}

	bool ModuleConfiguration::setUserProperty(const QString& name, const QVariant& value)
	{
		assert(name.isEmpty() == false);
		assert(value.isValid() == true);

		bool exists = m_userProperties.contains(name);

		if (exists == false)
		{
			return false;
		}

		ModuleConfigurationValue& p = m_userProperties[name];

		p.setValue(value.toString());

		return true;
	}

	QString ModuleConfiguration::lastError() const
	{
		return m_lastError;
	}

	void ModuleConfiguration::skipUnknownElement(QXmlStreamReader* reader, QString* errorMessage)
	{
		if (reader == nullptr || errorMessage == nullptr)
		{
			assert(reader);
			assert(errorMessage);
			return;
		}

		if (errorMessage->isEmpty() == false)
		{
			*errorMessage += "\n";
		}

		*errorMessage += tr("Unknown XML tag %1").arg(reader->name().toString());

		qDebug() << Q_FUNC_INFO << ("Unknown tag: ") << reader->name().toString() << endl;

		reader->readNext();

		while (reader->atEnd() == false)
		{
			if (reader->isEndElement() == true)
			{
				reader->readNext();
				break;
			}

			if (reader->isStartElement() == true)
			{
				skipUnknownElement(reader, errorMessage);
			}
			else
			{
				reader->readNext();
			}
		}

		return;
	}


	bool ModuleConfiguration::readStructure(const char* data)
	{
		m_lastError.clear();

		m_structures.clear();
		m_structures.reserve(64);

		m_variables.clear();
		m_variables.reserve(64);

		QXmlStreamReader reader(data);

		while (reader.atEnd() == false)
		{
			reader.readNext();

			if (reader.isStartElement() == true && reader.name() == "configuration")
			{
				break;
			}
		}

		if (reader.isStartElement() == false || reader.name() != "configuration")
		{
			m_lastError = tr("Parsing error, cant find Configuration tag");
			return false;
		}


		bool convertOk = true;
		bool ok = false;

		// Reading configuration params
		//
		QXmlStreamAttributes attr = reader.attributes();

		if (attr.hasAttribute("name"))
		{
			setName(attr.value("name").toString());
		}

		if (attr.hasAttribute("version"))
		{
			setVersion(attr.value("version").toInt(&ok));
			if (ok == false)
			{
				convertOk = false;
			}
		}

		if (attr.hasAttribute("uartid"))
		{
			setUartId(attr.value("uartid").toInt(&ok));
			if (ok == false)
			{
				convertOk = false;
			}
		}

		if (attr.hasAttribute("minFrameSize"))
		{
			setMinFrameSize(attr.value("minFrameSize").toInt(&ok));
			if (ok == false)
			{
				convertOk = false;
			}
		}

		if (convertOk == false)
		{
			m_lastError = tr("XML convert error.");
			return false;
		}

		// reading congfiguration children
		//
		reader.readNext();

		while (reader.atEnd() == false)
		{
			if (reader.isEndElement() == true)
			{
				reader.readNext();
				break;
			}

			if (reader.isStartElement())
			{
				bool knownTag = false;

				if (reader.name() == "declarations")
				{
					knownTag = true;
					readDeclaration(reader);
				}

				if (reader.name() == "definitions")
				{
					knownTag = true;
					readDefinition(reader);
				}

				if (knownTag == false)
				{
					skipUnknownElement(&reader, &m_lastError);
				}
			}
			else
			{
				reader.readNext();
			}
		}


		// --
		//
		createUserProperties(&m_lastError);

		return true;
	}

	void ModuleConfiguration::readDeclaration(QXmlStreamReader& reader)
	{
		reader.readNext();

		while (reader.atEnd() == false)
		{
			if (reader.isEndElement() == true)
			{
				reader.readNext();
				break;
			}

			if (reader.isStartElement() == true)
			{
				if (reader.name().compare("struct") == 0)
				{
					ModuleConfigurationStruct configStruct;
					configStruct.readStruct(reader, &m_lastError);
					m_structures[configStruct.name()] = configStruct;
				}
				else
				{
					skipUnknownElement(&reader, &m_lastError);
				}
			}
			else
			{
				reader.readNext();
			}
		}

		return;
	}

	void ModuleConfiguration::readDefinition(QXmlStreamReader& reader)
	{
		reader.readNext();

		while (reader.atEnd() == false)
		{
			if (reader.isEndElement() == true)
			{
				reader.readNext();
				break;
			}

			if (reader.isStartElement() == true)
			{
				if (reader.name().compare("variable") == 0)
				{
					ModuleConfigurationVariable var;
					var.readVariable(reader);
					m_variables.append(var);
				}
				else
				{
					skipUnknownElement(&reader, &m_lastError);
				}
			}
			else
			{
				reader.readNext();
			}
		}

		return;
	}

	void ModuleConfiguration::createUserProperties(QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return;
		}

		for (const ModuleConfigurationVariable& variable : m_variables)
		{
			if (m_structures.contains(variable.type()) == false)
			{
				*errorMessage += tr("Can't find structure %1 in variable %2").arg(variable.type()).arg(variable.name());
				continue;
			}

			ModuleConfigurationStruct& structure = m_structures[variable.type()];

			parseUserProperties(structure, "Configuration\\" + variable.name(), errorMessage);
		}

		return;
	}

	void ModuleConfiguration::parseUserProperties(const ModuleConfigurationStruct& structure, const QString& parentVariableName, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage != nullptr);
			return;
		}

		const QVector<ModuleConfigurationValue>& v = structure.values();

		QString varName;

		for (const ModuleConfigurationValue& structValue : v)
		{
			varName = parentVariableName;
			varName.push_back('\\');
			varName.push_back(structValue.name());

			if (m_structures.contains(structValue.type()) == true)
			{
				// It is nested stuctrure
				//
				ModuleConfigurationStruct& nestedStruct = m_structures[structValue.type()];
				parseUserProperties(nestedStruct, varName, errorMessage);
				continue;
			}

			// It is one of the trivial(?) types
			//
			if (structValue.userProperty() == true && m_userProperties.contains(varName) == false)
			{
				ModuleConfigurationValue v(structValue);

				v.setName(varName);					// Set Full qualified name
				v.setValue(v.defaultValue());

				m_userProperties.insert(varName, v);
			}
		}

		return;
	}


	bool ModuleConfiguration::hasConfiguration() const
	{
		return m_hasConfiguration;
	}

	void ModuleConfiguration::setHasConfiguration(bool value)
	{
		m_hasConfiguration = value;
	}

	const QString& ModuleConfiguration::name() const
	{
		return m_name;
	}

	void ModuleConfiguration::setName(const QString& value)
	{
		m_name = value;
	}

	int ModuleConfiguration::version() const
	{
		return m_version;
	}

	void ModuleConfiguration::setVersion(int value)
	{
		m_version = value;
	}

	int ModuleConfiguration::uartId() const
	{
		return m_uartID;
	}

	void ModuleConfiguration::setUartId(int value)
	{
		m_uartID = value;
	}

	int ModuleConfiguration::minFrameSize() const
	{
		return m_minFrameSize;
	}

	void ModuleConfiguration::setMinFrameSize(int value)
	{
		m_minFrameSize = value;
	}

	const std::string& ModuleConfiguration::structDescription() const
	{
		return m_xmlStructDesctription;
	}

	void ModuleConfiguration::setStructDescription(const std::string& value)
	{
		m_xmlStructDesctription = value;
	}
}
