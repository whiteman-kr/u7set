#include "../include/ConfigData.h"

using namespace std;

const int size_8 = 1;
const int size_16 = 2;
const int size_32 = 4;

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigStruct::ConfigStruct():
	m_dataSize(0),
	m_size(0),
	m_be(false)
{
}

ConfigStruct::ConfigStruct(const QString& name, int size, bool be):
m_dataSize(0)
{
	m_name = name;
	m_size = size;
	m_be = be;
}

const QString& ConfigStruct::name() const
{
	return m_name;
}

void ConfigStruct::setName(const QString& name)
{
	m_name = name;
}

int ConfigStruct::size() const
{
	return m_size;
}


void ConfigStruct::setSize(const int& size)
{
	m_size = size;
}

QList <ConfigValue>& ConfigStruct::values()
{
	return m_values;
}

int ConfigStruct::dataSize() const
{
	return m_dataSize;
}

void ConfigStruct::setDataSize(const int& dataSize)
{
	m_dataSize = dataSize;
}

int ConfigStruct::actualSize() const
{
	if (m_size == 0)
		return m_dataSize;

	return m_size;
}

void ConfigStruct::setBe(const bool& be)
{
	m_be = be;
}

bool ConfigStruct::be() const
{
	return m_be;
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigStruct::readStruct(QXmlStreamReader& reader)
{
    // Чтение параметров структуры
    //
    QXmlStreamAttributes attr = reader.attributes();

    if (attr.hasAttribute("name"))
    {
        setName(attr.value("name").toString());
    }

    bool ok = false;
    if (attr.hasAttribute("size"))
    {
        setSize(attr.value("size").toString().toInt(&ok));
    }

    if (attr.hasAttribute("byteorder"))
    {
        setBe(attr.value("byteorder").toString() == "be");
    }

    // Чтение дочерних элементов структуры
    //
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            if (reader.name() == "value")
            {
                ConfigValue value;
                value.readValue(reader);
                values().append(value);
            }
            else
                ConfigConfiguration::skipUnknownElement(reader);
        }
        else
        {
            reader.readNext();
        }
    }
}


