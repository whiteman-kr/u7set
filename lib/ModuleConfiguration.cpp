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
		Q_ASSERT(reader.name() == "value");

		bool ok = false;

		// Reading "Value" attributes and data
		//

		QXmlStreamAttributes attr = reader.attributes();
		if (attr.hasAttribute("name"))
		{
			setName(attr.value("name").toString());
		}

		if (attr.hasAttribute("type"))
		{
			setType(attr.value("type").toString());
		}

		if (attr.hasAttribute("offset"))
		{
			setOffset(attr.value("offset").toInt(&ok));
		}

		if (attr.hasAttribute("bit"))
		{
			setBit(attr.value("bit").toInt(&ok));
		}

		if (attr.hasAttribute("size"))
		{
			setBoolSize(attr.value("size").toInt(&ok));
		}

		if (attr.hasAttribute("user"))
		{
			setUserProperty(attr.value("user").compare("true", Qt::CaseInsensitive) == 0);
		}
		else
		{
			setUserProperty(false);
		}

		if (attr.hasAttribute("default"))
		{
			setDefaultValue(attr.value("default").toString());
		}

		setValue(defaultValue());

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

//	const std::shared_ptr<ModuleConfigurationStruct>& ModuleConfigurationValue::data() const
//	{
//		return m_data;
//	}

//	void ModuleConfigurationValue::setData(const std::shared_ptr<ModuleConfigurationStruct>& data)
//	{
//		m_data = data;
//	}


	// ----------------------------------------------------------------------------
	//
	//						ModulleConfigurationStruct
	//
	// ----------------------------------------------------------------------------
	ModuleConfigurationStruct::ModuleConfigurationStruct()
	{
	}

	ModuleConfigurationStruct::ModuleConfigurationStruct(const QString& name, int size, bool be) :
		m_name(name),
		m_dataSize(0),
		m_size(size),
		m_be(be)
	{
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

	const QList<ModuleConfigurationValue>& ModuleConfigurationStruct::values() const
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
	}

	ModuleConfiguration::~ModuleConfiguration()
	{
	}

	bool ModuleConfiguration::load(const Proto::ModuleConfiguration& message)
	{
		m_hasConfiguration = message.has_struct_description();

		if (message.has_struct_description() == false)
		{
			return true;
		}

		const std::string& description = message.struct_description();
		m_xmlStructDesctription = QString::fromStdString(description);

		bool result = readStructure(m_xmlStructDesctription);

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

		message->mutable_struct_description()->assign(m_xmlStructDesctription.toStdString());

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
			qDebug() << "Added User Property " << up.name() << " with value " << up.value();
		}

		return;
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


	bool ModuleConfiguration::readStructure(const QString& data)
	{
		m_lastError.clear();

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

		//createMembers();
		//setVals();

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
				if (reader.name() == "struct")
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
				if (reader.name() == "variable")
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

		m_userProperties.clear();

		for (const ModuleConfigurationVariable& variable : m_variables)
		{
			if (m_structures.contains(variable.type()) == false)
			{
				*errorMessage += tr("Can't find structure %1 in variable %2").arg(variable.type()).arg(variable.name());
				continue;
			}

			ModuleConfigurationStruct structure = m_structures.value(variable.type());

			parseUserProperties(structure, variable.name(), errorMessage);
		}

		return;
	}

	void ModuleConfiguration::parseUserProperties(const ModuleConfigurationStruct& structure, const QString& parentVariableName, QString* errorMessage)
	{
		if (parentVariableName == nullptr || errorMessage == nullptr)
		{
			assert(parentVariableName != nullptr);
			assert(errorMessage != nullptr);
			return;
		}

		int count = 0;
		for (const ModuleConfigurationValue& structValue : structure.values())
		{
			count ++;
			if (count > 2)
				break;

			QString varName = parentVariableName + "\\" + structValue.name();
			//QString varName = QString("aa%1=-").arg(count);

			if (m_structures.contains(structValue.type()) == true)
			{
				// It is nested stuctrure
				//
				ModuleConfigurationStruct nestedStruct = m_structures.value(structValue.type());
				parseUserProperties(nestedStruct, varName, errorMessage);
				continue;
			}

			// It is one of the trivial(?) types
			//
			if (structValue.userProperty() == true)
			{
				ModuleConfigurationValue v(structValue);

				v.setName(varName);					// Set Full qualified name
				v.setValue(v.defaultValue());

				m_userProperties[varName] = v;
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

	const QString& ModuleConfiguration::structDescription() const
	{
		return m_xmlStructDesctription;
	}

	void ModuleConfiguration::setStructDescription(const QString& value)
	{
		m_xmlStructDesctription = value;
	}
}
