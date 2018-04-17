#include <QXmlStreamAttributes>
#include <QFile>
#include <utility>

#include "Signal.h"
#include "DataSource.h"
#include "WUtils.h"

#include "../Proto/serialization.pb.h"

// --------------------------------------------------------------------------------------------------------
//
// Signal class implementation
//
// --------------------------------------------------------------------------------------------------------

QString Signal::BUS_SIGNAL_ID_SEPARATOR(".");

QString Signal::BUS_SIGNAL_MACRO_BUSTYPEID("$(BUSTYPEID)");
QString Signal::BUS_SIGNAL_MACRO_BUSID("$(BUSID)");
QString Signal::BUS_SIGNAL_MACRO_BUSSIGNALID("$(BUSSIGNALID)");
QString Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION("$(BUSSIGNALCAPTION)");


Signal::Signal()
{
	updateTuningValuesType();
}

Signal::Signal(const Signal& s)
{
	*this = s;
}

Signal::Signal(const Hardware::DeviceSignal& deviceSignal)
{
	if (deviceSignal.isDiagSignal() == true)
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

	updateTuningValuesType();

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

	// specific properties processing
	//
	m_specPropsStruct = deviceSignal.signalSpecPropsStruc();

	createSpecProps();
}

Signal::~Signal()
{
}

void Signal::setSignalType(E::SignalType type)
{
	m_signalType = type;
	updateTuningValuesType();
}

void Signal::setDataSize(E::SignalType signalType, E::AnalogAppSignalFormat dataFormat)
{
	switch(signalType)
	{
	case E::SignalType::Discrete:
		m_dataSize = DISCRETE_SIZE;
		break;

	case E::SignalType::Analog:

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

		break;

	default:
		assert(false);		// unknown signal type
	}
}

void Signal::setDataSizeW(int sizeW)
{
	m_dataSize = sizeW * SIZE_16BIT;
}

void Signal::setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat)
{
	m_analogSignalFormat = dataFormat;

	updateTuningValuesType();
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

		break;

	default:
		assert(false);
		return E::DataFormat::UnsignedInt;
	}
}

bool Signal::isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const
{
	if (signalType == E::SignalType::Bus)
	{
		assert(false);			// use isCompatibleFormat(signalType, busTtypeID)
		return false;
	}

	return isCompatibleFormatPrivate(signalType, dataFormat, size, byteOrder, "");
}

bool Signal::isCompatibleFormat(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, E::ByteOrder byteOrder) const
{
	switch(signalType)
	{
	case E::SignalType::Analog:

		switch(analogFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			return isCompatibleFormatPrivate(signalType, E::DataFormat::Float, FLOAT32_SIZE, byteOrder, "");

		case E::AnalogAppSignalFormat::SignedInt32:
			return isCompatibleFormatPrivate(signalType, E::DataFormat::SignedInt, SIGNED_INT32_SIZE, byteOrder, "");

		default:
			assert(false);
		}
		break;

	case E::SignalType::Discrete:
		return isCompatibleFormatPrivate(signalType, E::DataFormat::UnsignedInt, DISCRETE_SIZE, byteOrder, "");

	default:
		assert(false);
	}

	return false;
}

bool Signal::isCompatibleFormat(const SignalAddress16& sa16) const
{
	return isCompatibleFormatPrivate(sa16.signalType(), sa16.dataFormat(), sa16.dataSize(), sa16.byteOrder(), "");
}

bool Signal::isCompatibleFormat(const Signal& s) const
{
	if (s.signalType() == E::SignalType::Bus)
	{
		return isCompatibleFormat(E::SignalType::Bus, s.busTypeID());
	}

	return isCompatibleFormat(s.signalType(), s.analogSignalFormat(), s.byteOrder());
}

