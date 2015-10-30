#include "../include/PropertyObject.h"
#include <QDebug>

//
//
//			Class Property
//
//
Property::Property()
{
}

Property::~Property()
{
}

QString Property::caption() const
{
	return m_caption;
}

void Property::setCaption(QString value)
{
	m_caption = value;
}

QString Property::description() const
{
	return m_description;
}

void Property::setDescription(QString value)
{
	m_description = value;
}

bool Property::readOnly() const
{
	return m_readOnly;
}

void Property::setReadOnly(bool value)
{
	m_readOnly = value;
}

bool Property::updateFromPreset() const
{
	return m_updateFromPreset;
}

void Property::setUpdateFromPreset(bool value)
{
	m_updateFromPreset = value;
}

bool Property::visible() const
{
	return m_visible;
}

void Property::setVisible(bool value)
{
	m_visible = value;
}

int Property::precision() const
{
	return m_precision;
}

void Property::setPrecision(int value)
{
	if (value < 0)
	{
		assert(value >= 0);
		value = 0;
	}

	m_precision = value;
}


//
//
//			Class PropertyObject
//
//
PropertyObject::PropertyObject(QObject* parent) :
	QObject(parent)
{
}

PropertyObject::~PropertyObject()
{

	return;
}

std::vector<std::shared_ptr<Property>> PropertyObject::properties()
{
    std::vector<std::shared_ptr<Property>> result;
	result.reserve(m_properties.size());

    for (std::pair<QString, std::shared_ptr<Property>> p : m_properties)
	{
		result.push_back(p.second);
	}

	return result;
}

std::shared_ptr<Property> PropertyObject::propertyByCaption(QString caption)
{
    std::shared_ptr<Property> result = nullptr;

	auto it = m_properties.find(caption);

	if (it != m_properties.end())
	{
		result = it->second;
	}

	return result;
}

const std::shared_ptr<Property> PropertyObject::propertyByCaption(QString caption) const
{
    std::shared_ptr<Property> result = nullptr;

	auto it = m_properties.find(caption);

	if (it != m_properties.end())
	{
		result = it->second;
	}

	return result;
}

QVariant PropertyObject::propertyValue(QString caption) const
{
	auto it = m_properties.find(caption);

	if (it != m_properties.end())
	{
		QVariant result(it->second->value());
		return result;
	}
	else
	{
		return QVariant();
	}
}

bool PropertyObject::setPropertyValue(QString caption, const char* value)
{
	auto it = m_properties.find(caption);

	if (it == m_properties.end())
	{
		return false;
	}

    std::shared_ptr<Property> property = it->second;

	if (property->isEnum() == true)
	{
		property->setEnumValue(value);
		return true;
	}
	else
	{
        PropertyValue<QString>* propertyValue = dynamic_cast<PropertyValue<QString>*>(property.get());

		if (propertyValue == nullptr)
		{
			assert(propertyValue);
			return false;
		}

		propertyValue->setValue(QString(value));

		return true;
	}

	return false;
}

bool PropertyObject::setPropertyValue(QString caption, const QVariant& value)
{
	auto it = m_properties.find(caption);

	if (it == m_properties.end())
	{
		return false;
	}

	it->second->setValue(value);

	return true;
}

std::list<std::pair<int, QString>> PropertyObject::enumValues(QString property)
{
	std::list<std::pair<int, QString>> result;

	auto it = m_properties.find(property);

	if (it != m_properties.end())
	{
		result = it->second->enumValues();
	}

	return result;
}

