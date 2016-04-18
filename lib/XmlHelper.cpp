#include "../include/XmlHelper.h"


void XmlHelper::writeHostAddressPortElement(QXmlStreamWriter& xml, const QString& name, HostAddressPort& hostAddressPort)
{
	xml.writeStartElement(name);

	xml.writeAttribute("IP", hostAddressPort.addressStr());
	xml.writeAttribute("Port", QString::number(hostAddressPort.port()));

	xml.writeEndElement();
}


bool XmlHelper::readHostAddressPortElement(QXmlStreamReader& xml, const QString& name, HostAddressPort* hostAddressPort)
{
	if (xml.name() != name)
	{
		return false;
	}

	if (hostAddressPort == nullptr)
	{
		assert(false);
		return false;
	}

	hostAddressPort->setAddress(xml.attributes().value("IP").toString());
	hostAddressPort->setPort(xml.attributes().value("Port").toInt());

	return true;
}


void XmlHelper::writeHostAddressElement(QXmlStreamWriter& xml, const QString& name, QHostAddress& hostAddress)
{
	xml.writeStartElement(name);
	xml.writeAttribute("IP", hostAddress.toString());
	xml.writeEndElement();
}


bool XmlHelper::readHostAddressElement(QXmlStreamReader& xml, const QString& name, QHostAddress *hostAddress)
{
	if (xml.name() != name)
	{
		return false;
	}

	if (hostAddress == nullptr)
	{
		assert(false);
		return false;
	}

	hostAddress->setAddress(xml.attributes().value("IP").toString());
	return true;
}
