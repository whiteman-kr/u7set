#include "AppSignalSpecPropValues.h"
#include "AppSignal.h"
#include "AppSignalParam.h"
#include "../UtilsLib/WUtils.h"


// -------------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValue class implementation
//
// -------------------------------------------------------------------------------------------------------------

AppSignalSpecPropValue::AppSignalSpecPropValue()
{
}

bool AppSignalSpecPropValue::create(const std::shared_ptr<Property>& prop)
{
	if (prop == nullptr)
	{
		assert(false);
		return false;
	}

	return create(prop->caption(), prop->value(), prop->isEnum());
}

bool AppSignalSpecPropValue::create(const QString& name, const QVariant& value, bool isEnum)
{
	m_name = name;
	m_value = value;
	m_isEnum = isEnum;

	return true;
}

bool AppSignalSpecPropValue::setValue(const QString& name, const QVariant& value, bool isEnum)
{
	if (name != m_name)
	{
		assert(false);
		return false;
	}

	if (m_value.metaType() != value.metaType())
	{
		assert(false);
		return false;
	}

	if (m_isEnum != isEnum)
	{
		assert(false);
		return false;
	}

	m_value = value;

	return true;
}

bool AppSignalSpecPropValue::setAnyValue(const QString& name, const QVariant& value)
{
	return setValue(name, value, m_isEnum);
}

bool AppSignalSpecPropValue::save(Proto::SignalSpecPropValue* protoValue) const
{
	TEST_PTR_RETURN_FALSE(protoValue);

	protoValue->Clear();

	protoValue->set_name(m_name.toStdString());
	protoValue->set_type(m_value.metaType().id());
	protoValue->set_isenum(m_isEnum);

	switch (m_value.metaType().id())
	{
	case QMetaType::Int:
		protoValue->set_int32val(m_value.toInt());
		return true;

	case QMetaType::UInt:
		protoValue->set_uint32val(m_value.toUInt());
		return true;

	case QMetaType::LongLong:
		protoValue->set_int64val(m_value.toLongLong());
		return true;

	case QMetaType::ULongLong:
		protoValue->set_uint64val(m_value.toULongLong());
		return true;

	case QMetaType::Double:
		protoValue->set_doubleval(m_value.toDouble());
		return true;

	case QMetaType::Bool:
		protoValue->set_boolval(m_value.toBool());
		return true;

	case QMetaType::QString:
		protoValue->set_stringval(m_value.toString().toStdString());
		return true;

	default:
		assert(false);
	}

	return false;
}

bool AppSignalSpecPropValue::load(const Proto::SignalSpecPropValue& protoValue)
{
	m_name = QString::fromStdString(protoValue.name());

	QMetaType::Type type = static_cast<QMetaType::Type>(protoValue.type());

	m_isEnum = protoValue.isenum();

#ifdef QT_DEBUG
	if (m_isEnum == true && type != QMetaType::Int)
	{
		assert(false);
	}
#endif

	switch (type)
	{
	case QMetaType::Int:
		assert(protoValue.has_int32val());
		m_value.setValue(protoValue.int32val());
		return true;

	case QMetaType::UInt:
		assert(protoValue.has_uint32val());
		m_value.setValue(protoValue.uint32val());
		return true;

	case QMetaType::LongLong:
		assert(protoValue.has_int64val());
		m_value.setValue(protoValue.int64val());
		return true;

	case QMetaType::ULongLong:
		assert(protoValue.has_uint64val());
		m_value.setValue(protoValue.uint64val());
		return true;

	case QMetaType::Double:
		assert(protoValue.has_doubleval());
		m_value.setValue(protoValue.doubleval());
		return true;

	case QMetaType::Bool:
		assert(protoValue.has_boolval());
		m_value.setValue(protoValue.boolval());
		return true;

	case QMetaType::QString:
		assert(protoValue.has_stringval());
		m_value.setValue(QString::fromStdString(protoValue.stringval()));
		return true;

	default:
		assert(false);
	}

	return false;
}


