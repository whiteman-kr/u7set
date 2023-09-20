#pragma once

#include <memory>
#include <type_traits>

#include "../AppSignalLib/AppSignal.h"
#include "../CommonLib/PropertyObject.h"

class AppSignalPropertyBehavior
{
public:
	void setDependsOnPrecision(bool depends);
	bool dependsOnPrecision() const;

	void set(E::SignalType signalType, E::SignalInOutType inOutType, E::PropertyBehaviourType behaviour);

	E::PropertyBehaviourType get(E::SignalType signalType, E::SignalInOutType inOutType) const;
	E::PropertyBehaviourType get(const AppSignal& s) const;

	void clear();

private:
	int calcIndex(E::SignalType signalType, E::SignalInOutType inOutType) const;
	void privateSet(int index, E::PropertyBehaviourType behaviour);
	E::PropertyBehaviourType privateGet(int index) const;

private:
	static inline const int SIGNAL_TYPE_COUNT = QMetaEnum::fromType<E::SignalType>().keyCount();
	static inline const int IN_OUT_TYPE_COUNT = QMetaEnum::fromType<E::SignalInOutType>().keyCount();
	static inline const int TOTAL_SIGNAL_TYPE_COUNT = SIGNAL_TYPE_COUNT * IN_OUT_TYPE_COUNT;

	bool m_dependsOnPrecision = false;

	// property behaviour for all possible combinations of signalType and signalInOutType
	//
	// index in this array is calculated as signalType * IN_OUT_TYPE_COUNT + signalTypeCount
	//
	std::vector<E::PropertyBehaviourType> m_behaviourType = std::vector<E::PropertyBehaviourType>(
																TOTAL_SIGNAL_TYPE_COUNT,
																E::PropertyBehaviourType::Write);
};

struct AppSignalPropertyDescription
{
	AppSignalPropertyDescription();
	AppSignalPropertyDescription(const QString& propName,
								 QMetaType::Type propType,
								 std::function<QVariant (const AppSignal*)> getter,
								 std::function<void (AppSignal*, const QVariant&)> setter,
								 const std::map<int, QString>& propEnumValues);

	bool specificProperty = false;
	QString name;
	QMetaType::Type type = QMetaType::UnknownType;
	
	std::function<QVariant (const AppSignal*)> valueGetter;
	std::function<void (AppSignal*, const QVariant&)> valueSetter;

	//

	std::map<int, QString> enumValues;
	std::set<int> signalsWithThisProperty;			// set of Signal.ID

	//

	AppSignalPropertyBehavior behaviour;

	//

	bool isValid() const;

	void setEnumValues(const std::vector<std::pair<int, QString>>& enumValuesVector);
	void joinEnumValues(const std::vector<std::pair<int, QString>>& enumValuesVector);
	bool getEnumValuesVector(std::vector<std::pair<int, QString>>* enumValuesVector) const;

	bool isEnumProperty() const;
	QString getEnumValueStr(int enumValue) const;

	void appendSignalID(int signalID);
	bool isSignalHaveProperty(int signalID) const;
	bool isSpecificProperty() const;
};

class AppSignalProperties : public PropertyObject
{
	Q_OBJECT

public:
	static const QString appSignalIDTemplateCaption;
	static const QString customAppSignalIDTemplateCaption;
	static const QString appSignalCaptionTemplateCaption;

	static const QString categoryIdentification;
	static const QString categorySignalType;
	static const QString categoryDataFormat;
	static const QString categorySignalProcessing;
	static const QString categoryElectricParameters;
	static const QString categoryOnlineMonitoringSystem;
	static const QString categoryTuning;
	static const QString categoryExpertProperties;

public:
	explicit AppSignalProperties(const AppSignal& signal, bool savePropertyDescription = false);

	AppSignal& signal() { return m_signal; }
	const AppSignal& signal() const { return m_signal; }

	int signalID() const { return m_signal.ID(); }
	bool signalCheckedOut() const { return m_signal.checkedOut(); }

	void updateSpecPropValues();

	QString specPropStruct() const { return m_signal.specPropStruct(); }
	void setSpecPropStruct(const QString& specPropStruct);

	int getPrecision();

	std::vector<AppSignalPropertyDescription> getProperties() const { return m_propertyDescription; }

