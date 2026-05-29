#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "XmlHelper.h"
#include <CommonLib/ConstStrings.h>
#include <CommonLib/HostAddressPort.h>

// -------------------------------------------------------------------------------------
//
// XmlWriteHelper class implementation
//
// -------------------------------------------------------------------------------------

XmlWriteHelper::XmlWriteHelper(QXmlStreamWriter& xmlWriter) :
	m_xmlWriter(&xmlWriter)
{
}

XmlWriteHelper::XmlWriteHelper(QByteArray* data)
{
	TEST_PTR_RETURN(data);

	m_xmlLocalWriter = new QXmlStreamWriter(data);
	m_xmlWriter = m_xmlLocalWriter;
}

XmlWriteHelper::XmlWriteHelper(QString* xmlString)
{
	TEST_PTR_RETURN(xmlString);

	m_xmlLocalWriter = new QXmlStreamWriter(xmlString);
	m_xmlWriter = m_xmlLocalWriter;
}

XmlWriteHelper::~XmlWriteHelper()
{
	if (m_xmlLocalWriter != nullptr)
	{
		delete m_xmlLocalWriter;
	}
}

QXmlStreamWriter* XmlWriteHelper::xmlStreamWriter() const
{
	return m_xmlWriter;
}

void XmlWriteHelper::setAutoFormatting(bool autoFormatting)
{
	m_xmlWriter->setAutoFormatting(autoFormatting);
}

void XmlWriteHelper::writeStartDocument()
{
	m_xmlWriter->writeStartDocument();
}

void XmlWriteHelper::writeEndDocument()
{
	m_xmlWriter->writeEndDocument();
}

void XmlWriteHelper::writeStartElement(const QString& name)
{
	m_xmlWriter->writeStartElement(name);
}

void XmlWriteHelper::writeEndElement()
{
	m_xmlWriter->writeEndElement();
}

void XmlWriteHelper::writeStringAttribute(const QString& name, const QString& value)
{
	m_xmlWriter->writeAttribute(name, value);
}

void XmlWriteHelper::writeStringListAttribute(const QString& name, const QStringList& list)
{
#ifdef QT_DEBUG
	for(const QString& str : list)
	{
		Q_ASSERT(str.contains(Separator::SEMICOLON) == false);
	}
#endif

	writeStringAttribute(name, list.join(Separator::SEMICOLON));
}

void XmlWriteHelper::writeIntAttribute(const QString& name, int value, bool hex)
{
	if (hex == true)
	{
		m_xmlWriter->writeAttribute(name, "0x" + QString::number(value, 16).toUpper());
	}
	else
	{
		m_xmlWriter->writeAttribute(name, QString::number(value));
	}
}

void XmlWriteHelper::writeBoolAttribute(const QString& name, bool value)
{
	writeStringAttribute(name, value ? "true" : "false");
}

void XmlWriteHelper::writeInt64Attribute(const QString& name, qint64 value, bool hex)
{
	QString valueStr;

	if (hex == true)
	{
		valueStr = "0x" + QString::number(static_cast<qulonglong>(value), 16).toUpper();
	}
	else
	{
		valueStr = QString::number(static_cast<qlonglong>(value));
	}

	m_xmlWriter->writeAttribute(name, valueStr);
}

void XmlWriteHelper::writeUInt64Attribute(const QString& name, quint64 value, bool hex)
{
	QString valueStr;

	if (hex == true)
	{
		valueStr = "0x" + QString::number(static_cast<qulonglong>(value), 16).toUpper();
	}
	else
	{
		valueStr = QString::number(static_cast<qulonglong>(value));
	}

	m_xmlWriter->writeAttribute(name, valueStr);
}

void XmlWriteHelper::writeUInt32Attribute(const QString& name, quint32 value, bool hex)
{
	QString valueStr;

	if (hex == true)
	{
		valueStr = "0x" + QString::number(static_cast<ulong>(value), 16).toUpper();
	}
	else
	{
		valueStr = QString::number(static_cast<ulong>(value));
	}

	m_xmlWriter->writeAttribute(name, valueStr);
}

