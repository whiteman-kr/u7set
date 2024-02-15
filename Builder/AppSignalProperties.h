#pragma once

#include <memory>
#include <type_traits>

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

class AppSignalPropertyDescription
{
public:
	AppSignalPropertyDescription();

	// non enum property constructor
	//
	AppSignalPropertyDescription(const QString& propName,
								QMetaType::Type propType,
								bool isSpecificProperty,
								std::function<QVariant (const AppSignal*)> getter,
								std::function<void (AppSignal*, const QVariant&)> setter);

	// enum property constructor
	//
	AppSignalPropertyDescription(const QString& propName,
								QMetaType::Type propType,
								bool isSpecificProperty,
								std::function<QVariant (const AppSignal*)> getter,
								std::function<void (AppSignal*, const QVariant&)> setter,
								Hash specPropStructHash,
								const std::map<int, QString>& propEnumValues);

	void initNonEnumProp(const QString& propName,
						QMetaType::Type propType,
						bool isSpecificProperty,
						std::function<QVariant (const AppSignal*)> getter,
						std::function<void (AppSignal*, const QVariant&)> setter);


	void initEnumProp(const QString& propName,
						QMetaType::Type propType,
						bool isSpecificProperty,
						std::function<QVariant (const AppSignal*)> getter,
						std::function<void (AppSignal*, const QVariant&)> setter,
						Hash specPropStructHash,
						const std::map<int, QString>& propEnumValues);

	bool isValid() const;
	bool isSpecificProperty() const;
	bool isEnumProperty() const;

	const QString& name() const;
	QMetaType::Type type() const;

	E::PropertyBehaviourType getBehaviour(E::SignalType signalType, E::SignalInOutType inOutType) const;
	E::PropertyBehaviourType getBehaviour(const AppSignal& s) const;
	bool dependsOnPrecision() const;
	void clearBehaviour();
	void setBehaviour(const AppSignalPropertyBehavior& bh);

	void setEnumValues(Hash specPropStructHash, const std::vector<std::pair<int, QString>>& enumValuesVector);
	void checkEnumValues(Hash specPropStructHash, const std::vector<std::pair<int, QString>>& enumValuesVector);
	bool getEnumValuesVector(Hash specPropStructHash, std::vector<std::pair<int, QString>>* enumValuesVector) const;
	QString getEnumValueStr(Hash specPropStructHash, int enumValue) const;

	void appendSignalID(int signalID);
	bool isSignalHaveProperty(int signalID) const;

	std::function<QVariant (const AppSignal*)> getter() const;
	std::function<void (AppSignal*, const QVariant&)> setter();

private:
	void init(const QString& propName,
			  QMetaType::Type propType,
			  bool isSpecificProperty,
			  std::function<QVariant (const AppSignal*)> getter,
			  std::function<void (AppSignal*, const QVariant&)> setter,
			  bool isEnum,
			  Hash specPropStructHash,
			  const std::map<int, QString>& propEnumValues);

private:
	QString m_name;
	QMetaType::Type m_type = QMetaType::UnknownType;

	bool m_isSpecProp = false;

	std::function<QVariant (const AppSignal*)> m_valueGetter;
	std::function<void (AppSignal*, const QVariant&)> m_valueSetter;

	bool m_isEnumProp = false;

	std::map<Hash, std::map<int, QString>> m_enumsValues;		// specPropStructHash => enumValues
																// due to different specPropStructs can have different sets of
																// enum values with same property name
																// for example property 'SensorType'

	std::set<int> m_signalsWithThisProperty;					// set of Signal.ID()

	AppSignalPropertyBehavior m_behaviour;
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

	inline static const int NON_SPECIFIC_PROP_HASH = 0;

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

	void setReadOnly(bool readOnly);

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
	Q_INVOKABLE double lowPhysicalUnits() const { return m_signal.lowPhysicalUnits(nullptr); }
	Q_INVOKABLE double highPhysicalUnits() const { return m_signal.highPhysicalUnits(nullptr); }
	Q_INVOKABLE double lowValidRange() const { return m_signal.lowValidRange(nullptr); }
	Q_INVOKABLE double highValidRange() const { return m_signal.highValidRange(nullptr); }
	Q_INVOKABLE double electricLowLimit() const { return m_signal.electricLowLimit(nullptr); }
	Q_INVOKABLE double electricHighLimit() const { return m_signal.electricHighLimit(nullptr); }
	Q_INVOKABLE double inputLowLimit() const { return electricLowLimit(); }
	Q_INVOKABLE double inputHighLimit() const { return electricHighLimit(); }
	Q_INVOKABLE int electricUnit() const { return static_cast<int>(m_signal.electricUnit(nullptr));}
	Q_INVOKABLE int sensorType() const { return static_cast<int>(m_signal.sensorType(nullptr));}
	Q_INVOKABLE int outputMode() const { return static_cast<int>(m_signal.outputMode(nullptr));}
	Q_INVOKABLE int rloadOhm() const { return static_cast<int>(m_signal.rloadOhm(nullptr));}
	Q_INVOKABLE int jsInputUnitID() const { return electricUnit();}
	Q_INVOKABLE int jsInputSensorType() const { return sensorType();}
	Q_INVOKABLE int jsOutputMode() const { return outputMode();}
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

