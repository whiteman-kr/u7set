#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QBuffer>

#include "../include/SocketIO.h"


class XmlWriteHelper
{
private:
	QXmlStreamWriter* m_xmlWriter = nullptr;
	QXmlStreamWriter* m_xmlLocalWriter = nullptr;

public:
	XmlWriteHelper(QXmlStreamWriter& xmlWriter);
	XmlWriteHelper(QByteArray* data);
	~XmlWriteHelper();

	void setAutoFormatting(bool autoFormatting);
	void writeStartDocument();
	void writeEndDocument();

	void writeStartElement(const QString& name);
	void writeEndElement();

	void writeStringAttribute(const QString& name, const QString& value);
	void writeIntAttribute(const QString& name, int value);
	void writeBoolAttribute(const QString& name, bool value);
	void writeUlongAttribute(const QString& name, ulong value, bool hex = false);
	void writeDoubleAttribute(const QString& name, double value);

	void writeStringElement(const QString& name, const QString& value);
	void writeIntElement(const QString& name, int value);

	void writeHostAddressPort(const QString& nameIP, const QString& namePort, HostAddressPort& hostAddressPort);
	void writeHostAddress(const QString& nameIP, QHostAddress& hostAddress);
};


class XmlReadHelper
{
private:
	QXmlStreamReader* m_xmlReader = nullptr;
	QXmlStreamReader* m_xmlLocalReader = nullptr;

public:
	XmlReadHelper(QXmlStreamReader& xmlReader);
	XmlReadHelper(const QByteArray& data);
	~XmlReadHelper();

	bool readNextStartElement();
	void skipCurrentElement();

	QString name();

	bool atEnd();

	bool readStringAttribute(const QString& name, QString* value);
	bool readIntAttribute(const QString& name, int* value);
	bool readDoubleAttribute(const QString& name, double* value);
	bool readBoolAttribute(const QString& name, bool* value);
	bool readUlongAttribute(const QString& name, ulong* value);

	bool readStringElement(const QString& elementName, QString* value);
	bool readIntElement(const QString& elementName, int* value);

	bool readHostAddressPort(const QString& nameIP, const QString &namePort, HostAddressPort *hostAddressPort);
	bool readHostAddress(const QString& name, QHostAddress *hostAddress);

	bool findElement(const QString& elementName);
};

