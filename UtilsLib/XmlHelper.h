#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHostAddress>

#include "../UtilsLib/Address16.h"

class HostAddressPort;

class XmlWriteHelper
{
public:
	XmlWriteHelper(QXmlStreamWriter& xmlWriter);
	XmlWriteHelper(QByteArray* data);
	XmlWriteHelper(QString* xmlString);
	~XmlWriteHelper();

	QXmlStreamWriter* xmlStreamWriter() const;

	void setAutoFormatting(bool autoFormatting);
	void writeStartDocument();
	void writeEndDocument();

	void writeStartElement(const QString& name);
	void writeEndElement();

	void writeStringAttribute(const QString& name, const QString& value);
	void writeIntAttribute(const QString& name, int value, bool hex = false);
	void writeBoolAttribute(const QString& name, bool value);
	void writeUInt64Attribute(const QString& name, quint64 value, bool hex = false);
	void writeUInt32Attribute(const QString& name, quint32 value, bool hex);
	void writeDoubleAttribute(const QString& name, double value);
	void writeDoubleAttribute(const QString& name, double value, int decimalPlaces);
	void writeFloatAttribute(const QString& name, float value);
	void writeAddress16Attribute(const QString& name, const Address16& addr16);
	void writeSoftwareTypeAttribute(E::SoftwareType swType);

	void writeString(const QString& str);

	void writeStringElement(const QString& name, const QString& value);
	void writeIntElement(const QString& name, int value);
	void writeBoolElement(const QString& name, bool value);

	void writeHostAddressPort(const QString& nameIP, const QString& namePort, const HostAddressPort& hostAddressPort);
	void writeHostAddress(const QString& nameIP, const QHostAddress& hostAddress);

private:
	QXmlStreamWriter* m_xmlWriter = nullptr;
	QXmlStreamWriter* m_xmlLocalWriter = nullptr;
};

class XmlReadHelper
{
public:
	XmlReadHelper(QXmlStreamReader& xmlReader);
	XmlReadHelper(const QByteArray& data);
	XmlReadHelper(const QString& xmlString);
	~XmlReadHelper();

	bool readNextStartElement();
	void skipCurrentElement();

	QString name();

	bool atEnd();

	bool readStringAttribute(const QString& name, QString* value);
	bool readIntAttribute(const QString& name, int* value);
	bool readBoolAttribute(const QString& name, bool* value);
	bool readUInt64Attribute(const QString& name, qulonglong* value);
	bool readUInt32Attribute(const QString& name, quint32* value);
	bool readDoubleAttribute(const QString& name, double* value);
	bool readFloatAttribute(const QString& name, float* value);
	bool readAddress16Attribute(const QString& name, Address16* value);
	bool readSoftwareTypeAttribute(E::SoftwareType* swType);

	bool readStringElement(const QString& elementName, QString* value, bool find = false);
	bool readIntElement(const QString& elementName, int* value, bool find = false);
	bool readBoolElement(const QString& elementName, bool* value, bool find = false);

	bool readHostAddressPort(const QString& nameIP, const QString &namePort, HostAddressPort *hostAddressPort);
	bool readHostAddress(const QString& name, QHostAddress *hostAddress);

	bool findElement(const QString& elementName);
	bool checkElement(const QString& elementName);

private:
	QXmlStreamReader* m_xmlReader = nullptr;
	QXmlStreamReader* m_xmlLocalReader = nullptr;
};
