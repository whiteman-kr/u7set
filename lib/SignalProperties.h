#pragma once

#include <memory>

#include "Signal.h"
#include "PropertyObject.h"


class SignalSpecPropValue
{
public:
	SignalSpecPropValue();

	bool create(std::shared_ptr<Property> prop);
	bool create(const QString& name, const QVariant& value, bool isEnum);

	bool setValue(const QString& name, const QVariant& value, bool isEnum);
	bool setAnyValue(const QString& name, const QVariant& value);

	QString name() const { return m_name; }
	QVariant::Type type() const { return m_value.type(); }
	QVariant value() const { return m_value; }
	bool isEnum() const {return m_isEnum; }

	bool save(Proto::SignalSpecPropValue* protoValue) const;
	bool load(const Proto::SignalSpecPropValue& protoValue);

private:
	QString m_name;
	QVariant m_value;
	bool m_isEnum = false;
};

class SignalSpecPropValues
{
public:
	SignalSpecPropValues();

	bool create(const Signal& s);

	bool createFromSpecPropStruct(const QString& specPropStruct, bool buildNamesMap = true);
	bool updateFromSpecPropStruct(const QString& specPropStruct);

	bool isExists(const QString& name) const { return m_propNamesMap.contains(name); }

	bool setValue(const QString& name, const QVariant& value);

	bool setAnyValue(const QString& name, const QVariant& value);		// setter without isEnum checking

	template<typename ENUM_TYPE>
	bool setEnumValue(const QString& name, ENUM_TYPE enumItemValue);

	bool setValue(const SignalSpecPropValue& propValue);

	bool getValue(const QString& name, QVariant* qv) const;
	bool getValue(const QString& name, QVariant* qv, bool* isEnum) const;

	bool serializeValuesToArray(QByteArray* protoData) const;
	bool parseValuesFromArray(const QByteArray& protoData);

	bool save(Proto::SignalSpecPropValues* protoValues) const;

	const QVector<SignalSpecPropValue>& values() const { return m_specPropValues; }

private:
	void buildPropNamesMap();

	bool setValue(const QString& name, const QVariant& value, bool isEnum);

	int getPropertyIndex(const QString& name) const;

private:
	QVector<SignalSpecPropValue> m_specPropValues;
	QHash<QString, int> m_propNamesMap;									// prop name => index in m_propSpecValues

};

template<typename ENUM_TYPE>
bool SignalSpecPropValues::setEnumValue(const QString& name, ENUM_TYPE enumItemValue)
{
	if (std::is_enum<ENUM_TYPE>::value == false)
	{
		assert(false);
		return false;
	}

	return setValue(name, static_cast<int>(enumItemValue), true);
}


class SignalProperties : public PropertyObject
{
	Q_OBJECT

public:
	static const QString idCaption;
	static const QString signalGroupIDCaption;
	static const QString signalInstanceIDCaption;
	static const QString changesetIDCaption;
	static const QString checkedOutCaption;
	static const QString userIdCaption;
	static const QString channelCaption;
	static const QString createdCaption;
	static const QString deletedCaption;
	static const QString instanceCreatedCaption;
	static const QString typeCaption;
	static const QString inOutTypeCaption;
	static const QString cacheValidator;
	static const QString appSignalIDCaption;
	static const QString customSignalIDCaption;
	static const QString busTypeIDCaption;
	static const QString captionCaption;
	static const QString captionValidator;
	static const QString analogSignalFormatCaption;
	static const QString dataSizeCaption;
	static const QString lowADCCaption;
	static const QString highADCCaption;
	static const QString lowDACCaption;
	static const QString highDACCaption;
	static const QString lowEngeneeringUnitsCaption;
	static const QString highEngeneeringUnitsCaption;
	static const QString unitCaption;
	static const QString lowValidRangeCaption;
	static const QString highValidRangeCaption;
	static const QString electricLowLimitCaption;
	static const QString electricHighLimitCaption;
	static const QString electricUnitCaption;
	static const QString sensorTypeCaption;
	static const QString outputModeCaption;
	static const QString acquireCaption;
	static const QString decimalPlacesCaption;
	static const QString coarseApertureCaption;
	static const QString fineApertureCaption;
	static const QString adaptiveApertureCaption;
	static const QString filteringTimeCaption;
	static const QString spreadToleranceCaption;
	static const QString byteOrderCaption;
	static const QString equipmentIDCaption;
	static const QString enableTuningCaption;
	static const QString tuningDefaultValueCaption;
	static const QString tuningLowBoundCaption;
	static const QString tuningHighBoundCaption;
	static const QString specificPropertiesStructCaption;

