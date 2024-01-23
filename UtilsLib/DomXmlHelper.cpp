#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "DomXmlHelper.h"
#include "WUtils.h"

QString DomXmlHelper::errElementNotFound(const QString& elemName)
{
	return QString("Element is not found: %1").arg(elemName);
}

QString DomXmlHelper::errAttributeNotFound(const QDomElement& elem, const QString& attrName)
{
	return QString("Attribute is not found: %1 (element %2)").arg(attrName).arg(elem.tagName());
}

QString DomXmlHelper::errAttributeParsing(const QDomElement& elem, const QString& attrName)
{
	return QString("Attribute parsing error: %1 (element %2)").arg(attrName).arg(elem.tagName());
}

bool DomXmlHelper::getSingleChildElement(const QDomElement& parentElement, const QString& childElementTagName,
											QDomElement* childElem, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (childElem == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	QDomNodeList nodes = parentElement.elementsByTagName(childElementTagName);

	if (nodes.count() == 0)
	{
		*errMsg = QString("Child element %1 not found in parent element %2").
						arg(childElementTagName).
						arg(parentElement.tagName());
		return false;
	}

	if (nodes.count() > 1)
	{
		*errMsg = QString("More than one child element %1 in parent element %2").
					arg(childElementTagName).
					arg(parentElement.tagName());
		return false;
	}

	*childElem = nodes.item(0).toElement();

	return true;
}

//

bool DomXmlHelper::getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg, int base)
{
	return privateGetIntAttribute(true, elem, attrName, 0, value, errMsg, base);
}

bool DomXmlHelper::getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg)
{
	return privateGetStringAttribute(true, elem, attrName, QStringLiteral(""), value, errMsg);
}

bool DomXmlHelper::getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg)
{
	return privateGetBoolAttribute(true, elem, attrName, false, value, errMsg);
}

bool DomXmlHelper::getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg)
{
	return privateGetAddress16Attribute(true, elem, attrName, Address16(), value, errMsg);
}

bool DomXmlHelper::getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg)
{
	return privateGetUInt32Attribute(true, elem, attrName, 0, value, errMsg);
}

bool DomXmlHelper::getUInt64Attribute(const QDomElement& elem, const QString& attrName, quint64* value, QString* errMsg)
{
	return privateGetUInt64Attribute(true, elem, attrName, 0, value, errMsg);
}

//

bool DomXmlHelper::getIntAttributeIfExists(const QDomElement& elem, const QString& attrName, int defaultValue, int* value, QString* errMsg, int base)
{
	return privateGetIntAttribute(false, elem, attrName, defaultValue, value, errMsg, base);
}

bool DomXmlHelper::getStringAttributeIfExists(const QDomElement& elem, const QString& attrName, const QString& defaultValue, QString* value, QString* errMsg)
{
	return privateGetStringAttribute(false, elem, attrName, defaultValue, value, errMsg);
}

bool DomXmlHelper::getBoolAttributeIfExists(const QDomElement& elem, const QString& attrName, bool defaultValue, bool* value, QString* errMsg)
{
	return privateGetBoolAttribute(false, elem, attrName, defaultValue, value, errMsg);
}

bool DomXmlHelper::getAddress16AttributeIfExists(const QDomElement& elem, const QString& attrName, const Address16& defaultValue, Address16* value, QString* errMsg)
{
	return privateGetAddress16Attribute(false, elem, attrName, defaultValue, value, errMsg);
}

bool DomXmlHelper::getUInt32AttributeIfExists(const QDomElement& elem, const QString& attrName, quint32 defaultValue, quint32* value, QString* errMsg)
{
	return privateGetUInt32Attribute(false, elem, attrName, defaultValue, value, errMsg);
}

bool DomXmlHelper::getUInt64AttributeIfExists(const QDomElement& elem, const QString& attrName, quint64 defaultValue, quint64* value, QString* errMsg)
{
	return privateGetUInt64Attribute(false, elem, attrName, defaultValue, value, errMsg);
}

//

bool DomXmlHelper::privateGetIntAttribute(bool required, const QDomElement& elem,
										  const QString& attrName, int defaultValue,
										  int* value, QString* errMsg, int base)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toInt(&ok, base);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool DomXmlHelper::privateGetStringAttribute(bool required, const QDomElement& elem,
											 const QString& attrName, const QString& defaultValue,
											 QString* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	*value = elem.attribute(attrName);

	return true;
}

bool DomXmlHelper::privateGetBoolAttribute(bool required, const QDomElement& elem,
										   const QString& attrName, bool defaultValue,
										   bool* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = stringToBool(attrValue, &ok);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool DomXmlHelper::privateGetAddress16Attribute(bool required, const QDomElement& elem,
												const QString& attrName, const Address16& defaultValue,
												Address16* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	value->fromString(attrValue, &ok);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool DomXmlHelper::privateGetUInt32Attribute(bool required, const QDomElement& elem,
											 const QString& attrName, quint32 defaultValue,
											 quint32* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toULong(&ok, 0);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool DomXmlHelper::privateGetUInt64Attribute(bool required, const QDomElement& elem,
											 const QString& attrName, quint64 defaultValue,
											 quint64* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		if (required == true)
		{
			*errMsg = errAttributeNotFound(elem, attrName);
			return false;
		}

		*value = defaultValue;
		return true;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toULongLong(&ok, 0);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}
