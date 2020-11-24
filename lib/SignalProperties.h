#pragma once

#include <memory>
#include <type_traits>

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
	void setName(const QString& name) { m_name = name; }

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
	bool setEnumValue(const QString& name, int enumItemValue);

	bool setValue(const SignalSpecPropValue& propValue);

	bool getValue(const QString& name, QVariant* qv) const;
	bool getValue(const QString& name, QVariant* qv, bool* isEnum) const;

	bool serializeValuesToArray(QByteArray* protoData) const;
	bool parseValuesFromArray(const QByteArray& protoData);

	//bool save(Proto::SignalSpecPropValues* protoValues) const;

	const QVector<SignalSpecPropValue>& values() const { return m_specPropValues; }

	void append(const SignalSpecPropValue& value);

	bool replaceName(const QString& oldName, const QString& newName);			// returns true if replacing is occured

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
	static_assert(std::is_enum<ENUM_TYPE>::value == true);
	return setValue(name, static_cast<int>(enumItemValue), true);
}


struct SignalPropertyDescription
{
	QString name;
	QString caption;
	QVariant::Type type;

	std::vector<std::pair<int, QString>> enumValues;

	std::function<QVariant (const Signal*)> valueGetter;
	std::function<void (Signal*, const QVariant&)> valueSetter;
};


class SignalProperties : public PropertyObject
{
	Q_OBJECT

public:
	static QString generateCaption(const QString& name);

	static const QString idCaption;
	static const QString signalGroupIDCaption;
	static const QString signalInstanceIDCaption;
	static const QString changesetIDCaption;
	static const QString checkedOutCaption;
	static const QString userIdCaption;
	static const QString channelCaption;
	static const QString excludeFromBuildCaption;
	static const QString createdCaption;
	static const QString deletedCaption;
	static const QString instanceCreatedCaption;
	static const QString typeCaption;
	static const QString inOutTypeCaption;
	static const QString cacheValidator;
	static const QString upperCacheValidator;
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
	static const QString lowEngineeringUnitsCaption;
	static const QString highEngineeringUnitsCaption;
	static const QString unitCaption;
	static const QString lowValidRangeCaption;
	static const QString highValidRangeCaption;
	static const QString electricLowLimitCaption;
	static const QString electricHighLimitCaption;
	static const QString electricUnitCaption;
	static const QString rload_OhmCaption;
	static const QString sensorTypeCaption;
	static const QString R0_OhmCaption;
	static const QString outputModeCaption;
	static const QString acquireCaption;
	static const QString archiveCaption;
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
	static const QString tagsCaption;

	static const QString appSignalIDTemplateCaption;
	static const QString customAppSignalIDTemplateCaption;
	static const QString appSignalCaptionTemplateCaption;

	static const QString MISPRINT_lowEngineeringUnitsCaption;
	static const QString MISPRINT_highEngineeringUnitsCaption;

	static const QString categoryIdentification;
	static const QString categorySignalType;
	static const QString categoryDataFormat;
	static const QString categorySignalProcessing;
	static const QString categoryElectricParameters;
	static const QString categoryOnlineMonitoringSystem;
	static const QString categoryTuning;
	static const QString categoryExpertProperties;

	static const QString defaultInputAnalogSpecPropStruct;
	static const QString defaultOutputAnalogSpecPropStruct;
	static const QString defaultInternalAnalogSpecPropStruct;
	static const QString defaultBusChildAnalogSpecPropStruct;

	static const QString lastEditedSignalFieldValuePlace;

public:
	explicit SignalProperties(Signal& signal, bool savePropertyDescription = false);

	Signal& signal() { return m_signal; }
	void updateSpecPropValues();

	QString specPropStruct() const { return m_signal.specPropStruct(); }
	void setSpecPropStruct(const QString & specPropStruct);

	int getPrecision();

	std::vector<SignalPropertyDescription> getProperties() const { return m_propertyDescription; }

