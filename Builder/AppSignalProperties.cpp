#include "AppSignalProperties.h"
#include "../UtilsLib/WUtils.h"


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


AppSignalPropertyDescription::AppSignalPropertyDescription()
{
}

AppSignalPropertyDescription::AppSignalPropertyDescription(const QString& propName,
							 QMetaType::Type propType,
							 std::function<QVariant (const AppSignal*)> getter,
							 std::function<void (AppSignal*, const QVariant&)> setter,
							 const std::map<int, QString>& propEnumValues) :
	name(propName),
	type(propType),
	valueGetter(getter),
	valueSetter(setter),
	enumValues(propEnumValues)
{
}

bool AppSignalPropertyDescription::isValid() const
{
	return name.isEmpty() == false;
}

void AppSignalPropertyDescription::setEnumValues(const std::vector<std::pair<int, QString>>& enumValuesVector)
{
	enumValues.clear();

	for(const auto& p : enumValuesVector)
	{
		enumValues.emplace(p.first, p.second);
	}
}

void AppSignalPropertyDescription::joinEnumValues(const std::vector<std::pair<int, QString>>& enumValuesVector)
{
	for(const auto& p : enumValuesVector)
	{
#ifdef QT_DEBUG
		auto it = enumValues.find(p.first);

		if (it != enumValues.end())
		{
			Q_ASSERT(p.second == it->second);
		}
#endif
		enumValues.emplace(p.first, p.second);
	}
}

bool AppSignalPropertyDescription::getEnumValuesVector(std::vector<std::pair<int, QString>>* enumValuesVector) const
{
	TEST_PTR_RETURN_FALSE(enumValuesVector);

	if (isEnumProperty() == false)
	{
		Q_ASSERT(false);
		return false;
	}

	enumValuesVector->clear();
	enumValuesVector->reserve(enumValues.size());

	for(const auto& p : enumValues)
	{
		enumValuesVector->emplace_back(p.first, p.second);
	}

	return true;
}

bool AppSignalPropertyDescription::isEnumProperty() const
{
	return enumValues.empty() == false;
}

QString AppSignalPropertyDescription::getEnumValueStr(int enumValue) const
{
	auto it = enumValues.find(enumValue);

	if (it == enumValues.end())
	{
		return QString();
	}

	return it->second;
}

void AppSignalPropertyDescription::appendSignalID(int signalID)
{
	signalsWithThisProperty.insert(signalID);
}

bool AppSignalPropertyDescription::isSignalHaveProperty(int signalID) const
{
	return signalsWithThisProperty.contains(signalID);
}

bool AppSignalPropertyDescription::isSpecificProperty() const
{
	return specificProperty;
}

// -------------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValue class implementation
//
// -------------------------------------------------------------------------------------------------------------

const QString AppSignalProperties::categoryIdentification("1 Identification");
const QString AppSignalProperties::categorySignalType("2 Signal type");
const QString AppSignalProperties::categoryDataFormat("3 Data Format");
const QString AppSignalProperties::categorySignalProcessing("4 Signal processing");
const QString AppSignalProperties::categoryElectricParameters("5 Electric parameters");
const QString AppSignalProperties::categoryOnlineMonitoringSystem("6 Online Monitoring System");
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

void AppSignalProperties::setSpecPropStruct(const QString & specPropStruct)
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
		return 0;
	}

	bool ok = true;

	int precision = precisionProperty->value().toInt(&ok);

	if (ok == false)
	{
		return 0;
	}

	return precision;
}

