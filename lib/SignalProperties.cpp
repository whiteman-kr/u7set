#include "SignalProperties.h"
#include "../UtilsLib/WUtils.h"
#include "../u7/Settings.h"


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


// -------------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValue class implementation
//
// -------------------------------------------------------------------------------------------------------------


const QString SignalProperties::categoryIdentification("1 Identification");
const QString SignalProperties::categorySignalType("2 Signal type");
const QString SignalProperties::categoryDataFormat("3 Data Format");
const QString SignalProperties::categorySignalProcessing("4 Signal processing");
const QString SignalProperties::categoryElectricParameters("5 Electric parameters");
const QString SignalProperties::categoryOnlineMonitoringSystem("6 Online Monitoring System");
const QString SignalProperties::categoryTuning("7 Tuning");
const QString SignalProperties::categoryExpertProperties("8 Expert properties");


const QString SignalProperties::lastEditedSignalFieldValuePlace("SignalsTabPage/LastEditedSignal/");

QString SignalProperties::generateCaption(const QString& name)
{
	QString result;
	if (name.isEmpty())
	{
		assert(false);
		return result;
	}
	result += name[0].toUpper();
	for (int i = 1; i < name.count(); i++)
	{
		if (name[i].isUpper())
		{
			if (i + 1 < name.count() && name[i + 1].isUpper())	// abbreviation?
			{
				result += ' ';
				while (i < name.count() && name[i].isUpper())
				{
					result += name[i];
					i++;
				}
				result += ' ';
			}
			else
			{
				result += ' ' + name[i].toLower();
			}
		}
		else
		{
			result += name[i];
		}
	}
	return result.trimmed();
}

SignalProperties::SignalProperties(const AppSignal& signal, bool savePropertyDescription) :
	m_signal(signal)
{
	initProperties(savePropertyDescription);
}

void SignalProperties::updateSpecPropValues()
{
	for (const SignalSpecPropValue& value : m_specPropValues.values())
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

void SignalProperties::setSpecPropStruct(const QString & specPropStruct)
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

int SignalProperties::getPrecision()
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

void SignalProperties::initProperties(bool savePropertyDescription)
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

	auto excludeFromBuildProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::EXCLUDE_FROM_BUILD, true,
																		AppSignal::excludeFromBuild, AppSignal::setExcludeFromBuild, m_signal);
	excludeFromBuildProperty->setCategory(categorySignalProcessing);

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

	auto analogSignalFormatProperty = addProperty<E::AnalogAppSignalFormat>(AppSignalPropNames::ANALOG_SIGNAL_FORMAT, QString(), true,
																		  (std::function<E::AnalogAppSignalFormat(void)>)std::bind(&AppSignal::analogSignalFormat, &m_signal),
																		  std::bind(static_cast<void (AppSignal::*)(E::AnalogAppSignalFormat)>(&AppSignal::setAnalogSignalFormat), &m_signal, std::placeholders::_1));
	analogSignalFormatProperty->setCategory(categoryDataFormat);

	auto unitProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(QString, AppSignalPropNames::UNIT, true, AppSignal::unit, AppSignal::setUnit, m_signal);
	unitProperty->setCategory(categorySignalProcessing);

	auto decimalPlacesProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(int, AppSignalPropNames::DECIMAL_PLACES, true, AppSignal::decimalPlaces, AppSignal::setDecimalPlaces, m_signal);
	decimalPlacesProperty->setCategory(categoryOnlineMonitoringSystem);

	auto coarseApertureProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::COARSE_APERTURE, true, AppSignal::coarseAperture, AppSignal::setCoarseAperture, m_signal);
	coarseApertureProperty->setCategory(categoryOnlineMonitoringSystem);

	auto fineApertureProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(double, AppSignalPropNames::FINE_APERTURE, true, AppSignal::fineAperture, AppSignal::setFineAperture, m_signal);
	fineApertureProperty->setCategory(categoryOnlineMonitoringSystem);

	auto adaptiveApertureProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ADAPTIVE_APERTURE, true, AppSignal::adaptiveAperture, AppSignal::setAdaptiveAperture, m_signal);
	adaptiveApertureProperty->setCategory(categoryOnlineMonitoringSystem);

	auto acquireProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ACQUIRE, true, AppSignal::acquire, AppSignal::setAcquire, m_signal);
	acquireProperty->setCategory(categoryOnlineMonitoringSystem);

	auto archiveProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(bool, AppSignalPropNames::ARCHIVE, true, AppSignal::archive, AppSignal::setArchive, m_signal);
	archiveProperty->setCategory(categoryOnlineMonitoringSystem);

	auto byteOrderProperty = ADD_SIGNAL_PROPERTY_GETTER_SETTER(E::ByteOrder, AppSignalPropNames::BYTE_ORDER_PROP, true, AppSignal::byteOrder, AppSignal::setByteOrder, m_signal);
	byteOrderProperty->setCategory(categoryDataFormat);

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
															 SignalProperties::specPropStruct, SignalProperties::setSpecPropStruct);
	propSpecPropStruct->setCategory(categoryExpertProperties);
	propSpecPropStruct->setExpert(true);

	propSpecPropStruct->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);
}

void SignalProperties::createSpecificProperties()
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

void SignalProperties::deleteSpecificProperties()
{
	const QVector<SignalSpecPropValue>& values = m_specPropValues.values();

	for(const SignalSpecPropValue& value : values)
	{
		removeProperty(value.name());
	}
}


std::shared_ptr<OrderedHash<int, QString> > SignalProperties::generateOrderedHashFromStringArray(const char* const* array, size_t size)
{
	auto result = std::make_shared<OrderedHash<int, QString>>();
	for (size_t i = 0; i < size; i++)
	{
		result->append(static_cast<const int>(i), array[i]);
	}
	return result;
}

