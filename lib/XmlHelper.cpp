#include "../lib/XmlHelper.h"


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
	m_xmlLocalWriter = new QXmlStreamWriter(data);
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

void XmlWriteHelper::writeIntAttribute(const QString& name, int value)
{
	m_xmlWriter->writeAttribute(name, QString::number(value));
}

void XmlWriteHelper::writeBoolAttribute(const QString& name, bool value)
{
	writeStringAttribute(name, value ? "true" : "false");
}

void XmlWriteHelper::writeUInt64Attribute(const QString& name, quint64 value, bool hex)
{
	if (hex == true)
	{
		QString str;

		str.sprintf("0x%016llX", value);

		writeStringAttribute(name, str);
	}
	else
	{
		QString valueStr = QString::number(static_cast<qulonglong>(value));
		m_xmlWriter->writeAttribute(name, valueStr);
	}
}

void XmlWriteHelper::writeUInt32Attribute(const QString& name, quint32 value, bool hex)
{
	if (hex == true)
	{
		QString str;

		str.sprintf("0x%08uX", value);

		writeStringAttribute(name, str);
	}
	else
	{
		QString valueStr = QString::number(static_cast<ulong>(value));
		m_xmlWriter->writeAttribute(name, valueStr);
	}
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

void XmlWriteHelper::writeHostAddressPort(const QString& nameIP, const QString& namePort,HostAddressPort& hostAddressPort)
{
	writeStringElement(nameIP, hostAddressPort.addressStr());
	writeIntElement(namePort, hostAddressPort.port());
}

void XmlWriteHelper::writeHostAddress(const QString& nameIP, QHostAddress& hostAddress)
{
	writeStringElement(nameIP, hostAddress.toString());
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

bool XmlReadHelper::readIntAttribute(const QString& name, int* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	bool result = false;

	*value = m_xmlReader->attributes().value(name).toInt(&result);

	return result;
}

bool XmlReadHelper::readDoubleAttribute(const QString& name, double* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	bool result = false;

	*value = m_xmlReader->attributes().value(name).toDouble(&result);

	return result;
}

bool XmlReadHelper::readFloatAttribute(const QString& name, float* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	bool result = false;

	*value = m_xmlReader->attributes().value(name).toFloat(&result);

	return result;
}

bool XmlReadHelper::readUInt64Attribute(const QString& name, qulonglong *value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	QString str;

	bool result = true;

	str = m_xmlReader->attributes().value(name).toString();

	*value = str.toULongLong(&result, 0);

	return result;
}

bool XmlReadHelper::readUInt32Attribute(const QString& name, quint32 *value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	QString str;

	bool result = true;

	str = m_xmlReader->attributes().value(name).toString();

	*value = str.toULong(&result, 0);

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

bool XmlReadHelper::readStringAttribute(const QString& name, QString* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->attributes().hasAttribute(name) == false)
	{
		return false;
	}

	*value = m_xmlReader->attributes().value(name).toString();

	return true;
}

bool XmlReadHelper::readStringElement(const QString& elementName, QString* value)
{
	if (value == nullptr)
	{
		assert(false);
		return false;
	}

	if (name() != elementName)
	{
		return false;
	}

	QString str = m_xmlReader->readElementText();

	*value = str;

	//skipCurrentElement();

	return true;
}

bool XmlReadHelper::readIntElement(const QString& elementName, int* value)
{
	if (value == nullptr)
	{
		assert(false);
		return false;
	}

	if (name() != elementName)
	{
		return false;
	}

	QString str = m_xmlReader->readElementText();

	*value = str.toInt();

	//skipCurrentElement();

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

	if (findElement(nameIP) == false)
	{
		return false;
	}

	result &= readStringElement(nameIP, &addressStr);

	if (findElement(namePort) == false)
	{
		return false;
	}

	result &= readIntElement(namePort, &port);

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

	if (findElement(nameIP) == false)
	{
		return false;
	}

	bool result = readStringElement(nameIP, &addressStr);

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

	return false;
}