bool Signal::isCompatibleFormat(E::SignalType signalType, const QString& busTypeID) const
{
	if (signalType != E::SignalType::Bus)
	{
		assert(false);		// use other isCompatibelFormat functions
		return false;
	}

	return isCompatibleFormatPrivate(signalType,
									 E::DataFormat::UnsignedInt,		// param is not checked for Bus signals
									 SIZE_1BIT,							// param is not checked for Bus signals
									 E::BigEndian,						// param is not checked for Bus signals
									 busTypeID);
}


bool Signal::setSpecPropsStruct(const QString& specPropsStruct, bool updateExistsValues)
{
	m_specPropsStruct = specPropsStruct;

	if (updateExistsValues == true)
	{
		return updateSpecProps();
	}

	return createSpecProps();
}

void Signal::resetAddresses()
{
	m_ioBufAddr.reset();
	m_tuningAddr.reset();
	m_ualAddr.reset();
	m_regValueAddr.reset();
	m_regValidityAddr.reset();
}

QString Signal::regValueAddrStr() const
{
	return QString("(reg %1:%2)").arg(regValueAddr().offset()).arg(regValueAddr().bit());
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ElectricUnit))
{
	const QStringRef& strValue = attr.value(fieldName);

	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::ElectricUnit>(QString(strValue.constData()), nullptr));
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SensorType))
{
	const QStringRef& strValue = attr.value(fieldName);

	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::SensorType>(QString(strValue.constData()), nullptr));
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType))
{
	const QStringRef& strValue = attr.value(fieldName);

	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::SignalInOutType>(QString(strValue.constData()), nullptr));
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::AnalogAppSignalFormat))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::AnalogAppSignalFormat>(QString(strValue.constData()), nullptr));
}

void Signal::serializeFields(const QXmlStreamAttributes& attr)
{
	serializeField(attr, "ID", &Signal::setID);
	serializeField(attr, "SignalGroupID", &Signal::setSignalGroupID);
	serializeField(attr, "SignalInstanceID", &Signal::setSignalInstanceID);
//	serializeField(attr, "Channel", &Signal::setChannel);
	serializeField(attr, "Type", &Signal::setSignalType);
	serializeField(attr, "StrID", &Signal::setAppSignalID);
	serializeField(attr, "ExtStrID", &Signal::setCustomAppSignalID);
	serializeField(attr, "Name", &Signal::setCaption);
	serializeField(attr, "DataFormat", &Signal::setAnalogSignalFormat);
	serializeField(attr, "DataSize", &Signal::setDataSize);
	serializeField(attr, "LowADC", &Signal::setLowADC);
	serializeField(attr, "HighADC", &Signal::setHighADC);
	serializeField(attr, "LowEngeneeringUnits", &Signal::setLowEngeneeringUnits);
	serializeField(attr, "HighEngeneeringUnits", &Signal::setHighEngeneeringUnits);
	serializeField(attr, "Unit", &Signal::setUnit);
	serializeField(attr, "LowValidRange", &Signal::setLowValidRange);
	serializeField(attr, "HighValidRange", &Signal::setHighValidRange);
	serializeField(attr, "ElectricLowLimit", &Signal::setElectricLowLimit);
	serializeField(attr, "ElectricHighLimit", &Signal::setElectricHighLimit);
	serializeField(attr, "ElectricUnit", &Signal::setElectricUnit);
	serializeField(attr, "SensorType", &Signal::setSensorType);
	serializeField(attr, "OutputMode", &Signal::setOutputMode);
	serializeField(attr, "Acquire", &Signal::setAcquire);
	serializeField(attr, "DecimalPlaces", &Signal::setDecimalPlaces);
	serializeField(attr, "CoarseAperture", &Signal::setCoarseAperture);
	serializeField(attr, "FineAperture", &Signal::setFineAperture);
	serializeField(attr, "InOutType", &Signal::setInOutType);
	serializeField(attr, "DeviceStrID", &Signal::setEquipmentID);
	serializeField(attr, "FilteringTime", &Signal::setFilteringTime);
	serializeField(attr, "SpreadTolerance", &Signal::setSpreadTolerance);
	serializeField(attr, "ByteOrder", &Signal::setByteOrder);
	serializeField(attr, "RamAddr", &Signal::setUalAddr);
	serializeField(attr, "RegAddr", &Signal::setRegValueAddr);
}

