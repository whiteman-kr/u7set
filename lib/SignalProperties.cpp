#include "SignalProperties.h"


std::shared_ptr<OrderedHash<int, QString>> SignalProperties::m_sensorTypeHash = generateOrderedHashFromStringArray(SensorTypeStr, SENSOR_TYPE_COUNT);
std::shared_ptr<OrderedHash<int, QString>> SignalProperties::m_outputModeHash = generateOrderedHashFromStringArray(OutputModeStr, OUTPUT_MODE_COUNT);

const QString SignalProperties::idCaption("ID");										// Optimization, to share one string among all Signal instances
const QString SignalProperties::signalGroupIDCaption("SignalGroupID");
const QString SignalProperties::signalInstanceIDCaption("SignalInstanceID");
const QString SignalProperties::changesetIDCaption("ChangesetID");
const QString SignalProperties::checkedOutCaption("CheckedOut");
const QString SignalProperties::userIdCaption("UserID");
const QString SignalProperties::channelCaption("Channel");
const QString SignalProperties::createdCaption("Created");
const QString SignalProperties::deletedCaption("Deleted");
const QString SignalProperties::instanceCreatedCaption("InstanceCreated");
const QString SignalProperties::typeCaption("Type");
const QString SignalProperties::inOutTypeCaption("InOutType");
const QString SignalProperties::cacheValidator("^[#]?[A-Za-z\\d_]*$");
const QString SignalProperties::appSignalIDCaption("AppSignalID");
const QString SignalProperties::customSignalIDCaption("CustomAppSignalID");
const QString SignalProperties::busTypeIDCaption("BusTypeID");
const QString SignalProperties::captionCaption("Caption");
const QString SignalProperties::captionValidator("^.+$");
const QString SignalProperties::analogSignalFormatCaption("AnalogSignalFormat");
const QString SignalProperties::dataSizeCaption("DataSize");
const QString SignalProperties::lowADCCaption("LowADC");
const QString SignalProperties::highADCCaption("HighADC");
const QString SignalProperties::lowDACCaption("LowDAC");
const QString SignalProperties::highDACCaption("HighDAC");
const QString SignalProperties::lowEngeneeringUnitsCaption("LowEngeneeringUnits");
const QString SignalProperties::highEngeneeringUnitsCaption("HighEngeneeringUnits");
const QString SignalProperties::unitCaption("Unit");
const QString SignalProperties::lowValidRangeCaption("LowValidRange");
const QString SignalProperties::highValidRangeCaption("HighValidRange");
const QString SignalProperties::electricLowLimitCaption("ElectricLowLimit");
const QString SignalProperties::electricHighLimitCaption("ElectricHighLimit");
const QString SignalProperties::electricUnitCaption("ElectricUnit");
const QString SignalProperties::sensorTypeCaption("SensorType");
const QString SignalProperties::outputModeCaption("OutputMode");
const QString SignalProperties::acquireCaption("Acquire");
const QString SignalProperties::decimalPlacesCaption("DecimalPlaces");
const QString SignalProperties::coarseApertureCaption("CoarseAperture");
const QString SignalProperties::fineApertureCaption("FineAperture");
const QString SignalProperties::adaptiveApertureCaption("AdaptiveAperture");
const QString SignalProperties::filteringTimeCaption("FilteringTime");
const QString SignalProperties::spreadToleranceCaption("SpreadTolerance");
const QString SignalProperties::byteOrderCaption("ByteOrder");
const QString SignalProperties::equipmentIDCaption("EquipmentID");
const QString SignalProperties::enableTuningCaption("EnableTuning");
const QString SignalProperties::tuningDefaultValueCaption("TuningDefaultValue");
const QString SignalProperties::tuningLowBoundCaption("TuningLowBound");
const QString SignalProperties::tuningHighBoundCaption("TuningHighBound");

const QString SignalProperties::categoryIdentification("1 Identification");
const QString SignalProperties::categorySignalType("2 Signal type");
const QString SignalProperties::categoryDataFormat("3 Data Format");
const QString SignalProperties::categorySignalProcessing("4 Signal processing");
const QString SignalProperties::categoryElectricParameters("5 Electric parameters");
const QString SignalProperties::categoryOnlineMonitoringSystem("6 Online Monitoring System");
const QString SignalProperties::categoryTuning("7 Tuning");

const QString SignalProperties::lastEditedSignalFieldValuePlace("SignalsTabPage/LastEditedSignal/");

SignalProperties::SignalProperties(Signal& signal) :
	m_signal(signal)
{
	initProperties();
}


