#include <QXmlStreamAttributes>
#include <QFile>
#include "../lib/Signal.h"
#include "../lib/DataSource.h"


DataFormatList::DataFormatList()
{
	auto enumValues = E::enumValues<E::AnalogAppSignalFormat>();

	for (auto v : enumValues)
	{
		append(v.first, v.second);
	}
}


// --------------------------------------------------------------------------------------------------------
//
// Signal class implementation
//
// --------------------------------------------------------------------------------------------------------

std::shared_ptr<UnitList> Signal::unitList = std::make_shared<UnitList>();

Signal::Signal()
{
}


Signal::~Signal()
{
}


Signal::Signal(const Hardware::DeviceSignal& deviceSignal)
{
	if (deviceSignal.isDiagSignal())
	{
		assert(false);
		return;
	}

	m_signalType = deviceSignal.type();

	assert(m_signalType == E::SignalType::Analog || m_signalType == E::SignalType::Discrete);

	if (m_signalType == E::SignalType::Analog)
	{
		m_analogSignalFormat = deviceSignal.appSignalDataFormat();

		assert(m_analogSignalFormat == E::AnalogAppSignalFormat::Float32 || m_analogSignalFormat == E::AnalogAppSignalFormat::SignedInt32);

		m_lowADC = deviceSignal.appSignalLowAdc();
		m_highADC = deviceSignal.appSignalHighAdc();

		m_lowEngeneeringUnits = deviceSignal.appSignalLowEngUnits();
		m_highEngeneeringUnits = deviceSignal.appSignalHighEngUnits();;
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

	setDataSize(m_signalType, m_analogSignalFormat);
}


Signal& Signal::operator =(const Signal& signal)
{
	m_ID = signal.m_ID;
	m_signalGroupID = signal.m_signalGroupID;
	m_signalInstanceID = signal.m_signalInstanceID;
	m_changesetID = signal.m_changesetID;
	m_checkedOut = signal.m_checkedOut;
	m_userID = signal.m_userID;
	m_channel = signal.m_channel;
	m_signalType = signal.m_signalType;
	m_created = signal.m_created;
	m_deleted = signal.m_deleted;
	m_instanceCreated = signal.m_instanceCreated;
	m_instanceAction = signal.m_instanceAction;

	m_appSignalID = signal.m_appSignalID;
	m_hash = signal.m_hash;

	m_customAppSignalID = signal.m_customAppSignalID;
	m_caption = signal.m_caption;
	m_analogSignalFormat = signal.m_analogSignalFormat;
	m_dataSize = signal.m_dataSize;
	m_lowADC = signal.m_lowADC;
	m_highADC = signal.m_highADC;
	m_lowEngeneeringUnits = signal.m_lowEngeneeringUnits;
	m_highEngeneeringUnits = signal.m_highEngeneeringUnits;
	m_unitID = signal.m_unitID;
	m_lowValidRange = signal.m_lowValidRange;
	m_highValidRange = signal.m_highValidRange;
	m_unbalanceLimit = signal.m_unbalanceLimit;
	m_inputLowLimit = signal.m_inputLowLimit;
	m_inputHighLimit = signal.m_inputHighLimit;
	m_inputUnitID = signal.m_inputUnitID;
	m_inputSensorType = signal.m_inputSensorType;
	m_outputLowLimit = signal.m_outputLowLimit;
	m_outputHighLimit = signal.m_outputHighLimit;
	m_outputUnitID = signal.m_outputUnitID;
	m_outputMode = signal.m_outputMode;
	m_outputSensorType = signal.m_outputSensorType;
	m_acquire = signal.m_acquire;
	m_calculated = signal.m_calculated;
	m_normalState = signal.m_normalState;
	m_decimalPlaces = signal.m_decimalPlaces;
	m_aperture = signal.m_aperture;
	m_inOutType = signal.m_inOutType;
	m_equipmentID = signal.m_equipmentID;
	m_filteringTime = signal.m_filteringTime;
	m_spreadTolerance = signal.m_spreadTolerance;
	m_byteOrder = signal.m_byteOrder;
	m_enableTuning = signal.m_enableTuning;
	m_tuningDefaultValue = signal.m_tuningDefaultValue;

	m_ramAddr = signal.m_ramAddr;
	m_regValueAddr = signal.m_regValueAddr;
	m_regValidityAddr = signal.m_regValidityAddr;
	m_tuningAddr = signal.m_tuningAddr;

	m_lm = signal.m_lm;

	return *this;
}


E::DataFormat Signal::dataFormat() const
{
	switch(m_signalType)
	{
	case E::SignalType::Discrete:
		return E::DataFormat::UnsignedInt;

	case E::SignalType::Analog:

		switch(m_analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			return E::DataFormat::Float;

		case E::AnalogAppSignalFormat::SignedInt32:
			return E::DataFormat::SignedInt;

		default:
			assert(false);
			return E::DataFormat::UnsignedInt;
		}

	default:
		assert(false);
		return E::DataFormat::UnsignedInt;
	}
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(E::InputUnit))
{
    const QStringRef& strValue = attr.value(fieldName);
    if (strValue.isEmpty())
    {
        assert(false);
        return;
    }
    for (int i = 0; i < unitInfo.count(); i++)
    {
        if (strValue == unitInfo[i])
        {
            (this->*setter)(static_cast<E::InputUnit>(i));
            return;
        }
    }
    assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SensorType))
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
            (this->*setter)(static_cast<E::SensorType>(i));
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(E::AnalogAppSignalFormat))
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
			(this->*setter)(static_cast<E::AnalogAppSignalFormat>(dataFormatInfo.keyAt(i)));
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
	serializeField(attr, "Type", &Signal::setSignalType);
	serializeField(attr, "StrID", &Signal::setAppSignalID);
	serializeField(attr, "ExtStrID", &Signal::setCustomAppSignalID);
	serializeField(attr, "Name", &Signal::setCaption);
	serializeField(attr, "DataFormat", dataFormatInfo, &Signal::setAnalogSignalFormat);
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
    serializeField(attr, "InputSensorID", &Signal::setInputSensorType);
	serializeField(attr, "OutputLowLimit", &Signal::setOutputLowLimit);
	serializeField(attr, "OutputHighLimit", &Signal::setOutputHighLimit);
	serializeField(attr, "OutputUnitID", unitInfo, &Signal::setOutputUnitID);
	serializeField(attr, "OutputMode", &Signal::setOutputMode);
    serializeField(attr, "OutputSensorID", &Signal::setOutputSensorType);
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


