#include "AppSignalProperties.h"
#include "../UtilsLib/WUtils.h"

// ------------------------------------------------------------------------------------------------
//
// AppSignalPropertyBehavior class implementation
//
// ------------------------------------------------------------------------------------------------

void AppSignalPropertyBehavior::setDependsOnPrecision(bool depends)
{
	m_dependsOnPrecision = depends;
}

bool AppSignalPropertyBehavior::dependsOnPrecision() const
{
	return m_dependsOnPrecision;
}

void AppSignalPropertyBehavior::set(E::SignalType signalType,
									E::SignalInOutType inOutType,
									E::PropertyBehaviourType behaviour)
{
	privateSet(calcIndex(signalType, inOutType), behaviour);
}

E::PropertyBehaviourType AppSignalPropertyBehavior::get(E::SignalType signalType, E::SignalInOutType inOutType) const
{
	return privateGet(calcIndex(signalType, inOutType));
}

E::PropertyBehaviourType AppSignalPropertyBehavior::get(const AppSignal& s) const
{
	return get(s.signalType(), s.inOutType());
}

void AppSignalPropertyBehavior::clear()
{
	m_dependsOnPrecision = false;

	m_behaviourType = std::vector<E::PropertyBehaviourType>(TOTAL_SIGNAL_TYPE_COUNT,
															E::PropertyBehaviourType::Write);
}

int AppSignalPropertyBehavior::calcIndex(E::SignalType signalType, E::SignalInOutType inOutType) const
{
	return TO_INT(signalType) * IN_OUT_TYPE_COUNT + TO_INT(inOutType);
}

void AppSignalPropertyBehavior::privateSet(int index, E::PropertyBehaviourType behaviour)
{
	if (index >= 0 && index < static_cast<int>(m_behaviourType.size()))
	{
		m_behaviourType[index] = behaviour;
	}
	else
	{
		Q_ASSERT(false);
	}
}

E::PropertyBehaviourType AppSignalPropertyBehavior::privateGet(int index) const
{
	if (index >= 0 && index < static_cast<int>(m_behaviourType.size()))
	{
		return m_behaviourType[index];
	}

	Q_ASSERT(false);

	return E::PropertyBehaviourType::Write;
}

// ------------------------------------------------------------------------------------------------
//
// AppSignalPropertyDescription struct implementation
//
// ------------------------------------------------------------------------------------------------

AppSignalPropertyDescription::AppSignalPropertyDescription()
{
}

// non enum property constructor
//
AppSignalPropertyDescription::AppSignalPropertyDescription(	const QString& propName,
															QMetaType::Type propType,
															bool isSpecificProperty,
															std::function<QVariant (const AppSignal*)> getter,
															std::function<void (AppSignal*, const QVariant&)> setter)
{
	initNonEnumProp(propName, propType, isSpecificProperty, getter, setter);
}

// enum property constructor
//
AppSignalPropertyDescription::AppSignalPropertyDescription(	const QString& propName,
															QMetaType::Type propType,
															bool isSpecificProperty,
															std::function<QVariant (const AppSignal*)> getter,
															std::function<void (AppSignal*, const QVariant&)> setter,
															Hash specPropStructHash,
															const std::map<int, QString>& propEnumValues)
{
	initEnumProp(propName, propType, isSpecificProperty, getter, setter, specPropStructHash, propEnumValues);
}

void AppSignalPropertyDescription::initNonEnumProp(const QString& propName,
													QMetaType::Type propType,
													bool isSpecificProperty,
													std::function<QVariant (const AppSignal*)> getter,
													std::function<void (AppSignal*, const QVariant&)> setter)
{
	init(propName, propType, isSpecificProperty, getter, setter,
		 false, 0, std::map<int, QString>{});
}

void AppSignalPropertyDescription::initEnumProp(const QString& propName,
												QMetaType::Type propType,
												bool isSpecificProperty,
												std::function<QVariant (const AppSignal*)> getter,
												std::function<void (AppSignal*, const QVariant&)> setter,
												Hash specPropStructHash,
												const std::map<int, QString>& propEnumValues)
{
	init(propName, propType, isSpecificProperty, getter, setter,
		 true, specPropStructHash, propEnumValues);
}

bool AppSignalPropertyDescription::isValid() const
{
	return m_name.isEmpty() == false;
}

