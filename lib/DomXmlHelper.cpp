#include "DomXmlHelper.h"

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

bool DomXmlHelper::getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toInt(&ok);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool DomXmlHelper::getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	*value = elem.attribute(attrName);

	return true;
}

bool DomXmlHelper::getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	if (attrValue == "true")
	{
		*value = true;
		return true;
	}

	if (attrValue == "false")
	{
		*value = false;
		return true;
	}

	*errMsg = errAttributeParsing(elem, attrName);
	return false;
}

bool DomXmlHelper::getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
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

bool DomXmlHelper::getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg)
{
	if (value == nullptr)
	{
		Q_ASSERT(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
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
