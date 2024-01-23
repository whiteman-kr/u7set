#pragma once

#include <QString>
#include <QDomDocument>

#include "../UtilsLib/Address16.h"

class DomXmlHelper
{
public:
	static QString errElementNotFound(const QString& elemName);
	static QString errAttributeNotFound(const QDomElement& elem, const QString& attrName);
	static QString errAttributeParsing(const QDomElement& elem, const QString& attrName);

	static bool getSingleChildElement(const QDomElement& parentElement, const QString& childElementTagName,
									  QDomElement* childElem, QString* errMsg);

	static bool getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg, int base = 10);
	static bool getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg);
	static bool getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg);
	static bool getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg);
	static bool getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg);
	static bool getUInt64Attribute(const QDomElement& elem, const QString& attrName, quint64* value, QString* errMsg);

	static bool getIntAttributeIfExists(const QDomElement& elem, const QString& attrName, int defaultValue, int* value, QString* errMsg, int base = 10);
	static bool getStringAttributeIfExists(const QDomElement& elem, const QString& attrName, const QString& defaultValue, QString* value, QString* errMsg);
	static bool getBoolAttributeIfExists(const QDomElement& elem, const QString& attrName, bool defaultValue, bool* value, QString* errMsg);
	static bool getAddress16AttributeIfExists(const QDomElement& elem, const QString& attrName, const Address16& defaultValue, Address16* value, QString* errMsg);
	static bool getUInt32AttributeIfExists(const QDomElement& elem, const QString& attrName, quint32 defaultValue, quint32* value, QString* errMsg);
	static bool getUInt64AttributeIfExists(const QDomElement& elem, const QString& attrName, quint64 defaultValue, quint64* value, QString* errMsg);

private:
	static bool privateGetIntAttribute(bool required, const QDomElement& elem, const QString& attrName, int defaultValue, int* value, QString* errMsg, int base);
	static bool privateGetStringAttribute(bool required, const QDomElement& elem, const QString& attrName, const QString& defaultValue, QString* value, QString* errMsg);
	static bool privateGetBoolAttribute(bool required, const QDomElement& elem, const QString& attrName, bool defaultValue, bool* value, QString* errMsg);
	static bool privateGetAddress16Attribute(bool required, const QDomElement& elem, const QString& attrName, const Address16& defaultValue, Address16* value, QString* errMsg);
	static bool privateGetUInt32Attribute(bool required, const QDomElement& elem, const QString& attrName, quint32 defaultValue, quint32* value, QString* errMsg);
	static bool privateGetUInt64Attribute(bool required, const QDomElement& elem, const QString& attrName, quint64 defaultValue, quint64* value, QString* errMsg);

};