bool AppSignalPropertyDescription::isSpecificProperty() const
{
	return m_isSpecProp;
}

bool AppSignalPropertyDescription::isEnumProperty() const
{
	return m_isEnumProp;
}

const QString& AppSignalPropertyDescription::name() const
{
	return m_name;
}

QMetaType::Type AppSignalPropertyDescription::type() const
{
	return m_type;
}

E::PropertyBehaviourType AppSignalPropertyDescription::getBehaviour(E::SignalType signalType, E::SignalInOutType inOutType) const
{
	return m_behaviour.get(signalType, inOutType);
}

E::PropertyBehaviourType AppSignalPropertyDescription::getBehaviour(const AppSignal& s) const
{
	return m_behaviour.get(s);
}

bool AppSignalPropertyDescription::dependsOnPrecision() const
{
	return m_behaviour.dependsOnPrecision();
}

void AppSignalPropertyDescription::clearBehaviour()
{
	m_behaviour.clear();
}

void AppSignalPropertyDescription::setBehaviour(const AppSignalPropertyBehavior& bh)
{
	m_behaviour = bh;
}

void AppSignalPropertyDescription::setEnumValues(Hash specPropStructHash, const std::vector<std::pair<int, QString>>& enumValuesVector)
{
	if (m_isEnumProp == false)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_isSpecProp == false)
	{
		Q_ASSERT(specPropStructHash == AppSignalProperties::NON_SPECIFIC_PROP_HASH);
	}
	else
	{
		Q_ASSERT(specPropStructHash != AppSignalProperties::NON_SPECIFIC_PROP_HASH);
	}

	if (enumValuesVector.empty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	auto it = m_enumsValues.find(specPropStructHash);

	if (it == m_enumsValues.end())
	{
		auto [newIt, b] = m_enumsValues.emplace(specPropStructHash, std::map<int, QString>{ enumValuesVector.begin(),
																							enumValuesVector.end() } );
	}
	else
	{
		it->second = std::map<int, QString>{ enumValuesVector.begin(), enumValuesVector.end() };
	}
}

void AppSignalPropertyDescription::checkEnumValues(Hash specPropStructHash, const std::vector<std::pair<int, QString>>& enumValuesVector)
{
	if (m_isEnumProp == false)
	{
		Q_ASSERT(false);
		return;
	}

	if (enumValuesVector.empty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_isSpecProp == false)
	{
		specPropStructHash = AppSignalProperties::NON_SPECIFIC_PROP_HASH;
	}

	auto it = m_enumsValues.find(specPropStructHash);

	if (it == m_enumsValues.end())
	{
		setEnumValues(specPropStructHash, enumValuesVector);
		return;
	}

	std::map<int, QString>& enumValues = it->second;

	for(const auto& p : enumValuesVector)
	{
		auto it2 = enumValues.find(p.first);

		if (it2 != enumValues.end())
		{
			Q_ASSERT(p.second == it2->second);
		}
		else
		{
			Q_ASSERT(false);			// different enumValuesVector for same specPropStructHash, why?
		}
	}
}

bool AppSignalPropertyDescription::getEnumValuesVector(Hash specPropStructHash, std::vector<std::pair<int, QString>>* enumValuesVector) const
{
	TEST_PTR_RETURN_FALSE(enumValuesVector);

	enumValuesVector->clear();

	if (m_isEnumProp == false)
	{
		Q_ASSERT(false);
		return false;
	}

	if (m_isSpecProp == false)
	{
		specPropStructHash = AppSignalProperties::NON_SPECIFIC_PROP_HASH;
	}

	auto it = m_enumsValues.find(specPropStructHash);

	if (it == m_enumsValues.end())
	{
		Q_ASSERT(false);
		return false;
	}

	const std::map<int, QString>& enumValues = it->second;

	*enumValuesVector = std::vector<std::pair<int, QString>>{ enumValues.begin(), enumValues.end() };

	return true;
}