void Signal::writeToXml(XmlWriteHelper& xml)
{
/*	xml.writeStartElement("Signal");	// <Signal>

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
	xml.writeStringAttribute("Unit", unit());
	xml.writeDoubleAttribute("LowValidRange", lowValidRange());
	xml.writeDoubleAttribute("HighValidRange", highValidRange());
	xml.writeDoubleAttribute("ElectricLowLimit", electricLowLimit());
	xml.writeDoubleAttribute("ElectricHighLimit", electricHighLimit());
	xml.writeIntAttribute("ElectricUnit", electricUnit());
	xml.writeIntAttribute("SensorType", sensorType());
	xml.writeIntAttribute("OutputMode", outputModeInt());
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("CoarseAperture", coarseAperture());
	xml.writeDoubleAttribute("FineAperture", fineAperture());
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	xml.writeDoubleAttribute("FilteringTime", filteringTime());
	xml.writeDoubleAttribute("SpreadTolerance", spreadTolerance());
	xml.writeIntAttribute("ByteOrder", byteOrderInt());

	xml.writeBoolAttribute("EnableTuning", enableTuning());
	xml.writeFloatAttribute("TuningDefaultValue", tuningDefaultValue());
	xml.writeFloatAttribute("TuningLowBound", tuningLowBound());
	xml.writeFloatAttribute("TuningHighBound", tuningHighBound());

	xml.writeStringAttribute("BusTypeID", busTypeID());
	xml.writeBoolAttribute("AdaptiveAperture", adaptiveAperture());

	xml.writeIntAttribute("RamAddrOffset", ualAddr().offset());
	xml.writeIntAttribute("RamAddrBit", ualAddr().bit());
	xml.writeIntAttribute("ValueOffset", regValueAddr().offset());
	xml.writeIntAttribute("ValueBit", regValueAddr().bit());
	xml.writeIntAttribute("ValidityOffset", regValidityAddr().offset());
	xml.writeIntAttribute("ValidityBit", regValidityAddr().bit());

	xml.writeIntAttribute("TuningOffset", tuningAddr().offset());
	xml.writeIntAttribute("TuningBit", tuningAddr().bit());

	xml.writeEndElement();				// </Signal>*/


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
	xml.writeIntAttribute("UnitID", 0);
	xml.writeDoubleAttribute("LowValidRange", lowValidRange());
	xml.writeDoubleAttribute("HighValidRange", highValidRange());
	xml.writeDoubleAttribute("UnbalanceLimit", 1);
	xml.writeDoubleAttribute("InputLowLimit", electricLowLimit());
	xml.writeDoubleAttribute("InputHighLimit", electricHighLimit());
	xml.writeIntAttribute("InputUnitID", electricUnitInt());
	xml.writeIntAttribute("InputSensorID", sensorTypeInt());
	xml.writeDoubleAttribute("OutputLowLimit", electricLowLimit());
	xml.writeDoubleAttribute("OutputHighLimit", electricHighLimit());
	xml.writeIntAttribute("OutputUnitID", electricUnitInt());
	xml.writeIntAttribute("OutputMode", outputModeInt());
	xml.writeIntAttribute("OutputSensorID", sensorTypeInt());
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeBoolAttribute("Calculated", false);
	xml.writeIntAttribute("NormalState", 0);
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("Aperture", coarseAperture());
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	xml.writeDoubleAttribute("FilteringTime", filteringTime());
	xml.writeDoubleAttribute("SpreadTolerance", spreadTolerance());
	xml.writeIntAttribute("ByteOrder", byteOrderInt());

	xml.writeBoolAttribute("EnableTuning", enableTuning());
	xml.writeFloatAttribute("TuningDefaultValue", tuningDefaultValue().toFloat());
	xml.writeFloatAttribute("TuningLowBound", tuningLowBound().toFloat());
	xml.writeFloatAttribute("TuningHighBound", tuningHighBound().toFloat());

	xml.writeStringAttribute("BusTypeID", busTypeID());
	xml.writeBoolAttribute("AdaptiveAperture", adaptiveAperture());

	xml.writeIntAttribute("RamAddrOffset", ualAddr().offset());
	xml.writeIntAttribute("RamAddrBit", ualAddr().bit());
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
/*	bool result = true;

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
	result &= xml.readStringAttribute("Unit", &m_unit);
	result &= xml.readDoubleAttribute("LowValidRange", &m_lowValidRange);
	result &= xml.readDoubleAttribute("HighValidRange", &m_highValidRange);
	result &= xml.readDoubleAttribute("ElectricLowLimit", &m_electricLowLimit);
	result &= xml.readDoubleAttribute("ElectricHighLimit", &m_electricHighLimit);

	result &= xml.readIntAttribute("ElectricUnit", &intValue);
	m_electricUnit = static_cast<E::ElectricUnit>(intValue);

	result &= xml.readIntAttribute("SensorType", &intValue);
	m_sensorType = static_cast<E::SensorType>(intValue);

	result &= xml.readIntAttribute("OutputMode", &intValue);
	m_outputMode = static_cast<E::OutputMode>(intValue);

	result &= xml.readBoolAttribute("Acquire", &m_acquire);
	result &= xml.readIntAttribute("DecimalPlaces", &m_decimalPlaces);
	result &= xml.readDoubleAttribute("CoarseAperture", &m_coarseAperture);
	result &= xml.readDoubleAttribute("FineAperture", &m_fineAperture);

	result &= xml.readIntAttribute("InOutType", &intValue);
	m_inOutType = static_cast<E::SignalInOutType>(intValue);

	result &= xml.readDoubleAttribute("FilteringTime", &m_filteringTime);
	result &= xml.readDoubleAttribute("SpreadTolerance", &m_spreadTolerance);

	result &= xml.readIntAttribute("ByteOrder", &intValue);
	m_byteOrder = static_cast<E::ByteOrder>(intValue);

	result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);
	result &= xml.readFloatAttribute("TuningDefaultValue", &m_tuningDefaultValue);
	result &= xml.readFloatAttribute("TuningLowBound", &m_tuningLowBound);
	result &= xml.readFloatAttribute("TuningHighBound", &m_tuningHighBound);

	result &= xml.readStringAttribute("BusTypeID", &m_busTypeID);
	result &= xml.readBoolAttribute("AdaptiveAperture", &m_adaptiveAperture);

	int offset = 0;
	int bit = 0;

	result &= xml.readIntAttribute("RamAddrOffset", &offset);
	result &= xml.readIntAttribute("RamAddrBit", &bit);

	m_ualAddr.setOffset(offset);
	m_ualAddr.setBit(bit);

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
	m_tuningAddr.setBit(bit);*/

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

	result &= xml.readIntAttribute("UnitID", &intValue);

	result &= xml.readDoubleAttribute("LowValidRange", &m_lowValidRange);
	result &= xml.readDoubleAttribute("HighValidRange", &m_highValidRange);

	double unbalanceLimit = 0;
	result &= xml.readDoubleAttribute("UnbalanceLimit", &unbalanceLimit);

	result &= xml.readDoubleAttribute("InputLowLimit", &m_electricLowLimit);
	result &= xml.readDoubleAttribute("InputHighLimit", &m_electricHighLimit);

	result &= xml.readIntAttribute("InputUnitID", &intValue);

	result &= xml.readIntAttribute("InputSensorID", &intValue);
	m_sensorType = static_cast<E::SensorType>(intValue);

	result &= xml.readDoubleAttribute("OutputLowLimit", &m_electricLowLimit);
	result &= xml.readDoubleAttribute("OutputHighLimit", &m_electricHighLimit);
	result &= xml.readIntAttribute("OutputUnitID", &intValue);
	m_electricUnit = static_cast<E::ElectricUnit>(intValue);

	result &= xml.readIntAttribute("OutputMode", &intValue);
	m_outputMode = static_cast<E::OutputMode>(intValue);

	result &= xml.readIntAttribute("OutputSensorID", &intValue);
	m_sensorType = static_cast<E::SensorType>(intValue);

	result &= xml.readBoolAttribute("Acquire", &m_acquire);

	bool boolValue = false;
	result &= xml.readBoolAttribute("Calculated", &boolValue);
	result &= xml.readIntAttribute("NormalState", &intValue);
	result &= xml.readIntAttribute("DecimalPlaces", &m_decimalPlaces);
	result &= xml.readDoubleAttribute("Aperture", &m_coarseAperture);
	m_fineAperture = m_coarseAperture;

	result &= xml.readIntAttribute("InOutType", &intValue);
	m_inOutType = static_cast<E::SignalInOutType>(intValue);

	result &= xml.readDoubleAttribute("FilteringTime", &m_filteringTime);
	result &= xml.readDoubleAttribute("SpreadTolerance", &m_spreadTolerance);

	result &= xml.readIntAttribute("ByteOrder", &intValue);
	m_byteOrder = static_cast<E::ByteOrder>(intValue);

	result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);

	updateTuningValuesType();

	float value = 0;

	result &= xml.readFloatAttribute("TuningDefaultValue", &value);
	m_tuningDefaultValue.fromFloat(value);

	result &= xml.readFloatAttribute("TuningLowBound", &value);
	m_tuningLowBound.fromFloat(value);

	result &= xml.readFloatAttribute("TuningHighBound", &value);
	m_tuningHighBound.fromFloat(value);

	result &= xml.readStringAttribute("BusTypeID", &m_busTypeID);
	result &= xml.readBoolAttribute("AdaptiveAperture", &m_adaptiveAperture);

	int offset = 0;
	int bit = 0;

	result &= xml.readIntAttribute("RamAddrOffset", &offset);
	result &= xml.readIntAttribute("RamAddrBit", &bit);

	m_ualAddr.setOffset(offset);
	m_ualAddr.setBit(bit);

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


	return result;
}

