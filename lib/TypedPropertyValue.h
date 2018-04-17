#pragma once

#include <QtCore>
#include "../Proto/serialization.pb.h"


class TypedPropertyValue
{
public:
	TypedPropertyValue();

private:
	QString m_name;
	::Proto::TypedPropertyValueType m_type = ::Proto::TypedPropertyValueType::_Invalid;
	QVariant m_value;
};

