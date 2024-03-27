#pragma once

#include <memory>
#include <map>

#include <QVariant>
#include <QVector>

class Property;
class AppSignal;
class AppSignalParam;

namespace Proto
{
	class SignalSpecPropValue;
}

class AppSignalSpecPropValue
{
public:
	AppSignalSpecPropValue();

	bool create(const std::shared_ptr<Property>& prop);
	bool create(const QString& name, const QVariant& value, bool isEnum);

	bool setValue(const QString& name, const QVariant& value, bool isEnum);
	bool setAnyValue(const QString& name, const QVariant& value);

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	QMetaType type() const { return m_value.metaType(); }
	QVariant value() const { return m_value; }
	bool isEnum() const { return m_isEnum; }

	bool save(Proto::SignalSpecPropValue* protoValue) const;
	bool load(const Proto::SignalSpecPropValue& protoValue);

private:
	QString m_name;
	QVariant m_value;
	bool m_isEnum = false;
};

class AppSignalSpecPropValues
{
public:
	AppSignalSpecPropValues();

	bool create(const AppSignal& s);
	bool create(const AppSignalParam& s);

	bool createFromSpecPropStruct(const QString& specPropStruct, bool buildNamesMap = true);
	bool updateFromSpecPropStruct(const QString& specPropStruct);

	bool isExists(const QString& name) const { return m_propNamesMap.contains(name); }

	bool setValue(const QString& name, const QVariant& value);

	bool setAnyValue(const QString& name, const QVariant& value);		// setter without isEnum checking

	template<typename ENUM_TYPE>
	bool setEnumValue(const QString& name, ENUM_TYPE enumItemValue);
	bool setEnumValue(const QString& name, int enumItemValue);

	bool setValue(const AppSignalSpecPropValue& propValue);

	bool getValue(const QString& name, QVariant* qv) const;
	bool getValue(const QString& name, QVariant* qv, bool* isEnum) const;

	bool serializeValuesToArray(QByteArray* protoData) const;
	bool parseValuesFromArray(const QByteArray& protoData);

	const QVector<AppSignalSpecPropValue>& values() const { return m_specPropValues; }
	QVector<AppSignalSpecPropValue>& values() { return m_specPropValues; }

	void append(const AppSignalSpecPropValue& value);
	bool removeValue(const QString& propName);

	bool replaceName(const QString& oldName, const QString& newName);			// returns true if replacing is occured

private:
	void rebuildPropNamesMap();

	bool setValue(const QString& name, const QVariant& value, bool isEnum);

	int getPropertyIndex(const QString& name) const;

private:
	QVector<AppSignalSpecPropValue> m_specPropValues;
	std::map<QString, int> m_propNamesMap;									// prop name => index in m_propSpecValues
};

template<typename ENUM_TYPE>
bool AppSignalSpecPropValues::setEnumValue(const QString& name, ENUM_TYPE enumItemValue)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);
	return setValue(name, static_cast<int>(enumItemValue), true);
}