void Signal::serializeTo(Proto::AppSignal* s) const
{
	if (s == nullptr)
	{
		assert(false);
		return;
	}

	// Signal identificators

	s->set_appsignalid(m_appSignalID.toStdString());
	s->set_customappsignalid(m_customAppSignalID.toStdString());
	s->set_caption(m_caption.toStdString());
	s->set_equipmentid(m_equipmentID.toStdString());
	s->set_bustypeid(m_busTypeID.toStdString());
	s->set_channel(TO_INT(m_channel));

	// Signal type

	s->set_signaltype(TO_INT(m_signalType));
	s->set_inouttype(TO_INT(m_inOutType));

	// Signal format

	s->set_datasize(m_dataSize);
	s->set_byteorder(TO_INT(m_byteOrder));

	// Analog signal properties

	s->set_analogsignalformat(TO_INT(m_analogSignalFormat));
	s->set_unit(m_unit.toStdString());
	s->set_lowadc(m_lowADC);
	s->set_highadc(m_highADC);
	s->set_lowengeneeringunits(m_lowEngeneeringUnits);
	s->set_highengeneeringunits(m_highEngeneeringUnits);
	s->set_lowvalidrange(m_lowValidRange);
	s->set_highvalidrange(m_highValidRange);
	s->set_filteringtime(m_filteringTime);
	s->set_spreadtolerance(m_spreadTolerance);

	// Analog input/output signal properties

	s->set_electriclowlimit(m_electricLowLimit);
	s->set_electrichighlimit(m_electricHighLimit);
	s->set_electricunit(m_electricUnit);
	s->set_sensortype(m_sensorType);
	s->set_outputmode(m_outputMode);

	// Tuning signal properties

	s->set_enabletuning(m_enableTuning);
	m_tuningDefaultValue.save(s->mutable_tuningdefaultvalue());
	m_tuningLowBound.save(s->mutable_tuninglowbound());
	m_tuningHighBound.save(s->mutable_tuninghighbound());

	// Signal properties for MATS

	s->set_acquire(m_acquire);
	s->set_archive(m_archive);
	s->set_decimalplaces(m_decimalPlaces);
	s->set_coarseaperture(m_coarseAperture);
	s->set_fineaperture(m_fineAperture);
	s->set_adaptiveaperture(m_adaptiveAperture);

	// Signal fields from database

	Proto::AppSignalDbField* dbField = s->mutable_dbfield();

	if (dbField != nullptr)
	{
		dbField->set_id(m_ID);
		dbField->set_signalgroupid(m_signalGroupID);
		dbField->set_signalinstanceid(m_signalInstanceID);
		dbField->set_changesetid(m_changesetID);
		dbField->set_checkedout(m_checkedOut);
		dbField->set_userid(m_userID);
		dbField->set_created(m_created.toMSecsSinceEpoch());
		dbField->set_deleted(m_deleted);
		dbField->set_instancecreated(m_instanceCreated.toMSecsSinceEpoch());
		dbField->set_instanceaction(m_instanceAction.toInt());
	}
	else
	{
		assert(false);
	}

	// Signal properties calculated in compile-time

	Proto::AppSignalCalculatedParam* calcParam = s->mutable_calcparam();

	if (calcParam != nullptr)
	{
		calcParam->set_hash(calcHash(m_appSignalID));

		Proto::Address16* addr = nullptr;

		if (m_ioBufAddr.isValid() == true)
		{
			addr = calcParam->mutable_iobufaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_ioBufAddr.offset());
				addr->set_bit(m_ioBufAddr.bit());
			}
			else
			{
				assert(false);
			}
		}

		if (m_tuningAddr.isValid() == true)
		{
			addr = calcParam->mutable_tuningaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_tuningAddr.offset());
				addr->set_bit(m_tuningAddr.bit());
			}
			else
			{
				assert(false);
			}
		}

		if (m_ualAddr.isValid() == true)
		{
			addr = calcParam->mutable_ualaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_ualAddr.offset());
				addr->set_bit(m_ualAddr.bit());
			}
			else
			{
				assert(false);
			}
		}

		if (m_regBufAddr.isValid() == true)
		{
			addr = calcParam->mutable_regbufaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_regBufAddr.offset());
				addr->set_bit(m_regBufAddr.bit());
			}
			else
			{
				assert(false);
			}
		}

		if (m_regValueAddr.isValid() == true)
		{
			addr = calcParam->mutable_regvalueaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_regValueAddr.offset());
				addr->set_bit(m_regValueAddr.bit());
			}
			else
			{
				assert(false);
			}
		}

		if (m_regValidityAddr.isValid() == true)
		{
			addr = calcParam->mutable_regvalidityaddr();

			if (addr != nullptr)
			{
				addr->set_offset(m_regValidityAddr.offset());
				addr->set_bit(m_regValidityAddr.bit());
			}
			else
			{
				assert(false);
			}
		}
	}
	else
	{
		assert(false);
	}
}