void ConfigStruct::writeStruct(QXmlStreamWriter& writer)
{
    writer.writeStartElement("struct");

    writer.writeAttribute("name", name());
    writer.writeAttribute("size", QString::number(size()));
    writer.writeAttribute("byteorder", be() ? "be" : "le");

    for (int i = 0; i < values().size(); i++)
    {
        values()[i].writeValue(writer);
    }

    writer.writeEndElement();
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigConfiguration::ConfigConfiguration():
	m_version(1),
	m_uartID(0),
	m_minFrameSize(0)
{
}

const QString& ConfigConfiguration::name() const
{
	return m_name;
}

void ConfigConfiguration::setName(const QString& name)
{
	m_name = name;
}

int ConfigConfiguration::version() const
{
	return m_version;
}

void ConfigConfiguration::setVersion(const int& version)
{
	m_version = version;
}

int ConfigConfiguration::uartID() const
{
	return m_uartID;
}

void ConfigConfiguration::setUartID(const int& uartID)
{
	m_uartID = uartID;
}

int ConfigConfiguration::minFrameSize() const
{
	return m_minFrameSize;
}

void ConfigConfiguration::setMinFrameSize(const int& minFrameSize)
{
	m_minFrameSize = minFrameSize;
}

QList<ConfigStruct>& ConfigConfiguration::structures()
{
	return m_structures;
}

const QList<ConfigVariable> &ConfigConfiguration::variables() const
{
	return m_variables;
}

QList<ConfigVariable> &ConfigConfiguration::variables()
{
	return m_variables;
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::appendVariableItems(const std::shared_ptr<ConfigStruct> &pData)
{
    if (pData == nullptr)
    {
        Q_ASSERT(pData);
        return;
    }

    int structIndex = getStructureIndexByType(pData->name());
    if (structIndex == -1)
    {
        Q_ASSERT(0);
        return;
    }
    ConfigStruct& str = structures()[structIndex];

    for (int m = 0; m < str.values().size(); m++)
    {
        ConfigValue val(str.values()[m]);

        int childStructIndex = getStructureIndexByType(val.type());
        if (childStructIndex != -1)
        {
            // Вложенная структура
            //
            const ConfigStruct& inStr = structures()[childStructIndex];
            val.setData(std::make_shared<ConfigStruct>(inStr.name(), inStr.size(), inStr.be()));
            appendVariableItems(val.pData());
            pData->setDataSize (pData->dataSize() + val.pData()->actualSize());   // увеличить размер на размер вложенной структуры
        }
        else
        {
            // Простое значение
            //
            int typeSize = val.typeSize();
            int arraySize = val.arraySize();
            if (typeSize == -1 || arraySize == -1)
            {
                QMessageBox::critical(0, QString("Error"), QString("Wrong type description: ") + val.type());
                return;
            }
            else
            {
                // увеличить размер структуры на размер переменной
                //
                int valMaxAddress = val.offset() + typeSize * arraySize;

                if (pData->dataSize() < valMaxAddress)
                    pData->setDataSize(valMaxAddress);
            }
        }

        pData->values().append(val);
    }
}

//----------------------------------------------------------------------------------------------------------
//
int ConfigConfiguration::getStructureIndexByType(const QString& type)
{
    for (int s = 0; s < structures().size(); s++)
    {
        if (structures()[s].name() == type)
            return s;
    }
    return -1;
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::createMembers()
{
    for (int v = 0; v < variables().size(); v++)
    {
        ConfigVariable& var = variables()[v];

        int structIndex = getStructureIndexByType(var.type());
        if (structIndex == -1)
            continue;

        const ConfigStruct& str = structures()[structIndex];
        var.setData(std::make_shared<ConfigStruct>(str.name(), str.size(), str.be()));
        appendVariableItems(var.pData());
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::setVals()
{
    for (int v = 0; v < variables().size(); v++)
    {
        ConfigVariable& var = variables()[v];

        if (var.pData() == nullptr)
        {
            Q_ASSERT(0);
            continue;
        }

        setValsStruct(var.pData(), var.name());
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::setValsStruct(const std::shared_ptr<ConfigStruct>& pData, const QString& valName)
{
    if (pData == nullptr)
    {
        Q_ASSERT(pData);
        return;
    }

    for (int m = 0; m < pData->values().size(); m++)
    {
        ConfigValue& val = pData->values()[m];

        QString paramName = valName + "\\" + val.name();

        if (val.pData() != nullptr)
        {
            setValsStruct(val.pData(), paramName);
        }
        else
        {
            if (m_valMap.find(paramName) != m_valMap.end())
            {
                val.setValue(m_valMap[paramName]);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::getVals()
{
    m_valMap.clear();

    for (int v = 0; v < variables().size(); v++)
    {
        ConfigVariable& var = variables()[v];

        if (var.pData() == nullptr)
        {
            Q_ASSERT(0);
            continue;
        }

        getValsStruct(var.pData(), var.name());
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::getValsStruct(const std::shared_ptr<ConfigStruct>& pData, const QString& valName)
{
    if (pData == nullptr)
    {
        Q_ASSERT(pData);
        return;
    }

    for (int m = 0; m < pData->values().size(); m++)
    {
        ConfigValue& val = pData->values()[m];

        QString paramName = valName + "\\" + val.name();

        if (val.pData() != nullptr)
        {
            getValsStruct(val.pData(), paramName);
        }
        else
        {
            if (m_valMap.find(paramName) == m_valMap.end())
            {
                m_valMap[paramName] = val.value();
            }
            else
            {
                assert(false);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
//
void ConfigConfiguration::readConfiguration(QXmlStreamReader& reader)
{
    bool convertOk = true;
    bool ok = false;

    // Чтение параметров конфигурации
    //
    QXmlStreamAttributes attr = reader.attributes();

    if (attr.hasAttribute("name"))
    {
        setName(attr.value("name").toString());
    }
    if (attr.hasAttribute("version"))
    {
        setVersion(attr.value("version").toString().toInt(&ok));
        if (ok == false)
        {
            convertOk = false;
        }
    }
    if (attr.hasAttribute("uartid"))
    {
        setUartID(attr.value("uartid").toString().toInt(&ok));
        if (ok == false)
        {
            convertOk = false;
        }
    }
    if (attr.hasAttribute("minFrameSize"))
    {
        setMinFrameSize(attr.value("minFrameSize").toString().toInt(&ok));
        if (ok == false)
        {
            convertOk = false;
        }
    }

    if (convertOk == false)
    {
        reader.raiseError(QString("XML convert error"));
        return;
    }

    // чтение дочерних элементов конфигурации
    //
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            if (reader.name() == "declarations")
            {
                readDeclaration(reader);
            }
            else
            {
                if (reader.name() == "definitions")
                    readDefinition(reader);
                else
                    if (reader.name() == "data")
                        readData(reader);
                    else
                        skipUnknownElement(reader);
            }
        }
        else
        {
            reader.readNext();
        }
    }

    createMembers();
    setVals();
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::readDeclaration(QXmlStreamReader& reader)
{
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            if (reader.name() == "struct")
            {
                ConfigStruct configStruct;
                configStruct.readStruct(reader);
                structures().append(configStruct);

            }
            else
                skipUnknownElement(reader);
        }
        else
        {
            reader.readNext();
        }
    }

}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::readDefinition(QXmlStreamReader& reader)
{
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            if (reader.name() == "variable")
            {
                ConfigVariable var;
                var.readVariable(reader);
                variables().append(var);

            }
            else
                skipUnknownElement(reader);
        }
        else
        {
            reader.readNext();
        }
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::readData(QXmlStreamReader& reader)
{
    m_valMap.clear();

    // создать конфигурации перед чтением данных
    //
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            if (reader.name() == "var")
                readVar(reader);
            else
                skipUnknownElement(reader);
        }
        else
        {
            reader.readNext();
        }
    }
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::readVar(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.name() == "var");

    // Чтение параметров Variable
    //
    QString name;
    QString val;

    QXmlStreamAttributes attr = reader.attributes();
    if (attr.hasAttribute("name"))
    {
        name = attr.value("name").toString();
    }

    val = reader.readElementText();

    if (m_valMap.find(name) == m_valMap.end())
    {
        m_valMap[name] = val;
    }
    else
    {
        Q_ASSERT(false);
        QMessageBox::critical(0, QString("Read error"), QString("Duplicate var name: ") + name);

    }

    if (reader.isEndElement())
        reader.readNext();
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigConfiguration::skipUnknownElement(QXmlStreamReader& reader)
{
	qDebug() << Q_FUNC_INFO << ("Unknown tag: ") << reader.name() << endl;

    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }

        if (reader.isStartElement())
        {
            skipUnknownElement(reader);
        }
        else
        {
            reader.readNext();
        }
    }
}

void ConfigConfiguration::writeConfiguration(QXmlStreamWriter& writer)
{
    writer.writeStartElement("configuration");

    writer.writeAttribute("name", name());
    writer.writeAttribute("version", QString::number(version()));
    writer.writeAttribute("uartid", QString::number(uartID()));
    writer.writeAttribute("minFrameSize", QString::number(minFrameSize()));

    writeDeclaration(writer);
    writeDefinition(writer);
    writeData(writer);

    writer.writeEndElement();


}

void ConfigConfiguration::writeDeclaration(QXmlStreamWriter& writer)
{
    writer.writeStartElement("declarations");

    for (int i = 0; i < structures().size(); i++)
    {
        structures()[i].writeStruct(writer);
    }

    writer.writeEndElement();
}

void ConfigConfiguration::writeDefinition(QXmlStreamWriter& writer)
{
    writer.writeStartElement("definitions");

    for (int i = 0; i < variables().size(); i++)
    {
        variables()[i].writeVariable(writer);
    }

    writer.writeEndElement();
}

void ConfigConfiguration::writeData(QXmlStreamWriter& writer)
{
    writer.writeStartElement("data");

    getVals();

    QMapIterator<QString, QString> i(m_valMap);
    while (i.hasNext())
    {

        i.next();
        writer.writeStartElement("var");
        writer.writeAttribute("name", i.key());
        writer.writeCharacters(i.value());
        writer.writeEndElement();
    }

    writer.writeEndElement();
}


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigValue::ConfigValue()
{
}

ConfigValue::~ConfigValue()
{
}

int ConfigValue::typeSize()
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

int ConfigValue::arraySize()
{
	int brOpen = type().indexOf('[');
	int brClose = type().indexOf(']');

	if (brOpen == -1)
		return 1;   // открывающей скобки нет - значит, один операнд

	if (brClose == -1)
		return -1;  // закрывающей скобки нет - ошибка

	QString val = type().mid(brOpen + 1, brClose - brOpen - 1);

	bool ok = false;
	int size = val.toInt(&ok);
	if (ok == false)
		return -1;

	return size;
}

const QString& ConfigValue::name() const
{
	return m_name;
}

void ConfigValue::setName(const QString& name)
{
	m_name = name;
}

const QString& ConfigValue::type() const
{
	return m_type;
}

void ConfigValue::setType(const QString& type)
{
	m_type = type;
}

int ConfigValue::offset() const
{
	return m_offset;
}

void ConfigValue::setOffset(int offset)
{
	m_offset = offset;
}

int ConfigValue::bit() const
{
	return m_bit;
}

void ConfigValue::setBit(int bit)
{
	m_bit = bit;
}

int ConfigValue::boolSize() const
{
	return m_boolSize;
}

void ConfigValue::setBoolSize(int boolSize)
{
	m_boolSize = boolSize;
}

bool ConfigValue::userProperty() const
{
	return m_userProperty;
}

void ConfigValue::setUserProperty(bool value)
{
	m_userProperty = value;
}

const QString& ConfigValue::defaultValue() const
{
	return m_defaultValue;
}

void ConfigValue::setDefaultValue(const QString& defaultValue)
{
	m_defaultValue = defaultValue;
}

const QString& ConfigValue::value() const
{
	return m_value;
}

void ConfigValue::setValue(const QString& value)
{
	m_value = value;
}

const std::shared_ptr<ConfigStruct>& ConfigValue::pData() const
{
	return m_pData;
}

void ConfigValue::setData(const std::shared_ptr<ConfigStruct>& pData)
{
	m_pData = pData;
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigValue::readValue(QXmlStreamReader& reader)
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
        setOffset(attr.value("offset").toString().toInt(&ok));
    }

    if (attr.hasAttribute("bit"))
    {
        setBit(attr.value("bit").toString().toInt(&ok));
    }

    if (attr.hasAttribute("size"))
    {
        setBoolSize(attr.value("size").toString().toInt(&ok));
    }

	if (attr.hasAttribute("user"))
	{
		setUserProperty(attr.value("user").toString().compare("true", Qt::CaseInsensitive));
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

	// Завершение чтения
    //
    reader.readElementText();

    if (reader.isEndElement())
	{
        reader.readNext();
	}
}

void ConfigValue::writeValue(QXmlStreamWriter& writer)
{
    writer.writeStartElement("value");
    writer.writeAttribute("name", name());
    writer.writeAttribute("type", type());
    writer.writeAttribute("offset", QString::number(offset()));
    writer.writeAttribute("bit", QString::number(bit()));
    writer.writeAttribute("size", QString::number(boolSize()));
    writer.writeAttribute("default", defaultValue());
    writer.writeEndElement();
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigVariable::ConfigVariable():
	m_frameIndex(-1)
{
}

ConfigVariable::~ConfigVariable()
{
}

const QString& ConfigVariable::name() const
{
	return m_name;
}

void ConfigVariable::setName(const QString& name)
{
	m_name = name;
}

const QString& ConfigVariable::type() const
{
	return m_type;
}

void ConfigVariable::setType(const QString& type)
{
	m_type = type;
}

int ConfigVariable::frameIndex() const
{
	return m_frameIndex;
}

void ConfigVariable::setFrameIndex(const int& frameIndex)
{
	m_frameIndex = frameIndex;
}

const std::shared_ptr<ConfigStruct>& ConfigVariable::pData() const
{
	return m_pData;
}

void ConfigVariable::setData(const std::shared_ptr<ConfigStruct>& pData)
{
	m_pData = pData;
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigVariable::readVariable(QXmlStreamReader& reader)
{
    Q_ASSERT(reader.name() == "variable");

    // Чтение параметров Variable
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
    if (attr.hasAttribute("frameIndex"))
    {
        setFrameIndex(attr.value("frameIndex").toString().toInt());
    }

    reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();
}

void ConfigVariable::writeVariable(QXmlStreamWriter& writer)
{
    writer.writeStartElement("variable");
    writer.writeAttribute("name", name());
    writer.writeAttribute("type", type());
    writer.writeAttribute("frameIndex", QString::number(frameIndex()));
    writer.writeEndElement();
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigDataReader::ConfigDataReader()
{
}


//------------------------------------------------------------------------------------------------------
//
bool ConfigDataReader::isLoaded() const
{
    return !m_frameData.empty();
}

//------------------------------------------------------------------------------------------------------
//
bool ConfigDataReader::load(const QString& fileName)
{
	QFile file(fileName);

	bool ok = file.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		return false;
	}

	QByteArray data;
	data = file.readAll();

	ok = load(data);
	return ok;
}

//------------------------------------------------------------------------------------------------------
//
bool ConfigDataReader::load(const QByteArray& data)
{
    m_frameData.clear();

    QJsonDocument document = QJsonDocument::fromJson(data);

    if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
    {
        return false;
    }

    QJsonObject object = document.object();

    int configNo = 0;
    QJsonValue jConfigVal = object.value("config" + QString::number(configNo));
    if (jConfigVal.isUndefined() == true || jConfigVal.isObject() == false)
    {
        return false;
    }

    QJsonObject jConfig = jConfigVal.toObject();

    if (jConfig.value("name").isUndefined() == true)
    {
        return false;
    }
    m_name = jConfig.value("name").toString();

    if (jConfig.value("version").isUndefined() == true)
    {
        return false;
    }
    m_version = (int)jConfig.value("version").toDouble();

    if (jConfig.value("uartID").isUndefined() == true)
    {
        return false;
    }
    m_uartID = (int)jConfig.value("uartID").toDouble();

    if (jConfig.value("minFrameSize").isUndefined() == true)
    {
        return false;
    }
    m_minFrameSize = (int)jConfig.value("minFrameSize").toDouble();

    if (jConfig.value("changeset").isUndefined() == true)
    {
        return false;
    }
    m_changeset = (int)jConfig.value("changeset").toDouble();

    if (jConfig.value("fileName").isUndefined() == true)
    {
        return false;
    }
    m_fileName = jConfig.value("fileName").toString();

    if (jConfig.value("framesCount").isUndefined() == true)
    {
        return false;
    }
    int framesCount = (int)jConfig.value("framesCount").toDouble();

    for (int v = 0; v < framesCount; v++)
    {
        ConfigDataItem item;

        QJsonValue jFrameVal = jConfig.value("frame" + QString::number(v));
        if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
        {
            assert(false);

            m_frameData.clear();
            return false;
        }

        QJsonObject jFrame = jFrameVal.toObject();

        if (jFrame.value("frameIndex").isUndefined() == true)
        {
            assert(false);

            m_frameData.clear();
            return false;
        }

        item.m_index = (int)jFrame.value("frameIndex").toDouble();

        if (jFrame.value("data").isUndefined() == true || jFrame.value("data").isArray() == false)
        {
            assert(false);

            m_frameData.clear();
            return false;
        }


        QJsonArray array = jFrame.value("data").toArray();
        for (int i = 0; i < array.size(); i++)
        {
			//int v = array[i].toInt();
			//int v = array[i].toInt();
            item.m_data.push_back((int)array[i].toDouble());
        }


        m_frameData.push_back(item);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------
//
const QString& ConfigDataReader::name() const
{
    return m_name;
}

//------------------------------------------------------------------------------------------------------
//
const QString& ConfigDataReader::fileName() const
{
    return m_fileName;
}

//------------------------------------------------------------------------------------------------------
//
int ConfigDataReader::version() const
{
    return m_version;
}

//------------------------------------------------------------------------------------------------------
//
int ConfigDataReader::uartID() const
{
    return m_uartID;
}

//------------------------------------------------------------------------------------------------------
//
int ConfigDataReader::minFrameSize() const
{
    return m_minFrameSize;
}

//------------------------------------------------------------------------------------------------------
//
int ConfigDataReader::framesCount() const
{
    return (int)m_frameData.size();
}

int ConfigDataReader::changeset() const
{
    return m_changeset;
}


//------------------------------------------------------------------------------------------------------
//
std::vector<int> ConfigDataReader::getFramesNo() const
{
    std::vector<int> result;

    for (int v = 0; v < (int)m_frameData.size(); v++)
    {
        result.push_back(m_frameData[v].m_index);
    }

    return result;
}

//------------------------------------------------------------------------------------------------------
//
const std::vector<uint8_t>& ConfigDataReader::frameData(int frameIndex) const
{
    for (int v = 0; v < (int)m_frameData.size(); v++)
    {
        if (m_frameData[v].m_index == frameIndex)
        {
            return m_frameData[v].m_data;
        }
    }

    assert(false);
    static std::vector<uint8_t> emptyResult;
    return emptyResult;
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::readConfig(const QByteArray& data)
{
	if (data.isEmpty() == true)
	{
		assert(data.isEmpty() == false);
		return false;
	}

	QXmlStreamReader reader(data);

	m_configurations.clear();

	reader.readNext();
	while (!reader.atEnd())
	{
		if (reader.isStartElement())
		{
			if (reader.name() == "configuration")
			{
                ConfigConfiguration config;
                config.readConfiguration(reader);
                m_configurations.append(config);
			}
			else
			{
				reader.raiseError(tr("The root element is not named \"configuration!\""));
			}
		}
		else
		{
			reader.readNext();
		}
	}

	if (reader.hasError())
	{
		QMessageBox::critical(0, tr("Error"), reader.errorString());
		return false;
	}

	return true;

}

bool ConfigData::saveConfig(QByteArray& data)
{
    QXmlStreamWriter writer(&data);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    for (int i = 0; i < m_configurations.size(); i++)
    {
        m_configurations[i].writeConfiguration(writer);
    }

    writer.writeEndDocument();

    if (writer.hasError())
    {
        QMessageBox::critical(0, tr("Error!"), tr("XML Write Error!"));
        return false;
    }

    return true;
}

bool ConfigData::load(const DbFile& file)
{
    m_fileInfo = file;

    bool result = readConfig(file.data());
	return result;
}

bool ConfigData::save(DbFile* file)
{
    if (file == nullptr)
    {
        assert(file);
        return false;
    }

    *file = m_fileInfo;

	QByteArray data;

    bool ok = saveConfig(data);
	assert(ok == true);

    QString str(data);

	file->swapData(data);
	return file;
}
/*
bool ConfigData::saveData(const QString& fileName) const
{
	QSettings settings(fileName, QSettings::IniFormat);

	for (int c = 0; c < m_configurations.size(); c++)
	{
        const ConfigConfiguration& config = m_configurations[c];

		settings.beginGroup(config.name());

		for (int v = 0; v < config.variables().size(); v++)
		{
            const ConfigVariable& var = config.variables()[v];
			if (var.pData() == nullptr)
			{
				Q_ASSERT(0);
				continue;
			}

			settings.beginGroup(var.name());

			saveStructure(var.pData(), settings);

			settings.endGroup();
		}

		settings.endGroup();
	}

	return true;
}*/
/*
//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::saveData(QByteArray &data) const
{
	// Save Document
	//
	QJsonObject jObject;

	for (int c = 0; c < m_configurations.size(); c++)
	{
		const ConfigConfiguration& config = m_configurations[c];

		QJsonObject jConfig;

		for (int v = 0; v < config.variables().size(); v++)
		{
			const ConfigVariable& var = config.variables()[v];
			if (var.pData() == nullptr)
			{
				Q_ASSERT(0);
				continue;
			}

			QJsonObject jVar;
			saveStructure(var.pData(), jVar);
			jConfig.insert(var.name(), jVar);
		}

		jObject.insert(config.name(), jConfig);
	}

	data = QJsonDocument(jObject).toJson();

	return true;
}
*/
//----------------------------------------------------------------------------------------------------------
//
/*
void ConfigData::saveStructure(const std::shared_ptr<ConfigStruct> &pData, QSettings& settings) const
{
	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return;
	}

	for (int m = 0; m < pData->values().size(); m++)
	{
		ConfigValue& val = pData->values()[m];

		if (val.pData() != nullptr)
		{
			settings.beginGroup(val.name());
			saveStructure(val.pData(), settings);
			settings.endGroup();
		}
		else
		{
			settings.setValue(val.name(), val.value());
		}
	}
}
*/
//----------------------------------------------------------------------------------------------------------
//
/*
void ConfigData::saveStructure(const std::shared_ptr<ConfigStruct> &pData, QJsonObject& jVar) const
{
	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return;
	}

	for (int m = 0; m < pData->values().size(); m++)
	{
		ConfigValue& val = pData->values()[m];

		if (val.pData() != nullptr)
		{
			QJsonObject jVal;
			saveStructure(val.pData(), jVal);
			jVar.insert(val.name(), jVal);
		}
		else
		{
			jVar.insert(val.name(), val.value());
		}
	}
}
*/
/*
//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::loadData(const QString& fileName)
{
	QSettings settings(fileName, QSettings::IniFormat);

	for (int c = 0; c < m_configurations.size(); c++)
	{
		ConfigConfiguration& config = m_configurations[c];

		settings.beginGroup(config.name());

		for (int v = 0; v < config.variables().size(); v++)
		{
			ConfigVariable& var = config.variables()[v];

			if (var.pData() == nullptr)
			{
				Q_ASSERT(0);
				continue;
			}

			settings.beginGroup(var.name());

			loadStructure(var.pData(), settings);

			settings.endGroup();
		}

		settings.endGroup();
	}

	return true;
}
*/
/*
//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::loadData(const QByteArray& data)
{
	QJsonDocument document = QJsonDocument::fromJson(data);

	if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
		return false;

	///I WANT TO READ  "descriptionFileName" FROM THE FILE

	QJsonObject object = document.object();

	for (int c = 0; c < m_configurations.size(); c++)
	{
		ConfigConfiguration& config = m_configurations[c];

		QJsonValue jConfigVal = object.value(config.name());
		if (jConfigVal.isUndefined() == true || jConfigVal.isObject() == false)
			continue;

		QJsonObject jConfig = jConfigVal.toObject();

		for (int v = 0; v < config.variables().size(); v++)
		{
			ConfigVariable& var = config.variables()[v];

			if (var.pData() == nullptr)
			{
				Q_ASSERT(0);
				continue;
			}

			QJsonValue jVar = jConfig.value(var.name());
			if (jVar.isUndefined() == true || jVar.isObject() == false)
				continue;

			loadStructure(var.pData(), jVar.toObject());

		}
	}

	return true;

}
*/
//----------------------------------------------------------------------------------------------------------
//
/*
void ConfigData::loadStructure(const std::shared_ptr<ConfigStruct>& pData, QSettings& settings)
{
	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return;
	}

	for (int m = 0; m < pData->values().size(); m++)
	{
		ConfigValue& val = pData->values()[m];

		if (val.pData() != nullptr)
		{
			settings.beginGroup(val.name());
			loadStructure(val.pData(), settings);
			settings.endGroup();
		}
		else
		{
			val.setValue(settings.value(val.name(), val.defaultValue()).toString());
		}
	}
}

//----------------------------------------------------------------------------------------------------------
//
void ConfigData::loadStructure(const std::shared_ptr<ConfigStruct>& pData, QJsonObject jVar)
{
	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return;
	}

	for (int m = 0; m < pData->values().size(); m++)
	{
		ConfigValue& val = pData->values()[m];

		if (val.pData() != nullptr)
		{
			QJsonValue jVal = jVar.value(val.name());
			if (jVal.isUndefined() == true || jVal.isObject() == false)
				continue;

			loadStructure(val.pData(), jVal.toObject());
		}
		else
		{
			QJsonValue jVal = jVar.value(val.name());
			if (jVal.isUndefined() == true)
			{
				val.setValue(val.defaultValue());
			}
			else
			{
				val.setValue(jVal.toString());
			}
		}
	}
}
*/
/*
//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::compile(const QString& fileName) const
{
	QSettings settings(fileName, QSettings::IniFormat);

	settings.setValue("configCount", m_configurations.size());

	for (int c = 0; c < m_configurations.size(); c++)
	{
		settings.beginGroup("config" + QString().number(c));

        const ConfigConfiguration& config = m_configurations[c];

		// подсчитать количество разных frameIndex'ов
		QList<int> frames;
		for (int v = 0; v < config.variables().size(); v++)
		{
            const ConfigVariable& var = config.variables()[v];
			if (frames.contains(var.frameIndex()) == false)
				frames.append(var.frameIndex());
		}

		settings.setValue("name", config.name());
		settings.setValue("version", config.version());
		settings.setValue("fileName", config.fileName());
		settings.setValue("uartID", config.uartID());
		settings.setValue("minFrameSize", config.minFrameSize());
        settings.setValue("changeset", config.changeset());
        settings.setValue("framesCount", frames.count());

		for (int f = 0; f < frames.count(); f++)
		{
			// ищем конфигурации, у которых frameIndex = frames[f]
			//
			settings.beginGroup("frame" + QString().number(f));

			QByteArray frameArray;

			for (int v = 0; v < config.variables().size(); v++)
			{
                const ConfigVariable& var = config.variables()[v];

				if (var.pData() == nullptr)
				{
					Q_ASSERT(0);
					continue;
				}

				if (var.frameIndex() != frames[f])
					continue;

				if (var.pData()->size() != 0 && var.pData()->dataSize() > var.pData()->size())
				{
					QMessageBox::critical(0, tr("Compile error"), tr("Wrong size, variable ") + var.name() + tr(" has wrong size! Compilation stopped."));
					return false;
				}

				QByteArray array(var.pData()->actualSize(), 0);

				if (compileStructure(var.pData(), array, 0) == false)
					return false;

				frameArray.append(array);
			}

			settings.setValue("frameIndex", QString().number(frames[f]));
			settings.setValue("data", QVariant(frameArray));

			settings.endGroup();
		}

		settings.endGroup();
	}

	return true;
}*/

//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::compile(QByteArray& data) const
{
	QJsonObject jObject;

	jObject.insert("configCount", m_configurations.size());

	for (int c = 0; c < m_configurations.size(); c++)
	{
		QJsonObject jConfig;

        const ConfigConfiguration& config = m_configurations[c];

		// подсчитать количество разных frameIndex'ов
		QList<int> frames;
		for (int v = 0; v < config.variables().size(); v++)
		{
            const ConfigVariable& var = config.variables()[v];
			if (frames.contains(var.frameIndex()) == false)
				frames.append(var.frameIndex());
		}

		jConfig.insert("name", config.name());
		jConfig.insert("version", config.version());
		jConfig.insert("fileName", m_fileInfo.fileName());
		jConfig.insert("uartID", config.uartID());
		jConfig.insert("minFrameSize", config.minFrameSize());
		jConfig.insert("changeset", m_fileInfo.changeset());
		jConfig.insert("framesCount", frames.count());

		for (int f = 0; f < frames.count(); f++)
		{
			// ищем конфигурации, у которых frameIndex = frames[f]
			//
			QJsonObject jFrame;

			QByteArray frameArray;

			for (int v = 0; v < config.variables().size(); v++)
			{
                const ConfigVariable& var = config.variables()[v];

				if (var.pData() == nullptr)
				{
					Q_ASSERT(0);
					continue;
				}

				if (var.frameIndex() != frames[f])
					continue;

				if (var.pData()->size() != 0 && var.pData()->dataSize() > var.pData()->size())
				{
					QMessageBox::critical(0, tr("Compile error"), tr("Wrong size, variable ") + var.name() + tr(" has wrong size! Compilation stopped."));
					return false;
				}

				QByteArray array(var.pData()->actualSize(), 0);

				if (compileStructure(var.pData(), array, 0) == false)
					return false;

				frameArray.append(array);
			}

			jFrame.insert("frameIndex", frames[f]);

			QJsonArray array;
			for (int i = 0; i < frameArray.size(); i++)
				array.push_back(QJsonValue(frameArray[i]));
			jFrame.insert("data", array);

			jConfig.insert("frame" + QString().number(f), jFrame);
		}

		jObject.insert("config" + QString().number(c), jConfig);
	}

	data = QJsonDocument(jObject).toJson();

	return true;
}

//----------------------------------------------------------------------------------------------------------
//
bool ConfigData::compileStructure(const std::shared_ptr<ConfigStruct> &pData, QByteArray& array, int baseAddress) const
{
	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return false;
	}

	int lastAddress = baseAddress;

	for (int m = 0; m < pData->values().size(); m++)
	{
		ConfigValue& val = pData->values()[m];

		if (val.pData() != nullptr)
		{
			if (val.pData()->size() != 0 && val.pData()->dataSize() > val.pData()->size())
			{
				QMessageBox::critical(0, tr("Compile error"), tr("Variable ") + val.name() + tr(" has wrong size! Compilation stopped."));
				return false;
			}

			if (compileStructure(val.pData(), array, lastAddress) == false)
				return false;

			lastAddress += val.pData()->actualSize();
		}
		else
		{
			// расшифровываем значение переменной в пакет
			//
			QStringList valuesList = val.value().split(QRegExp("\\s+"));

			if (valuesList.count() != val.arraySize())
			{
				Q_ASSERT(0);
				return false;
			}

			QList<int> values;  // значения, переведенные из строки в число

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
							Q_ASSERT(0);
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

						if (pData->be())
						{
							ofs = val.boolSize() - ofs - 1;
						}

						if (ofs < 0 || ofs >= val.boolSize())
						{
							Q_ASSERT(0);
							QMessageBox::critical(0, "Error", val.name() + tr(": wrong size (") + QString::number(val.boolSize()) + tr(") and bit") + QString::number(val.bit()));
							return false;
						}

						array[address + ofs] = array[address + ofs] | (1 << bit);
					}
				}
				else
				{
					switch (val.typeSize())
					{
					case size_8:
						array[address + v] = values[v];
						break;
					case size_16:
						if (pData->be())
						{
							array[address + v * 2 + 1] = values[v] & 0xff;
							array[address + v * 2] = (values[v] >> 8) & 0xff;
						}
						else
						{
							array[address + v * 2] = values[v] & 0xff;
							array[address + v * 2 + 1] = (values[v] >> 8) & 0xff;
						}
						break;
					case size_32:
						if (pData->be())
						{
							array[address + v * 4 + 3] = values[v] & 0xff;
							array[address + v * 4 + 2] = (values[v] >> 8) & 0xff;
							array[address + v * 4 + 1] = (values[v] >> 16) & 0xff;
							array[address + v * 4] = (values[v] >> 24) & 0xff;
						}
						else
						{
							array[address + v * 4] = values[v] & 0xff;
							array[address + v * 4 + 1] = (values[v] >> 8) & 0xff;
							array[address + v * 4 + 2] = (values[v] >> 16) & 0xff;
							array[address + v * 4 + 3] = (values[v] >> 24) & 0xff;
						}
						break;
					default:
						Q_ASSERT(0);
					}
				}
			}

			lastAddress = address + val.typeSize() * val.arraySize();
		}
	}
	return true;
}

bool ConfigData::compile(ConfigDataReader* reader) const
{
    if (reader == nullptr)
    {
        assert(reader);
        return false;
    }

    QByteArray array;
    if (compile(array) == false)
        return false;

    if (reader->load(array) == false)
        return false;

    return true;
}

QList<ConfigConfiguration>& ConfigData::configurations()
{
	return m_configurations;
}

void ConfigData::setFileName(const QString& fileName)
{
	m_fileInfo.setFileName(fileName);
	return;
}

const DbFileInfo& ConfigData::fileInfo() const
{
	return m_fileInfo;
}



//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//
ConfigDataModelNode::ConfigDataModelNode()
{
    parent = nullptr;
    type = Root;
}

ConfigDataModelNode::ConfigDataModelNode(const std::shared_ptr<ConfigConfiguration>& config, ConfigDataModelNode* parent)
{
    this->parent = parent;

    object = config;
    type = Configuration;
}

ConfigDataModelNode::ConfigDataModelNode(const std::shared_ptr<ConfigVariable>& variable, ConfigDataModelNode* parent)
{
    this->parent = parent;

    object = variable;

    type = Variable;

}

ConfigDataModelNode::ConfigDataModelNode(const std::shared_ptr<ConfigValue>& value, ConfigDataModelNode* parent)
{
    this->parent = parent;

    object = value;

    type = Value;

}

ConfigDataModelNode::~ConfigDataModelNode()
{
    qDeleteAll(children);
}

ConfigDataModel::ConfigDataModel(QObject* parent)
    :QAbstractItemModel(parent)
{
    rootNode = nullptr;
}

ConfigDataModel::~ConfigDataModel()
{
    if (rootNode != nullptr)
    {
        delete rootNode;
    }
}

ConfigDataModelNode* ConfigDataModel::nodeFromIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<ConfigDataModelNode*>(index.internalPointer());
    }
    return rootNode;

}

QModelIndex	ConfigDataModel::index(int row, int column, const QModelIndex & parent) const
{

    if (rootNode == nullptr || row < 0 || column < 0)
        return QModelIndex();
    ConfigDataModelNode* parentNode = nodeFromIndex(parent);
    ConfigDataModelNode* childNode = parentNode->children.value(row);
    if (childNode == nullptr)
        return QModelIndex();
    return createIndex(row, column, childNode);

}

QModelIndex	ConfigDataModel::parent(const QModelIndex & index) const
{
    ConfigDataModelNode* node = nodeFromIndex(index);
    if (node == nullptr)
        return QModelIndex();

    ConfigDataModelNode* parentNode = node->parent;
    if (parentNode == nullptr)
        return QModelIndex();

    ConfigDataModelNode* grandParentNode = parentNode->parent;
    if (grandParentNode == nullptr)
        return QModelIndex();

    int row = grandParentNode->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);

}

int	ConfigDataModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;
    ConfigDataModelNode* parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return 0;
    return parentNode->children.count();
}

int	ConfigDataModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 8;
}

QVariant ConfigDataModel::data(const QModelIndex & index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    ConfigDataModelNode* node = nodeFromIndex(index);
    if (node == nullptr)
        return QVariant();

    int column = index.column();

    if (node->type == ConfigDataModelNode::Configuration)
    {
        if (column == 0)
        {
            ConfigConfiguration* config = static_cast<ConfigConfiguration*>(node->object.get());
            return config->name();
        }
    }

    if (node->type == ConfigDataModelNode::Variable)
    {
        ConfigVariable* var = static_cast<ConfigVariable*>(node->object.get());
        if (var == nullptr)
        {
            assert(var);
            return QVariant();
        }

        if (column == 0)
        {
            return var->name();
        }
        if (column == 1)
        {
            return var->type();
        }
        if (column == 2)
        {
            return var->frameIndex();
        }
        if (column == 3)
        {
            return var->pData()->actualSize();
        }
    }

    if (node->type == ConfigDataModelNode::Value)
    {
        ConfigValue* val = static_cast<ConfigValue*>(node->object.get());
        if (val == nullptr)
        {
            assert(val);
            return QVariant();
        }

        if (column == 0)
        {
            return val->name();
        }

        if (column == 1)
        {
            return val->type();
        }

        if (val->pData() != nullptr)
        {
            if (column == 3)
            {
                return val->pData()->actualSize();
            }
        }
        else
        {
            if (column == 3)
            {
                int typeSize = val->typeSize();
                int arraySize = val->arraySize();
                if (typeSize == -1 || arraySize == -1)
                {
                    return tr("???");
                }
                else
                {
                    return QString().number(typeSize * arraySize);
                }
            }

            if (column == 4)
            {
                return val->offset();
            }

            if (column == 5)
            {
                if (val->bit() != -1)
                {
                    return val->bit();
                }
            }

            if (column == 6)
            {
                return val->defaultValue();
            }

            if (column == 7)
            {
                return val->value();
            }
        }
    }

    return QVariant();

}

QVariant ConfigDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
    case 0: return tr("Name");
    case 1: return tr("Type");
    case 2: return tr("FrameIndex");
    case 3: return tr("Size");
    case 4: return tr("Offset");
    case 5: return tr("Bit");
    case 6: return tr("Default");
    case 7: return tr("Value");
    }

    return QVariant();

}

void ConfigDataModel::setRootNode(ConfigDataModelNode* rootNode)
{
    beginResetModel();

    if (this->rootNode != nullptr)
    {
        delete this->rootNode;
    }
    this->rootNode = rootNode;

    endResetModel();
}
