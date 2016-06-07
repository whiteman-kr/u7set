#include "../lib/Signal.h"
#include <QXmlStreamAttributes>
#include <QFile>

DataFormatList::DataFormatList()
{
	auto enumValues = E::enumValues<E::DataFormat>();

	for (auto v : enumValues)
	{
		append(v.first, v.second);
	}
}


std::shared_ptr<UnitList> Signal::m_unitList = std::make_shared<UnitList>();


Signal::Signal(bool initProperties) :
	PropertyObject()
{
	if (initProperties == true)
	{
		InitProperties();		// same as Signal::Signal()
	}
	else
	{
		//	InitProperties();	// no init properties fot faster construction
	}
}


Signal::Signal() :
	PropertyObject()
{
	InitProperties();
}


Signal::~Signal()
{
}


Signal::Signal(const Hardware::DeviceSignal& deviceSignal) : PropertyObject()
{
	if (deviceSignal.isDiagSignal())
	{
		assert(false);
		return;
	}

	if (deviceSignal.isAnalogSignal())
	{
		m_type = E::SignalType::Analog;
	}
	else
	{
		if (deviceSignal.isDiscreteSignal())
		{
			m_type = E::SignalType::Discrete;
		}
		else
		{
			assert(false);			// invalid deviceSignalType
		}
	}

	if (deviceSignal.isInputSignal() || deviceSignal.isValiditySignal())
	{
		m_inOutType = E::SignalInOutType::Input;
	}
	else
	{
		if (deviceSignal.isOutputSignal())
		{
			m_inOutType = E::SignalInOutType::Output;
		}
		else
		{
			m_inOutType = E::SignalInOutType::Internal;
		}
	}

	m_appSignalID = QString("#%1").arg(deviceSignal.equipmentIdTemplate());
	m_customAppSignalID = deviceSignal.equipmentIdTemplate();

	QString deviceSignalStrID = deviceSignal.equipmentIdTemplate();

	int pos = deviceSignalStrID.lastIndexOf(QChar('_'));

	if (pos != -1)
	{
		deviceSignalStrID = deviceSignalStrID.mid(pos + 1);
	}

	m_caption = QString("Signal #%1").arg(deviceSignalStrID);
	m_equipmentID = deviceSignal.equipmentIdTemplate();

	if (m_type == E::SignalType::Analog)
	{
		m_dataFormat = E::DataFormat::Float;
		m_dataSize = 32;
	}
	else
	{
		m_dataFormat = E::DataFormat::UnsignedInt;
		m_dataSize = deviceSignal.size();
	}

	InitProperties();
}


Signal& Signal::operator =(const Signal& signal)
{
	m_ID = signal.ID();
	m_signalGroupID = signal.signalGroupID();
	m_signalInstanceID = signal.signalInstanceID();
	m_changesetID = signal.changesetID();
	m_checkedOut = signal.checkedOut();
	m_userID = signal.userID();
	m_channel = signal.channel();
	m_type = signal.type();
	m_created = signal.created();
	m_deleted = signal.deleted();
	m_instanceCreated = signal.instanceCreated();
	m_instanceAction = signal.instanceAction();

	m_appSignalID = signal.appSignalID();
	m_customAppSignalID = signal.customAppSignalID();
	m_caption = signal.caption();
	m_dataFormat = signal.dataFormat();
	m_dataSize = signal.dataSize();
	m_lowADC = signal.lowADC();
	m_highADC = signal.highADC();
	m_lowEngeneeringUnits = signal.lowEngeneeringUnits();
	m_highEngeneeringUnits = signal.highEngeneeringUnits();
	m_unitID = signal.unitID();
	m_lowValidRange = signal.lowValidRange();
	m_highValidRange = signal.highValidRange();
	m_unbalanceLimit = signal.unbalanceLimit();
	m_inputLowLimit = signal.inputLowLimit();
	m_inputHighLimit = signal.inputHighLimit();
	m_inputUnitID = signal.inputUnitID();
	m_inputSensorID = signal.inputSensorID();
	m_outputLowLimit = signal.outputLowLimit();
	m_outputHighLimit = signal.outputHighLimit();
	m_outputUnitID = signal.outputUnitID();
	m_outputMode = signal.outputMode();
	m_outputSensorID = signal.outputSensorID();
	m_acquire = signal.acquire();
	m_calculated = signal.calculated();
	m_normalState = signal.normalState();
	m_decimalPlaces = signal.decimalPlaces();
	m_aperture = signal.aperture();
	m_inOutType = signal.inOutType();
	m_equipmentID = signal.equipmentID();
	m_filteringTime = signal.filteringTime();
	m_spreadTolerance = signal.spreadTolerance();
	m_byteOrder = signal.byteOrder();
	m_enableTuning = signal.enableTuning();
	m_tuningDefaultValue = signal.tuningDefaultValue();

	return *this;
}

