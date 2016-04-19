#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../include/SocketIO.h"


class XmlWriteHelper
{
private:
	QXmlStreamWriter& m_xmlWriter;

public:
	XmlWriteHelper(QXmlStreamWriter& xmlWriter);

	void writeStartElement(const QString& name);
	void writeEndElement();

	void writeStringAttribute(const QString& name, const QString& value);
	void writeIntAttribute(const QString& name, int value);

	void writeStringElement(const QString& name, const QString& value);
	void writeIntElement(const QString& name, int value);

	void writeHostAddressPort(const QString& nameIP, const QString& namePort, HostAddressPort& hostAddressPort);
	void writeHostAddress(const QString& nameIP, QHostAddress& hostAddress);
};


class XmlReadHelper
{
private:
	QXmlStreamReader& m_xmlReader;

public:
	XmlReadHelper(QXmlStreamReader& xmlReader);

	bool readNextStartElement();
	void skipCurrentElement();

	QString name();

	bool atEnd();

	bool readStringAttribute(const QString& name, QString* value);
	bool readIntAttribute(const QString& name, int* value);

	bool readStringElement(const QString& elementName, QString* value);
	bool readIntElement(const QString& elementName, int* value);

	bool readHostAddressPort(const QString& nameIP, const QString &namePort, HostAddressPort *hostAddressPort);
	bool readHostAddress(const QString& name, QHostAddress *hostAddress);
};

