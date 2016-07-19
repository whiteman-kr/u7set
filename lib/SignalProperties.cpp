#include "SignalProperties.h"


SignalProperties::SignalProperties(Signal& signal) :
	m_signal(signal)
{
	initProperties();
}


void SignalProperties::initProperties()
{
	static const QString idCaption("ID");										// Optimization, to share one string among all Signal instances
	static const QString signalGroupIDCaption("SignalGroupID");
	static const QString signalInstanceIDCaption("SignalInstanceID");
	static const QString changesetIDCaption("ChangesetID");
	static const QString checkedOutCaption("CheckedOut");
	static const QString userIdCaption("UserID");
	static const QString channelCaption("Channel");
	static const QString createdCaption("Created");
	static const QString deletedCaption("Deleted");
	static const QString instanceCreatedCaption("InstanceCreated");
	static const QString instanceActionCaption("InstanceAction");
	static const QString typeCaption("Type");
	static const QString inOutTypeCaption("InOutType");
	static const QString cacheValidator1("^#[A-Za-z][A-Za-z\\d_]*$");
	static const QString cacheValidator2("^[A-Za-z][A-Za-z\\d_]*$");
	static const QString appSignalIDCaption("AppSignalID");
	static const QString customSignalIDCaption("CustomAppSignalID");
	static const QString captionCaption("Caption");
	static const QString captionValidator("^.+$");
	static const QString dataFormatCaption("DataFormat");
	static const QString dataSizeCaption("DataSize");
	static const QString lowADCCaption("LowADC");
	static const QString highADCCaption("HighADC");
	static const QString lowDACCaption("LowDAC");
	static const QString highDACCaption("HighDAC");
	static const QString lowEngeneeringUnitsCaption("LowEngeneeringUnits");
	static const QString highEngeneeringUnitsCaption("HighEngeneeringUnits");
	static const QString unitCaption("Unit");
	static const QString lowValidRangeCaption("LowValidRange");
	static const QString highValidRangeCaption("HighValidRange");
	static const QString unbalanceLimitCaption("UnbalanceLimit");
	static const QString outputModeCaption("OutputMode");
	static const QString acquireCaption("Acquire");
	static const QString normalStateCaption("NormalState");
	static const QString decimalPlacesCaption("DecimalPlaces");
	static const QString apertureCaption("Aperture");
	static const QString filteringTimeCaption("FilteringTime");
	static const QString spreadToleranceCaption("SpreadTolerance");
	static const QString byteOrderCaption("ByteOrder");
	static const QString equipmentIDCaption("EquipmentID");
	static const QString enableTuningCaption("EnableTuning");
	static const QString tuningDefaultValueCaption("TuningDefaultValue");
	static const QString identificationCategory("1 Identification");
	static const QString signalTypeCategory("2 Signal type");
	static const QString dataFormatCategory("3 Data Format");
	static const QString signalProcessingCategory("4 Signal processing");
	static const QString onlineMonitoringSystemCategory("5 Online Monitoring System");
	static const QString tuningCategory("6 Tuning");

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
	ADD_PROPERTY_GETTER_INDIRECT(E::InstanceAction, instanceActionCaption, false, Signal::instanceAction, m_signal);

	auto signalTypeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::SignalType, typeCaption, true, Signal::type, Signal::setType, m_signal);
	signalTypeProperty->setCategory(signalTypeCategory);

	auto signalInOutTypeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::SignalInOutType, inOutTypeCaption, true, Signal::inOutType, Signal::setInOutType, m_signal);
	signalInOutTypeProperty->setCategory(signalTypeCategory);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, appSignalIDCaption, true, Signal::appSignalID, Signal::setAppSignalID, m_signal);
	strIdProperty->setValidator(cacheValidator1);
	strIdProperty->setCategory(identificationCategory);

	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, customSignalIDCaption, true, Signal::customAppSignalID, Signal::setCustomAppSignalID, m_signal);
	extStrIdProperty->setValidator(cacheValidator2);
	extStrIdProperty->setCategory(identificationCategory);

	auto nameProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, captionCaption, true, Signal::caption, Signal::setCaption, m_signal);
	nameProperty->setValidator(captionValidator);
	nameProperty->setCategory(identificationCategory);

	auto equipmentProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(QString, equipmentIDCaption, true, Signal::equipmentID, Signal::setEquipmentID, m_signal);
	equipmentProperty->setCategory(identificationCategory);

	auto enableTuningProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, enableTuningCaption, true, Signal::enableTuning, Signal::setEnableTuning, m_signal);
	enableTuningProperty->setCategory(tuningCategory);

	auto tuningDefaultValueProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, tuningDefaultValueCaption, true, Signal::tuningDefaultValue, Signal::setTuningDefaultValue, m_signal);
	tuningDefaultValueProperty->setCategory(tuningCategory);

	auto dataFormatProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::DataFormat, dataFormatCaption, true, Signal::dataFormat, Signal::setDataFormat, m_signal);
	dataFormatProperty->setCategory(dataFormatCategory);

	auto dataSizeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, dataSizeCaption, true, Signal::dataSize, Signal::setDataSize, m_signal);
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

		auto lowADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowADCCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowADCProperty->setCategory(signalProcessingCategory);

		auto highADCProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highADCCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highADCProperty->setCategory(signalProcessingCategory);

		auto lowDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, lowDACCaption, true, Signal::lowADC, Signal::setLowADC, m_signal);
		lowDACProperty->setCategory(signalProcessingCategory);

		auto highDACProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, highDACCaption, true, Signal::highADC, Signal::setHighADC, m_signal);
		highDACProperty->setCategory(signalProcessingCategory);

		auto lowEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowEngeneeringUnitsCaption, true, Signal::lowEngeneeringUnits, Signal::setLowEngeneeringUnits, m_signal);
		lowEngeneeringUnitsProperty->setCategory(signalProcessingCategory);

		auto highEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highEngeneeringUnitsCaption, true, Signal::highEngeneeringUnits, Signal::setHighEngeneeringUnits, m_signal);
		highEngeneeringUnitsProperty->setCategory(signalProcessingCategory);

		auto lowValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, lowValidRangeCaption, true, Signal::lowValidRange, Signal::setLowValidRange, m_signal);
		lowValidRangeProperty->setCategory(signalProcessingCategory);

		auto highValidRangeProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, highValidRangeCaption, true, Signal::highValidRange, Signal::setHighValidRange, m_signal);
		highValidRangeProperty->setCategory(signalProcessingCategory);

		auto outputModePropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::OutputMode, outputModeCaption, true, Signal::outputMode, Signal::setOutputMode, m_signal);
		outputModePropetry->setCategory(signalProcessingCategory);

		auto unitProperty = ADD_PROPERTY_DYNAMIC_ENUM_INDIRECT(unitCaption, true, Signal::unitList, Signal::unitID, Signal::setUnitID, m_signal);
		unitProperty->setCategory(dataFormatCategory);

		auto unbalanceLimitProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, unbalanceLimitCaption, true, Signal::unbalanceLimit, Signal::setUnbalanceLimit, m_signal);
		unbalanceLimitProperty->setCategory(onlineMonitoringSystemCategory);

		auto decimalPlacesProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, decimalPlacesCaption, true, Signal::decimalPlaces, Signal::setDecimalPlaces, m_signal);
		decimalPlacesProperty->setCategory(onlineMonitoringSystemCategory);

		auto apertureProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, apertureCaption, true, Signal::aperture, Signal::setAperture, m_signal);
		apertureProperty->setCategory(onlineMonitoringSystemCategory);

		auto spreadToleranceProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, spreadToleranceCaption, true, Signal::spreadTolerance, Signal::setSpreadTolerance, m_signal);
		spreadToleranceProperty->setCategory(signalProcessingCategory);

		auto filteringTimePropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, filteringTimeCaption, true, Signal::filteringTime, Signal::setFilteringTime, m_signal);
		filteringTimePropetry->setPrecision(6);
		filteringTimePropetry->setCategory(signalProcessingCategory);

		/*auto inputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, inputLowLimitCaption, true, Signal::inputLowLimit, Signal::setInputLowLimit);
		inputLowLimitPropetry->setCategory(inputSensorCategory);

		auto inputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, inputHighLimitCaption, true, Signal::inputHighLimit, Signal::setInputHighLimit);
		inputHighLimitPropetry->setCategory(inputSensorCategory);

		auto inputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(inputUnitCaption, true, m_unitList, Signal::inputUnitID, Signal::setInputUnitID);
		inputUnitIDPropetry->setCategory(inputSensorCategory);

		auto inputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(inputSensorCaption, true, sensorList, Signal::inputSensorID, Signal::setInputSensorID);
		inputSensorPropetry->setCategory(inputSensorCategory);

		auto outputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, outputLowLimitCaption, true, Signal::outputLowLimit, Signal::setOutputLowLimit);
		outputLowLimitPropetry->setCategory(outputSensorCategory);

		auto outputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER_INDIRECT(double, outputHighLimitCaption, true, Signal::outputHighLimit, Signal::setOutputHighLimit);
		outputHighLimitPropetry->setCategory(outputSensorCategory);

		auto outputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(outputUnitCaption, true, m_unitList, Signal::outputUnitID, Signal::setOutputUnitID);
		outputUnitIDPropetry->setCategory(outputSensorCategory);

		auto outputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(outputSensorCaption, true, sensorList, Signal::outputSensorID, Signal::setOutputSensorID);
		outputSensorPropetry->setCategory(outputSensorCategory);*/
	}
	else
	{
		auto normalStateProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(int, normalStateCaption, true, Signal::normalState, Signal::setNormalState, m_signal);
		normalStateProperty->setCategory(onlineMonitoringSystemCategory);
	}

	auto acquireProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(bool, acquireCaption, true, Signal::acquire, Signal::setAcquire, m_signal);
	acquireProperty->setCategory(onlineMonitoringSystemCategory);

	auto byteOrderProperty = ADD_PROPERTY_GETTER_SETTER_INDIRECT(E::ByteOrder, byteOrderCaption, true, Signal::byteOrder, Signal::setByteOrder, m_signal);
	byteOrderProperty->setCategory(dataFormatCategory);
}
