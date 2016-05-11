#include "../include/Signal.h"
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
	m_lowLimit = signal.lowLimit();
	m_highLimit = signal.highLimit();
	m_unitID = signal.unitID();
	m_adjustment = signal.adjustment();
	m_dropLimit = signal.dropLimit();
	m_excessLimit = signal.excessLimit();
	m_unbalanceLimit = signal.unbalanceLimit();
	m_inputLowLimit = signal.inputLowLimit();
	m_inputHighLimit = signal.inputHighLimit();
	m_inputUnitID = signal.inputUnitID();
	m_inputSensorID = signal.inputSensorID();
	m_outputLowLimit = signal.outputLowLimit();
	m_outputHighLimit = signal.outputHighLimit();
	m_outputUnitID = signal.outputUnitID();
	m_outputRangeMode = signal.outputRangeMode();
	m_outputSensorID = signal.outputSensorID();
	m_acquire = signal.acquire();
	m_calculated = signal.calculated();
	m_normalState = signal.normalState();
	m_decimalPlaces = signal.decimalPlaces();
	m_aperture = signal.aperture();
	m_inOutType = signal.inOutType();
	m_equipmentID = signal.equipmentID();
	m_filteringTime = signal.filteringTime();
	m_maxDifference = signal.maxDifference();
	m_byteOrder = signal.byteOrder();
	m_enableTuning = signal.enableTuning();
	m_tuningDefaultValue = signal.tuningDefaultValue();

	return *this;
}

