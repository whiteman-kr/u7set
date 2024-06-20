#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../UtilsLib/Address16.h"
#include "../UtilsLib/WUtils.h"

class QHostAddress;
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
	void writeStringListAttribute(const QString& name, const QStringList& list);
	void writeIntAttribute(const QString& name, int value, bool hex = false);
	void writeBoolAttribute(const QString& name, bool value);
	void writeInt64Attribute(const QString& name, qint64 value, bool hex = false);
	void writeUInt64Attribute(const QString& name, quint64 value, bool hex = false);
	void writeUInt32Attribute(const QString& name, quint32 value, bool hex);
	void writeDoubleAttribute(const QString& name, double value);
	void writeDoubleAttribute(const QString& name, double value, int decimalPlaces);
	void writeFloatAttribute(const QString& name, float value);
	void writeAddress16Attribute(const QString& name, const Address16& addr16);
	void writeIPv4PortAttribute(const QString& name, const HostAddressPort& addr);
	void writeIPv4Attribute(const QString& name, const QHostAddress& addr);
	void writeUuidAttribute(const QString& name, const QUuid& guid);

	void writeString(const QString& str);

	void writeStringElement(const QString& name, const QString& value);
	void writeIntElement(const QString& name, int value);
	void writeBoolElement(const QString& name, bool value);

	void writeHostAddressPort(const QString& nameIP, const QString& namePort, const HostAddressPort& hostAddressPort);
	void writeHostAddress(const QString& nameIP, const QHostAddress& hostAddress);

	void writeQVariantAttribute(const QString& name, const QVariant& qv);

	template<typename ENUM_TYPE>
	void writeEnumKeyElement(const QString& name, ENUM_TYPE value);

	template<typename ENUM_TYPE>
	void writeEnumKeyAttribute(const QString& name, ENUM_TYPE value);		// writes Str name of enum item

	template<typename ENUM_TYPE>
	void writeEnumValueAttribute(const QString& name, ENUM_TYPE value);		// writes Int value of enum item

	template<typename ENUM_TYPE>
	void writeEnumKeyValueAttribute(const QString& name, ENUM_TYPE value);	// writes Str name and Int value of enum item

private:
	QXmlStreamWriter* m_xmlWriter = nullptr;
	QXmlStreamWriter* m_xmlLocalWriter = nullptr;
};

template<typename ENUM_TYPE>
void XmlWriteHelper::writeEnumKeyElement(const QString& name, ENUM_TYPE value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	writeStringElement(name, E::valueToString<ENUM_TYPE>(value));
}

template<typename ENUM_TYPE>
void XmlWriteHelper::writeEnumKeyAttribute(const QString& name, ENUM_TYPE value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	writeStringAttribute(name, E::valueToString<ENUM_TYPE>(value));
}

template<typename ENUM_TYPE>
void XmlWriteHelper::writeEnumValueAttribute(const QString& name, ENUM_TYPE value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	writeIntAttribute(name + QString(XmlAttribute::ENUM_VALUE_SUFFIX), static_cast<int>(value));
}

template<typename ENUM_TYPE>
void XmlWriteHelper::writeEnumKeyValueAttribute(const QString& name, ENUM_TYPE value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	writeEnumKeyAttribute(name, value);
	writeEnumValueAttribute(name, value);
}

//

class XmlReadHelper
{
public:
	XmlReadHelper(QXmlStreamReader& xmlReader);
	XmlReadHelper(const QByteArray& data);
	XmlReadHelper(const QString& xmlString);
	~XmlReadHelper();

	QXmlStreamReader* xmlStreamReader() const;

	bool readNextStartElement();
	void skipCurrentElement();

	QString name();

	bool atEnd();

	bool readStringAttribute(const QString& name, QString* value);
	bool readStringListAttribute(const QString& name, QStringList* list);
	bool readIntAttribute(const QString& name, int* value);
	bool readBoolAttribute(const QString& name, bool* value);
	bool readInt64Attribute(const QString& name, qlonglong* value);
	bool readUInt64Attribute(const QString& name, qulonglong* value);
	bool readUInt32Attribute(const QString& name, quint32* value);
	bool readDoubleAttribute(const QString& name, double* value);
	bool readFloatAttribute(const QString& name, float* value);
	bool readAddress16Attribute(const QString& name, Address16* value);
	bool readIPv4PortAttribute(const QString& name, HostAddressPort* addr);
	bool readIPv4Attribute(const QString& name, QHostAddress* addr);
	bool readUuidAttribute(const QString& name, QUuid* guid);

	template<typename ENUM_TYPE>
	bool readEnumAttribute(const QString& name, ENUM_TYPE* value);

	bool readStringElement(const QString& elementName, QString* value, bool find = false);
	bool readIntElement(const QString& elementName, int* value, bool find = false);
	bool readBoolElement(const QString& elementName, bool* value, bool find = false);

	bool readHostAddressPort(const QString& nameIP, const QString &namePort, HostAddressPort *hostAddressPort);
	bool readHostAddress(const QString& name, QHostAddress *hostAddress);

	QString elementText();

	bool findElement(const QString& elementName);
	bool checkElement(const QString& elementName);

	QXmlStreamAttributes attributes() const { return m_xmlReader->attributes(); }

	bool readQVariantAttribute(const QString& name, QVariant* qv);

	template<typename ENUM_TYPE>
	bool readEnumKeyElement(const QString& name, ENUM_TYPE* value, bool find = false);

	template<typename ENUM_TYPE>
	bool readEnumKeyAttribute(const QString& name, ENUM_TYPE* value);

	template<typename ENUM_TYPE>
	bool readEnumValueAttribute(const QString& name, ENUM_TYPE* value);

	bool readEnumValueAttributeAsInt(const QString& name, int* value);

private:
	QXmlStreamReader* m_xmlReader = nullptr;
	QXmlStreamReader* m_xmlLocalReader = nullptr;
};


template<typename ENUM_TYPE>
bool XmlReadHelper::readEnumKeyElement(const QString& name, ENUM_TYPE* value, bool find)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	TEST_PTR_RETURN_FALSE(value);

	QString valueStr;

	bool res = readStringElement(name, &valueStr, find);

	RETURN_IF_FALSE(res);

	*value = E::stringToValue<ENUM_TYPE>(valueStr, &res);

	return res;
}

template<typename ENUM_TYPE>
bool XmlReadHelper::readEnumKeyAttribute(const QString& name, ENUM_TYPE* value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	if(value == nullptr)
	{
		assert(false);
		return false;
	}

	QString str;
	bool result = false;

	result = readStringAttribute(name, &str);

	if (result == false)
	{
		return false;
	}

	*value = E::stringToValue<ENUM_TYPE>(str, &result);

	return result;
}

template<typename ENUM_TYPE>
bool XmlReadHelper::readEnumValueAttribute(const QString& name, ENUM_TYPE* value)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	int intVal = 0;

	bool result = false;

	result = readEnumValueAttributeAsInt(name, &intVal);

	RETURN_IF_FALSE(result);

	if (E::contains<ENUM_TYPE>(intVal) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	*value = static_cast<ENUM_TYPE>(intVal);

	return result;
}

