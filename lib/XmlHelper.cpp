#include "../include/XmlHelper.h"



XmlHelper::XmlHelper(QXmlStreamWriter& xmlWriter) :
	m_xmlWriter(&xmlWriter)
{
}


XmlHelper::XmlHelper(QXmlStreamReader& xmlReader) :
	m_xmlReader(&xmlReader)
{
}


void XmlHelper::writeHostAddressPortElement(const QString& name, HostAddressPort& hostAddressPort)
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeStartElement(name);

	m_xmlWriter->writeAttribute("IP", hostAddressPort.addressStr());
	m_xmlWriter->writeAttribute("Port", QString::number(hostAddressPort.port()));

	m_xmlWriter->writeEndElement();
}


bool XmlHelper::readHostAddressPortElement(const QString& name, HostAddressPort* hostAddressPort)
{
	if (m_xmlReader == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->name() != name)
	{
		return false;
	}

	if (hostAddressPort == nullptr)
	{
		assert(false);
		return false;
	}

	hostAddressPort->setAddress(m_xmlReader->attributes().value("IP").toString());
	hostAddressPort->setPort(m_xmlReader->attributes().value("Port").toInt());

	m_xmlReader->skipCurrentElement();

	return true;
}


void XmlHelper::writeHostAddressElement(const QString& name, QHostAddress& hostAddress)
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeStartElement(name);
	m_xmlWriter->writeAttribute("IP", hostAddress.toString());
	m_xmlWriter->writeEndElement();
}


bool XmlHelper::readHostAddressElement(const QString& name, QHostAddress *hostAddress)
{
	if (m_xmlReader == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_xmlReader->name() != name)
	{
		return false;
	}

	if (hostAddress == nullptr)
	{
		assert(false);
		return false;
	}

	hostAddress->setAddress(m_xmlReader->attributes().value("IP").toString());

	m_xmlReader->skipCurrentElement();

	return true;
}


void XmlHelper::writeStartElement(const QString& name)
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeStartElement(name);
}


void XmlHelper::writeEndElement()
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeEndElement();
}


void XmlHelper::writeAttribute(const QString& name, const QString& value)
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeAttribute(name, value);
}


void XmlHelper::writeTextElement(const QString& name, const QString& value)
{
	if (m_xmlWriter == nullptr)
	{
		assert(false);
		return;
	}

	m_xmlWriter->writeTextElement(name, value);
}


bool XmlHelper::readIntAttribute(const QString& name, int* value)
{
	if (m_xmlReader == nullptr)
	{
		assert(false);
		return false;
	}

	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QStringRef strValue = m_xmlReader->attributes().value(name);

	if (strValue == "")
	{
		return false;
	}

	*value = strValue.toInt();

	return true;
}
