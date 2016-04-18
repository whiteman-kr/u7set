#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../include/SocketIO.h"

class XmlHelper
{
	QXmlStreamWriter* m_xmlWriter = nullptr;
	QXmlStreamReader* m_xmlReader = nullptr;

public:
	XmlHelper(QXmlStreamWriter& xmlWriter);

	void writeStartElement(const QString& name);
	void writeEndElement();
	void writeAttribute(const QString& name, const QString& value);
	void writeTextElement(const QString& name, const QString& value);

	void writeHostAddressPortElement(const QString& name, HostAddressPort& hostAddressPort);
	void writeHostAddressElement(const QString& name, QHostAddress& hostAddress);

	//

	XmlHelper(QXmlStreamReader& xmlReader);

	bool readIntAttribute(const QString& name, int* value);

	bool readHostAddressPortElement(const QString& name, HostAddressPort *hostAddressPort);
	bool readHostAddressElement(const QString& name, QHostAddress *hostAddress);

};