void Signal::InitProperties()
{
	ADD_PROPERTY_GETTER(int, ID, false, Signal::ID);
	ADD_PROPERTY_GETTER(int, SignalGroupID, false, Signal::signalGroupID);
	ADD_PROPERTY_GETTER(int, SignalInstanceID, false, Signal::signalInstanceID);
	ADD_PROPERTY_GETTER(int, ChangesetID, false, Signal::changesetID);
	ADD_PROPERTY_GETTER(bool, CheckedOut, false, Signal::checkedOut);
	ADD_PROPERTY_GETTER(int, UserID, false, Signal::userID);
	ADD_PROPERTY_GETTER(E::Channel, Channel, false, Signal::channel);
	ADD_PROPERTY_GETTER(QDateTime, Created, false, Signal::created);
	ADD_PROPERTY_GETTER(bool, Deleted, false, Signal::deleted);
	ADD_PROPERTY_GETTER(QDateTime, InstanceCreated, false, Signal::instanceCreated);
	ADD_PROPERTY_GETTER(E::InstanceAction, InstanceAction, false, Signal::instanceAction);

	ADD_PROPERTY_GETTER_SETTER(E::SignalType, Type, false, Signal::type, Signal::setType);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, AppSignalID, true, Signal::appSignalID, Signal::setAppSignalID);
	strIdProperty->setValidator("^#[A-Za-z][A-Za-z\\d_]*$");
	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, CustomAppSignalID, true, Signal::customAppSignalID, Signal::setCustomAppSignalID);
	extStrIdProperty->setValidator("^[A-Za-z][A-Za-z\\d_]*$");
	auto nameProperty = ADD_PROPERTY_GETTER_SETTER(QString, Caption, true, Signal::caption, Signal::setCaption);
	nameProperty->setValidator("^.+$");
	ADD_PROPERTY_GETTER_SETTER(E::DataFormat, DataFormat, true, Signal::dataFormat, Signal::setDataFormat);
	ADD_PROPERTY_GETTER_SETTER(int, DataSize, true, Signal::dataSize, Signal::setDataSize);
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
		ADD_PROPERTY_GETTER_SETTER(int, LowADC, true, Signal::lowADC, Signal::setLowADC);
		ADD_PROPERTY_GETTER_SETTER(int, HighADC, true, Signal::highADC, Signal::setHighADC);
		ADD_PROPERTY_GETTER_SETTER(double, LowLimit, true, Signal::lowLimit, Signal::setLowLimit);
		ADD_PROPERTY_GETTER_SETTER(double, HighLimit, true, Signal::highLimit, Signal::setHighLimit);
		ADD_PROPERTY_DYNAMIC_ENUM(Unit, true, m_unitList, Signal::unitID, Signal::setUnitID);
		ADD_PROPERTY_GETTER_SETTER(double, Adjustment, true, Signal::adjustment, Signal::setAdjustment);
		ADD_PROPERTY_GETTER_SETTER(double, DropLimit, true, Signal::dropLimit, Signal::setDropLimit);
		ADD_PROPERTY_GETTER_SETTER(double, ExcessLimit, true, Signal::excessLimit, Signal::setExcessLimit);
		ADD_PROPERTY_GETTER_SETTER(double, UnbalanceLimit, true, Signal::unbalanceLimit, Signal::setUnbalanceLimit);
		auto inputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, InputLowLimit, true, Signal::inputLowLimit, Signal::setInputLowLimit);
		inputLowLimitPropetry->setCategory("Input sensor");
		auto inputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, InputHighLimit, true, Signal::inputHighLimit, Signal::setInputHighLimit);
		inputHighLimitPropetry->setCategory("Input sensor");
		auto inputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(InputUnit, true, m_unitList, Signal::inputUnitID, Signal::setInputUnitID);/*ADD_PROPERTY_GETTER_SETTER(int, InputUnitID, true, Signal::inputUnitID, Signal::setInputUnitID);*/
		inputUnitIDPropetry->setCategory("Input sensor");
		auto inputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(InputSensor, true, sensorList, Signal::inputSensorID, Signal::setInputSensorID);
		inputSensorPropetry->setCategory("Input sensor");
		auto outputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, OutputLowLimit, true, Signal::outputLowLimit, Signal::setOutputLowLimit);
		outputLowLimitPropetry->setCategory("Output sensor");
		auto outputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, OutputHighLimit, true, Signal::outputHighLimit, Signal::setOutputHighLimit);
		outputHighLimitPropetry->setCategory("Output sensor");
		auto outputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(OutputUnit, true, m_unitList, Signal::outputUnitID, Signal::setOutputUnitID);/*ADD_PROPERTY_GETTER_SETTER(int, OutputUnitID, true, Signal::outputUnitID, Signal::setOutputUnitID);*/
		outputUnitIDPropetry->setCategory("Output sensor");
		auto outputRangeModePropetry = ADD_PROPERTY_GETTER_SETTER(E::OutputRangeMode, OutputRangeMode, true, Signal::outputRangeMode, Signal::setOutputRangeMode);
		outputRangeModePropetry->setCategory("Output sensor");
		auto outputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(OutputSensor, true, sensorList, Signal::outputSensorID, Signal::setOutputSensorID);
		outputSensorPropetry->setCategory("Output sensor");
	}
	ADD_PROPERTY_GETTER_SETTER(bool, Acquire, true, Signal::acquire, Signal::setAcquire);
	if (isAnalog())
	{
		ADD_PROPERTY_GETTER_SETTER(bool, Calculated, true, Signal::calculated, Signal::setCalculated);
		ADD_PROPERTY_GETTER_SETTER(int, NormalState, true, Signal::normalState, Signal::setNormalState);
		ADD_PROPERTY_GETTER_SETTER(int, DecimalPlaces, true, Signal::decimalPlaces, Signal::setDecimalPlaces);
		ADD_PROPERTY_GETTER_SETTER(double, Aperture, true, Signal::aperture, Signal::setAperture);
		auto filteringTimePropetry = ADD_PROPERTY_GETTER_SETTER(double, FilteringTime, true, Signal::filteringTime, Signal::setFilteringTime);
		filteringTimePropetry->setPrecision(6);
		ADD_PROPERTY_GETTER_SETTER(double, MaxDifference, true, Signal::maxDifference, Signal::setMaxDifference);
	}
	ADD_PROPERTY_GETTER_SETTER(E::SignalInOutType, InOutType, true, Signal::inOutType, Signal::setInOutType);
	ADD_PROPERTY_GETTER_SETTER(E::ByteOrder, ByteOrder, true, Signal::byteOrder, Signal::setByteOrder);
	ADD_PROPERTY_GETTER_SETTER(QString, EquipmentID, true, Signal::equipmentID, Signal::setEquipmentID);
	ADD_PROPERTY_GETTER_SETTER(bool, EnableTuning, true, Signal::enableTuning, Signal::setEnableTuning);
	ADD_PROPERTY_GETTER_SETTER(double, TuningDefaultValue, true, Signal::tuningDefaultValue, Signal::setTuningDefaultValue);
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputRangeMode))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < OUTPUT_RANGE_MODE_COUNT; i++)
	{
		if (strValue == OutputRangeModeStr[i])
		{
			(this->*setter)(static_cast<E::OutputRangeMode>(i));
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
	serializeField(attr, "LowLimit", &Signal::setLowLimit);
	serializeField(attr, "HighLimit", &Signal::setHighLimit);
	serializeField(attr, "UnitID", unitInfo, &Signal::setUnitID);
	serializeField(attr, "Adjustment", &Signal::setAdjustment);
	serializeField(attr, "DropLimit", &Signal::setDropLimit);
	serializeField(attr, "ExcessLimit", &Signal::setExcessLimit);
	serializeField(attr, "UnbalanceLimit", &Signal::setUnbalanceLimit);
	serializeField(attr, "InputLowLimit", &Signal::setInputLowLimit);
	serializeField(attr, "InputHighLimit", &Signal::setInputHighLimit);
	serializeField(attr, "InputUnitID", unitInfo, &Signal::setInputUnitID);
	serializeSensorField(attr, "InputSensorID", &Signal::setInputSensorID);
	serializeField(attr, "OutputLowLimit", &Signal::setOutputLowLimit);
	serializeField(attr, "OutputHighLimit", &Signal::setOutputHighLimit);
	serializeField(attr, "OutputUnitID", unitInfo, &Signal::setOutputUnitID);
	serializeField(attr, "OutputRangeMode", &Signal::setOutputRangeMode);
	serializeSensorField(attr, "OutputSensorID", &Signal::setOutputSensorID);
	serializeField(attr, "Acquire", &Signal::setAcquire);
	serializeField(attr, "Calculated", &Signal::setCalculated);
	serializeField(attr, "NormalState", &Signal::setNormalState);
	serializeField(attr, "DecimalPlaces", &Signal::setDecimalPlaces);
	serializeField(attr, "Aperture", &Signal::setAperture);
	serializeField(attr, "InOutType", &Signal::setInOutType);
	serializeField(attr, "DeviceStrID", &Signal::setEquipmentID);
	serializeField(attr, "FilteringTime", &Signal::setFilteringTime);
	serializeField(attr, "MaxDifference", &Signal::setMaxDifference);
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
	xml.writeDoubleAttribute("LowLimit", lowLimit());
	xml.writeDoubleAttribute("HighLimit", highLimit());
	xml.writeIntAttribute("UnitID", unitID());
	xml.writeDoubleAttribute("Adjustment", adjustment());
	xml.writeDoubleAttribute("DropLimit", dropLimit());
	xml.writeDoubleAttribute("ExcessLimit", excessLimit());
	xml.writeDoubleAttribute("UnbalanceLimit", unbalanceLimit());
	xml.writeDoubleAttribute("InputLowLimit", inputLowLimit());
	xml.writeDoubleAttribute("InputHighLimit", inputHighLimit());
	xml.writeIntAttribute("InputUnitID", inputUnitID());
	xml.writeIntAttribute("InputSensorID", inputSensorID());
	xml.writeDoubleAttribute("OutputLowLimit", outputLowLimit());
	xml.writeDoubleAttribute("OutputHighLimit", outputHighLimit());
	xml.writeIntAttribute("OutputUnitID", outputUnitID());
	xml.writeIntAttribute("OutputRangeMode", outputRangeModeInt());
	xml.writeIntAttribute("OutputSensorID", outputSensorID());
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeBoolAttribute("Calculated", calculated());
	xml.writeIntAttribute("NormalState", normalState());
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("Aperture", aperture());
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	xml.writeDoubleAttribute("FilteringTime", filteringTime());
	xml.writeDoubleAttribute("MaxDifference", maxDifference());
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

	QString str;

	result &= xml.readStringAttribute("Type", &str);

	if (str == "Analog")
	{
		m_type = E::SignalType::Analog;
	}
	else
	{
		m_type = E::SignalType::Discrete;
	}

	result &= xml.readStringAttribute("AppSignalID", &m_appSignalID);
	result &= xml.readStringAttribute("CustomAppSignalID", &m_customAppSignalID);
	result &= xml.readStringAttribute("Caption", &m_caption);
	result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);

	result &= xml.readIntAttribute("DataFormat", &intValue);
	m_dataFormat = static_cast<E::DataFormat>(intValue);

	result &= xml.readIntAttribute("DataSize", &m_dataSize);
	result &= xml.readIntAttribute("LowADC", &m_lowADC);
	result &= xml.readIntAttribute("HighADC", &m_highADC);
	result &= xml.readDoubleAttribute("LowLimit", &m_lowLimit);
	result &= xml.readDoubleAttribute("HighLimit", &m_highLimit);
	result &= xml.readIntAttribute("UnitID", &m_unitID);
	result &= xml.readDoubleAttribute("Adjustment", &m_adjustment);
	result &= xml.readDoubleAttribute("DropLimit", &m_dropLimit);
	result &= xml.readDoubleAttribute("ExcessLimit", &m_excessLimit);
	result &= xml.readDoubleAttribute("UnbalanceLimit", &m_unbalanceLimit);
	result &= xml.readDoubleAttribute("InputLowLimit", &m_inputLowLimit);
	result &= xml.readDoubleAttribute("InputHighLimit", &m_inputHighLimit);
	result &= xml.readIntAttribute("InputUnitID", &m_inputUnitID);
	result &= xml.readIntAttribute("InputSensorID", &m_inputSensorID);
	result &= xml.readDoubleAttribute("OutputLowLimit", &m_outputLowLimit);
	result &= xml.readDoubleAttribute("OutputHighLimit", &m_outputHighLimit);
	result &= xml.readIntAttribute("OutputUnitID", &m_outputUnitID);

	result &= xml.readIntAttribute("OutputRangeMode", &intValue);
	m_outputRangeMode = static_cast<E::OutputRangeMode>(intValue);

	result &= xml.readIntAttribute("OutputSensorID", &m_outputSensorID);
	result &= xml.readBoolAttribute("Acquire", &m_acquire);
	result &= xml.readBoolAttribute("Calculated", &m_calculated);
	result &= xml.readIntAttribute("NormalState", &m_normalState);
	result &= xml.readIntAttribute("DecimalPlaces", &m_decimalPlaces);
	result &= xml.readDoubleAttribute("Aperture", &m_aperture);

	result &= xml.readIntAttribute("InOutType", &intValue);
	m_inOutType = static_cast<E::SignalInOutType>(intValue);

	result &= xml.readDoubleAttribute("FilteringTime", &m_filteringTime);
	result &= xml.readDoubleAttribute("MaxDifference", &m_maxDifference);

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