	bool isNonSpecificPropertyExists(const QString& propertyName) const;

	static bool isPropertyExists(const AppSignal& signal, const QString& propertyName);

	static QString lastEditedSignalPropsPrefix(const AppSignal& s);

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
	void initProperties(bool savePropertyDescription);
	void createSpecificProperties();
	void deleteSpecificProperties();

	template <class TYPE>
	typename std::enable_if<std::is_enum<TYPE>::value == true>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const AppSignal&)> getter,
						   std::function<void (AppSignal&, TYPE)> setter = std::function<void (AppSignal&, TYPE)>());

	template <class TYPE>
	typename std::enable_if<std::is_enum<TYPE>::value == false && std::is_same<TYPE, TuningValue>::value == false>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const AppSignal&)> getter,
						   std::function<void (AppSignal&, TYPE)> setter = std::function<void (AppSignal&, TYPE)>());

	template <class TYPE>
	typename std::enable_if<std::is_same<TYPE, TuningValue>::value == true>::type
	addPropertyDescription(const QString& name,
						   std::function<TYPE (const AppSignal&)> getter,
						   std::function<void (AppSignal&, TYPE)> setter = std::function<void (AppSignal&, TYPE)>());

private:
	AppSignal m_signal;
	AppSignalSpecPropValues m_specPropValues;

	std::vector<AppSignalPropertyDescription> m_propertyDescription;
};

template <class TYPE>
typename std::enable_if<std::is_enum<TYPE>::value == true>::type
AppSignalProperties::addPropertyDescription(const QString& name,
					   std::function<TYPE (const AppSignal&)> getter,
					   std::function<void (AppSignal&, TYPE)> setter)
{
	static_assert(std::is_enum<TYPE>::value);

	AppSignalPropertyDescription newProperty;

	newProperty.name = name;

	newProperty.enumValues = E::enumValuesMap<TYPE>();
	newProperty.type = QMetaType::Int;

	newProperty.valueGetter = [getter](const AppSignal* s){ return TO_INT(getter(*s)); };
	if (setter == nullptr)
	{
		newProperty.valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.valueSetter = [setter](AppSignal* s, const QVariant& v){ setter(*s, IntToEnum<TYPE>(v.toInt())); };
	}

	m_propertyDescription.push_back(newProperty);
}

template <class TYPE>
typename std::enable_if<std::is_enum<TYPE>::value == false && std::is_same<TYPE, TuningValue>::value == false>::type
AppSignalProperties::addPropertyDescription(const QString& name,
					   std::function<TYPE (const AppSignal&)> getter,
					   std::function<void (AppSignal&, TYPE)> setter)
{
	static_assert(std::is_enum<TYPE>::value == false);

	AppSignalPropertyDescription newProperty;

	newProperty.name = name;
	newProperty.type = static_cast<QMetaType::Type>(qMetaTypeId<TYPE>());

	newProperty.valueGetter = [getter](const AppSignal* s)
	{
		QVariant value = QVariant::fromValue<TYPE>(getter(*s));
		if (value.typeId() == QMetaType::QString)
		{
			value = QVariant::fromValue<QString>(value.toString().replace(QChar::LineFeed, QChar::Space));
		}
		return value;
	};
	if (setter == nullptr)
	{
		newProperty.valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.valueSetter = [setter](AppSignal* s, const QVariant& v){ setter(*s, v.value<TYPE>()); };
	}

	m_propertyDescription.push_back(newProperty);
}

template <class TYPE>
typename std::enable_if<std::is_same<TYPE, TuningValue>::value == true>::type
AppSignalProperties::addPropertyDescription(const QString& name,
					   std::function<TYPE (const AppSignal&)> getter,
					   std::function<void (AppSignal&, TYPE)> setter)
{
	AppSignalPropertyDescription newProperty;

	newProperty.name = name;
	newProperty.type = static_cast<QMetaType::Type>(qMetaTypeId<TuningValue>());

	newProperty.valueGetter = [getter](const AppSignal* s){ return getter(*s).toVariant(); };
	if (setter == nullptr)
	{
		newProperty.valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.valueSetter = [getter, setter](AppSignal* s, const QVariant& v)
		{
			TuningValue newValue(getter(*s));
			if (v.typeId() == QMetaType::QString)
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

