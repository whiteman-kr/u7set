#include "WUtils.h"

#include "TypedPropertyValue.h"

TypedPropertyValue::TypedPropertyValue()
{
	m_type = Proto::TypedPropertyValueType::_Invalid;
	memset(&m_value, 0, sizeof(m_value));
}

TypedPropertyValue::TypedPropertyValue(const QVariant& qVarValue)
{

}


bool TypedPropertyValue::save(Proto::TypedPropertyValue* protoValue)
{
	TEST_PTR_RETURN_FALSE(protoValue);

	protoValue->Clear();

	protoValue->set_name(m_name.toStdString());
	protoValue->set_type(m_type);

	switch(m_type)
	{
	case Proto::TypedPropertyValueType::_Invalid:
		assert(false);
		return true;

	case Proto::TypedPropertyValueType::_Int32:
		protoValue->set_valueint32(m_value.valueInt32);
		return true;

	case Proto::TypedPropertyValueType::_UInt32:
		protoValue->set_valueuint32(m_value.valueUInt32);
		return true;

	case Proto::TypedPropertyValueType::_Int64:
		protoValue->set_valueint64(m_value.valueInt64);
		return true;

	case Proto::TypedPropertyValueType::_UInt64:
		protoValue->set_valueuint64(m_value.valueUInt64);
		return true;

	case Proto::TypedPropertyValueType::_Double:
		protoValue->set_valuedouble(m_value.valueDouble);
		return true;

	case Proto::TypedPropertyValueType::_Float:
		protoValue->set_valuedouble(m_value.valueFloat);
		return true;

	case Proto::TypedPropertyValueType::_Bool:
		protoValue->set_valuebool(m_value.valueBool);
		return true;

	case Proto::TypedPropertyValueType::_String:
		TEST_PTR_RETURN_FALSE(m_value.valueString);
		protoValue->set_valuestring(m_value.valueString->toStdString());
		return true;

	case Proto::TypedPropertyValueType::_Enum:
		TEST_PTR_RETURN_FALSE(m_value.valueEnum);
		protoValue->set_enumname(m_value.valueEnum->name.toStdString());
		protoValue->set_enumitemname(m_value.valueEnum->itemName.toStdString());
		protoValue->set_enumitemvalue(m_value.valueEnum->itemValue);
		return true;

	default:
		assert(false);
	}

	return false;
}

bool loadPropertyValue(const Proto::TypedPropertyValue &propertyValue, const QString* name, QVariant* value)
{
	TEST_PTR_RETURN_FALSE(name);
	TEST_PTR_RETURN_FALSE(value);

//		name = propertyValue.name();

	return false;
}