// ----------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValues class implementation
//
// ----------------------------------------------------------------------------------------------------------

AppSignalSpecPropValues::AppSignalSpecPropValues()
{
}


bool AppSignalSpecPropValues::create(const AppSignal& s)
{
	bool result = true;

	result &= createFromSpecPropStruct(s.specPropStruct());
	result &= parseValuesFromArray(s.protoSpecPropValues());

	return result;
}

bool AppSignalSpecPropValues::create(const AppSignalParam& s)
{
	bool result = true;

	result &= createFromSpecPropStruct(s.specificPropertyStruct());
	result &= parseValuesFromArray(s.protoSpecificPropertyValues());

	return result;
}



bool AppSignalSpecPropValues::createFromSpecPropStruct(const QString& specPropStruct, bool buildNamesMap)
{
	m_specPropValues.clear();
	m_propNamesMap.clear();

	if (specPropStruct.isEmpty() == true)
	{
		return true;
	}

	PropertyObject pob;

	std::pair<bool, QString> result = pob.parseSpecificPropertiesStruct(specPropStruct);

	if (result.first == false)
	{
		assert(false);
		return false;
	}

	std::vector<std::shared_ptr<Property>> properties = pob.properties();

	for (const std::shared_ptr<Property>& property : properties)
	{
		AppSignalSpecPropValue specPropValue;

		bool createResult = specPropValue.create(property);
		if (createResult == false)
		{
			Q_ASSERT(createResult);
			return false;
		}

		m_specPropValues.append(specPropValue);
	}

	if (buildNamesMap == true)
	{
		rebuildPropNamesMap();
	}

	return true;
}

bool AppSignalSpecPropValues::updateFromSpecPropStruct(const QString& specPropStruct)
{
	PropertyObject pob;

	std::pair<bool, QString> pobResult = pob.parseSpecificPropertiesStruct(specPropStruct);

	if (pobResult.first == false)
	{
		return false;
	}

	rebuildPropNamesMap();

	std::set<QString> namesToDelete;
	QHash<QString, std::shared_ptr<Property>> namesToCreate;

	std::vector<std::shared_ptr<Property>> properties = pob.properties();

	for (const std::shared_ptr<Property>& property : properties)
	{
		QString propName = property->caption();

		if (isExists(propName) == false)
		{
			namesToCreate.insert(propName, property);
			continue;
		}

		// value of property is exists
		//
		QVariant value;
		bool isEnum = false;

		getValue(propName, &value, &isEnum);

		// checking that property end value types are equal
		//
		if (property->value().metaType() == value.metaType() && property->isEnum() == isEnum)
		{
			// equal, update existing value if nessesery
			//
			if (property->updateFromPreset() == true)
			{
				setValue(propName, property->value(), property->isEnum());
			}
		}
		else
		{
			// property type has been changed, recreate value
			//
			namesToDelete.insert(propName);
			namesToCreate.insert(propName, property);
		}
	}

	for (const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (pob.propertyByCaption(specPropValue.name()) == nullptr)
		{
			namesToDelete.insert(specPropValue.name());
		}
	}

	QVector<AppSignalSpecPropValue> newSpecPropValues;

	for (const auto& propVal : m_specPropValues)
	{
		if (namesToDelete.contains(propVal.name()) == false)
		{
			newSpecPropValues.emplace_back(propVal);
		}
	}

	// create new property value, set to default
	//
	AppSignalSpecPropValue specPropValue;

	for (const std::shared_ptr<Property>& property : namesToCreate)
	{
		specPropValue.create(property);
		newSpecPropValues.emplace_back(specPropValue);
	}

	m_specPropValues.swap(newSpecPropValues);

	rebuildPropNamesMap();

	return true;
}

bool AppSignalSpecPropValues::setValue(const QString& name, const QVariant& value)
{
	return setValue(name, value, false);
}

bool AppSignalSpecPropValues::setAnyValue(const QString& name, const QVariant& value)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setAnyValue(name, value);
}