void Signal::serializeFrom(const Proto::AppSignal& s)
{
	// Signal identificators

	m_appSignalID = QString::fromStdString(s.appsignalid());
	m_customAppSignalID = QString::fromStdString(s.customappsignalid());
	m_caption = QString::fromStdString(s.caption());
	m_equipmentID = QString::fromStdString(s.equipmentid());
	m_busTypeID = QString::fromStdString(s.bustypeid());
	m_channel = static_cast<E::Channel>(s.channel());

	// Signal type

	m_signalType = static_cast<E::SignalType>(s.signaltype());
	m_inOutType = static_cast<E::SignalInOutType>(s.inouttype());

	// Signal format

	m_dataSize = s.datasize();
	m_byteOrder = static_cast<E::ByteOrder>(s.byteorder());

	// Analog signal properties

	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(s.analogsignalformat());
	m_unit = QString::fromStdString(s.unit());
	m_lowADC = s.lowadc();
	m_highADC = s.highadc();
	m_lowEngeneeringUnits = s.lowengeneeringunits();
	m_highEngeneeringUnits = s.highengeneeringunits();
	m_lowValidRange = s.lowvalidrange();
	m_highValidRange = s.highvalidrange();
	m_filteringTime = s.filteringtime();
	m_spreadTolerance = s.spreadtolerance();

	// Analog input/output signal properties

	m_electricLowLimit = s.electriclowlimit();
	m_electricHighLimit = s.electrichighlimit();
	m_electricUnit = static_cast<E::ElectricUnit>(s.electricunit());
	m_sensorType = static_cast<E::SensorType>(s.sensortype());
	m_outputMode = static_cast<E::OutputMode>(s.outputmode());

	// Tuning signal properties

	m_enableTuning = s.enabletuning();
	m_tuningDefaultValue.load(s.tuningdefaultvalue());
	m_tuningLowBound.load(s.tuninglowbound());
	m_tuningHighBound.load(s.tuninghighbound());

	//	Signal properties for MATS

	m_acquire = s.acquire();
	m_archive = s.archive();
	m_decimalPlaces = s.decimalplaces();
	m_coarseAperture = s.coarseaperture();
	m_fineAperture = s.fineaperture();
	m_adaptiveAperture = s.adaptiveaperture();

	// Signal fields from database

	const Proto::AppSignalDbField& dbFiled = s.dbfield();

	m_ID = dbFiled.id();
	m_signalGroupID = dbFiled.signalgroupid();
	m_signalInstanceID = dbFiled.signalinstanceid();
	m_changesetID = dbFiled.changesetid();
	m_checkedOut = dbFiled.checkedout();
	m_userID = dbFiled.userid();
	m_created.setMSecsSinceEpoch(dbFiled.created());
	m_deleted = dbFiled.deleted();
	m_instanceCreated.setMSecsSinceEpoch(dbFiled.instancecreated());
	m_instanceAction = static_cast<VcsItemAction::VcsItemActionType>(dbFiled.instanceaction());

	// Signal properties calculated in compile-time

	const Proto::AppSignalCalculatedParam& calcParam = s.calcparam();

	m_hash = calcParam.hash();

	m_ioBufAddr.setOffset(calcParam.iobufaddr().offset());
	m_ioBufAddr.setBit(calcParam.iobufaddr().bit());

	m_tuningAddr.setOffset(calcParam.tuningaddr().offset());
	m_tuningAddr.setBit(calcParam.tuningaddr().bit());

	m_ualAddr.setOffset(calcParam.ualaddr().offset());
	m_ualAddr.setBit(calcParam.ualaddr().bit());

	m_regBufAddr.setOffset(calcParam.regbufaddr().offset());
	m_regBufAddr.setBit(calcParam.regbufaddr().bit());

	m_regValueAddr.setOffset(calcParam.regvalueaddr().offset());
	m_regValueAddr.setBit(calcParam.regvalueaddr().bit());

	m_regValidityAddr.setOffset(calcParam.regvalidityaddr().offset());
	m_regValidityAddr.setBit(calcParam.regvalidityaddr().bit());
}

