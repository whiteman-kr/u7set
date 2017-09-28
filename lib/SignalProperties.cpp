#include "SignalProperties.h"


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
	signalTypeProperty->setCategory(signalTypeCategory);

	auto signalInOutTypeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::SignalInOutType, inOutTypeCaption, true, Signal::inOutType, Signal::setInOutType, m_signal);
	signalInOutTypeProperty->setCategory(signalTypeCategory);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, appSignalIDCaption, true, Signal::appSignalID, Signal::setAppSignalID, m_signal);
	strIdProperty->setValidator(cacheValidator);
	strIdProperty->setCategory(identificationCategory);

	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, customSignalIDCaption, true, Signal::customAppSignalID, Signal::setCustomAppSignalID, m_signal);
	extStrIdProperty->setValidator(cacheValidator);
	extStrIdProperty->setCategory(identificationCategory);

	auto busTypeIDProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, busTypeIDCaption, true, Signal::busTypeID, Signal::setBusTypeID, m_signal);
	busTypeIDProperty->setCategory(identificationCategory);

	auto nameProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, captionCaption, true, Signal::caption, Signal::setCaption, m_signal);
	nameProperty->setValidator(captionValidator);
	nameProperty->setCategory(identificationCategory);

	auto equipmentProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, equipmentIDCaption, true, Signal::equipmentID, Signal::setEquipmentID, m_signal);
	equipmentProperty->setCategory(identificationCategory);

	auto enableTuningProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, enableTuningCaption, true, Signal::enableTuning, Signal::setEnableTuning, m_signal);
	enableTuningProperty->setCategory(tuningCategory);

	auto tuningDefaultValueProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, tuningDefaultValueCaption, true, Signal::tuningDefaultValue, Signal::setTuningDefaultValue, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningDefaultValueProperty);
	tuningDefaultValueProperty->setCategory(tuningCategory);

	auto tuningLowBoundProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, tuningLowBoundCaption, true, Signal::tuningLowBound, Signal::setTuningLowBound, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningLowBoundProperty);
	tuningLowBoundProperty->setCategory(tuningCategory);

	auto tuningHighBoundProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, tuningHighBoundCaption, true, Signal::tuningHighBound, Signal::setTuningHighBound, m_signal);
	m_propertiesDependentOnPrecision.push_back(tuningHighBoundProperty);
	tuningHighBoundProperty->setCategory(tuningCategory);

	auto dataSizeProperty = addProperty<int>(dataSizeCaption, QString(), true,
										(std::function<int(void)>)std::bind(&Signal::dataSize, &m_signal),
										std::bind(static_cast<void (Signal::*)(int)>(&Signal::setDataSize), &m_signal, std::placeholders::_1));

	dataSizeProperty->setCategory(dataFormatCategory);

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

		analogSignalFormatProperty->setCategory(dataFormatCategory);

		auto lowADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowADCCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowADCProperty->setCategory(signalProcessingCategory);

		auto highADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highADCCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highADCProperty->setCategory(signalProcessingCategory);

		auto lowDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowDACCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowDACProperty->setCategory(signalProcessingCategory);

		auto highDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highDACCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highDACProperty->setCategory(signalProcessingCategory);

		auto lowEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowEngeneeringUnitsCaption, true, Signal::lowEngeneeringUnits, Signal::setLowEngeneeringUnits, m_signal);
		m_propertiesDependentOnPrecision.push_back(lowEngeneeringUnitsProperty);
		lowEngeneeringUnitsProperty->setCategory(signalProcessingCategory);

		auto highEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highEngeneeringUnitsCaption, true, Signal::highEngeneeringUnits, Signal::setHighEngeneeringUnits, m_signal);
		m_propertiesDependentOnPrecision.push_back(highEngeneeringUnitsProperty);
		highEngeneeringUnitsProperty->setCategory(signalProcessingCategory);

		auto lowValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowValidRangeCaption, true, Signal::lowValidRange, Signal::setLowValidRange, m_signal);
		m_propertiesDependentOnPrecision.push_back(lowValidRangeProperty);
		lowValidRangeProperty->setCategory(signalProcessingCategory);

		auto highValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highValidRangeCaption, true, Signal::highValidRange, Signal::setHighValidRange, m_signal);
		m_propertiesDependentOnPrecision.push_back(highValidRangeProperty);
		highValidRangeProperty->setCategory(signalProcessingCategory);

		auto outputModePropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::OutputMode, outputModeCaption, true, Signal::outputMode, Signal::setOutputMode, m_signal);
		outputModePropetry->setCategory(signalProcessingCategory);

		auto unit = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, unitCaption, true, Signal::unit, Signal::setUnit, m_signal);
		unit->setCategory(signalProcessingCategory);

		auto decimalPlacesProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, decimalPlacesCaption, true, Signal::decimalPlaces, Signal::setDecimalPlaces, m_signal);
		decimalPlacesProperty->setCategory(onlineMonitoringSystemCategory);

		auto coarseApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, coarseApertureCaption, true, Signal::coarseAperture, Signal::setCoarseAperture, m_signal);
		coarseApertureProperty->setCategory(onlineMonitoringSystemCategory);

		auto fineApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, fineApertureCaption, true, Signal::fineAperture, Signal::setFineAperture, m_signal);
		fineApertureProperty->setCategory(onlineMonitoringSystemCategory);

		auto adaptiveApertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, adaptiveApertureCaption, true, Signal::adaptiveAperture, Signal::setAdaptiveAperture, m_signal);
		adaptiveApertureProperty->setCategory(onlineMonitoringSystemCategory);

		auto spreadToleranceProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, spreadToleranceCaption, true, Signal::spreadTolerance, Signal::setSpreadTolerance, m_signal);
		m_propertiesDependentOnPrecision.push_back(spreadToleranceProperty);
		spreadToleranceProperty->setCategory(signalProcessingCategory);

		auto filteringTimePropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, filteringTimeCaption, true, Signal::filteringTime, Signal::setFilteringTime, m_signal);
		filteringTimePropetry->setPrecision(6);
		filteringTimePropetry->setCategory(signalProcessingCategory);
	}

	auto acquireProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, acquireCaption, true, Signal::acquire, Signal::setAcquire, m_signal);
	acquireProperty->setCategory(onlineMonitoringSystemCategory);

	auto byteOrderProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::ByteOrder, byteOrderCaption, true, Signal::byteOrder, Signal::setByteOrder, m_signal);
	byteOrderProperty->setCategory(dataFormatCategory);
}