QString AppSignalPropertyDescription::getEnumValueStr(Hash specPropStructHash, int enumValue) const
{
	if (m_isEnumProp == false)
	{
		Q_ASSERT(false);
		return QString();
	}

	if (m_isSpecProp == false)
	{
		specPropStructHash = AppSignalProperties::NON_SPECIFIC_PROP_HASH;
	}

	auto it = m_enumsValues.find(specPropStructHash);

	if (it == m_enumsValues.end())
	{
		Q_ASSERT(false);
		return QString();
	}

	const std::map<int, QString>& enumValues = it->second;

	auto it2 = enumValues.find(enumValue);

	if (it2 == enumValues.end())
	{
		Q_ASSERT(false);
		return QString();
	}

	return it2->second;
}

void AppSignalPropertyDescription::appendSignalID(int signalID)
{
	m_signalsWithThisProperty.insert(signalID);
}

bool AppSignalPropertyDescription::isSignalHaveProperty(int signalID) const
{
	return m_signalsWithThisProperty.contains(signalID);
}

std::function<QVariant (const AppSignal*)> AppSignalPropertyDescription::getter() const
{
	return m_valueGetter;
}

std::function<void (AppSignal*, const QVariant&)> AppSignalPropertyDescription::setter()
{
	return m_valueSetter;
}

void AppSignalPropertyDescription::init(const QString& propName,
										QMetaType::Type propType,
										bool isSpecificProperty,
										std::function<QVariant (const AppSignal*)> getter,
										std::function<void (AppSignal*, const QVariant&)> setter,
										bool isEnum,
										Hash specPropStructHash,
										const std::map<int, QString>& propEnumValues)
{
	m_name = propName;
	m_type = propType;
	m_isSpecProp = isSpecificProperty;
	m_valueGetter = getter;
	m_valueSetter= setter;
	m_isEnumProp = isEnum;

	if (m_isEnumProp == true)
	{
		if (m_isSpecProp)
		{
			Q_ASSERT(specPropStructHash != AppSignalProperties::NON_SPECIFIC_PROP_HASH);
		}
		else
		{
			Q_ASSERT(specPropStructHash == AppSignalProperties::NON_SPECIFIC_PROP_HASH);
		}

		if (propEnumValues.empty() == false)
		{
			m_enumsValues.emplace(specPropStructHash, propEnumValues);
		}
		else
		{
			Q_ASSERT(false);
		}
	}
	else
	{
		m_enumsValues.clear();
	}
}


// --------------------------------------------------------------------------------------------------
//
// AppSignalProperties class implementation
//
// --------------------------------------------------------------------------------------------------

const QString AppSignalProperties::categoryIdentification("1 Identification");
const QString AppSignalProperties::categorySignalType("2 Signal type");
const QString AppSignalProperties::categoryDataFormat("3 Data format");
const QString AppSignalProperties::categorySignalProcessing("4 Signal processing");
const QString AppSignalProperties::categoryElectricParameters("5 Electric parameters");
const QString AppSignalProperties::categoryOnlineMonitoringSystem("6 Online monitoring system");
const QString AppSignalProperties::categoryTuning("7 Tuning");
const QString AppSignalProperties::categoryExpertProperties("8 Expert properties");

AppSignalProperties::AppSignalProperties(const AppSignal& signal, bool savePropertyDescription) :
	m_signal(signal)
{
	initProperties(savePropertyDescription);
}

void AppSignalProperties::updateSpecPropValues()
{
	for (const AppSignalSpecPropValue& value : m_specPropValues.values())
	{
		std::shared_ptr<Property> property = propertyByCaption(value.name());

		if (property == nullptr)
		{
			assert(false);
			continue;
		}

		m_specPropValues.setAnyValue(property->caption(), property->value());
	}

	QByteArray valuesData;

	m_specPropValues.serializeValuesToArray(&valuesData);
	m_signal.setProtoSpecPropValues(valuesData);
}

void AppSignalProperties::setSpecPropStruct(const QString& specPropStruct)
{
	deleteSpecificProperties();

	bool result = m_specPropValues.updateFromSpecPropStruct(specPropStruct);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_signal.setSpecPropStruct(specPropStruct);

	QByteArray protoData;

	m_specPropValues.serializeValuesToArray(&protoData);

	m_signal.setProtoSpecPropValues(protoData);

	createSpecificProperties();
}

int AppSignalProperties::getPrecision()
{
	std::shared_ptr<Property> precisionProperty = propertyByCaption(AppSignalPropNames::DECIMAL_PLACES);

	if (precisionProperty == nullptr)
	{
		return -1;				// is not an error!
	}

	bool ok = true;

	int precision = precisionProperty->value().toInt(&ok);

	RETURN_VALUE_IF_FALSE(ok, 0);

	return precision;
}