void XmlWriteHelper::writeDoubleAttribute(const QString& name, double value)
{
	writeStringAttribute(name, QString::number(value));
}

void XmlWriteHelper::writeDoubleAttribute(const QString& name, double value, int decimalPlaces)
{
	writeStringAttribute(name, QString::number(value, 'f', decimalPlaces));
}

void XmlWriteHelper::writeFloatAttribute(const QString& name, float value)
{
	writeStringAttribute(name, QString::number(value));
}

void XmlWriteHelper::writeAddress16Attribute(const QString& name, const Address16& addr16)
{
	writeStringAttribute(name, addr16.toString());
}

void XmlWriteHelper::writeIPv4PortAttribute(const QString& name, const HostAddressPort& addr)
{
	writeStringAttribute(name, addr.addressPortStr());
}

void XmlWriteHelper::writeIPv4Attribute(const QString& name, const QHostAddress& addr)
{
	writeStringAttribute(name, addr.toString());
}

void XmlWriteHelper::writeUuidAttribute(const QString& name, const QUuid& guid)
{
	writeStringAttribute(name, guid.toString(QUuid::WithBraces));
}

void XmlWriteHelper::writeString(const QString& str)
{
	m_xmlWriter->writeCharacters(str);
}

void XmlWriteHelper::writeStringElement(const QString& name, const QString& value)
{
	m_xmlWriter->writeTextElement(name, value);
}

void XmlWriteHelper::writeIntElement(const QString& name, int value)
{
	m_xmlWriter->writeTextElement(name, QString::number(value));
}

void XmlWriteHelper::writeBoolElement(const QString& name, bool value)
{
	m_xmlWriter->writeTextElement(name, value == true ? "true" : "false");
}

void XmlWriteHelper::writeHostAddressPort(const QString& nameIP, const QString& namePort, const HostAddressPort& hostAddressPort)
{
	writeStringElement(nameIP, hostAddressPort.addressStr());
	writeIntElement(namePort, hostAddressPort.port());
}

void XmlWriteHelper::writeHostAddress(const QString& nameIP, const QHostAddress& hostAddress)
{
	writeStringElement(nameIP, hostAddress.toString());
}

void XmlWriteHelper::writeQVariantAttribute(const QString& name, const QVariant& qv)
{
	switch(qv.typeId())
	{
	case QMetaType::Type::Bool:
		writeBoolAttribute(name, qv.toBool());
		break;

	case QMetaType::Type::Int:
		writeInt64Attribute(name, qv.toInt());
		break;

	case QMetaType::Type::LongLong:
		writeInt64Attribute(name, qv.toLongLong());
		break;

	case QMetaType::Type::UInt:
		writeUInt64Attribute(name, qv.toUInt());
		break;

	case QMetaType::Type::ULongLong:
		writeUInt64Attribute(name, qv.toULongLong());
		break;

	case QMetaType::Type::Double:
		writeDoubleAttribute(name, qv.toDouble());
		break;

	case QMetaType::Type::QString:
		writeStringAttribute(name, qv.toString());
		break;

	default:
		Q_ASSERT(false);			// writing is not implemented for this QVarian::Type
	}
}



// -------------------------------------------------------------------------------------
//
// XmlReadHelper class implementation
//
// -------------------------------------------------------------------------------------

XmlReadHelper::XmlReadHelper(QXmlStreamReader& xmlReader) :
	m_xmlReader(&xmlReader)
{
}

XmlReadHelper::XmlReadHelper(const QByteArray& data)
{
	m_xmlLocalReader = new QXmlStreamReader(data);
	m_xmlReader = m_xmlLocalReader;
}

XmlReadHelper::XmlReadHelper(const QString& xmlString)
{
	m_xmlLocalReader = new QXmlStreamReader(xmlString);
	m_xmlReader = m_xmlLocalReader;
}

XmlReadHelper::~XmlReadHelper()
{
	if (m_xmlLocalReader != nullptr)
	{
		delete m_xmlLocalReader;
	}
}