bool AppSignalSpecPropValues::setEnumValue(const QString& name, int enumItemValue)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setValue(name, QVariant(enumItemValue), true);
}

bool AppSignalSpecPropValues::setValue(const AppSignalSpecPropValue& propValue)
{
	return setValue(propValue.name(), propValue.value(), propValue.isEnum());
}

bool AppSignalSpecPropValues::getValue(const QString& name, QVariant* qv) const
{
	TEST_PTR_RETURN_FALSE(qv);

	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	*qv = m_specPropValues[index].value();

	return true;
}

bool AppSignalSpecPropValues::getValue(const QString& name, QVariant* qv, bool* isEnum) const
{
	TEST_PTR_RETURN_FALSE(qv);
	TEST_PTR_RETURN_FALSE(isEnum);

	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	*qv = m_specPropValues[index].value();
	*isEnum = m_specPropValues[index].isEnum();

	return true;
}

bool AppSignalSpecPropValues::serializeValuesToArray(QByteArray* protoData) const
{
	TEST_PTR_RETURN_FALSE(protoData);

	Proto::SignalSpecPropValues protoValues;

	for (const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		Proto::SignalSpecPropValue* protoValue = protoValues.add_value();
		specPropValue.save(protoValue);
	}

	protoData->resize(static_cast<int>(protoValues.ByteSizeLong()));

	protoValues.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(protoData->data()));

	return true;
}

bool AppSignalSpecPropValues::parseValuesFromArray(const QByteArray& protoData)
{
	m_specPropValues.clear();

	Proto::SignalSpecPropValues protoValues;

	bool result = protoValues.ParseFromArray(protoData.constData(), static_cast<int>(protoData.size()));

	if (result == false)
	{
		return false;
	}

	int count = protoValues.value_size();

	for (int i = 0; i < count; i++)
	{
		AppSignalSpecPropValue specPropValue;

		specPropValue.load(protoValues.value(i));

		m_specPropValues.append(specPropValue);
	}

	rebuildPropNamesMap();

	return true;
}

void AppSignalSpecPropValues::append(const AppSignalSpecPropValue& value)
{
	Q_ASSERT(m_propNamesMap.contains(value.name()) == false);

	int index = m_specPropValues.size();

	m_specPropValues.append(value);
	m_propNamesMap.emplace(value.name(), index);
}

bool AppSignalSpecPropValues::removeValue(const QString& propName)
{
	auto it = m_propNamesMap.find(propName);

	if (it == m_propNamesMap.end())
	{
		return false;
	}

	int index = it->second;

	m_specPropValues.erase(m_specPropValues.begin() + index);

	rebuildPropNamesMap();

	return true;
}

bool AppSignalSpecPropValues::replaceName(const QString& oldName, const QString& newName)
{
	bool replacingIsOccured = false;

	for (AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (specPropValue.name() == oldName)
		{
			specPropValue.setName(newName);
			replacingIsOccured = true;
			break;
		}
	}

	return replacingIsOccured;
}

void AppSignalSpecPropValues::rebuildPropNamesMap()
{
	m_propNamesMap.clear();

	int index = 0;

	for (const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (m_propNamesMap.contains(specPropValue.name()) == false)
		{
			m_propNamesMap.emplace(specPropValue.name(), index);
		}
		else
		{
			assert(false);			// duplicate property name
		}

		index++;
	}
}

bool AppSignalSpecPropValues::setValue(const QString& name, const QVariant& value, bool isEnum)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setValue(name, value, isEnum);
}

int AppSignalSpecPropValues::getPropertyIndex(const QString& name) const
{
	if (m_propNamesMap.empty() == false)
	{
		auto it = m_propNamesMap.find(name);

		if (it == m_propNamesMap.end())
		{
			return -1;
		}

		return it->second;
	}

	int index = 0;

	for (const AppSignalSpecPropValue& propValue : m_specPropValues)
	{
		if (propValue.name() == name)
		{
			return index;
		}

		index++;
	}

	return -1;
}