void Signal::initCalculatedProperties()
{
	m_hash = calcHash(m_appSignalID);
}

bool Signal::isCompatibleFormatPrivate(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder, const QString& busTypeID) const
{
	if (m_signalType != signalType)
	{
		return false;
	}

	switch(m_signalType)
	{
	case E::SignalType::Analog:
			if (m_byteOrder != byteOrder)
			{
				return false;
			}

			switch(m_analogSignalFormat)
			{
			case E::AnalogAppSignalFormat::Float32:
				return (dataFormat == E::DataFormat::Float && size == FLOAT32_SIZE);

			case E::AnalogAppSignalFormat::SignedInt32:
				return (dataFormat == E::DataFormat::SignedInt && size == SIGNED_INT32_SIZE);

			default:
				assert(false);
			}

			return false;

	case E::SignalType::Discrete:
		return size == DISCRETE_SIZE;

	case E::SignalType::Bus:
		return m_busTypeID == busTypeID;
	}

	assert(false);
	return false;
}


void Signal::updateTuningValuesType()
{
	TuningValueType tvType = TuningValue::getTuningValueType(m_signalType, m_analogSignalFormat);

	m_tuningDefaultValue.setType(tvType);
	m_tuningLowBound.setType(tvType);
	m_tuningHighBound.setType(tvType);
}