QXmlStreamReader* XmlReadHelper::xmlStreamReader() const
{
	return m_xmlReader;
}

bool XmlReadHelper::readNextStartElement()
{
	return m_xmlReader->readNextStartElement();
}

QXmlStreamReader::TokenType XmlReadHelper::readNext()
{
	return m_xmlReader->readNext();
}

void XmlReadHelper::skipCurrentElement()
{
	m_xmlReader->skipCurrentElement();
}

QXmlStreamReader::TokenType XmlReadHelper::tokenType()
{
	return m_xmlReader->tokenType();
}

QString XmlReadHelper::name()
{
	return m_xmlReader->name().toString();
}

bool XmlReadHelper::atEnd()
{
	return m_xmlReader->atEnd();
}

bool XmlReadHelper::readStringAttribute(const QString& name, QString* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QXmlStreamAttributes attrs = m_xmlReader->attributes();

	if (attrs.hasAttribute(name) == false)
	{
		return false;
	}

	*value = attrs.value(name).toString();

	return true;
}

bool XmlReadHelper::readStringListAttribute(const QString& name, QStringList* list)
{
	TEST_PTR_RETURN_FALSE(list);

	QString str;

	if (readStringAttribute(name, &str) == false)
	{
		return false;
	}

	*list = str.split(Separator::SEMICOLON, Qt::SkipEmptyParts);

	return true;
}

bool XmlReadHelper::readIntAttribute(const QString& name, int* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toInt(&result, 0);

	return result;
}