	std::function<QVariant (const AppSignal*)> getterFunc = [getter](const AppSignal* s)
															{
																return TO_INT(getter(*s));
															};

	std::function<void (AppSignal*, const QVariant&)> setterFunc;

	if (setter == nullptr)
	{
		setterFunc = [](AppSignal*, const QVariant&){};
	}
	else
	{
		setterFunc = [setter](AppSignal* s, const QVariant& v)
					{
						setter(*s, IntToEnum<TYPE>(v.toInt()));
					};
	}

	AppSignalPropertyDescription newProperty;

	newProperty.initEnumProp(name, QMetaType::Int, false, getterFunc, setterFunc, NON_SPECIFIC_PROP_HASH, E::enumValuesMap<TYPE>());

/*	newProperty.m_name = name;

	newProperty.m_enumsValues.emplace(AppSignalPropertyDescription::NON_SPECIFIC_PROP_HASH, E::enumValuesMap<TYPE>());
	newProperty.m_type = QMetaType::Int;

	newProperty.m_valueGetter = [getter](const AppSignal* s){ return TO_INT(getter(*s)); };
	if (setter == nullptr)
	{
		newProperty.m_valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.m_valueSetter = [setter](AppSignal* s, const QVariant& v){ setter(*s, IntToEnum<TYPE>(v.toInt())); };
	}*/

	m_propertyDescription.push_back(newProperty);
}

template <class TYPE>
typename std::enable_if<std::is_enum<TYPE>::value == false && std::is_same<TYPE, TuningValue>::value == false>::type
AppSignalProperties::addPropertyDescription(const QString& name,
					   std::function<TYPE (const AppSignal&)> getter,
					   std::function<void (AppSignal&, TYPE)> setter)
{
	static_assert(std::is_enum<TYPE>::value == false);

	std::function<QVariant (const AppSignal*)> getterFunc = [getter](const AppSignal* s)
	{
		QVariant value = QVariant::fromValue<TYPE>(getter(*s));
		if (value.typeId() == QMetaType::QString)
		{
			value = QVariant::fromValue<QString>(value.toString().replace(QChar::LineFeed, QChar::Space));
		}
		return value;
	};

	std::function<void (AppSignal*, const QVariant&)> setterFunc;

	if (setter == nullptr)
	{
		setterFunc = [](AppSignal*, const QVariant&){};
	}
	else
	{
		setterFunc = [setter](AppSignal* s, const QVariant& v){ setter(*s, v.value<TYPE>()); };
	}


	AppSignalPropertyDescription newProperty;

	newProperty.initNonEnumProp(name, static_cast<QMetaType::Type>(qMetaTypeId<TYPE>()), false,
								getterFunc, setterFunc);

/*	newProperty.m_name = name;
	newProperty.m_type = static_cast<QMetaType::Type>(qMetaTypeId<TYPE>())

	newProperty.m_valueGetter = [getter](const AppSignal* s)
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
		newProperty.m_valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.m_valueSetter = [setter](AppSignal* s, const QVariant& v){ setter(*s, v.value<TYPE>()); };
	}*/

	m_propertyDescription.push_back(newProperty);
}

template <class TYPE>
typename std::enable_if<std::is_same<TYPE, TuningValue>::value == true>::type
AppSignalProperties::addPropertyDescription(const QString& name,
					   std::function<TYPE (const AppSignal&)> getter,
					   std::function<void (AppSignal&, TYPE)> setter)
{
	std::function<QVariant (const AppSignal*)> getterFunc = [getter](const AppSignal* s)
															{
																return getter(*s).toVariant();
															};

	std::function<void (AppSignal*, const QVariant&)> setterFunc;

	if (setter == nullptr)
	{
		setterFunc = [](AppSignal*, const QVariant&) {};
	}
	else
	{
		setterFunc = [getter, setter](AppSignal* s, const QVariant& v)
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

	AppSignalPropertyDescription newProperty;

	newProperty.initNonEnumProp(name, static_cast<QMetaType::Type>(qMetaTypeId<TuningValue>()),
								false, getterFunc, setterFunc);

/*	newProperty.m_name = name;
	newProperty.m_type = static_cast<QMetaType::Type>(qMetaTypeId<TuningValue>());

	newProperty.m_valueGetter = [getter](const AppSignal* s){ return getter(*s).toVariant(); };
	if (setter == nullptr)
	{
		newProperty.m_valueSetter = [](AppSignal*, const QVariant&){};
	}
	else
	{
		newProperty.m_valueSetter = [getter, setter](AppSignal* s, const QVariant& v)
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
	}*/

	m_propertyDescription.push_back(newProperty);
}

