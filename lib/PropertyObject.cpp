#include "../lib/PropertyObject.h"
#include <QDebug>
#include "../Proto/serialization.pb.h"


//
//
//			Class Property
//
//
Property::Property() :
	m_flags(0)
{
}

Property::~Property()
{
}

void Property::saveValue(::Proto::Property* protoProperty) const
{
	protoProperty->set_name(m_caption.toStdString());

	QString valueStr;

	QVariant value = this->value();

	if (isEnum() == true)
	{
		valueStr = value.toString();
	}
	else
	{
		switch (value.type())
		{
		case QVariant::Bool:
			valueStr = value.toBool() ? "t" : "f";
			break;
		case QVariant::Int:
			valueStr.setNum(value.toInt());
			break;
		case QVariant::UInt:
			valueStr.setNum(value.toUInt());
			break;
		case QVariant::String:
			valueStr = value.toString();
			break;
		case QVariant::Double:
			valueStr.setNum(value.toDouble());
			break;
		default:
			assert(false);
		}
	}

	protoProperty->set_value(valueStr.toLocal8Bit());
}

bool Property::loadValue(const ::Proto::Property& protoProperty)
{
	if (protoProperty.name() != m_caption.toStdString())
	{
		assert(protoProperty.name() == m_caption.toStdString());
		return false;
	}

	bool ok = false;
	QString sv(protoProperty.value().c_str());
	QVariant value = this->value();

	if (isEnum() == true)
	{
		setValue(protoProperty.value().c_str());
		return true;
	}

	switch (value.type())
	{
		case QVariant::Bool:
			{
				value = (sv == "t") ? true : false;
				ok = true;
			}
			break;
		case QVariant::Int:
			{
				qint32 i = sv.toInt(&ok);
				value = QVariant(i);
			}
			break;
		case QVariant::UInt:
			{
				quint32 ui = sv.toUInt(&ok);
				value = QVariant(ui);
			}
			break;
		case QVariant::String:
			{
				value = sv;
				ok = true;
			}
			break;
		case QVariant::Double:
			{
				double d = sv.toDouble(&ok);
				value = QVariant(d);
			}
			break;
		default:
			assert(false);
	}

	if (ok == true)
	{
		setValue(value);
	}

	return ok;
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

void Property::copy(const Property* source)
{
	if (source == nullptr ||
		source == this)
	{
		assert(source);
		return;
	}

	m_caption = source->m_caption;
	m_description = source->m_description;
	m_category = source->m_category;
	m_validator = source->m_validator;
	m_flags = source->m_flags;
	m_precision = source->m_precision;

	return;
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

std::vector<std::shared_ptr<Property>> PropertyObject::properties() const
{
    std::vector<std::shared_ptr<Property>> result;
	result.reserve(m_properties.size());

	for (const std::pair<uint, std::shared_ptr<Property>>& p : m_properties)
	{
		result.push_back(p.second);
	}

	return result;
}

void PropertyObject::removeAllProperties()
{
	m_properties.clear();
}

bool PropertyObject::removeProperty(const QString& caption)
{
	uint hash = qHash(caption);
	size_t removed = m_properties.erase(hash);
	return removed > 0;
}

void PropertyObject::addProperties(std::vector<std::shared_ptr<Property>> properties)
{
	for (auto p : properties)
	{
		uint hash = qHash(p->caption());
		m_properties[hash] = p;
	}
}

void PropertyObject::removeSpecificProperties()
{
	for(auto it = m_properties.begin(); it != m_properties.end();)
	{
		if(it->second->specific() == true)
		{
			it = m_properties.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool PropertyObject::propertyExists(QString caption) const
{
	uint hash = qHash(caption);
	auto it = m_properties.find(hash);
    return it != m_properties.end();
}

std::shared_ptr<Property> PropertyObject::propertyByCaption(QString caption)
{
    std::shared_ptr<Property> result = nullptr;

	uint hash = qHash(caption);
	auto it = m_properties.find(hash);

	if (it != m_properties.end())
	{
		result = it->second;
	}

	return result;
}

const std::shared_ptr<Property> PropertyObject::propertyByCaption(QString caption) const
{
    std::shared_ptr<Property> result = nullptr;

	uint hash = qHash(caption);
	auto it = m_properties.find(hash);

	if (it != m_properties.end())
	{
		result = it->second;
	}

	return result;
}

QVariant PropertyObject::propertyValue(QString caption) const
{
	uint hash = qHash(caption);
	auto it = m_properties.find(hash);

	if (it != m_properties.end())
	{
		QVariant result(it->second->value());
		return result;
	}
	else
	{
        qDebug() << "PropertyObject::propertyValue: property not found: " << caption;
		return QVariant();
	}
}

bool PropertyObject::setPropertyValue(QString caption, const char* value)
{
	uint hash = qHash(caption);
	auto it = m_properties.find(hash);

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

		propertyValue->setValueDirect(QString(value));

		return true;
	}

	return false;
}

bool PropertyObject::setPropertyValue(QString caption, const QVariant& value)
{
	uint hash = qHash(caption);
	auto it = m_properties.find(hash);

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

	uint hash = qHash(property);
	auto it = m_properties.find(hash);

	if (it != m_properties.end())
	{
		result = it->second->enumValues();
	}

	return result;
}