void Signal::InitProperties()
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
	/*static const QString inputLowLimitCaption("InputLowLimit");
	static const QString inputHighLimitCaption("InputHighLimit");
	static const QString inputUnitCaption("InputUnit");
	static const QString inputSensorCaption("InputSensor");
	static const QString outputLowLimitCaption("OutputLowLimit");
	static const QString outputHighLimitCaption("OutputHighLimit");
	static const QString outputUnitCaption("OutputUnit");
	static const QString outputSensorCaption("OutputSensor");*/
	static const QString outputModeCaption("OutputMode");
	static const QString acquireCaption("Acquire");
	/*static const QString calculatedCaption("Calculated");*/
	static const QString normalStateCaption("NormalState");
	static const QString decimalPlacesCaption("DecimalPlaces");
	static const QString apertureCaption("Aperture");
	static const QString filteringTimeCaption("FilteringTime");
	static const QString spreadToleranceCaption("SpreadTolerance");
	static const QString byteOrderCaption("ByteOrder");
	static const QString equipmentIDCaption("EquipmentID");
	static const QString enableTuningCaption("EnableTuning");
	static const QString tuningDefaultValueCaption("TuningDefaultValue");

	/*static const QString inputSensorCategory("Input sensor");
	static const QString outputSensorCategory("Output sensor");*/
	static const QString identificationCategory("1 Identification");
	static const QString signalTypeCategory("2 Signal type");
	static const QString dataFormatCategory("3 Data Format");
	static const QString signalProcessingCategory("4 Signal processing");
	static const QString onlineMonitoringSystemCategory("5 Online Monitoring System");
	static const QString tuningCategory("6 Tuning");

	ADD_PROPERTY_GETTER(int, idCaption, false, Signal::ID);
	ADD_PROPERTY_GETTER(int, signalGroupIDCaption, false, Signal::signalGroupID);
	ADD_PROPERTY_GETTER(int, signalInstanceIDCaption, false, Signal::signalInstanceID);
	ADD_PROPERTY_GETTER(int, changesetIDCaption, false, Signal::changesetID);
	ADD_PROPERTY_GETTER(bool, checkedOutCaption, false, Signal::checkedOut);
	ADD_PROPERTY_GETTER(int, userIdCaption, false, Signal::userID);
	ADD_PROPERTY_GETTER(E::Channel, channelCaption, false, Signal::channel);
	ADD_PROPERTY_GETTER(QDateTime, createdCaption, false, Signal::created);
	ADD_PROPERTY_GETTER(bool, deletedCaption, false, Signal::deleted);
	ADD_PROPERTY_GETTER(QDateTime, instanceCreatedCaption, false, Signal::instanceCreated);
	ADD_PROPERTY_GETTER(E::InstanceAction, instanceActionCaption, false, Signal::instanceAction);

	auto signalTypeProperty = ADD_PROPERTY_GETTER_SETTER(E::SignalType, typeCaption, true, Signal::type, Signal::setType);
	signalTypeProperty->setCategory(signalTypeCategory);
	auto signalInOutTypeProperty = ADD_PROPERTY_GETTER_SETTER(E::SignalInOutType, inOutTypeCaption, true, Signal::inOutType, Signal::setInOutType);
	signalInOutTypeProperty->setCategory(signalTypeCategory);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, appSignalIDCaption, true, Signal::appSignalID, Signal::setAppSignalID);
	strIdProperty->setValidator(cacheValidator1);
	strIdProperty->setCategory(identificationCategory);
	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, customSignalIDCaption, true, Signal::customAppSignalID, Signal::setCustomAppSignalID);
	extStrIdProperty->setValidator(cacheValidator2);
	extStrIdProperty->setCategory(identificationCategory);
	auto nameProperty = ADD_PROPERTY_GETTER_SETTER(QString, captionCaption, true, Signal::caption, Signal::setCaption);
	nameProperty->setValidator(captionValidator);
	nameProperty->setCategory(identificationCategory);
	auto equipmentProperty = ADD_PROPERTY_GETTER_SETTER(QString, equipmentIDCaption, true, Signal::equipmentID, Signal::setEquipmentID);
	equipmentProperty->setCategory(identificationCategory);

	auto enableTuningProperty = ADD_PROPERTY_GETTER_SETTER(bool, enableTuningCaption, true, Signal::enableTuning, Signal::setEnableTuning);
	enableTuningProperty->setCategory(tuningCategory);
	auto tuningDefaultValueProperty = ADD_PROPERTY_GETTER_SETTER(double, tuningDefaultValueCaption, true, Signal::tuningDefaultValue, Signal::setTuningDefaultValue);
	tuningDefaultValueProperty->setCategory(tuningCategory);

	auto dataFormatProperty = ADD_PROPERTY_GETTER_SETTER(E::DataFormat, dataFormatCaption, true, Signal::dataFormat, Signal::setDataFormat);
	dataFormatProperty->setCategory(dataFormatCategory);
	auto dataSizeProperty = ADD_PROPERTY_GETTER_SETTER(int, dataSizeCaption, true, Signal::dataSize, Signal::setDataSize);
	dataSizeProperty->setCategory(dataFormatCategory);

	if (isAnalog())
	{
		static std::shared_ptr<OrderedHash<int, QString>> sensorList = std::make_shared<OrderedHash<int, QString>>();

		if (sensorList->isEmpty())
		{
			for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
			{
				sensorList->append(i, SensorTypeStr[i]);
			}
		}

		auto lowADCProperty = ADD_PROPERTY_GETTER_SETTER(int, lowADCCaption, true, Signal::lowADC, Signal::setLowADC);
		lowADCProperty->setCategory(signalProcessingCategory);
		auto highADCProperty = ADD_PROPERTY_GETTER_SETTER(int, highADCCaption, true, Signal::highADC, Signal::setHighADC);
		highADCProperty->setCategory(signalProcessingCategory);
		auto lowDACProperty = ADD_PROPERTY_GETTER_SETTER(int, lowDACCaption, true, Signal::lowADC, Signal::setLowADC);
		lowDACProperty->setCategory(signalProcessingCategory);
		auto highDACProperty = ADD_PROPERTY_GETTER_SETTER(int, highDACCaption, true, Signal::highADC, Signal::setHighADC);
		highDACProperty->setCategory(signalProcessingCategory);

		auto lowEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER(double, lowEngeneeringUnitsCaption, true, Signal::lowEngeneeringUnits, Signal::setLowEngeneeringUnits);
		lowEngeneeringUnitsProperty->setCategory(signalProcessingCategory);
		auto highEngeneeringUnitsProperty = ADD_PROPERTY_GETTER_SETTER(double, highEngeneeringUnitsCaption, true, Signal::highEngeneeringUnits, Signal::setHighEngeneeringUnits);
		highEngeneeringUnitsProperty->setCategory(signalProcessingCategory);

		auto lowValidRangeProperty = ADD_PROPERTY_GETTER_SETTER(double, lowValidRangeCaption, true, Signal::lowValidRange, Signal::setLowValidRange);
		lowValidRangeProperty->setCategory(signalProcessingCategory);
		auto highValidRangeProperty = ADD_PROPERTY_GETTER_SETTER(double, highValidRangeCaption, true, Signal::highValidRange, Signal::setHighValidRange);
		highValidRangeProperty->setCategory(signalProcessingCategory);

		auto outputModePropetry = ADD_PROPERTY_GETTER_SETTER(E::OutputMode, outputModeCaption, true, Signal::outputMode, Signal::setOutputMode);
		outputModePropetry->setCategory(signalProcessingCategory);

		auto unitProperty = ADD_PROPERTY_DYNAMIC_ENUM(unitCaption, true, m_unitList, Signal::unitID, Signal::setUnitID);
		unitProperty->setCategory(dataFormatCategory);

		auto unbalanceLimitProperty = ADD_PROPERTY_GETTER_SETTER(double, unbalanceLimitCaption, true, Signal::unbalanceLimit, Signal::setUnbalanceLimit);
		unbalanceLimitProperty->setCategory(onlineMonitoringSystemCategory);

		/*auto inputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, inputLowLimitCaption, true, Signal::inputLowLimit, Signal::setInputLowLimit);
		inputLowLimitPropetry->setCategory(inputSensorCategory);

		auto inputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, inputHighLimitCaption, true, Signal::inputHighLimit, Signal::setInputHighLimit);
		inputHighLimitPropetry->setCategory(inputSensorCategory);

		auto inputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(inputUnitCaption, true, m_unitList, Signal::inputUnitID, Signal::setInputUnitID);
		inputUnitIDPropetry->setCategory(inputSensorCategory);

		auto inputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(inputSensorCaption, true, sensorList, Signal::inputSensorID, Signal::setInputSensorID);
		inputSensorPropetry->setCategory(inputSensorCategory);

		auto outputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, outputLowLimitCaption, true, Signal::outputLowLimit, Signal::setOutputLowLimit);
		outputLowLimitPropetry->setCategory(outputSensorCategory);

		auto outputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, outputHighLimitCaption, true, Signal::outputHighLimit, Signal::setOutputHighLimit);
		outputHighLimitPropetry->setCategory(outputSensorCategory);

		auto outputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(outputUnitCaption, true, m_unitList, Signal::outputUnitID, Signal::setOutputUnitID);
		outputUnitIDPropetry->setCategory(outputSensorCategory);

		auto outputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(outputSensorCaption, true, sensorList, Signal::outputSensorID, Signal::setOutputSensorID);
		outputSensorPropetry->setCategory(outputSensorCategory);*/
	}

	auto acquireProperty = ADD_PROPERTY_GETTER_SETTER(bool, acquireCaption, true, Signal::acquire, Signal::setAcquire);
	acquireProperty->setCategory(onlineMonitoringSystemCategory);

	if (isAnalog() == true)
	{
		//ADD_PROPERTY_GETTER_SETTER(bool, calculatedCaption, true, Signal::calculated, Signal::setCalculated);

		auto decimalPlacesProperty = ADD_PROPERTY_GETTER_SETTER(int, decimalPlacesCaption, true, Signal::decimalPlaces, Signal::setDecimalPlaces);
		decimalPlacesProperty->setCategory(onlineMonitoringSystemCategory);
		auto apertureProperty = ADD_PROPERTY_GETTER_SETTER(double, apertureCaption, true, Signal::aperture, Signal::setAperture);
		apertureProperty->setCategory(onlineMonitoringSystemCategory);

		auto spreadToleranceProperty = ADD_PROPERTY_GETTER_SETTER(double, spreadToleranceCaption, true, Signal::spreadTolerance, Signal::setSpreadTolerance);
		spreadToleranceProperty->setCategory(signalProcessingCategory);
		auto filteringTimePropetry = ADD_PROPERTY_GETTER_SETTER(double, filteringTimeCaption, true, Signal::filteringTime, Signal::setFilteringTime);
		filteringTimePropetry->setPrecision(6);
		filteringTimePropetry->setCategory(signalProcessingCategory);
	}
	else
	{
		auto normalStateProperty = ADD_PROPERTY_GETTER_SETTER(int, normalStateCaption, true, Signal::normalState, Signal::setNormalState);
		normalStateProperty->setCategory(onlineMonitoringSystemCategory);
	}

	auto byteOrderProperty = ADD_PROPERTY_GETTER_SETTER(E::ByteOrder, byteOrderCaption, true, Signal::byteOrder, Signal::setByteOrder);
	byteOrderProperty->setCategory(dataFormatCategory);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	if (strValue == "true")
	{
		(this->*setter)(true);
		return;
	}
	if (strValue == "false")
	{
		(this->*setter)(false);
		return;
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	bool ok = false;
	int intValue = strValue.toInt(&ok);
	if (!ok)
	{
		assert(false);
		return;
	}
	(this->*setter)(intValue);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	bool ok = false;
	double doubleValue = strValue.toDouble(&ok);
	if (!ok)
	{
		assert(false);
		return;
	}
	(this->*setter)(doubleValue);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&))
{
	const QStringRef& strValue = attr.value(fieldName);
	(this->*setter)(strValue.toString());
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalType))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	if (strValue == "Analog")
	{
		(this->*setter)(E::SignalType::Analog);
		return;
	}
	if (strValue == "Discrete")
	{
		(this->*setter)(E::SignalType::Discrete);
		return;
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputMode))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < OUTPUT_MODE_COUNT; i++)
	{
		if (strValue == OutputModeStr[i])
		{
			(this->*setter)(static_cast<E::OutputMode>(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		if (strValue == InOutTypeStr[i])
		{
			(this->*setter)(static_cast<E::SignalInOutType>(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ByteOrder))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	auto el = E::enumValues<E::ByteOrder>();

	for (auto e : el)
	{
		if (e.second == strValue)
		{
			(this->*setter)(static_cast<E::ByteOrder>(e.first));
			return;
		}
	}

	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	Address16 address(0, 0);
	address.fromString(strValue.toString());
	(this->*setter)(address);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(E::DataFormat))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < dataFormatInfo.count(); i++)
	{
		if (strValue == dataFormatInfo[i])
		{
			(this->*setter)(static_cast<E::DataFormat>(dataFormatInfo.keyAt(i)));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	for (int i = 0; i < unitInfo.count(); i++)
	{
		if (strValue == unitInfo[i])
		{
			(this->*setter)(unitInfo.keyAt(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeSensorField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
	{
		if (strValue == SensorTypeStr[i])
		{
			(this->*setter)(i);
			return;
		}
	}
	assert(false);
}

void Signal::serializeFields(const QXmlStreamAttributes& attr, DataFormatList& dataFormatInfo, UnitList &unitInfo)
{
	serializeField(attr, "ID", &Signal::setID);
	serializeField(attr, "SignalGroupID", &Signal::setSignalGroupID);
	serializeField(attr, "SignalInstanceID", &Signal::setSignalInstanceID);
//	serializeField(attr, "Channel", &Signal::setChannel);
	serializeField(attr, "Type", &Signal::setType);
	serializeField(attr, "StrID", &Signal::setAppSignalID);
	serializeField(attr, "ExtStrID", &Signal::setCustomAppSignalID);
	serializeField(attr, "Name", &Signal::setCaption);
	serializeField(attr, "DataFormat", dataFormatInfo, &Signal::setDataFormat);
	serializeField(attr, "DataSize", &Signal::setDataSize);
	serializeField(attr, "LowADC", &Signal::setLowADC);
	serializeField(attr, "HighADC", &Signal::setHighADC);
	serializeField(attr, "LowEngeneeringUnits", &Signal::setLowEngeneeringUnits);
	serializeField(attr, "HighEngeneeringUnits", &Signal::setHighEngeneeringUnits);
	serializeField(attr, "UnitID", unitInfo, &Signal::setUnitID);
	//serializeField(attr, "Adjustment", &Signal::setAdjustment);
	serializeField(attr, "LowValidRange", &Signal::setLowValidRange);
	serializeField(attr, "HighValidRange", &Signal::setHighValidRange);
	serializeField(attr, "UnbalanceLimit", &Signal::setUnbalanceLimit);
	serializeField(attr, "InputLowLimit", &Signal::setInputLowLimit);
	serializeField(attr, "InputHighLimit", &Signal::setInputHighLimit);
	serializeField(attr, "InputUnitID", unitInfo, &Signal::setInputUnitID);
	serializeSensorField(attr, "InputSensorID", &Signal::setInputSensorID);
	serializeField(attr, "OutputLowLimit", &Signal::setOutputLowLimit);
	serializeField(attr, "OutputHighLimit", &Signal::setOutputHighLimit);
	serializeField(attr, "OutputUnitID", unitInfo, &Signal::setOutputUnitID);
	serializeField(attr, "OutputMode", &Signal::setOutputMode);
	serializeSensorField(attr, "OutputSensorID", &Signal::setOutputSensorID);
	serializeField(attr, "Acquire", &Signal::setAcquire);
	serializeField(attr, "Calculated", &Signal::setCalculated);
	serializeField(attr, "NormalState", &Signal::setNormalState);
	serializeField(attr, "DecimalPlaces", &Signal::setDecimalPlaces);
	serializeField(attr, "Aperture", &Signal::setAperture);
	serializeField(attr, "InOutType", &Signal::setInOutType);
	serializeField(attr, "DeviceStrID", &Signal::setEquipmentID);
	serializeField(attr, "FilteringTime", &Signal::setFilteringTime);
	serializeField(attr, "SpreadTolerance", &Signal::setSpreadTolerance);
	serializeField(attr, "ByteOrder", &Signal::setByteOrder);
	serializeField(attr, "RamAddr", &Signal::setRamAddr);
	serializeField(attr, "RegAddr", &Signal::setRegValueAddr);
}



bool Signal::isCompatibleDataFormat(Afb::AfbDataFormat afbDataFormat) const
{
	if (m_dataFormat == E::DataFormat::Float && afbDataFormat == Afb::AfbDataFormat::Float)
	{
		return true;
	}

	if (m_dataFormat == E::DataFormat::SignedInt && afbDataFormat == Afb::AfbDataFormat::SignedInt)
	{
		return true;
	}

	if (m_dataFormat == E::DataFormat::UnsignedInt && afbDataFormat == Afb::AfbDataFormat::UnsignedInt)
	{
		return true;
	}

	return false;
}

void Signal::setReadOnly(bool value)
{
	for (auto property : properties())
	{
		property->setReadOnly(value);
	}
}


SignalSet::SignalSet()
{
}


SignalSet::~SignalSet()
{
	clear();
}


void SignalSet::clear()
{
	SignalPtrOrderedHash::clear();

	m_groupSignals.clear();
}


void SignalSet::append(const int& signalID, Signal *signal)
{
	SignalPtrOrderedHash::append(signalID, signal);

	m_groupSignals.insert(signal->signalGroupID(), signalID);
}


void SignalSet::remove(const int& signalID)
{
	Signal signal = value(signalID);

	SignalPtrOrderedHash::remove(signalID);

	m_groupSignals.remove(signal.signalGroupID(), signalID);
}


void SignalSet::removeAt(const int index)
{
	const Signal& signal = SignalPtrOrderedHash::operator [](index);

	int signalGroupID = signal.signalGroupID();
	int signalID = signal.ID();

	SignalPtrOrderedHash::removeAt(index);

	m_groupSignals.remove(signalGroupID, signalID);
}


QVector<int> SignalSet::getChannelSignalsID(const Signal& signal)
{
	return getChannelSignalsID(signal.signalGroupID());
}


QVector<int> SignalSet::getChannelSignalsID(int signalGroupID)
{
	QVector<int> channelSignalsID;

	QList<int> signalsID = m_groupSignals.values(signalGroupID);

	int signalCount = signalsID.count();

	for(int i = 0; i< signalCount; i++)
	{
		channelSignalsID.append(signalsID.at(i));
	}

	return channelSignalsID;
}


void SignalSet::reserve(int n)
{
	SignalPtrOrderedHash::reserve(n);
	m_groupSignals.reserve(n);
}


void SignalSet::resetAddresses()
{
	int signalCount = count();

	for(int i = 0; i < signalCount; i++)
	{
		(*this)[i].resetAddresses();
	}
}


void SerializeSignalsFromXml(const QString& filePath, UnitList& unitInfo, SignalSet& signalSet)
{
	QXmlStreamReader applicationSignalsReader;
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly))
	{
		DataFormatList dataFormatInfo;
		applicationSignalsReader.setDevice(&file);

		while (!applicationSignalsReader.atEnd())
		{
			QXmlStreamReader::TokenType token = applicationSignalsReader.readNext();

			switch (token)
			{
			case QXmlStreamReader::StartElement:
			{
				const QXmlStreamAttributes& attr = applicationSignalsReader.attributes();
				if (applicationSignalsReader.name() == "Unit")
				{
					unitInfo.append(attr.value("ID").toInt(), attr.value("Name").toString());
				}
				if (applicationSignalsReader.name() == "Signal")
				{
					Signal* pSignal = new Signal;
					pSignal->serializeFields(attr, dataFormatInfo, unitInfo);
					signalSet.append(pSignal->ID(), pSignal);
				}
				break;
			}
			default:
				continue;
			}
		}
		if (applicationSignalsReader.hasError())
		{
			qDebug() << "Parse applicationSignals.xml error";
		}
	}
}


void Signal::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement("Signal");	// <Signal>

	xml.writeIntAttribute("ID", ID());
	xml.writeIntAttribute("GroupID", signalGroupID());
	xml.writeIntAttribute("InstanceID", signalInstanceID());
	xml.writeIntAttribute("Channel", channelInt());
	xml.writeIntAttribute("Type", typeInt());
	xml.writeStringAttribute("AppSignalID", appSignalID());
	xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
	xml.writeStringAttribute("Caption", caption());
	xml.writeStringAttribute("EquipmentID", equipmentID());
	xml.writeIntAttribute("DataFormat", dataFormatInt());
	xml.writeIntAttribute("DataSize", dataSize());
	xml.writeIntAttribute("LowADC", lowADC());
	xml.writeIntAttribute("HighADC", highADC());
	xml.writeDoubleAttribute("LowEngeneeringUnits", lowEngeneeringUnits());
	xml.writeDoubleAttribute("HighEngeneeringUnits", highEngeneeringUnits());
	xml.writeIntAttribute("UnitID", unitID());
	xml.writeDoubleAttribute("LowValidRange", lowValidRange());
	xml.writeDoubleAttribute("HighValidRange", highValidRange());
	xml.writeDoubleAttribute("UnbalanceLimit", unbalanceLimit());
	xml.writeDoubleAttribute("InputLowLimit", inputLowLimit());
	xml.writeDoubleAttribute("InputHighLimit", inputHighLimit());
	xml.writeIntAttribute("InputUnitID", inputUnitID());
	xml.writeIntAttribute("InputSensorID", inputSensorID());
	xml.writeDoubleAttribute("OutputLowLimit", outputLowLimit());
	xml.writeDoubleAttribute("OutputHighLimit", outputHighLimit());
	xml.writeIntAttribute("OutputUnitID", outputUnitID());
	xml.writeIntAttribute("OutputMode", outputModeInt());
	xml.writeIntAttribute("OutputSensorID", outputSensorID());
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeBoolAttribute("Calculated", calculated());
	xml.writeIntAttribute("NormalState", normalState());
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("Aperture", aperture());
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	xml.writeDoubleAttribute("FilteringTime", filteringTime());
	xml.writeDoubleAttribute("SpreadTolerance", spreadTolerance());
	xml.writeIntAttribute("ByteOrder", byteOrderInt());
	xml.writeBoolAttribute("EnableTuning", enableTuning());
	xml.writeDoubleAttribute("TuningDefaultValue", tuningDefaultValue());

	xml.writeIntAttribute("RamAddrOffset", ramAddr().offset());
	xml.writeIntAttribute("RamAddrBit", ramAddr().bit());
	xml.writeIntAttribute("ValueOffset", regValueAddr().offset());
	xml.writeIntAttribute("ValueBit", regValueAddr().bit());
	xml.writeIntAttribute("ValidityOffset", regValidityAddr().offset());
	xml.writeIntAttribute("ValidityBit", regValidityAddr().bit());

	xml.writeIntAttribute("TuningOffset", tuningAddr().offset());
	xml.writeIntAttribute("TuningBit", tuningAddr().bit());

	xml.writeEndElement();				// </Signal>
}


bool Signal::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	if (xml.name() != "Signal")
	{
		return false;
	}

	result &= xml.readIntAttribute("ID", &m_ID);
	result &= xml.readIntAttribute("GroupID", &m_signalGroupID);
	result &= xml.readIntAttribute("InstanceID", &m_signalInstanceID);

	int intValue = 0;

	result &= xml.readIntAttribute("Channel", &intValue);
	m_channel = static_cast<E::Channel>(intValue);

	int type = 0;

	result &= xml.readIntAttribute("Type", &type);
	m_type = static_cast<E::SignalType>(type);

	result &= xml.readStringAttribute("AppSignalID", &m_appSignalID);
	result &= xml.readStringAttribute("CustomAppSignalID", &m_customAppSignalID);
	result &= xml.readStringAttribute("Caption", &m_caption);
	result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);

	result &= xml.readIntAttribute("DataFormat", &intValue);
	m_dataFormat = static_cast<E::DataFormat>(intValue);

	result &= xml.readIntAttribute("DataSize", &m_dataSize);
	result &= xml.readIntAttribute("LowADC", &m_lowADC);
	result &= xml.readIntAttribute("HighADC", &m_highADC);
	result &= xml.readDoubleAttribute("LowEngeneeringUnits", &m_lowEngeneeringUnits);
	result &= xml.readDoubleAttribute("HighEngeneeringUnits", &m_highEngeneeringUnits);
	result &= xml.readIntAttribute("UnitID", &m_unitID);
	result &= xml.readDoubleAttribute("LowValidRange", &m_lowValidRange);
	result &= xml.readDoubleAttribute("HighValidRange", &m_highValidRange);
	result &= xml.readDoubleAttribute("UnbalanceLimit", &m_unbalanceLimit);
	result &= xml.readDoubleAttribute("InputLowLimit", &m_inputLowLimit);
	result &= xml.readDoubleAttribute("InputHighLimit", &m_inputHighLimit);
	result &= xml.readIntAttribute("InputUnitID", &m_inputUnitID);
	result &= xml.readIntAttribute("InputSensorID", &m_inputSensorID);
	result &= xml.readDoubleAttribute("OutputLowLimit", &m_outputLowLimit);
	result &= xml.readDoubleAttribute("OutputHighLimit", &m_outputHighLimit);
	result &= xml.readIntAttribute("OutputUnitID", &m_outputUnitID);

	result &= xml.readIntAttribute("OutputMode", &intValue);
	m_outputMode = static_cast<E::OutputMode>(intValue);

	result &= xml.readIntAttribute("OutputSensorID", &m_outputSensorID);
	result &= xml.readBoolAttribute("Acquire", &m_acquire);
	result &= xml.readBoolAttribute("Calculated", &m_calculated);
	result &= xml.readIntAttribute("NormalState", &m_normalState);
	result &= xml.readIntAttribute("DecimalPlaces", &m_decimalPlaces);
	result &= xml.readDoubleAttribute("Aperture", &m_aperture);

	result &= xml.readIntAttribute("InOutType", &intValue);
	m_inOutType = static_cast<E::SignalInOutType>(intValue);

	result &= xml.readDoubleAttribute("FilteringTime", &m_filteringTime);
	result &= xml.readDoubleAttribute("SpreadTolerance", &m_spreadTolerance);

	result &= xml.readIntAttribute("ByteOrder", &intValue);
	m_byteOrder = static_cast<E::ByteOrder>(intValue);

	result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);
	result &= xml.readDoubleAttribute("TuningDefaultValue", &m_tuningDefaultValue);

	int offset = 0;
	int bit = 0;

	result &= xml.readIntAttribute("RamAddrOffset", &offset);
	result &= xml.readIntAttribute("RamAddrBit", &bit);

	m_ramAddr.setOffset(offset);
	m_ramAddr.setBit(bit);

	offset = bit = 0;

	result &= xml.readIntAttribute("ValueOffset", &offset);
	result &= xml.readIntAttribute("ValueBit", &bit);

	m_regValueAddr.setOffset(offset);
	m_regValueAddr.setBit(bit);

	result &= xml.readIntAttribute("ValidityOffset", &offset);
	result &= xml.readIntAttribute("ValidityBit", &bit);

	m_regValidityAddr.setOffset(offset);
	m_regValidityAddr.setBit(bit);

	result &= xml.readIntAttribute("TuningOffset", &offset);
	result &= xml.readIntAttribute("TuningBit", &bit);

	m_tuningAddr.setOffset(offset);
	m_tuningAddr.setBit(bit);

	return result;
}


void Signal::serializeToProtoAppSignal(Proto::AppSignal* s) const
{
	if (s == nullptr)
	{
		assert(false);
		return;
	}

	s->set_id(m_ID);
	s->set_signalgroupid(m_signalGroupID);
	s->set_signalinstanceid(m_signalInstanceID);
	s->set_changesetid(m_changesetID);
	s->set_checkedout(m_checkedOut);
	s->set_userid(m_userID);
	s->set_subsystemchannel(TO_INT(m_channel));
	s->set_type(m_type);
	s->set_created(m_created.toMSecsSinceEpoch());
	s->set_deleted(m_deleted);
	s->set_instancecreated(m_instanceCreated.toMSecsSinceEpoch());
	s->set_instanceaction(m_instanceAction);
	s->set_appsignalid(m_appSignalID.toStdString());
	s->set_customappsignalid(m_customAppSignalID.toStdString());
	s->set_caption(m_caption.toStdString());
	s->set_dataformat(m_dataFormat);
	s->set_datasize(m_dataSize);
	s->set_lowadc(m_lowADC);
	s->set_highadc(m_highADC);
	s->set_lowengeneeringunits(m_lowEngeneeringUnits);
	s->set_highengeneeringunits(m_highEngeneeringUnits);
	s->set_unitid(m_unitID);
	s->set_lowvalidrange(m_lowValidRange);
	s->set_highvalidrange(m_highValidRange);
	s->set_unbalancelimit(m_unbalanceLimit);
	s->set_inputlowlimit(m_inputLowLimit);
	s->set_inputhighlimit(m_inputHighLimit);
	s->set_inputunitid(m_inputUnitID);
	s->set_inputsensorid(m_inputSensorID);
	s->set_outputlowlimit(m_outputLowLimit);
	s->set_outputhighlimit(m_outputHighLimit);
	s->set_outputunitid(m_outputUnitID);
	s->set_outputmode(m_outputMode);
	s->set_outputsensorid(m_outputSensorID);
	s->set_acquire(m_acquire);
	s->set_calculated(m_calculated);
	s->set_normalstate(m_normalState);
	s->set_decimalplaces(m_decimalPlaces);
	s->set_aperture(m_aperture);
	s->set_inouttype(TO_INT(m_inOutType));
	s->set_equipmentid(m_equipmentID.toStdString());
	s->set_filteringtime(m_filteringTime);
	s->set_spreadtolerance(m_spreadTolerance);
	s->set_byteorder(TO_INT(m_byteOrder));
	s->set_enabletuning(m_enableTuning);
	s->set_tuningdefaultvalue(m_tuningDefaultValue);

	s->set_hash(calcHash(m_appSignalID));

	s->set_regvalueaddroffset(m_regValueAddr.offset());
	s->set_regvalueaddrbit(m_regValueAddr.bit());

	s->set_regvalidityaddroffset(m_regValidityAddr.offset());
	s->set_regvalidityaddrbit(m_regValidityAddr.bit());

	s->set_iobufferaddroffset(m_ioBufferAddr.offset());
	s->set_iobufferaddrbit(m_ioBufferAddr.bit());

	s->set_ramaddroffset(m_ramAddr.offset());
	s->set_ramaddrbit(m_ramAddr.bit());
}


void Signal::serializeFromProtoAppSignal(const Proto::AppSignal* s)
{
	if (s == nullptr)
	{
		assert(false);
		return;
	}

	if (s->has_id())
	{
		m_ID = s->id();
	}

	if (s->has_signalgroupid())
	{
		m_signalGroupID = s->signalgroupid();
	}

	if (s->has_signalinstanceid())
	{
		m_signalInstanceID = s->signalinstanceid();
	}

	if (s->has_changesetid())
	{
		m_changesetID = s->changesetid();
	}

	if (s->has_checkedout())
	{
		m_checkedOut = s->checkedout();
	}

	if (s->has_userid())
	{
		m_userID = s->userid();
	}

	if (s->has_subsystemchannel())
	{
		m_channel = static_cast<E::Channel>(s->subsystemchannel());
	}

	if (s->has_type())
	{
		m_type = static_cast<E::SignalType>(s->type());
	}

	if (s->has_created())
	{
		m_created.setMSecsSinceEpoch(s->created());
	}

	if (s->has_deleted())
	{
		m_deleted = s->deleted();
	}

	if (s->has_instancecreated())
	{
		m_instanceCreated.setMSecsSinceEpoch(s->instancecreated());
	}

	if (s->has_instanceaction())
	{
		m_instanceAction = static_cast<E::InstanceAction>(s->instanceaction());
	}

	if (s->has_appsignalid())
	{
		m_appSignalID = QString::fromStdString(s->appsignalid());
	}

	if (s->has_customappsignalid())
	{
		m_customAppSignalID = QString::fromStdString(s->customappsignalid());
	}

	if (s->has_caption())
	{
		m_caption = QString::fromStdString(s->caption());
	}

	if(s->has_dataformat())
	{
		m_dataFormat = static_cast<E::DataFormat>(s->dataformat());
	}

	if (s->has_datasize())
	{
		m_dataSize = s->datasize();
	}

	if (s->has_lowadc())
	{
		m_lowADC = s->lowadc();
	}

	if (s->has_highadc())
	{
		m_highADC = s->highadc();
	}

	if (s->has_lowengeneeringunits())
	{
		m_lowEngeneeringUnits = s->lowengeneeringunits();
	}

	if (s->has_highengeneeringunits())
	{
		m_highEngeneeringUnits = s->highengeneeringunits();
	}

	if (s->has_unitid())
	{
		m_unitID = s->unitid();
	}

	if (s->has_lowvalidrange())
	{
		m_lowValidRange = s->lowvalidrange();
	}

	if (s->has_highvalidrange())
	{
		m_highValidRange = s->highvalidrange();
	}

	if (s->has_unbalancelimit())
	{
		m_unbalanceLimit = s->unbalancelimit();
	}

	if (s->has_inputlowlimit())
	{
		m_inputLowLimit = s->inputlowlimit();
	}

	if (s->has_inputhighlimit())
	{
		m_inputHighLimit = s->inputhighlimit();
	}

	if (s->has_inputunitid())
	{
		m_inputUnitID = s->inputunitid();
	}

	if (s->has_inputsensorid())
	{
		m_inputSensorID = s->inputsensorid();
	}

	if (s->has_outputlowlimit())
	{
		m_outputLowLimit = s->outputlowlimit();
	}

	if (s->has_outputhighlimit())
	{
		m_outputHighLimit = s->outputhighlimit();
	}

	if (s->has_outputunitid())
	{
		m_outputUnitID = s->outputunitid();
	}

	if (s->has_outputmode())
	{
		m_outputMode = static_cast<E::OutputMode>(s->outputmode());
	}

	if (s->has_outputsensorid())
	{
		m_outputSensorID = s->outputsensorid();
	}

	if (s->has_acquire())
	{
		m_acquire = s->acquire();
	}

	if (s->has_calculated())
	{
		m_calculated = s->calculated();
	}

	if (s->has_normalstate())
	{
		m_normalState = s->normalstate();
	}

	if (s->has_decimalplaces())
	{
		m_decimalPlaces = s->decimalplaces();
	}

	if (s->has_aperture())
	{
		m_aperture = s->aperture();
	}

	if (s->has_inouttype())
	{
		m_inOutType = static_cast<E::SignalInOutType>(s->inouttype());
	}

	if (s->has_equipmentid())
	{
		m_equipmentID = QString::fromStdString(s->equipmentid());
	}

	if (s->has_filteringtime())
	{
		m_filteringTime = s->filteringtime();
	}

	if (s->has_spreadtolerance())
	{
		m_spreadTolerance = s->spreadtolerance();
	}

	if (s->has_byteorder())
	{
		m_byteOrder = static_cast<E::ByteOrder>(s->byteorder());
	}

	if (s->has_enabletuning())
	{
		m_enableTuning = s->enabletuning();
	}

	if (s->has_tuningdefaultvalue())
	{
		m_tuningDefaultValue = s->tuningdefaultvalue();
	}

	if (s->has_hash())
	{
		m_hash = s->hash();
	}

	if (s->has_regvalueaddroffset())
	{
		m_regValueAddr.setOffset(s->regvalueaddroffset());
	}

	if (s->has_regvalueaddrbit())
	{
		m_regValueAddr.setBit(s->regvalueaddrbit());
	}

	if (s->has_regvalidityaddroffset())
	{
		m_regValidityAddr.setOffset(s->regvalidityaddroffset());
	}

	if (s->has_regvalidityaddrbit())
	{
		m_regValidityAddr.setBit(s->regvalidityaddrbit());
	}

	if (s->has_iobufferaddroffset())
	{
		m_ioBufferAddr.setOffset(s->iobufferaddroffset());
	}

	if (s->has_iobufferaddrbit())
	{
		m_ioBufferAddr.setBit(s->iobufferaddrbit());
	}

	if (s->has_ramaddroffset())
	{
		m_ramAddr.setOffset(s->ramaddroffset());
	}

	if (s->has_ramaddrbit())
	{
		m_ramAddr.setBit(s->ramaddrbit());
	}
}


void SignalSet::buildStrID2IndexMap()
{
	int signalCount = count();

	m_strID2IndexMap.clear();

	for(int i = 0; i < signalCount; i++)
	{
		const Signal& s = (*this)[i];

		if (m_strID2IndexMap.contains(s.appSignalID()))
		{
			qDebug() << "Duplicate signal strID " << s.appSignalID();
			continue;
		}

		m_strID2IndexMap.insert(s.appSignalID(), i);
	}
}


bool SignalSet::contains(const QString& appSignalID)
{
	if (count() > 0 && m_strID2IndexMap.isEmpty() == true)
	{
		assert(false);		//call buildStrID2IndexMap() before
		return false;
	}

	return m_strID2IndexMap.contains(appSignalID);
}


Signal* SignalSet::getSignal(const QString& appSignalID)
{
	if (count() > 0 && m_strID2IndexMap.isEmpty() == true)
	{
		assert(false);		//	call buildStrID2IndexMap() before
		return nullptr;
	}

	if (m_strID2IndexMap.contains(appSignalID))
	{
		return &(*this)[m_strID2IndexMap[appSignalID]];
	}

	assert(false);			//	appSignalD is not found
	return nullptr;
}
