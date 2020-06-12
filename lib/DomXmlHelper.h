#pragma once

#include <QString>
#include <QDomDocument>

#include "Address16.h"

class DomXmlHelper
{
public:
	static QString errElementNotFound(const QString& elemName);
	static QString errAttributeNotFound(const QDomElement& elem, const QString& attrName);
	static QString errAttributeParsing(const QDomElement& elem, const QString& attrName);

	static bool getSingleChildElement(const QDomElement& parentElement, const QString& childElementTagName,
									  QDomElement* childElem, QString* errMsg);

	static bool getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg);
	static bool getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg);
	static bool getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg);
	static bool getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg);
	static bool getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg);
};