bool Signal::createSpecProps()
{
	m_specPropsValues.clear();

	if (m_specPropsStruct.isEmpty() == true)
	{
		return true;
	}

	PropertyObject pob;

	std::pair<bool, QString> result = pob.parseSpecificPropertiesStruct(m_specPropsStruct);

	if (result.first == false)
	{
		assert(false);
		return false;
	}

	std::vector<std::shared_ptr<Property>> properties = pob.properties();

	::Proto::PropertyValues propValues;

	for(std::shared_ptr<Property> property : properties)
	{
		::Proto::Property* protoProperty = propValues.add_propertyvalue();

		::Proto::saveProperty(protoProperty, property);
	}

	int size = propValues.ByteSize();

	m_specPropsValues.resize(size);

	propValues.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(m_specPropsValues.data()));

	return true;
}


// --------------------------------------------------------------------------------------------------------
//
// SignalSet class implementation
//
// --------------------------------------------------------------------------------------------------------

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

void SignalSet::reserve(int n)
{
	SignalPtrOrderedHash::reserve(n);
	m_groupSignals.reserve(n);
}

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

bool SignalSet::contains(const QString& appSignalID) const
{
	if (count() > 0 && m_strID2IndexMap.isEmpty() == true)
	{
		assert(false);		//call buildStrID2IndexMap() before
		return false;
	}

	return m_strID2IndexMap.contains(appSignalID.trimmed());
}

