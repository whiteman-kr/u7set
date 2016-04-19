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

<<<<<<< HEAD
	QString addressStr;

	bool result = readStringElement(nameIP, &addressStr);

	if (result == true)
	{
		hostAddress->setAddress(addressStr);
	}

	return result;
=======
	hostAddress->setAddress(xml.attributes().value("IP").toString());
	return true;
>>>>>>> b78d8d6ea2847d5e61f340b715d92187b4cf5263
}
