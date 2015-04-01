
#include "../include/ModuleConfiguration.h"
#include "../include/Crc.h"
#include <QMap>
#include <QHash>
#include <QQmlEngine>

namespace Hardware
{
	ModuleConfFirmware::ModuleConfFirmware()
	{
	}

	ModuleConfFirmware::~ModuleConfFirmware()
	{
	}

    void ModuleConfFirmware::init(QString type, QString name, int uartId, int frameSize, int frameCount)
	{
        m_type = type;
		m_name = name;
		m_uartId = uartId;
		m_frameSize = frameSize;

		m_frames.clear();
		m_frames.resize(frameCount);

		for (int i = 0; i < frameCount; i++)
		{
			m_frames[i].resize(frameSize);
		}

		return;
	}

    bool ModuleConfFirmware::save(QString projectName, QString userName)
    {
        QJsonObject jObject;

        for (int i = 0; i < frameCount(); i++)
        {
            std::vector<quint8>& frame = m_frames[i];

            QJsonObject jFrame;

            QJsonArray array;
            for (int j = 0; j < frame.size(); j++)
                array.push_back(QJsonValue(frame[j]));

            jFrame.insert("data", array);
            jFrame.insert("frameIndex", i);

            jObject.insert("z_frame_" + QString().number(i), jFrame);
        }

        jObject.insert("userName", userName);
        jObject.insert("projectName", projectName);
        jObject.insert("type", type());
        jObject.insert("name", name());
        jObject.insert("uartId", uartId());
        jObject.insert("frameSize", frameSize());
        jObject.insert("framesCount", frameCount());

        QByteArray data;
        data = QJsonDocument(jObject).toJson();

        QFile file(type() + tr("_") + name() + tr(".mcb"));
        if (file.open(QIODevice::WriteOnly)  == false)
        {
            return false;
        }

        bool result = true;

        if (file.write(data) == -1)
        {
            result = false;
        }

        file.close();
        return result;
    }