Signal* SignalSet::getSignal(const QString& appSignalID)
{
	if (count() > 0 && m_strID2IndexMap.isEmpty() == true)
	{
		assert(false);		//	call buildStrID2IndexMap() before
		return nullptr;
	}

	int index = m_strID2IndexMap.value(appSignalID.trimmed(), -1);

	if (index == -1)
	{
		return nullptr;
	}

	return &(*this)[index];
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


QVector<int> SignalSet::getChannelSignalsID(const Signal& signal) const
{
	return getChannelSignalsID(signal.signalGroupID());
}

QVector<int> SignalSet::getChannelSignalsID(int signalGroupID) const
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

void SignalSet::resetAddresses()
{
	int signalCount = count();

	for(int i = 0; i < signalCount; i++)
	{
		(*this)[i].resetAddresses();
	}
}


//

void SerializeSignalsFromXml(const QString& filePath, SignalSet& signalSet)
{
	QXmlStreamReader applicationSignalsReader;
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly))
	{
		applicationSignalsReader.setDevice(&file);

		while (!applicationSignalsReader.atEnd())
		{
			QXmlStreamReader::TokenType token = applicationSignalsReader.readNext();

			switch (token)
			{
			case QXmlStreamReader::StartElement:
			{
				const QXmlStreamAttributes& attr = applicationSignalsReader.attributes();

				if (applicationSignalsReader.name() == "Signal")
				{
					Signal* pSignal = new Signal;
					pSignal->serializeFields(attr);
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