	static const QString categoryIdentification;
	static const QString categorySignalType;
	static const QString categoryDataFormat;
	static const QString categorySignalProcessing;
	static const QString categoryElectricParameters;
	static const QString categoryOnlineMonitoringSystem;
	static const QString categoryTuning;
	static const QString categoryExpertProperties;

	static const QString lastEditedSignalFieldValuePlace;

public:
	explicit SignalProperties(Signal& signal);

	Signal& signal() { return m_signal; }
	void updateSpecPropValues();

	const std::vector<Property*>& propertiesDependentOnPrecision() { return m_propertiesDependentOnPrecision; }
	void addPropertyDependentOnPrecision(Property* dependentProperty);

	QString specPropStruct() const { return m_signal.specPropStruct(); }
	void setSpecPropStruct(const QString & specPropStruct);

	Q_INVOKABLE QString appSignalID() const { return m_signal.appSignalID(); }
	Q_INVOKABLE QString customAppSignalID() const { return m_signal.customAppSignalID(); }
	Q_INVOKABLE QString caption() const { return m_signal.caption(); }
	Q_INVOKABLE int dataSize() const { return m_signal.dataSize(); }
	Q_INVOKABLE int lowADC() const { return m_signal.lowADC(); }
	Q_INVOKABLE int highADC() const { return m_signal.highADC(); }
	Q_INVOKABLE double lowEngeneeringUnits() const { return m_signal.lowEngeneeringUnits(); }
	Q_INVOKABLE double highEngeneeringUnits() const { return m_signal.highEngeneeringUnits(); }
	Q_INVOKABLE double lowValidRange() const { return m_signal.lowValidRange(); }
	Q_INVOKABLE double highValidRange() const { return m_signal.highValidRange(); }
	Q_INVOKABLE double inputLowLimit() const { return m_signal.electricLowLimit(); }
	Q_INVOKABLE double inputHighLimit() const { return m_signal.electricHighLimit(); }
	Q_INVOKABLE int jsInputUnitID() const { return static_cast<int>(m_signal.electricUnit());}
	Q_INVOKABLE int jsInputSensorType() const { return static_cast<int>(m_signal.sensorType());}
	Q_INVOKABLE int jsOutputMode() const { return static_cast<int>(m_signal.outputMode());}
	Q_INVOKABLE bool acquire() const { return m_signal.acquire(); }
	Q_INVOKABLE int decimalPlaces() const { return m_signal.decimalPlaces(); }
	Q_INVOKABLE double aperture() const { return m_signal.coarseAperture(); }
	Q_INVOKABLE E::SignalInOutType inOutType() const { return m_signal.inOutType(); }
	Q_INVOKABLE QString equipmentID() const { return m_signal.equipmentID(); }
	Q_INVOKABLE double filteringTime() const { return m_signal.filteringTime(); }
	Q_INVOKABLE double spreadTolerance() const { return m_signal.spreadTolerance(); }
	Q_INVOKABLE E::ByteOrder byteOrder() const { return m_signal.byteOrder(); }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_signal.byteOrder()); }
	Q_INVOKABLE bool enableTuning() const { return m_signal.enableTuning(); }
	Q_INVOKABLE float tuningDefaultValue() const { return m_signal.tuningDefaultValue().toFloat(); }
	Q_INVOKABLE float tuningLowBound() const { return m_signal.tuningLowBound().toFloat(); }
	Q_INVOKABLE float tuningHighBound() const { return m_signal.tuningHighBound().toFloat(); }

private:
	void initProperties();

	void createSpecificProperties();
	void deleteSpecificProperties();

	static std::shared_ptr<OrderedHash<int, QString>> generateOrderedHashFromStringArray(const char* const* array, size_t size);

private:
	Signal m_signal;
	SignalSpecPropValues m_specPropValues;

	static std::shared_ptr<OrderedHash<int, QString>> m_sensorTypeHash;
	static std::shared_ptr<OrderedHash<int, QString>> m_outputModeHash;

	std::vector<Property*> m_propertiesDependentOnPrecision;
};



