#include <QXmlStreamAttributes>
#include <QFile>
#include "../lib/Signal.h"
#include "../lib/DataSource.h"
#include "../lib/WUtils.h"

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

Signal::~Signal()
{
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

void Signal::initCalculatedProperties()
{
	m_hash = calcHash(m_appSignalID);
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

	(this->*setter)(E::stringToValue<E::ElectricUnit>(QString(strValue.constData())));
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SensorType))
{
	const QStringRef& strValue = attr.value(fieldName);

	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::SensorType>(QString(strValue.constData())));
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType))
{
	const QStringRef& strValue = attr.value(fieldName);

	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	(this->*setter)(E::stringToValue<E::SignalInOutType>(QString(strValue.constData())));
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

	(this->*setter)(E::stringToValue<E::AnalogAppSignalFormat>(QString(strValue.constData())));
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
	m_tuningAddr.setBit(bit);

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
	s->set_analogsignalformat(TO_INT(m_analogSignalFormat));

	// Analog signal properties

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
	s->set_tuningdefaultvalue(m_tuningDefaultValue);
	s->set_tuninglowbound(m_tuningLowBound);
	s->set_tuninghighbound(m_tuningHighBound);

	// Signal properties for MATS

	s->set_acquire(m_acquire);
	s->set_decimalplaces(m_decimalPlaces);
	s->set_coarseaperture(m_coarseAperture);
	s->set_fineaperture(m_fineAperture);
	s->set_adaptiveaperture(m_adaptiveAperture);

	// Signal fields from database

	s->set_id(m_ID);
	s->set_signalgroupid(m_signalGroupID);
	s->set_signalinstanceid(m_signalInstanceID);
	s->set_changesetid(m_changesetID);
	s->set_checkedout(m_checkedOut);
	s->set_userid(m_userID);
	s->set_created(m_created.toMSecsSinceEpoch());
	s->set_deleted(m_deleted);
	s->set_instancecreated(m_instanceCreated.toMSecsSinceEpoch());
	s->set_instanceaction(m_instanceAction.toInt());

	// Signal properties calculated in compile-time

	s->set_hash(calcHash(m_appSignalID));
	s->set_unit(m_unit.toStdString());

	if (m_ioBufAddr.isValid() == true)
	{
		s->mutable_iobufaddr()->set_offset(m_ioBufAddr.offset());
		s->mutable_iobufaddr()->set_bit(m_ioBufAddr.bit());
	}

	if (m_tuningAddr.isValid() == true)
	{
		s->mutable_tuningaddr()->set_offset(m_tuningAddr.offset());
		s->mutable_tuningaddr()->set_bit(m_tuningAddr.bit());
	}

	if (m_ualAddr.isValid() == true)
	{
		s->mutable_ualaddr()->set_offset(m_ualAddr.offset());
		s->mutable_ualaddr()->set_bit(m_ualAddr.bit());
	}

	if (m_regBufAddr.isValid() == true)
	{
		s->mutable_regbufaddr()->set_offset(m_regBufAddr.offset());
		s->mutable_regbufaddr()->set_bit(m_regBufAddr.bit());
	}

	if (m_regValueAddr.isValid() == true)
	{
		s->mutable_regvalueaddr()->set_offset(m_regValueAddr.offset());
		s->mutable_regvalueaddr()->set_bit(m_regValueAddr.bit());
	}

	if (m_regValidityAddr.isValid() == true)
	{
		s->mutable_regvalidityaddr()->set_offset(m_regValidityAddr.offset());
		s->mutable_regvalidityaddr()->set_bit(m_regValidityAddr.bit());
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
	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(s.analogsignalformat());

	// Analog signal properties

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
	m_tuningDefaultValue = s.tuningdefaultvalue();
	m_tuningLowBound = s.tuninglowbound();
	m_tuningHighBound = s.tuninghighbound();

	//	Signal properties for MATS

	m_acquire = s.acquire();
	m_decimalPlaces = s.decimalplaces();
	m_coarseAperture = s.coarseaperture();
	m_fineAperture = s.fineaperture();
	m_adaptiveAperture = s.adaptiveaperture();

	// Signal fields from database

	m_ID = s.id();
	m_signalGroupID = s.signalgroupid();
	m_signalInstanceID = s.signalinstanceid();
	m_changesetID = s.changesetid();
	m_checkedOut = s.checkedout();
	m_userID = s.userid();
	m_created.setMSecsSinceEpoch(s.created());
	m_deleted = s.deleted();
	m_instanceCreated.setMSecsSinceEpoch(s.instancecreated());
	m_instanceAction = static_cast<VcsItemAction::VcsItemActionType>(s.instanceaction());

	// Signal properties calculated in compile-time

	m_hash = s.hash();
	m_unit = QString::fromStdString(s.unit());

	m_ioBufAddr.setOffset(s.iobufaddr().offset());
	m_ioBufAddr.setBit(s.iobufaddr().bit());

	m_tuningAddr.setOffset(s.tuningaddr().offset());
	m_tuningAddr.setBit(s.tuningaddr().bit());

	m_ualAddr.setOffset(s.ualaddr().offset());
	m_ualAddr.setBit(s.ualaddr().bit());

	m_regBufAddr.setOffset(s.regbufaddr().offset());
	m_regBufAddr.setBit(s.regbufaddr().bit());

	m_regValueAddr.setOffset(s.regvalueaddr().offset());
	m_regValueAddr.setBit(s.regvalueaddr().bit());

	m_regValidityAddr.setOffset(s.regvalidityaddr().offset());
	m_regValidityAddr.setBit(s.regvalidityaddr().bit());
}


/*
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

	if (unitList() != nullptr && unitList()->contains(m_unitID) == true)
	{
		message->set_unit(unitList()->value(m_unitID).toStdString());
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
	message->set_aperture(m_roughAperture);
	message->set_filteringtime(m_filteringTime);
	message->set_spreadtolerance(m_spreadTolerance);
	message->set_enabletuning(m_enableTuning);
	message->set_tuningdefaultvalue(m_tuningDefaultValue);

	return;
}
*/

// --------------------------------------------------------------------------------------------------------
//
// SignalSet class implementation
//
// --------------------------------------------------------------------------------------------------------

SignalSet::SignalSet(VFrame30::BusSet* busSet, Builder::IssueLogger* log) :
	m_busSet(busSet),
	m_log(log)
{
	assert(busSet != nullptr);
	assert(log != nullptr);
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

bool SignalSet::contains(const QString& appSignalID)
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


bool SignalSet::checkSignals()
{
	bool result = true;

	int signalCount = count();

	if (signalCount == 0)
	{
		return true;
	}

	LOG_EMPTY_LINE(m_log);
	LOG_MESSAGE(m_log, QString(tr("Checking application signals...")));

	QHash<QString, int> appSignalIDs;
	QHash<QString, int> customAppSignalIDs;

	appSignalIDs.reserve(static_cast<int>(signalCount * 1.3));

	m_busSignals.clear();

	for(int i = 0; i < signalCount; i++)
	{
		Signal& s = (*this)[i];

		if (s.ID() > m_maxSignalID)
		{
			m_maxSignalID = s.ID();
		}

		// check AppSignalID
		//
		if (appSignalIDs.contains(s.appSignalID()) == true)
		{
			// Application signal identifier '%1' is not unique.
			//
			m_log->errALC5016(s.appSignalID());
			result = false;
			continue;
		}

		appSignalIDs.insert(s.appSignalID(), i);

		// check CustomAppSignalID
		//
		if (customAppSignalIDs.contains(s.customAppSignalID()) == true)
		{
			// Custom application signal identifier '%1' is not unique.
			//
			m_log->errALC5017(s.customAppSignalID());
			result = false;
			continue;
		}

		customAppSignalIDs.insert(s.customAppSignalID(), i);

		// check other signal properties
		//
		if (s.byteOrder() != E::ByteOrder::BigEndian)
		{
			// Signal '%1' has Little Endian byte order.
			//
			m_log->wrnALC5070(s.appSignalID());
		}

		switch(s.signalType())
		{
		case E::SignalType::Discrete:
			if (s.dataSize() != 1)
			{
				// Discrete signal '%1' must have DataSize equal to 1.
				//
				m_log->errALC5014(s.appSignalID());
				result = false;
			}
			break;

		case E::SignalType::Analog:
			if (s.dataSize() != 32)
			{
				// Analog signal '%1' must have DataSize equal to 32.
				//
				m_log->errALC5015(s.appSignalID());
				result = false;
			}

			if (s.coarseAperture() <= 0 || s.fineAperture() <= 0)
			{
				// Analog signal '%1' aperture should be greate then 0.
				//
				m_log->errALC5090(s.appSignalID());
				result = false;
			}

			if (s.coarseAperture() < s.fineAperture())
			{
				// Coarse aperture of signal '%1' less then fine aperture.
				//
				m_log->wrnALC5093(s.appSignalID());
				result = false;
			}

			break;

		case E::SignalType::Bus:
			if (m_busSet->hasBus(s.busTypeID()) == false)
			{
				//  Bus type ID '%1' of signal '%2' is undefined.
				//
				m_log->errALC5092(s.busTypeID(), s.appSignalID());
				result = false;
			}
			else
			{
				m_busSignals.insert(i, s.appSignalID());
			}
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// check tuningable signals properties
		//
		if (s.enableTuning() == false)
		{
			continue;
		}

		if (s.isAnalog() == true)
		{
			if (s.lowEngeneeringUnits() >= s.highEngeneeringUnits())
			{
				// LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.
				//
				m_log->errALC5068(s.appSignalID());
				result = false;
			}
			else
			{
				// limits OK
				//
				if (s.tuningDefaultValue() < s.lowEngeneeringUnits() ||
					s.tuningDefaultValue() > s.highEngeneeringUnits())
				{
					// TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits toHighEngeneeringUnits.
					//
					m_log->errALC5069(s.appSignalID());
					result = false;
				}
			}
		}
	}

	if (result == true)
	{
		LOG_SUCCESS(m_log, QString(tr("Ok")))
	}

	return result;

}

bool SignalSet::expandBusSignals()
{
	if (m_busSignals.count() == 0)
	{
		return true;
	}

	bool result = true;

	for(QString busSignalID : m_busSignals)
	{
		Signal* s = getSignal(busSignalID);

		if (s == nullptr)
		{
			result = false;
			LOG_INTERNAL_ERROR(m_log);
			continue;
		}

		const VFrame30::Bus& bus = m_busSet->bus(s->busTypeID());

		const std::vector<VFrame30::BusSignal>& busSignals = bus.busSignals();

		for(const VFrame30::BusSignal& busSignal : busSignals)
		{
			result &= appendBusSignal(*s, bus, busSignal);
		}
	}

	return result;
}

bool SignalSet::bindSignalsToLMs(Hardware::EquipmentSet* equipment)
{
	TEST_PTR_RETURN_FALSE(equipment);

	int signalCount = count();

	bool result = true;

	for(int i = 0; i < signalCount; i++)
	{
		Signal& s = (*this)[i];

		// check EquipmentID
		//
		s.setLm(nullptr);

		if (s.equipmentID().isEmpty() == true)
		{
			// Application signal '%1' is not bound to any device object.
			//
			m_log->wrnALC5012(s.appSignalID());
		}
		else
		{
			std::shared_ptr<Hardware::DeviceObject> device = equipment->deviceObjectSharedPointer(s.equipmentID());

			if (device == nullptr)
			{
				// Application signal '%1' is bound to unknown device object '%2'.
				//
				m_log->errALC5013(s.appSignalID(), s.equipmentID());
				result = false;
			}

			bool deviceOK = false;

			switch(device->deviceType())
			{
			case Hardware::DeviceType::Module:
				{
					std::shared_ptr<Hardware::DeviceModule> module = std::dynamic_pointer_cast<Hardware::DeviceModule>(device);

					if (module != nullptr && module->isLogicModule() == true)
					{
						s.setLm(module);
						deviceOK = true;
					}
				}
				break;

			case Hardware::DeviceType::Signal:
				{
					Hardware::DeviceChassis* chassis = const_cast<Hardware::DeviceChassis*>(device->getParentChassis());

					if (chassis == nullptr)
					{
						assert(false);
						continue;
					}

					std::shared_ptr<Hardware::DeviceModule> module = chassis->getLogicModuleSharedPointer();

					if (module != nullptr && module->isLogicModule() == true)
					{
						s.setLm(module);
						deviceOK = true;
					}
				}
				break;
			}

			if (deviceOK == false)
			{
				// The signal '%1' can be bind only to Logic Module or Equipment Signal.
				//
				m_log->errALC5031(s.appSignalID());
				result = false;
				continue;
			}
		}
	}

	return result;
}

void SignalSet::initCalculatedSignalsProperties()
{
	int signalsCount = count();

	for(int i = 0; i < signalsCount; i++)
	{
		Signal& s = (*this)[i];

		s.initCalculatedProperties();
	}
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

bool SignalSet::appendBusSignal(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal)
{
	bool result = true;

	Signal* newSignal = new Signal();

	newSignal->setAppSignalID(QString(s.appSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()));
	newSignal->setCustomAppSignalID(QString(s.customAppSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()));

	QString caption = buildBusSignalCaption(s, bus, busSignal);

	newSignal->setCaption(caption);
	newSignal->setEquipmentID(s.equipmentID());
	newSignal->setBusTypeID(s.busTypeID());
	//newSignal->setChannel(s.channel());

	newSignal->setSignalType(busSignal.type());
	newSignal->setInOutType(E::SignalInOutType::Internal);

	newSignal->setByteOrder(E::ByteOrder::BigEndian);

	switch(newSignal->signalType())
	{
	case E::SignalType::Analog:
		newSignal->setUnit(busSignal.units());
		newSignal->setDataSize(SIZE_32BIT);
		newSignal->setAnalogSignalFormat(busSignal.analogFormat());

		newSignal->setLowADC(busSignal.busAnalogLowLimit());
		newSignal->setHighADC(busSignal.busAnalogHighLimit());

		newSignal->setLowEngeneeringUnits(busSignal.busAnalogLowLimit());
		newSignal->setHighEngeneeringUnits(busSignal.busAnalogHighLimit());

		newSignal->setLowValidRange(busSignal.busAnalogLowLimit());
		newSignal->setHighValidRange(busSignal.busAnalogHighLimit());

		break;

	case E::SignalType::Discrete:
		newSignal->setUnit("");
		newSignal->setDataSize(SIZE_1BIT);

		break;

	default:
		assert(false);
	}

	newSignal->setEnableTuning(false);

	newSignal->setAcquire(s.acquire());
	newSignal->setDecimalPlaces(2);				// !!!
	newSignal->setCoarseAperture(1);
	newSignal->setFineAperture(0.5);
	newSignal->setAdaptiveAperture(false);

/*
	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	VcsItemAction m_instanceAction = VcsItemAction::Added;*/

	if (result == true)
	{
		m_maxSignalID++;

		append(m_maxSignalID, newSignal);
	}
	else
	{
		delete newSignal;
	}

	return result;
}

QString SignalSet::buildBusSignalCaption(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal)
{
	QString caption = s.caption();

	caption.replace(Signal::BUS_SIGNAL_MACRO_BUSTYPEID, bus.busTypeId());
	caption.replace(Signal::BUS_SIGNAL_MACRO_BUSID, s.customAppSignalID());
	caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALID, busSignal.signalId());
	caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION, busSignal.caption());

	return caption;
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

