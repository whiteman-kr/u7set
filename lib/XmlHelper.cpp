#include "../include/XmlHelper.h"


// -------------------------------------------------------------------------------------
//
// XmlWriteHelper class implementation
//
// -------------------------------------------------------------------------------------

XmlWriteHelper::XmlWriteHelper(QXmlStreamWriter& xmlWriter) :
	m_xmlWriter(xmlWriter)
{
}


XmlWriteHelper::XmlWriteHelper(QByteArray* data) :
	m_xmlWriter(*(m_xmlLocalWriter = new QXmlStreamWriter(data)))
{
}


XmlWriteHelper::~XmlWriteHelper()
{
	if (m_xmlLocalWriter != nullptr)
	{
		delete m_xmlLocalWriter;
	}
}


void XmlWriteHelper::setAutoFormatting(bool autoFormatting)
{
	m_xmlWriter.setAutoFormatting(autoFormatting);
}


void XmlWriteHelper::writeStartDocument()
{
	m_xmlWriter.writeStartDocument();
}


void XmlWriteHelper::writeEndDocument()
{
	m_xmlWriter.writeEndDocument();
}


void XmlWriteHelper::writeStartElement(const QString& name)
{
	m_xmlWriter.writeStartElement(name);
}


void XmlWriteHelper::writeEndElement()
{
	m_xmlWriter.writeEndElement();
}


void XmlWriteHelper::writeStringAttribute(const QString& name, const QString& value)
{
	m_xmlWriter.writeAttribute(name, value);
}


void XmlWriteHelper::writeIntAttribute(const QString& name, int value)
{
	m_xmlWriter.writeAttribute(name, QString::number(value));
}


void XmlWriteHelper::writeBoolAttribute(const QString& name, bool value)
{
	writeStringAttribute(name, value ? "true" : "false");
}


void XmlWriteHelper::writeUlongAttribute(const QString& name, ulong value, bool hex)
{
	if (hex == true)
	{
		QString str;

		str.sprintf("0x%08X", value);

		writeStringAttribute(name, str);
	}
	else
	{
		m_xmlWriter.writeAttribute(name, QString::number(value));
	}
}


void XmlWriteHelper::writeDoubleAttribute(const QString& name, double value)
{
	writeStringAttribute(name, QString::number(value));
}


void XmlWriteHelper::writeStringElement(const QString& name, const QString& value)
{
	m_xmlWriter.writeTextElement(name, value);
}


void XmlWriteHelper::writeIntElement(const QString& name, int value)
{
	m_xmlWriter.writeTextElement(name, QString::number(value));
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
	m_xmlReader(xmlReader)
{
}


XmlReadHelper::XmlReadHelper(const QByteArray& data) :
	m_xmlReader(*(m_xmlLocalReader = new QXmlStreamReader(data)))
{
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
	return m_xmlReader.readNextStartElement();
}


void XmlReadHelper::skipCurrentElement()
{
	m_xmlReader.skipCurrentElement();
}


QString XmlReadHelper::name()
{
	return m_xmlReader.name().toString();
}


bool XmlReadHelper::atEnd()
{
	return m_xmlReader.atEnd();
}


bool XmlReadHelper::readIntAttribute(const QString& name, int* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader.attributes().hasAttribute(name) == false)
	{
		return false;
	}

	*value = m_xmlReader.attributes().value(name).toInt();

	return true;
}


bool XmlReadHelper::readDoubleAttribute(const QString& name, double* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader.attributes().hasAttribute(name) == false)
	{
		return false;
	}

	*value = m_xmlReader.attributes().value(name).toDouble();

	return true;
}


bool XmlReadHelper::readUlongAttribute(const QString& name, ulong* value)
{
	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader.attributes().hasAttribute(name) == false)
	{
		return false;
	}

	QString str;

	bool result = true;

	str = m_xmlReader.attributes().value(name).toString();

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

	if (m_xmlReader.attributes().hasAttribute(name) == false)
	{
		return false;
	}

	*value = m_xmlReader.attributes().value(name).toString();

	return true;
}


bool XmlReadHelper::readStringElement(const QString& elementName, QString* value)
{
	if (value == nullptr)
	{
		assert(false);
		return false;
	}

	readNextStartElement();

	if (name() != elementName)
	{
		return false;
	}

	QString str = m_xmlReader.readElementText();

	*value = str;

	return true;
}


bool XmlReadHelper::readIntElement(const QString& elementName, int* value)
{
	if (value == nullptr)
	{
		assert(false);
		return false;
	}

	readNextStartElement();

	if (name() != elementName)
	{
		return false;
	}

	QString str = m_xmlReader.readElementText();

	*value = str.toInt();

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

	result &= readStringElement(nameIP, &addressStr);
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

	bool result = readStringElement(nameIP, &addressStr);

	if (result == true)
	{
		hostAddress->setAddress(addressStr);
	}

	return result;
}


bool XmlReadHelper::findElement(const QString& elementName)
{
	while(m_xmlReader.atEnd() == false)
	{
		if (m_xmlReader.readNextStartElement() == false)
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