    bool ModuleConfFirmware::load(QString fileName)
    {
        m_frames.clear();

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)  == false)
        {
            return false;
        }

        QByteArray data;
        data = file.readAll();

        file.close();

        QJsonDocument document = QJsonDocument::fromJson(data);

        if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
        {
            return false;
        }

        QJsonObject jConfig = document.object();

        /*int configNo = 0;
        QJsonValue jConfigVal = object.value("config" + QString::number(configNo));
        if (jConfigVal.isUndefined() == true || jConfigVal.isObject() == false)
        {
            return false;
        }*/

        //QJsonObject jConfig = jConfigVal.toObject();

        if (jConfig.value("type").isUndefined() == true)
        {
            return false;
        }
        m_type = jConfig.value("type").toString();

        if (jConfig.value("name").isUndefined() == true)
        {
            return false;
        }
        m_name = jConfig.value("name").toString();

        /*if (jConfig.value("version").isUndefined() == true)
        {
            return false;
        }
        m_version = (int)jConfig.value("version").toDouble();*/

        if (jConfig.value("uartId").isUndefined() == true)
        {
            return false;
        }
        m_uartId = (int)jConfig.value("uartId").toDouble();

        if (jConfig.value("frameSize").isUndefined() == true)
        {
            return false;
        }
        m_frameSize = (int)jConfig.value("frameSize").toDouble();

        /*if (jConfig.value("changeset").isUndefined() == true)
        {
            return false;
        }
        m_changeset = (int)jConfig.value("changeset").toDouble();*/

        /*if (jConfig.value("fileName").isUndefined() == true)
        {
            return false;
        }
        m_fileName = jConfig.value("fileName").toString();*/

        if (jConfig.value("framesCount").isUndefined() == true)
        {
            return false;
        }
        int framesCount = (int)jConfig.value("framesCount").toDouble();

        for (int v = 0; v < framesCount; v++)
        {
            //ConfigDataItem item;

            QJsonValue jFrameVal = jConfig.value("z_frame_" + QString::number(v));
            if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            QJsonObject jFrame = jFrameVal.toObject();

            if (jFrame.value("frameIndex").isUndefined() == true)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            //item.m_index = (int)jFrame.value("frameIndex").toDouble();

            if (jFrame.value("data").isUndefined() == true || jFrame.value("data").isArray() == false)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            std::vector<quint8> frame;

            QJsonArray array = jFrame.value("data").toArray();
            for (int i = 0; i < array.size(); i++)
            {
                //int v = array[i].toInt();
                //int v = array[i].toInt();
                frame.push_back((int)array[i].toInt());
            }


            m_frames.push_back(frame);
        }

        return true;

    }

    bool ModuleConfFirmware::isEmpty() const
    {
        return m_frames.size() == 0;
    }

	bool ModuleConfFirmware::setData8(int frameIndex, int offset, quint8 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
            offset > frameSize() - sizeof(data))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint8* ptr = static_cast<quint8*>(m_frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

	bool ModuleConfFirmware::setData16(int frameIndex, int offset, quint16 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
            offset > frameSize() - sizeof(data))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint16* ptr = reinterpret_cast<quint16*>(m_frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

	bool ModuleConfFirmware::setData32(int frameIndex, int offset, quint32 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
            offset > frameSize() - sizeof(data))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint32* ptr = reinterpret_cast<quint32*>(m_frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

    bool ModuleConfFirmware::setData64(int frameIndex, int offset, quint64 data)
    {
        if (frameIndex >= static_cast<int>(m_frames.size()) ||
            offset > frameSize() - sizeof(data))
        {
            qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
            return false;
        }

        quint64* ptr = reinterpret_cast<quint64*>(m_frames[frameIndex].data() + offset);
        *ptr = data;

        return true;
    }

    bool ModuleConfFirmware::storeCrc64(int frameIndex, int start, int count, int offset)
    {
        if (frameIndex >= static_cast<int>(m_frames.size()) ||
            offset > frameSize() - sizeof(quint64) || start + count >= frameSize())
        {
            qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
            return false;
        }

        quint64 result = Crc::crc64(m_frames[frameIndex].data() + start, count);
        setData64(frameIndex, offset, result);

        //qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

        return true;
    }

    std::vector<quint8> ModuleConfFirmware::frame(int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= frameCount())
        {
            Q_ASSERT(false);
            return std::vector<quint8>();
        }

        return m_frames[frameIndex];
    }


    QString ModuleConfFirmware::type() const
    {
        return m_type;
    }

    QString ModuleConfFirmware::name() const
	{
		return m_name;
	}

	int ModuleConfFirmware::uartId() const
	{
		return m_uartId;
	}

	int ModuleConfFirmware::frameSize() const
	{
		return m_frameSize;
	}

	int ModuleConfFirmware::frameCount() const
	{
        return static_cast<int>(m_frames.size());
	}


	ModuleConfCollection::ModuleConfCollection()
	{
	}

	ModuleConfCollection::~ModuleConfCollection()
	{
	}

    QObject* ModuleConfCollection::jsGet(QString type, QString name, int uartId, int frameSize, int frameCount)
	{
		bool newFirmware = m_firmwares.count(name) == 0;

		ModuleConfFirmware& fw = m_firmwares["name"];

		if (newFirmware == true)
		{
            fw.init(type, name, uartId, frameSize, frameCount);
		}

		QQmlEngine::setObjectOwnership(&fw, QQmlEngine::ObjectOwnership::CppOwnership);
		return &fw;
	}

    bool ModuleConfCollection:: save(QString projectName, QString userName)
    {
        for (auto i = m_firmwares.begin(); i != m_firmwares.end(); i++)
        {
            ModuleConfFirmware& f = i->second;
            if (f.save(projectName, userName) == false)
            {
                return false;
            }
        }
        return true;
    }


	// ----------------------------------------------------------------------------
	//
	//						ModuleConfigurationValue
	//
	// ----------------------------------------------------------------------------
	const int size_8 = 1;
	const int size_16 = 2;
	const int size_32 = 4;

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

	int ModuleConfigurationValue::typeSize() const
	{
		if (type().startsWith("bool"))
		{
			if (m_bit == 0)
					return 1;

			if ((m_bit % 8) == 0)
				return m_bit / 8;

			return (m_bit / 8) + 1;
		}

		if (type().startsWith("quint8"))
			return size_8;

		if (type().startsWith("quint16"))
			return size_16;

		if (type().startsWith("quint32"))
			return size_32;

		if (type().startsWith("qint8"))
			return size_8;

		if (type().startsWith("qint16"))
			return size_16;

		if (type().startsWith("qint32"))
			return size_32;

		return -1;
	}

	int ModuleConfigurationValue::arraySize() const
	{
		int brOpen = type().indexOf('[');
		int brClose = type().indexOf(']');

		if (brOpen == -1)
			return 1;

		if (brClose == -1)
			return -1;

		QString val = type().mid(brOpen + 1, brClose - brOpen - 1);

		bool ok = false;
		int size = val.toInt(&ok);
		if (ok == false)
			return -1;

		return size;
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

//	const std::shared_ptr<ModuleConfigurationStruct>& ModuleConfigurationVariable::data() const
//	{
//		return m_data;
//	}

//	void ModuleConfigurationVariable::setData(const std::shared_ptr<ModuleConfigurationStruct>& data)
//	{
//		m_data = data;
//	}


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
		m_name.fromStdString(message.name());

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
		message->mutable_name()->assign(m_name.toStdString());

		for (const ModuleConfigurationValue& v : m_userProperties)
		{
			::Proto::ModuleConfigurationValue* pv = message->add_values();
			pv->set_name(v.name().toStdString());
			pv->set_value(v.value().toStdString());
		}

		return;
	}

	bool ModuleConfiguration::compile(McFirmwareOld* dest, const QString& deviceStrId, int changeset, QString* errorString) const
	{
		if (dest == nullptr || errorString == nullptr)
		{
			assert(dest);
			assert(errorString);
			return false;
		}

		errorString->clear();

		if (dest->uartId() != uartId())
		{
			*errorString = tr("UartIds of the destination firmware and source configuration are different. (destination: %1, source: %2)")
						  .arg(dest->uartId())
						  .arg(uartId());
			return false;
		}

		for (const ModuleConfigurationVariable& var : m_variables)
		{
			McDataChunk chunk(deviceStrId, changeset, var.frameIndex());
			bool ok = compileVariable(var, &chunk, errorString);

			if (ok == false)
			{
				return false;
			}

			dest->addChunk(chunk);
		}

		return true;
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

	bool ModuleConfiguration::compileVariable(const ModuleConfigurationVariable& var, McDataChunk* chunk, QString* errorString) const
	{
		if (chunk == nullptr || errorString == nullptr)
		{
			assert(chunk != nullptr);
			assert(errorString != nullptr);
			return false;
		}

		assert(chunk->frameIndex == var.frameIndex());

		QString type = var.type();

		auto foundStruct = std::find_if(m_structures.begin(), m_structures.end(),
				  [&type](const ModuleConfigurationStruct& s)
			{
				return s.name() == type;
			});

		if (foundStruct == m_structures.end())
		{
			*errorString = (tr("Can't find the structure %1.").arg(type));
			return false;
		}

		// count this and included structures' sizes, place them to the map
		//
		QMap<QString, int> structSizeMap;
		countStructureSize(*foundStruct, structSizeMap);

		int size = getStructureSize(*foundStruct, structSizeMap);
		if (size > 0)
		{
			// compile only structures with non-zero size
			//
			qDebug()<<tr("compiling %1, actualsize = %2").arg(foundStruct->name()).arg(size);

			chunk->data.resize(size);
			chunk->data.fill(0);

			if (compileStructure(*foundStruct, chunk, 0, structSizeMap, errorString) == false)
			{
				return false;
			}

			QString s;
			for(int i = 0; i < chunk->data.size(); i++)
			{
				s = s + QString::number((quint8)(chunk->data[i]), 16) + " ";
			}
			qDebug()<<s;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------------------
	//
	bool ModuleConfiguration::compileStructure(const ModuleConfigurationStruct& compileStruct, McDataChunk* chunk, int baseAddress, QMap<QString, int>& structSizeMap, QString* errorString) const
	{
		if (chunk == nullptr || errorString == nullptr)
		{
			assert(chunk != nullptr);
			assert(errorString != nullptr);
			return false;
		}

		auto dataSize = structSizeMap.find(compileStruct.name());
		if (dataSize == structSizeMap.end())
		{
			assert(false);
			*errorString = tr("Structure %1 dataSize is undefined! Compilation stopped.").arg(compileStruct.name());
			return false;
		}

		if (compileStruct.size() != 0 && *dataSize > compileStruct.size())
		{
			*errorString = tr("Structure %1 size is not enough to place all data! Compilation stopped.").arg(compileStruct.name());
			return false;
		}

		int lastAddress = baseAddress;

		for (int m = 0; m < compileStruct.values().size(); m++)
		{
			const ModuleConfigurationValue& val = compileStruct.values()[m];

			QString type = val.type();

			auto foundStruct = std::find_if(m_structures.begin(), m_structures.end(),
					  [&type](const ModuleConfigurationStruct& s)
				{
					return s.name() == type;
				});

			// if value is not a structure, check if it is "macro" value, and decode it
			//
			if (foundStruct == m_structures.end())
			{
				QStringList valuesList;

				if (val.value().startsWith("$(") && val.value().endsWith(")"))
				{
					QString macroValue = val.value();
					macroValue.remove(0, 2);
					macroValue.remove(macroValue.length() - 1, 1);
					macroValue = tr("Configuration\\") + macroValue;

					//
					auto userValue = m_userProperties.find(macroValue);
					if (userValue == m_userProperties.end())
					{
						*errorString = tr("Structure %1, value %2 - user property %3 was not found!.").arg(compileStruct.name()).arg(val.name()).arg(macroValue);
						return false;
					}

					valuesList = userValue->value().split(QRegExp("\\s+"));
				}
				else
				{
					// this is a simple value, not a structure
					//
					valuesList = val.value().split(QRegExp("\\s+"));
				}

				if (val.typeSize() == -1 || val.arraySize() == -1 || val.offset() == -1)
				{
					*errorString = tr("Structure %1, value %2 - typeSize, arraySize or offset can't be -1.").arg(compileStruct.name()).arg(val.name());
					return false;
				}

				if (valuesList.count() != val.arraySize())
				{
					*errorString = tr("Structure %1, value %2 - array size (%3) does not match values count (%4).").arg(compileStruct.name()).arg(val.name()).arg(val.arraySize()).arg(valuesList.count());
					return false;
				}

				QList<int> values;  

				bool ok = false;

				for (int v = 0; v < val.arraySize(); v++)
				{
					if (valuesList[v] == "true")
					{
						values.append(1);
					}
					else
					{
						if (valuesList[v] == "false")
						{
							values.append(0);
						}
						else
						{
							values.append(valuesList[v].toInt(&ok, valuesList[v].startsWith("0x") ? 16 : 10));
							if (ok == false)
							{
								*errorString = tr("Structure %1, value (%2) has incorrect format.").arg(compileStruct.name()).arg(val.arraySize()).arg(valuesList[v]);
								return false;
							}
						}
					}
				}

				int address = baseAddress + val.offset();

				for (int v = 0; v < val.arraySize(); v++)
				{
					if (val.type() == "bool")
					{
						if (values[v] != 0)
						{
							int ofs = val.bit() / 8;
							int bit = val.bit() % 8;

							if (compileStruct.be())
							{
								ofs = val.boolSize() - ofs - 1;
							}

							if (ofs < 0 || ofs >= val.boolSize())
							{
								*errorString = tr("Structure %1, bool value %2: wrong size (%3) and bit (%4).").arg(compileStruct.name()).arg(val.name().arg(val.boolSize()).arg(val.bit()));
								return false;
							}

							assert(chunk->data.size() < address + ofs);
							chunk->data[address + ofs] = chunk->data[address + ofs] | (1 << bit);
						}
					}
					else
					{
						if (address + v * val.typeSize() + val.typeSize() - 1 >= chunk->data.size())
						{
							assert(address + v * val.typeSize() + val.typeSize() - 1 < chunk->data.size());
							*errorString = tr("Structure %1, value %2: address (%3) is out of range.").arg(compileStruct.name()).arg(val.name().arg(val.boolSize()).arg(address + v * 2 + 1));
							return false;
						}

						switch (val.typeSize())
						{
						case size_8:
							chunk->data[address + v] = values[v];
							break;
						case size_16:
							if (compileStruct.be())
							{
								chunk->data[address + v * 2 + 1] = values[v] & 0xff;
								chunk->data[address + v * 2] = (values[v] >> 8) & 0xff;
							}
							else
							{

								chunk->data[address + v * 2] = values[v] & 0xff;
								chunk->data[address + v * 2 + 1] = (values[v] >> 8) & 0xff;
							}
							break;
						case size_32:
							if (compileStruct.be())
							{
								chunk->data[address + v * 4 + 3] = values[v] & 0xff;
								chunk->data[address + v * 4 + 2] = (values[v] >> 8) & 0xff;
								chunk->data[address + v * 4 + 1] = (values[v] >> 16) & 0xff;
								chunk->data[address + v * 4] = (values[v] >> 24) & 0xff;
							}
							else
							{
								chunk->data[address + v * 4] = values[v] & 0xff;
								chunk->data[address + v * 4 + 1] = (values[v] >> 8) & 0xff;
								chunk->data[address + v * 4 + 2] = (values[v] >> 16) & 0xff;
								chunk->data[address + v * 4 + 3] = (values[v] >> 24) & 0xff;
							}
							break;
						default:
							Q_ASSERT(0);
						}
					}
				}

				lastAddress = address + val.typeSize() * val.arraySize();
			}
			else
			{
				// this is a structure
				//
				if (compileStructure(*foundStruct, chunk, lastAddress, structSizeMap, errorString) == false)
				{
					return false;
				}

				lastAddress += getStructureSize(*foundStruct, structSizeMap);
			}
		}
		return true;
	}

	bool ModuleConfiguration::countStructureSize(const ModuleConfigurationStruct& compileStruct, QMap<QString, int>& structSizeMap) const
	{
		// calculate structure's size recursively and place the result to the map
		//
		int size = 0;

		for (int m = 0; m < compileStruct.values().size(); m++)
		{
			const ModuleConfigurationValue& val = compileStruct.values()[m];

			QString type = val.type();

			auto foundStruct = std::find_if(m_structures.begin(), m_structures.end(),
					  [&type](const ModuleConfigurationStruct& s)
				{
					return s.name() == type;
				});

			if (foundStruct == m_structures.end())
			{
				// simple value
				//
				int typeSize = val.typeSize();
				int arraySize = val.arraySize();
				if (typeSize == -1 || arraySize == -1 || val.offset() == -1)
				{
					return false;
				}
				else
				{
					int valMaxAddress = val.offset() + typeSize * arraySize;
					if (size < valMaxAddress)
					{
						size = valMaxAddress;
					}
				}
			}
			else
			{
				// included structure
				//
				if (countStructureSize(*foundStruct, structSizeMap) == false)
				{
					return false;
				}

				size += getStructureSize(*foundStruct, structSizeMap);
			}
		}

		structSizeMap.insert(compileStruct.name(), size);

		return true;
	}

	int ModuleConfiguration::getStructureSize(const ModuleConfigurationStruct& compileStruct, QMap<QString, int>& structSizeMap) const
	{
		// if the "size" parameter is present, return it. Otherwise, return the calculated size
		//
		if (compileStruct.size() != -1)
		{
			return compileStruct.size();
		}

		auto structSize = structSizeMap.find(compileStruct.name());
		if (structSize != structSizeMap.end())
		{
			return *structSize;
		}

		return -1;
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

	//
	// Compiled chunk of module configuration
	//
	McDataChunk::McDataChunk(const QString deviceStrId, int deviceChangeset, int frameIndex) :
		deviceStrId(deviceStrId),
		deviceChangeset(deviceChangeset),
		frameIndex(frameIndex)
	{

	}

	//
	// McFirmware -- Compiled chunk of module configuration
	//
	McFirmwareOld::McFirmwareOld()
	{
	}

	McFirmwareOld::~McFirmwareOld()
	{
	}

	QString McFirmwareOld::name() const
	{
		QString n(m_name);
		return n;
	}

	void McFirmwareOld::setName(const QString& value)
	{
		m_name = value;
	}

	int McFirmwareOld::uartId() const
	{
		return m_uartID;
	}

	void McFirmwareOld::setUartId(int value)
	{
		m_uartID = value;
	}

	int McFirmwareOld::frameSize() const
	{
		return m_frameSize;
	}

	void McFirmwareOld::setFrameSize(int value)
	{
		m_frameSize = value;
	}

	void McFirmwareOld::addChunk(const McDataChunk& chunk)
	{
		m_data.push_back(chunk);
	}
}