// for DeviceSignal.dataFormat conversion
//
/*
void Signal::setAnalogSignalFormat(E::DataFormat dataFormat)
{
	//	assert(dataFormat == E::DataFormat::Float || dataFormat == E::DataFormat::SignedInt);

	if (dataFormat == E::DataFormat::UnsignedInt)
	{
		dataFormat = E::DataFormat::SignedInt;
	}

	// values of corresponding members of enums E::AppSignalDataFormat and E::DataFormat are equal!
	//
	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(dataFormat);
}*/


void Signal::setDataSize(E::SignalType signalType, E::AnalogAppSignalFormat dataFormat)
{
	if (signalType == E::SignalType::Discrete)
	{
		m_dataSize = DISCRETE_SIZE;
	}
	else
	{
		if (signalType == E::SignalType::Analog)
		{
			switch(dataFormat)
			{
			case E::AnalogAppSignalFormat::Float32:
				m_dataSize = FLOAT32_SIZE;
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				m_dataSize = SIGNED_INT32_SIZE;
				break;

			default:
				assert(false);		// unknown data format
			}
		}
		else
		{
			assert(false);			// unknown signal type
		}
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
	m_strID2IndexMap.clear();
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
	xml.writeIntAttribute("Type", signalTypeInt());
	xml.writeStringAttribute("AppSignalID", appSignalID());
	xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
	xml.writeStringAttribute("Caption", caption());
	xml.writeStringAttribute("EquipmentID", equipmentID());
	xml.writeIntAttribute("DataFormat", analogSignalFormatInt());
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
	xml.writeIntAttribute("InputSensorID", inputSensorType());
	xml.writeDoubleAttribute("OutputLowLimit", outputLowLimit());
	xml.writeDoubleAttribute("OutputHighLimit", outputHighLimit());
	xml.writeIntAttribute("OutputUnitID", outputUnitID());
	xml.writeIntAttribute("OutputMode", outputModeInt());
	xml.writeIntAttribute("OutputSensorID", outputSensorType());
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
	m_signalType = static_cast<E::SignalType>(type);

	result &= xml.readStringAttribute("AppSignalID", &m_appSignalID);
	result &= xml.readStringAttribute("CustomAppSignalID", &m_customAppSignalID);
	result &= xml.readStringAttribute("Caption", &m_caption);
	result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);

	result &= xml.readIntAttribute("DataFormat", &intValue);
	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(intValue);

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

    result &= xml.readIntAttribute("InputUnitID", &intValue);
    m_inputUnitID = static_cast<E::InputUnit>(intValue);

    result &= xml.readIntAttribute("InputSensorID", &intValue);
    m_inputSensorType = static_cast<E::SensorType>(intValue);

	result &= xml.readDoubleAttribute("OutputLowLimit", &m_outputLowLimit);
	result &= xml.readDoubleAttribute("OutputHighLimit", &m_outputHighLimit);
	result &= xml.readIntAttribute("OutputUnitID", &m_outputUnitID);

	result &= xml.readIntAttribute("OutputMode", &intValue);
	m_outputMode = static_cast<E::OutputMode>(intValue);

    result &= xml.readIntAttribute("OutputSensorID", &intValue);
    m_outputSensorType = static_cast<E::SensorType>(intValue);

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
	s->set_type(m_signalType);
	s->set_created(m_created.toMSecsSinceEpoch());
	s->set_deleted(m_deleted);
	s->set_instancecreated(m_instanceCreated.toMSecsSinceEpoch());
	s->set_instanceaction(m_instanceAction.toInt());
	s->set_appsignalid(m_appSignalID.toStdString());
	s->set_customappsignalid(m_customAppSignalID.toStdString());
	s->set_caption(m_caption.toStdString());
	s->set_dataformat(static_cast<::google::protobuf::int32>(m_analogSignalFormat));
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
	s->set_inputsensorid(m_inputSensorType);
	s->set_outputlowlimit(m_outputLowLimit);
	s->set_outputhighlimit(m_outputHighLimit);
	s->set_outputunitid(m_outputUnitID);
	s->set_outputmode(m_outputMode);
	s->set_outputsensorid(m_outputSensorType);
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
		m_signalType = static_cast<E::SignalType>(s->type());
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
		m_instanceAction = static_cast<VcsItemAction::VcsItemActionType>(s->instanceaction());
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
		m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(s->dataformat());
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
        m_inputUnitID = static_cast<E::InputUnit>(s->inputunitid());
	}

	if (s->has_inputsensorid())
	{
        m_inputSensorType = static_cast<E::SensorType>(s->inputsensorid());
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
        m_outputSensorType = static_cast<E::SensorType>(s->outputsensorid());
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

void Signal::serializeToProtoAppSignalParam(Proto::AppSignalParam* message) const
{
	if (message == nullptr)
	{
		assert(message);
		return;
	}

	message->set_hash(calcHash(m_appSignalID));
	message->set_appsignalid(m_appSignalID.toStdString());
	message->set_customsignalid(m_customAppSignalID.toStdString());
	message->set_caption(m_caption.toStdString());
	message->set_equipmentid(m_equipmentID.toStdString());

	message->set_channel(static_cast<int>(m_channel));
	message->set_inouttype(static_cast<int>(m_inOutType));
	message->set_signaltype(m_signalType);
	message->set_analogsignalformat(static_cast<int>(m_analogSignalFormat));
	message->set_byteorder(m_byteOrder);

	message->set_unitid(m_unitID);

	if (unitList != nullptr && unitList->contains(m_unitID))
	{
		message->set_unit(unitList->value(m_unitID).toStdString());
	}
	else
	{
		message->set_unit("???");
	}

	message->set_lowvalidrange(m_lowValidRange);
	message->set_highvalidrange(m_highValidRange);
	message->set_lowengeneeringunits(m_lowEngeneeringUnits);
	message->set_highengeneeringunits(m_highEngeneeringUnits);

	message->set_inputlowlimit(m_inputLowLimit);
	message->set_inputhighlimit(m_inputHighLimit);
	message->set_inputunitid(m_inputUnitID);
	message->set_inputsensortype(m_inputSensorType);

	message->set_outputlowlimit(m_outputLowLimit);
	message->set_outputhighlimit(m_outputHighLimit);
	message->set_outputunitid(m_outputUnitID);
	message->set_outputmode(m_outputMode);
	message->set_outputsensortype(m_outputSensorType);

	message->set_precision(m_decimalPlaces);
	message->set_aperture(m_aperture);
	message->set_filteringtime(m_filteringTime);
	message->set_spreadtolerance(m_spreadTolerance);
	message->set_enabletuning(m_enableTuning);
	message->set_tuningdefaultvalue(m_tuningDefaultValue);

	return;
}


bool Signal::isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const
{
	if (m_signalType != signalType)
	{
		return false;
	}

	if (m_byteOrder != byteOrder)
	{
		return false;
	}

	if (m_signalType == E::SignalType::Analog)
	{
		if (m_analogSignalFormat == E::AnalogAppSignalFormat::Float32 &&
			(dataFormat == E::DataFormat::Float && size == FLOAT32_SIZE))
		{
			return true;
		}

		if (m_analogSignalFormat == E::AnalogAppSignalFormat::SignedInt32 &&
			(dataFormat == E::DataFormat::SignedInt && size == SIGNED_INT32_SIZE))
		{
			return true;
		}

		return false;
	}

	if (m_signalType == E::SignalType::Discrete)
	{
		if (size == DISCRETE_SIZE)
		{
			return true;
		}

		return false;
	}

	assert(false);
	return false;
}


bool Signal::isCompatibleFormat(const SignalAddress16& sa16) const
{
	return isCompatibleFormat(sa16.signalType(), sa16.dataFormat(), sa16.dataSize(), sa16.byteOrder());
}



QString Signal::regValueAddrStr() const
{
	return QString("(reg %1:%2)").arg(regValueAddr().offset()).arg(regValueAddr().bit());
}


// --------------------------------------------------------------------------------------------------------
//
// SignalSet class implementation
//
// --------------------------------------------------------------------------------------------------------

void SignalSet::buildID2IndexMap()
{
	m_strID2IndexMap.clear();

	int signalCount = count();

	if (signalCount == 0)
	{
		return;
	}

	m_strID2IndexMap.reserve(static_cast<int>(signalCount * 1.3));

	for(int i = 0; i < signalCount; i++)
	{
		Signal& s = (*this)[i];

		if (m_strID2IndexMap.contains(s.appSignalID()) == true)
		{
			assert(false && "There are at least two signals with same AppSignalID");
		}
		else
		{
			m_strID2IndexMap.insert(s.appSignalID(), i);
		}
	}
}

bool SignalSet::ID2IndexMapIsEmpty()
{
	return m_strID2IndexMap.isEmpty();
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

	int index = m_strID2IndexMap.value(appSignalID, -1);

	if (index == -1)
	{
		return nullptr;
	}

	return &(*this)[index];
}
