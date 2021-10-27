#ifndef UTILS_LIB_DOMAIN
#error Don't include this file in the project! Link UtilsLib instead.
#endif

#include "XmlHelper.h"
#include "../lib/ConstStrings.h"
#include "../UtilsLib/WUtils.h"
#include "../CommonLib/HostAddressPort.h"

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

void XmlWriteHelper::writeSoftwareTypeAttribute(E::SoftwareType swType)
{
	writeStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, E::valueToString<E::SoftwareType>(swType));
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

bool XmlReadHelper::readNextStartElement()
{
	return m_xmlReader->readNextStartElement();
}

void XmlReadHelper::skipCurrentElement()
{
	m_xmlReader->skipCurrentElement();
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
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QXmlStreamAttributes attrs = m_xmlReader->attributes();

	if (attrs.hasAttribute(name) == false)
	{
		return false;
	}

	*value = attrs.value(name).toString();

	return true;
}

bool XmlReadHelper::readIntAttribute(const QString& name, int* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toInt(&result, 0);

	return result;
}

bool XmlReadHelper::readBoolAttribute(const QString& name, bool* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString boolStr;

	bool result = readStringAttribute(name, &boolStr);

	if (result == false)
	{
		return false;
	}

	boolStr = boolStr.trimmed().toLower();

	if (boolStr == "true")
	{
		*value = true;
	}
	else
	{
		if (boolStr == "false")
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

bool XmlReadHelper::readInt64Attribute(const QString& name, qlonglong *value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toLongLong(&result, 0);

	return result;
}

bool XmlReadHelper::readUInt64Attribute(const QString& name, qulonglong *value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toULongLong(&result, 0);

	return result;
}

bool XmlReadHelper::readUInt32Attribute(const QString& name, quint32* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toULong(&result, 0);

	return result;
}

bool XmlReadHelper::readDoubleAttribute(const QString& name, double* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toDouble(&result);

	return result;
}

bool XmlReadHelper::readFloatAttribute(const QString& name, float* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;

	bool result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = str.toFloat(&result);

	return result;
}

bool XmlReadHelper::readAddress16Attribute(const QString& name, Address16* value)
{
	QString addr16Str;

	bool result = readStringAttribute(name, &addr16Str);

	if (result == false)
	{
		return false;
	}

	value->fromString(addr16Str, &result);

	return result;
}

bool XmlReadHelper::readSoftwareTypeAttribute(E::SoftwareType* swType)
{
	TEST_PTR_RETURN_FALSE(swType);

	QString swTypeStr;

	bool result = readStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, &swTypeStr);

	if (result == false)
	{
		return false;
	}

	bool ok = true;

	*swType = E::stringToValue<E::SoftwareType>(swTypeStr, &ok);

	if (ok == false)
	{
		*swType = E::SoftwareType::Unknown;
		return false;
	}

	return true;
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

	QString str = m_xmlReader->readElementText();

	*value = str;

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
	if (hostAddressPort == nullptr)
	{
		assert(false);
		return false;
	}

	QString addressStr;
	int port = 0;

	bool result = true;

	result &= readStringElement(nameIP, &addressStr, true);

	result &= readIntElement(namePort, &port, true);

	if (result == true)
	{
		hostAddressPort->setAddress(addressStr);
		hostAddressPort->setPort(port);
	}

	return result;
}

bool XmlReadHelper::readHostAddress(const QString& nameIP, QHostAddress *hostAddress)
{
	if (hostAddress == nullptr)
	{
		assert(false);
		return false;
	}

	QString addressStr;

	bool result = readStringElement(nameIP, &addressStr, true);

	if (result == true)
	{
		hostAddress->setAddress(addressStr);
	}

	return result;
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

//	qDebug() << "XmlReadHelper: element is not found -" << elementName;

	return false;
}

bool XmlReadHelper::checkElement(const QString& elementName)
{
	if (name() == elementName)
	{
		return true;
	}

	qDebug() << "XmlReadHelper: element does not match. Current - " << name() << ", required -" << elementName;

	return false;
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
			readBoolAttribute(name, &v);
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
		Q_ASSERT(false);			// writing is not implemented for this QVarian::Type
	}

	return result;
}