bool AppSignalProperties::isNonSpecificPropertyExists(const QString& propertyName) const
{
	Q_ASSERT(m_propertyDescription.size() > 0);

	for(const AppSignalPropertyDescription& prop : m_propertyDescription)
	{
		if (prop.name == propertyName)
		{
			return true;
		}
	}

	return false;
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

	auto signalTypeProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::SignalType, AppSignalPropNames::TYPE,
																true, AppSignal::signalType, AppSignal::setSignalType, m_signal);
	signalTypeProperty->setCategory(categorySignalType);

	auto signalInOutTypeProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::SignalInOutType, AppSignalPropNames::IN_OUT_TYPE, true, AppSignal::inOutType, AppSignal::setInOutType, m_signal);
	signalInOutTypeProperty->setCategory(categorySignalType);

	auto strIdProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::APP_SIGNAL_ID, true, AppSignal::appSignalID, AppSignal::setAppSignalID, m_signal);
	strIdProperty->setValidator(AppSignal::IDENTIFICATORS_VALIDATOR);
	strIdProperty->setCategory(categoryIdentification);

	auto extStrIdProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::CUSTOM_APP_SIGNAL_ID, true, AppSignal::customAppSignalID, AppSignal::setCustomAppSignalID, m_signal);
	extStrIdProperty->setValidator(AppSignal::IDENTIFICATORS_VALIDATOR);
	extStrIdProperty->setCategory(categoryIdentification);

	auto busTypeIDProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::BUS_TYPE_ID, true, AppSignal::busTypeID, AppSignal::setBusTypeID, m_signal);
	busTypeIDProperty->setCategory(categoryIdentification);

	auto nameProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::CAPTION, true, AppSignal::caption, AppSignal::setCaption, m_signal);
	nameProperty->setValidator(AppSignal::CAPTION_VALIDATOR);
	nameProperty->setCategory(categoryIdentification);

	auto equipmentProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::EQUIPMENT_ID, true, AppSignal::equipmentID, AppSignal::setEquipmentID, m_signal);
	equipmentProperty->setCategory(categoryIdentification);

	auto enableTuningProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ENABLE_TUNING, true, AppSignal::enableTuning, AppSignal::setEnableTuning, m_signal);
	enableTuningProperty->setCategory(categoryTuning);

	auto tuningDefaultValueProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_DEFAULT_VALUE, true, AppSignal::tuningDefaultValue, AppSignal::setTuningDefaultValue, m_signal);
	tuningDefaultValueProperty->setCategory(categoryTuning);

	auto tuningLowBoundProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_LOW_BOUND, true, AppSignal::tuningLowBound, AppSignal::setTuningLowBound, m_signal);
	tuningLowBoundProperty->setCategory(categoryTuning);

	auto tuningHighBoundProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(TuningValue, AppSignalPropNames::TUNING_HIGH_BOUND, true, AppSignal::tuningHighBound, AppSignal::setTuningHighBound, m_signal);
	tuningHighBoundProperty->setCategory(categoryTuning);

	if (savePropertyDescription)
	{
		addPropertyDescription<int>(AppSignalPropNames::DATA_SIZE,
									&AppSignal::dataSize,
									static_cast<void (AppSignal::*)(int)>(&AppSignal::setDataSize));

		addPropertyDescription<E::AnalogAppSignalFormat>(AppSignalPropNames::ANALOG_SIGNAL_FORMAT,
														 &AppSignal::analogSignalFormat,
														 static_cast<void (AppSignal::*)(E::AnalogAppSignalFormat)>(&AppSignal::setAnalogSignalFormat));
	}

	auto dataSizeProperty = addProperty<int>(AppSignalPropNames::DATA_SIZE, QString(), true,
										(std::function<int(void)>)std::bind(&AppSignal::dataSize, &m_signal),
										std::bind(static_cast<void (AppSignal::*)(int)>(&AppSignal::setDataSize), &m_signal, std::placeholders::_1));

	dataSizeProperty->setCategory(categoryDataFormat);

	auto byteOrderProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::ByteOrder, AppSignalPropNames::BYTE_ORDER_PROP, true, AppSignal::byteOrder, AppSignal::setByteOrder, m_signal);
	byteOrderProperty->setCategory(categoryDataFormat);


	auto analogSignalFormatProperty = addProperty<E::AnalogAppSignalFormat>(AppSignalPropNames::ANALOG_SIGNAL_FORMAT, QString(), true,
																		  (std::function<E::AnalogAppSignalFormat(void)>)std::bind(&AppSignal::analogSignalFormat, &m_signal),
																		  std::bind(static_cast<void (AppSignal::*)(E::AnalogAppSignalFormat)>(&AppSignal::setAnalogSignalFormat), &m_signal, std::placeholders::_1));
	analogSignalFormatProperty->setCategory(categoryDataFormat);

	auto excludeFromBuildProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::EXCLUDE_FROM_BUILD, true,
																		AppSignal::excludeFromBuild, AppSignal::setExcludeFromBuild, m_signal);
	excludeFromBuildProperty->setCategory(categorySignalProcessing);

	auto unitProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::UNIT, true, AppSignal::unit, AppSignal::setUnit, m_signal);
	unitProperty->setCategory(categorySignalProcessing);

	auto decimalPlacesProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(int, AppSignalPropNames::DECIMAL_PLACES, true, AppSignal::decimalPlaces, AppSignal::setDecimalPlaces, m_signal);
	decimalPlacesProperty->setCategory(categoryOnlineMonitoringSystem);

	auto coarseApertureProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::COARSE_APERTURE, true, AppSignal::coarseAperture, AppSignal::setCoarseAperture, m_signal);
	coarseApertureProperty->setPrecision(4);
	coarseApertureProperty->setCategory(categoryOnlineMonitoringSystem);

	auto fineApertureProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::FINE_APERTURE, true, AppSignal::fineAperture, AppSignal::setFineAperture, m_signal);
	fineApertureProperty->setPrecision(4);
	fineApertureProperty->setCategory(categoryOnlineMonitoringSystem);

	auto apertureTypeProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::ApertureType, AppSignalPropNames::APERTURE_TYPE,
															true, AppSignal::apertureType, AppSignal::setApertureType, m_signal);
	apertureTypeProperty->setCategory(categoryOnlineMonitoringSystem);
	apertureTypeProperty->setDescription(QString("RangePercent - aperture is set in % of signal range\n"
												 "ValuePercent - aperture is set in % of current signal value\n"
												 "AbsValue - aperture is set in absolute engineering units"));

	auto acquireProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ACQUIRE, true, AppSignal::acquire, AppSignal::setAcquire, m_signal);
	acquireProperty->setCategory(categoryOnlineMonitoringSystem);

	auto archiveProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ARCHIVE, true, AppSignal::archive, AppSignal::setArchive, m_signal);
	archiveProperty->setCategory(categoryOnlineMonitoringSystem);

	auto tagsProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::TAGS, true, AppSignal::tagsStr, AppSignal::setTagsStr, m_signal);
	tagsProperty->setCategory(categoryOnlineMonitoringSystem);
	tagsProperty->setSpecificEditor(E::PropertySpecificEditor::Tags);

	// append signal specific properties
	//

	createSpecificProperties();

	if (savePropertyDescription)
	{
		addPropertyDescription<QString>(AppSignalPropNames::SPECIFIC_PROPERTIES_STRUCT,
										&AppSignal::specPropStruct);
	}
	auto propSpecPropStruct = ADD_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::SPECIFIC_PROPERTIES_STRUCT, true,
															 AppSignalProperties::specPropStruct, AppSignalProperties::setSpecPropStruct);
	propSpecPropStruct->setCategory(categoryExpertProperties);
	propSpecPropStruct->setExpert(true);

	propSpecPropStruct->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);
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