bool AppSignalProperties::isNonSpecificPropertyExists(const QString& propertyName) const
{
	Q_ASSERT(m_propertyDescription.size() > 0);

	for(const AppSignalPropertyDescription& prop : m_propertyDescription)
	{
		if (prop.name() == propertyName)
		{
			return true;
		}
	}

	return false;
}

void AppSignalProperties::setReadOnly(bool readOnly)
{
	std::vector<std::shared_ptr<Property>> props = properties();

	for (auto& p : props)
	{
		TEST_PTR_CONTINUE(p);

		p->setReadOnly(readOnly);
	}
}

bool AppSignalProperties::isPropertyExists(const AppSignal& signal, const QString& propertyName)
{
	AppSignalProperties signalProperties(signal, true);

	if (signalProperties.isNonSpecificPropertyExists(propertyName) == true)
	{
		return true;
	}

	PropertyObject propObj;

	propObj.parseSpecificPropertiesStruct(signal.specPropStruct());

	return propObj.propertyExists(propertyName);
}

QString AppSignalProperties::lastEditedSignalPropsPrefix(const AppSignal& s)
{
	return "SignalsTabPage/LastEditedSignalProps/" + E::valueToString(s.signalType()) + "/";
}

#define ADD_SIGNAL_PROPERTY_GETTER(TYPE, NAME, VISIBLE, GETTER, OWNER) \
	ADD_PROPERTY_GETTER_INDIRECT(TYPE, NAME, VISIBLE, GETTER, OWNER); \
	if (savePropertyDescription == true) \
	{ \
		addPropertyDescription<TYPE>(NAME, &GETTER); \
	}

#define ADD_SIGNAL_PROPERTY_GETTER_SETTER(TYPE, NAME, VISIBLE, GETTER, SETTER, OWNER) \
	ADD_PROPERTY_GETTER_SETTER_INDIRECT(TYPE, NAME, VISIBLE, GETTER, SETTER, OWNER); \
	if (savePropertyDescription == true) \
	{ \
		addPropertyDescription<TYPE>(NAME, &GETTER, &SETTER); \
	}