	Q_INVOKABLE QString appSignalID() const { return m_signal.appSignalID(); }
	Q_INVOKABLE QString customAppSignalID() const { return m_signal.customAppSignalID(); }
	Q_INVOKABLE QString caption() const { return m_signal.caption(); }
	Q_INVOKABLE int dataSize() const { return m_signal.dataSize(); }
	Q_INVOKABLE int lowADC() const { return m_signal.lowADC(nullptr); }
	Q_INVOKABLE int highADC() const { return m_signal.highADC(nullptr); }
	Q_INVOKABLE double lowEngineeringUnits() const { return m_signal.lowEngineeringUnits(nullptr); }
	Q_INVOKABLE double highEngineeringUnits() const { return m_signal.highEngineeringUnits(nullptr); }
	Q_INVOKABLE double lowValidRange() const { return m_signal.lowValidRange(nullptr); }
	Q_INVOKABLE double highValidRange() const { return m_signal.highValidRange(nullptr); }
	Q_INVOKABLE double inputLowLimit() const { return m_signal.electricLowLimit(nullptr); }
	Q_INVOKABLE double inputHighLimit() const { return m_signal.electricHighLimit(nullptr); }
	Q_INVOKABLE int jsInputUnitID() const { return static_cast<int>(m_signal.electricUnit(nullptr));}
	Q_INVOKABLE int jsInputSensorType() const { return static_cast<int>(m_signal.sensorType(nullptr));}
	Q_INVOKABLE int jsOutputMode() const { return static_cast<int>(m_signal.outputMode(nullptr));}
	Q_INVOKABLE bool acquire() const { return m_signal.acquire(); }
	Q_INVOKABLE int decimalPlaces() const { return m_signal.decimalPlaces(); }
	Q_INVOKABLE double aperture() const { return m_signal.coarseAperture(); }
	Q_INVOKABLE E::SignalInOutType inOutType() const { return m_signal.inOutType(); }
	Q_INVOKABLE QString equipmentID() const { return m_signal.equipmentID(); }
	Q_INVOKABLE double filteringTime() const { return m_signal.filteringTime(nullptr); }
	Q_INVOKABLE double spreadTolerance() const { return m_signal.spreadTolerance(nullptr); }
	Q_INVOKABLE E::ByteOrder byteOrder() const { return m_signal.byteOrder(); }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_signal.byteOrder()); }
	Q_INVOKABLE bool enableTuning() const { return m_signal.enableTuning(); }
	Q_INVOKABLE float tuningDefaultValue() const { return m_signal.tuningDefaultValue().toFloat(); }
	Q_INVOKABLE float tuningLowBound() const { return m_signal.tuningLowBound().toFloat(); }
	Q_INVOKABLE float tuningHighBound() const { return m_signal.tuningHighBound().toFloat(); }

private:
	void initProperties(bool savePropertyDescription = false);

	template <class TYPE>
	typename std::enable_if<std::is_enum<TYPE>::value == true>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const Signal&)> getter,
						   std::function<void (Signal&, TYPE)> setter = std::function<void (Signal&, TYPE)>())
	{
		static_assert(std::is_enum<TYPE>::value);

		SignalPropertyDescription newProperty;

		newProperty.name = name;
		newProperty.caption = generateCaption(name);

		newProperty.enumValues = E::enumValues<TYPE>();
		newProperty.type = QVariant::Int;

		newProperty.valueGetter = [getter](const Signal* s){ return TO_INT(getter(*s)); };
		if (setter == nullptr)
		{
			newProperty.valueSetter = [](Signal*, const QVariant&){};
		}
		else
		{
			newProperty.valueSetter = [setter](Signal* s, const QVariant& v){ setter(*s, IntToEnum<TYPE>(v.toInt())); };
		}

		m_propertyDescription.push_back(newProperty);
	}

	template <class TYPE>
	typename std::enable_if<std::is_enum<TYPE>::value == false && std::is_same<TYPE, TuningValue>::value == false>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const Signal&)> getter,
						   std::function<void (Signal&, TYPE)> setter = std::function<void (Signal&, TYPE)>())
	{
		static_assert(std::is_enum<TYPE>::value == false);

		SignalPropertyDescription newProperty;

		newProperty.name = name;
		newProperty.caption = generateCaption(name);

		newProperty.type = static_cast<QVariant::Type>(qMetaTypeId<TYPE>());

		newProperty.valueGetter = [getter](const Signal* s)
		{
			QVariant value = QVariant::fromValue<TYPE>(getter(*s));
			if (value.type() == QVariant::String)
			{
				value = QVariant::fromValue<QString>(value.toString().replace(QChar::LineFeed, QChar::Space));
			}
			return value;
		};
		if (setter == nullptr)
		{
			newProperty.valueSetter = [](Signal*, const QVariant&){};
		}
		else
		{
			newProperty.valueSetter = [setter](Signal* s, const QVariant& v){ setter(*s, v.value<TYPE>()); };
		}

		m_propertyDescription.push_back(newProperty);
	}

	template <class TYPE>
	typename std::enable_if<std::is_same<TYPE, TuningValue>::value == true>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const Signal&)> getter,
						   std::function<void (Signal&, TYPE)> setter = std::function<void (Signal&, TYPE)>())
	{
		SignalPropertyDescription newProperty;

		newProperty.name = name;
		newProperty.caption = generateCaption(name);

		newProperty.type = static_cast<QVariant::Type>(qMetaTypeId<TuningValue>());

		newProperty.valueGetter = [getter](const Signal* s){ return getter(*s).toVariant(); };
		if (setter == nullptr)
		{
			newProperty.valueSetter = [](Signal*, const QVariant&){};
		}
		else
		{
			newProperty.valueSetter = [getter, setter](Signal* s, const QVariant& v)
			{
				TuningValue newValue(getter(*s));
				if (v.type() == QVariant::String)
				{
					bool ok = false;
					newValue.fromString(v.toString(), &ok);
					assert(ok == true);
				}
				else
				{
					newValue.fromVariant(v);
				}
				setter(*s, newValue);
			};
		}

		m_propertyDescription.push_back(newProperty);
	}

	void createSpecificProperties();
	void deleteSpecificProperties();

	static std::shared_ptr<OrderedHash<int, QString>> generateOrderedHashFromStringArray(const char* const* array, size_t size);

private:
	Signal m_signal;
	SignalSpecPropValues m_specPropValues;

	std::vector<SignalPropertyDescription> m_propertyDescription;

	static std::shared_ptr<OrderedHash<int, QString>> m_outputModeHash;
};