void SignalProperties::initProperties()
{
	ADD_PROPERTY_GETTER_INDIRECT(int, idCaption, false, Signal::ID, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(int, signalGroupIDCaption, false, Signal::signalGroupID, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(int, signalInstanceIDCaption, false, Signal::signalInstanceID, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(int, changesetIDCaption, false, Signal::changesetID, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(bool, checkedOutCaption, false, Signal::checkedOut, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(int, userIdCaption, false, Signal::userID, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(E::Channel, channelCaption, false, Signal::channel, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(QDateTime, createdCaption, false, Signal::created, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(bool, deletedCaption, false, Signal::deleted, m_signal);
	ADD_PROPERTY_GETTER_INDIRECT(QDateTime, instanceCreatedCaption, false, Signal::instanceCreated, m_signal);

	auto signalTypeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::SignalType, typeCaption, true, Signal::signalType, Signal::setSignalType, m_signal);
	signalTypeProperty->setCategory(categorySignalType);

	auto signalInOutTypeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::SignalInOutType, inOutTypeCaption, true, Signal::inOutType, Signal::setInOutType, m_signal);
	signalInOutTypeProperty->setCategory(categorySignalType);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, appSignalIDCaption, true, Signal::appSignalID, Signal::setAppSignalID, m_signal);
	strIdProperty->setValidator(cacheValidator);
	strIdProperty->setCategory(categoryIdentification);

	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, customSignalIDCaption, true, Signal::customAppSignalID, Signal::setCustomAppSignalID, m_signal);
	extStrIdProperty->setValidator(cacheValidator);
	extStrIdProperty->setCategory(categoryIdentification);

	auto busTypeIDProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, busTypeIDCaption, true, Signal::busTypeID, Signal::setBusTypeID, m_signal);
	busTypeIDProperty->setCategory(categoryIdentification);

	auto nameProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, captionCaption, true, Signal::caption, Signal::setCaption, m_signal);
	nameProperty->setValidator(captionValidator);
	nameProperty->setCategory(categoryIdentification);

	auto equipmentProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, equipmentIDCaption, true, Signal::equipmentID, Signal::setEquipmentID, m_signal);
	equipmentProperty->setCategory(categoryIdentification);

	auto enableTuningProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, enableTuningCaption, true, Signal::enableTuning, Signal::setEnableTuning, m_signal);
	enableTuningProperty->setCategory(categoryTuning);

	auto tuningDefaultValueProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(TuningValue, tuningDefaultValueCaption, true, Signal::tuningDefaultValue, Signal::setTuningDefaultValue, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningDefaultValueProperty);
	tuningDefaultValueProperty->setCategory(categoryTuning);

	auto tuningLowBoundProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(TuningValue, tuningLowBoundCaption, true, Signal::tuningLowBound, Signal::setTuningLowBound, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningLowBoundProperty);
	tuningLowBoundProperty->setCategory(categoryTuning);

	auto tuningHighBoundProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(TuningValue, tuningHighBoundCaption, true, Signal::tuningHighBound, Signal::setTuningHighBound, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningHighBoundProperty);
	tuningHighBoundProperty->setCategory(categoryTuning);

	auto dataSizeProperty = addProperty<int>(dataSizeCaption, QString(), true,
										(std::function<int(void)>)std::bind(&Signal::dataSize, &m_signal),
										std::bind(static_cast<void (Signal::*)(int)>(&Signal::setDataSize), &m_signal, std::placeholders::_1));

	dataSizeProperty->setCategory(categoryDataFormat);

	if (m_signal.isAnalog() == true)
	{
		static std::shared_ptr<OrderedHash<int, QString>> sensorList = std::make_shared<OrderedHash<int, QString>>();

		if (sensorList->isEmpty())
		{
			for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
			{
				sensorList->append(i, SensorTypeStr[i]);
			}
		}

		auto analogSignalFormatProperty = addProperty<E::AnalogAppSignalFormat>(analogSignalFormatCaption, QString(), true,
																			  (std::function<E::AnalogAppSignalFormat(void)>)std::bind(&Signal::analogSignalFormat, &m_signal),
																			  std::bind(static_cast<void (Signal::*)(E::AnalogAppSignalFormat)>(&Signal::setAnalogSignalFormat), &m_signal, std::placeholders::_1));

		analogSignalFormatProperty->setCategory(categoryDataFormat);

		auto lowADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowADCCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowADCProperty->setCategory(categorySignalProcessing);

		auto highADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highADCCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highADCProperty->setCategory(categorySignalProcessing);

		auto lowDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowDACCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowDACProperty->setCategory(categorySignalProcessing);

		auto highDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highDACCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highDACProperty->setCategory(categorySignalProcessing);

		auto lowEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowEngeneeringUnitsCaption, true, Signal::lowEngeneeringUnits, Signal::setLowEngeneeringUnits, m_signal);
		m_propertiesDependentOnPrecision.push_back(lowEngeneeringUnitsProperty);
		lowEngeneeringUnitsProperty->setCategory(categorySignalProcessing);

		auto highEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highEngeneeringUnitsCaption, true, Signal::highEngeneeringUnits, Signal::setHighEngeneeringUnits, m_signal);
		m_propertiesDependentOnPrecision.push_back(highEngeneeringUnitsProperty);
		highEngeneeringUnitsProperty->setCategory(categorySignalProcessing);

		auto lowValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowValidRangeCaption, true, Signal::lowValidRange, Signal::setLowValidRange, m_signal);
		m_propertiesDependentOnPrecision.push_back(lowValidRangeProperty);
		lowValidRangeProperty->setCategory(categorySignalProcessing);

		auto highValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highValidRangeCaption, true, Signal::highValidRange, Signal::setHighValidRange, m_signal);
		m_propertiesDependentOnPrecision.push_back(highValidRangeProperty);
		highValidRangeProperty->setCategory(categorySignalProcessing);

		auto unitProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, unitCaption, true, Signal::unit, Signal::setUnit, m_signal);
		unitProperty->setCategory(categorySignalProcessing);

		auto electricLowLimitProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, electricLowLimitCaption, true, Signal::electricLowLimit, Signal::setElectricLowLimit, m_signal);
		electricLowLimitProperty->setPrecision(3);
		electricLowLimitProperty->setCategory(categoryElectricParameters);

		auto electricHighLimitProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, electricHighLimitCaption, true, Signal::electricHighLimit, Signal::setElectricHighLimit, m_signal);
		electricHighLimitProperty->setPrecision(3);
		electricHighLimitProperty->setCategory(categoryElectricParameters);

		auto electricUnitProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::ElectricUnit, electricUnitCaption, true, Signal::electricUnit, Signal::setElectricUnit, m_signal);
		electricUnitProperty->setCategory(categoryElectricParameters);

		auto sensorTypeProperty = ADD_PROPERTY_DYNAMIC_ENUM_INDIRECT(sensorTypeCaption, true, m_sensorTypeHash, Signal::sensorType, Signal::setSensorTypeInt, m_signal);
		sensorTypeProperty->setCategory(categoryElectricParameters);

		auto outputModePropetry = ADD_PROPERTY_DYNAMIC_ENUM_INDIRECT(outputModeCaption, true, m_outputModeHash, Signal::outputMode, Signal::setOutputModeInt, m_signal);
		outputModePropetry->setCategory(categoryElectricParameters);

		auto decimalPlacesProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, decimalPlacesCaption, true, Signal::decimalPlaces, Signal::setDecimalPlaces, m_signal);
		decimalPlacesProperty->setCategory(categoryOnlineMonitoringSystem);

		auto coarseApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, coarseApertureCaption, true, Signal::coarseAperture, Signal::setCoarseAperture, m_signal);
		coarseApertureProperty->setCategory(categoryOnlineMonitoringSystem);

		auto fineApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, fineApertureCaption, true, Signal::fineAperture, Signal::setFineAperture, m_signal);
		fineApertureProperty->setCategory(categoryOnlineMonitoringSystem);

		auto adaptiveApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, adaptiveApertureCaption, true, Signal::adaptiveAperture, Signal::setAdaptiveAperture, m_signal);
		adaptiveApertureProperty->setCategory(categoryOnlineMonitoringSystem);

		auto spreadToleranceProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, spreadToleranceCaption, true, Signal::spreadTolerance, Signal::setSpreadTolerance, m_signal);
		m_propertiesDependentOnPrecision.push_back(spreadToleranceProperty);
		spreadToleranceProperty->setCategory(categorySignalProcessing);

		auto filteringTimePropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, filteringTimeCaption, true, Signal::filteringTime, Signal::setFilteringTime, m_signal);
		filteringTimePropetry->setPrecision(6);
		filteringTimePropetry->setCategory(categorySignalProcessing);
	}

	auto acquireProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, acquireCaption, true, Signal::acquire, Signal::setAcquire, m_signal);
	acquireProperty->setCategory(categoryOnlineMonitoringSystem);

	auto byteOrderProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::ByteOrder, byteOrderCaption, true, Signal::byteOrder, Signal::setByteOrder, m_signal);
	byteOrderProperty->setCategory(categoryDataFormat);

	// append signal specific properties
	//
	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(QString("4;SensorType2;5 Electric parameters;DynamicEnum [mV_Type_B=14,mV_Type_E=15,mV_Type_J=16,mV_Type_K=17,mV_Type_N=18,mV_Type_R=19,mV_Type_S=20,mV_Type_T=21,mV_Raw_Mul_8=22,mV_Raw_Mul_32=23];;;mV_Type_B;0;false;false;;true;None"));

	if (result.first == false)
	{
		assert(false);
		return;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		// set value of property

		QVariant qv(23);

		specificProperty->setValue(qv);

		addProperty(specificProperty);
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