bool XmlReadHelper::readBoolAttribute(const QString& name, bool* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString boolStr;

	bool result = readStringAttribute(name, &boolStr);

	RETURN_IF_FALSE(result);

	boolStr = boolStr.trimmed().toLower();

	if (boolStr == "true" || boolStr == "yes")
	{
		*value = true;
	}
	else
	{
		if (boolStr == "false" || boolStr == "no")
		{
			*value = false;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	return true;
}

bool XmlReadHelper::readInt64Attribute(const QString& name, qint64 *value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toLongLong(&result, 0);

	return result;
}

bool XmlReadHelper::readUInt64Attribute(const QString& name, quint64* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toULongLong(&result, 0);

	return result;
}

bool XmlReadHelper::readUInt32Attribute(const QString& name, quint32* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toULong(&result, 0);

	return result;
}

bool XmlReadHelper::readDoubleAttribute(const QString& name, double* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toDouble(&result);

	return result;
}

bool XmlReadHelper::readFloatAttribute(const QString& name, float* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString str;

	bool result = readStringAttribute(name, &str);

	RETURN_IF_FALSE(result);

	*value = str.toFloat(&result);

	return result;
}

bool XmlReadHelper::readAddress16Attribute(const QString& name, Address16* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString addr16Str;

	bool result = readStringAttribute(name, &addr16Str);

	RETURN_IF_FALSE(result);

	value->fromString(addr16Str, &result);

	return result;
}

bool XmlReadHelper::readIPv4PortAttribute(const QString& name, HostAddressPort* addr)
{
	TEST_PTR_RETURN_FALSE(addr);

	QString addrStr;

	bool result = readStringAttribute(name, &addrStr);

	RETURN_IF_FALSE(result);

	addr->setAddressPortStr(addrStr, 0);

	return true;
}

bool XmlReadHelper::readIPv4Attribute(const QString& name, QHostAddress* addr)
{
	TEST_PTR_RETURN_FALSE(addr);

	QString addrStr;

	bool result = readStringAttribute(name, &addrStr);

	RETURN_IF_FALSE(result);

	return addr->setAddress(addrStr);
}

bool XmlReadHelper::readUuidAttribute(const QString& name, QUuid* guid)
{
	TEST_PTR_RETURN_FALSE(guid);

	QString uuidStr;

	bool result = readStringAttribute(name, &uuidStr);

	RETURN_IF_FALSE(result);

	*guid = QUuid::fromString(uuidStr);

	return !guid->isNull();
}

bool XmlReadHelper::readStringElement(const QString& elementName, QString* value, bool find)
{
	TEST_PTR_RETURN_FALSE(value);

	if (find == true)
	{
		if (findElement(elementName) == false)
		{
			return false;
		}
	}

	if (checkElement(elementName) == false)
	{
		return false;
	}

	*value = m_xmlReader->readElementText();

	return true;
}

bool XmlReadHelper::readIntElement(const QString& elementName, int* value, bool find)
{
	TEST_PTR_RETURN_FALSE(value);

	if (find == true)
	{
		if (findElement(elementName) == false)
		{
			return false;
		}
	}

	if (checkElement(elementName) == false)
	{
		return false;
	}

	QString str = m_xmlReader->readElementText();

	bool ok = true;

	*value = str.toInt(&ok);

	return ok;
}

bool XmlReadHelper::readBoolElement(const QString& elementName, bool* value, bool find)
{
	TEST_PTR_RETURN_FALSE(value);

	if (find == true)
	{
		if (findElement(elementName) == false)
		{
			return false;
		}
	}

	if (checkElement(elementName) == false)
	{
		return false;
	}

	QString str = m_xmlReader->readElementText();

	if (str == "true")
	{
		*value = true;
	}
	else
	{
		if (str == "false")
		{
			*value = false;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	return true;
}

bool XmlReadHelper::readHostAddressPort(const QString& nameIP, const QString& namePort, HostAddressPort* hostAddressPort)
{
	TEST_PTR_RETURN_FALSE(hostAddressPort);

	QString addressStr;
	int port = 0;

	bool result = true;

	result &= readStringElement(nameIP, &addressStr, true);

	result &= readIntElement(namePort, &port, true);

	RETURN_IF_FALSE(result);

	if (addressStr.isEmpty())
	{
		hostAddressPort->clear();
	}
	else
	{
		result &= hostAddressPort->setAddress(addressStr);
		hostAddressPort->setPort(port);
	}

	return result;
}

bool XmlReadHelper::readHostAddress(const QString& nameIP, QHostAddress* hostAddress)
{
	TEST_PTR_RETURN_FALSE(hostAddress);

	QString addressStr;

	bool result = readStringElement(nameIP, &addressStr, true);

	RETURN_IF_FALSE(result);

	return hostAddress->setAddress(addressStr);
}

QString XmlReadHelper::elementText()
{
	return m_xmlReader->readElementText();
}

bool XmlReadHelper::findElement(const QString& elementName)
{
	while(m_xmlReader->atEnd() == false)
	{
		if (m_xmlReader->readNextStartElement() == false)
		{
			continue;
		}

		if (name() == elementName)
		{
			return true;
		}
	}

	return false;
}

bool XmlReadHelper::checkElement(const QString& elementName)
{
	return (name() == elementName);
}

bool XmlReadHelper::readQVariantAttribute(const QString& name, QVariant* qv)
{
	TEST_PTR_RETURN_FALSE(qv);

	bool result = true;

	switch(qv->typeId())
	{
	case QMetaType::Type::Bool:
		{
			bool v = false;
			result = readBoolAttribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::Int:
		{
			int v = 0;
			result = readIntAttribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::LongLong:
		{
			qlonglong v = 0;
			result = readInt64Attribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::UInt:
		{
			uint v = 0;
			result = readUInt32Attribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::ULongLong:
		{
			qulonglong v = 0;
			result = readUInt64Attribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::Double:
		{
			double v = 0;
			result = readDoubleAttribute(name, &v);
			*qv = v;
		}
		break;

	case QMetaType::Type::QString:
		{
			QString v = 0;
			result = readStringAttribute(name, &v);
			*qv = v;
		}
		break;

	default:
		Q_ASSERT(false);			// reading is not implemented for this QVarian::Type
		result = false;
	}

	return result;
}

bool XmlReadHelper::readEnumValueAttributeAsInt(const QString& name, int* value)
{
	TEST_PTR_RETURN_FALSE(value);

	QString attrName = name + QString(XmlAttribute::ENUM_VALUE_SUFFIX);

	return readIntAttribute(attrName, value);;
}

