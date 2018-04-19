#pragma once

#include <QtCore>
#include <memory>

#include "PropertyObject.h"
#include "../Proto/serialization.pb.h"

class TypedPropertyValue
{
private:
	struct Enum
	{
		QString name;
		QString itemName;
		int itemValue;
	};

	union Value
	{
		qint32 valueInt32;
		quint32 valueUInt32;
		qint64 valueInt64;
		quint64 valueUInt64;
		double valueDouble;
		float valueFloat;
		bool valueBool;
		QString* valueString;
		Enum* valueEnum;
	};

public:
	TypedPropertyValue();
	TypedPropertyValue(const QVariant& qVarValue);
	TypedPropertyValue(std::shared_ptr<Property> prop);
	TypedPropertyValue(const Property* prop);

	QString name() const { return m_name; }
	Proto::TypedPropertyValueType type() const { return m_type; }
//	QVariant value() const { return m_value; }

	bool save(Proto::TypedPropertyValue* protoValue);
	bool load(Proto::TypedPropertyValue& propertyValue);

private:
	QString m_name;
	Proto::TypedPropertyValueType m_type = Proto::TypedPropertyValueType::_Invalid;
	Value m_value;
};