void AppSignalProperties::initProperties(bool savePropertyDescription)
{
	ADD_SIGNAL_PROPERTY_GETTER(int, AppSignalPropNames::ID, false, AppSignal::ID, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(int, AppSignalPropNames::SIGNAL_GROUP_ID, false, AppSignal::signalGroupID, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(int, AppSignalPropNames::SIGNAL_INSTANCE_ID, false, AppSignal::signalInstanceID, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(int, AppSignalPropNames::CHANGESET_ID, false, AppSignal::changesetID, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(bool, AppSignalPropNames::CHECKED_OUT, false, AppSignal::checkedOut, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(int, AppSignalPropNames::USER_ID, false, AppSignal::userID, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(E::Channel, AppSignalPropNames::CHANNEL, false, AppSignal::channel, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(QDateTime, AppSignalPropNames::CREATED, false, AppSignal::created, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(bool, AppSignalPropNames::DELETED, false, AppSignal::deleted, m_signal);
	ADD_SIGNAL_PROPERTY_GETTER(QDateTime, AppSignalPropNames::INSTANCE_CREATED, false, AppSignal::instanceCreated, m_signal);

	// 1 Identification

	auto propAppSignalID = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::APP_SIGNAL_ID,
															 true, AppSignal::appSignalID, AppSignal::setAppSignalID, m_signal);
	propAppSignalID->setValidator(AppSignal::IDENTIFICATORS_VALIDATOR);
	propAppSignalID->setCategory(categoryIdentification);
	propAppSignalID->setViewOrder(10);

	auto propCustomAppSignalID = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
																   true, AppSignal::customAppSignalID, AppSignal::setCustomAppSignalID, m_signal);
	propCustomAppSignalID->setValidator(AppSignal::IDENTIFICATORS_VALIDATOR);
	propCustomAppSignalID->setCategory(categoryIdentification);
	propCustomAppSignalID->setViewOrder(20);

	auto propCaption = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::CAPTION,
														 true, AppSignal::caption, AppSignal::setCaption, m_signal);
	propCaption->setValidator(AppSignal::CAPTION_VALIDATOR);
	propCaption->setCategory(categoryIdentification);
	propCaption->setViewOrder(30);

	auto propEquipmentID = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::EQUIPMENT_ID,
															 true, AppSignal::equipmentID, AppSignal::setEquipmentID, m_signal);
	propEquipmentID->setCategory(categoryIdentification);
	propEquipmentID->setViewOrder(40);

	// 2 Signal type

	auto propSignalInOutType = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::SignalInOutType, AppSignalPropNames::IN_OUT_TYPE,
																 true, AppSignal::inOutType, AppSignal::setInOutType, m_signal);
	propSignalInOutType->setCategory(categorySignalType);
	propSignalInOutType->setViewOrder(10);

	auto propSignalType = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::SignalType, AppSignalPropNames::TYPE,
															true, AppSignal::signalType, AppSignal::setSignalType, m_signal);
	propSignalType->setCategory(categorySignalType);
	propSignalType->setViewOrder(20);

	// 3 Data format

	auto propByteOrder = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::ByteOrder, AppSignalPropNames::BYTE_ORDER_PROP,
														   true, AppSignal::byteOrder, AppSignal::setByteOrder, m_signal);
	propByteOrder->setCategory(categoryDataFormat);
	propByteOrder->setViewOrder(10);

	auto propDataSize = ADD_SIGNAL_PROPERTY_GETTER_SETTER(int, AppSignalPropNames::DATA_SIZE,
														  true, AppSignal::dataSize, AppSignal::setDataSize, m_signal);
	propDataSize->setCategory(categoryDataFormat);
	propDataSize->setViewOrder(20);

	auto propAnalogSignalFormat = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::AnalogAppSignalFormat, AppSignalPropNames::ANALOG_SIGNAL_FORMAT,
														  true, AppSignal::analogSignalFormat, AppSignal::setAnalogSignalFormat, m_signal);
	propAnalogSignalFormat->setCategory(categoryDataFormat);
	propAnalogSignalFormat->setViewOrder(30);

	auto propBusTypeID = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::BUS_TYPE_ID,
														   true, AppSignal::busTypeID, AppSignal::setBusTypeID, m_signal);
	propBusTypeID->setCategory(categoryDataFormat);
	propBusTypeID->setViewOrder(40);

	// 4 Signal processing

	auto propUnit = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::UNIT, true, AppSignal::unit, AppSignal::setUnit, m_signal);
	propUnit->setCategory(categorySignalProcessing);
	propUnit->setViewOrder(90);

	auto propExcludeFromBuild = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::EXCLUDE_FROM_BUILD,
																  true,	AppSignal::excludeFromBuild, AppSignal::setExcludeFromBuild, m_signal);
	propExcludeFromBuild->setCategory(categorySignalProcessing);
	propExcludeFromBuild->setViewOrder(100);

	auto propInvertSignal = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::INVERT_SIGNAL, true, AppSignal::invertSignal, AppSignal::setInvertSignal, m_signal);
	propInvertSignal->setCategory(categorySignalProcessing);
	propInvertSignal->setViewOrder(110);

	auto propSwCalcFunc = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::SoftwareCalcFunction, AppSignalPropNames::SOFTWARE_CALC_FUNCTION,
															true, AppSignal::swCalcFunction, AppSignal::setSwCalcFunction, m_signal);
	propSwCalcFunc->setCategory(categorySignalProcessing);
	propSwCalcFunc->setViewOrder(120);

	// 5 Electric parameters ()

	// 6 Online monitoring system

	auto propAcquire = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ACQUIRE,
														 true, AppSignal::acquire, AppSignal::setAcquire, m_signal);
	propAcquire->setCategory(categoryOnlineMonitoringSystem);
	propAcquire->setViewOrder(10);

	auto propArchive = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ARCHIVE,
														 true, AppSignal::archive, AppSignal::setArchive, m_signal);
	propArchive->setCategory(categoryOnlineMonitoringSystem);
	propArchive->setViewOrder(20);

	auto propReserved = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::RESERVED, true, AppSignal::reserved, AppSignal::setReserved, m_signal);
	propReserved->setCategory(categoryOnlineMonitoringSystem);
	propReserved->setViewOrder(25);

	auto propApertureType = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::ApertureType, AppSignalPropNames::APERTURE_TYPE,
															true, AppSignal::apertureType, AppSignal::setApertureType, m_signal);
	propApertureType->setCategory(categoryOnlineMonitoringSystem);
	propApertureType->setDescription(QString("RangePercent - aperture is set in % of signal range\n"
												 "ValuePercent - aperture is set in % of current signal value\n"
												 "AbsValue - aperture is set in absolute engineering units"));
	propApertureType->setViewOrder(30);

	auto propFineAperture = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::FINE_APERTURE,
															  true, AppSignal::fineAperture, AppSignal::setFineAperture, m_signal);
	propFineAperture->setPrecision(4);
	propFineAperture->setCategory(categoryOnlineMonitoringSystem);
	propFineAperture->setViewOrder(40);

	auto propCoarseAperture = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::COARSE_APERTURE,
																true, AppSignal::coarseAperture, AppSignal::setCoarseAperture, m_signal);
	propCoarseAperture->setPrecision(4);
	propCoarseAperture->setCategory(categoryOnlineMonitoringSystem);
	propCoarseAperture->setViewOrder(50);

	auto propDecimalPlaces = ADD_SIGNAL_PROPERTY_GETTER_SETTER(int, AppSignalPropNames::DECIMAL_PLACES,
															   true, AppSignal::decimalPlaces, AppSignal::setDecimalPlaces, m_signal);
	propDecimalPlaces->setCategory(categoryOnlineMonitoringSystem);
	propDecimalPlaces->setViewOrder(60);

	auto propTags = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::TAGS,
													  true, AppSignal::tagsStr, AppSignal::setTagsStr, m_signal);
	propTags->setCategory(categoryOnlineMonitoringSystem);
	propTags->setSpecificEditor(E::PropertySpecificEditor::Tags);
	propTags->setViewOrder(70);

	// 7 Tuning

	auto propEnableTuning = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ENABLE_TUNING,
															  true, AppSignal::enableTuning, AppSignal::setEnableTuning, m_signal);
	propEnableTuning->setCategory(categoryTuning);
	propEnableTuning->setViewOrder(10);

	auto propTuningDefaultValue = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_DEFAULT_VALUE,
																	true, AppSignal::tuningDefaultValue, AppSignal::setTuningDefaultValue, m_signal);
	propTuningDefaultValue->setCategory(categoryTuning);
	propTuningDefaultValue->setViewOrder(20);

	auto propTuningLowBound = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_LOW_BOUND,
																true, AppSignal::tuningLowBound, AppSignal::setTuningLowBound, m_signal);
	propTuningLowBound->setCategory(categoryTuning);
	propTuningLowBound->setViewOrder(30);

	auto propTuningHighBound = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_HIGH_BOUND,
																 true, AppSignal::tuningHighBound, AppSignal::setTuningHighBound, m_signal);
	propTuningHighBound->setCategory(categoryTuning);
	propTuningHighBound->setViewOrder(40);

	// append signal specific properties
	//
	createSpecificProperties();

	auto propSpecPropStruct = ADD_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::SPECIFIC_PROPERTIES_STRUCT, true,
															 AppSignalProperties::specPropStruct, AppSignalProperties::setSpecPropStruct);
	propSpecPropStruct->setCategory(categoryExpertProperties);
	propSpecPropStruct->setExpert(true);
	propSpecPropStruct->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);
	propSpecPropStruct->setViewOrder(10);

	if (savePropertyDescription)
	{
		addPropertyDescription<QString>(AppSignalPropNames::SPECIFIC_PROPERTIES_STRUCT,
										&AppSignal::specPropStruct);
	}
}

void AppSignalProperties::createSpecificProperties()
{
	m_specPropValues.create(m_signal);

	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(m_signal.specPropStruct());

	if (result.first == false)
	{
		assert(false);
		return;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		QVariant qv;

		bool getValueResult = m_specPropValues.getValue(specificProperty->caption(), &qv);

		Q_ASSERT(getValueResult);
		Q_UNUSED(getValueResult)

		specificProperty->setValue(qv);

		addProperty(specificProperty);
	}
}

void AppSignalProperties::deleteSpecificProperties()
{
	const QVector<AppSignalSpecPropValue>& values = m_specPropValues.values();

	for(const AppSignalSpecPropValue& value : values)
	{
		removeProperty(value.name());
	}
}

