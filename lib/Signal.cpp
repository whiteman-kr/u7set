#include <QXmlStreamAttributes>
#include <QFile>
#include <utility>

#include "Signal.h"
#include "DataSource.h"
#include "WUtils.h"
#include "SignalProperties.h"

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
	m_specPropStruct = deviceSignal.signalSpecPropsStruc();

	SignalSpecPropValues spv;

	spv.createFromSpecPropStruct(m_specPropStruct);

	spv.serializeValuesToArray(&m_protoSpecPropValues);
}

Signal::~Signal()
{
	if (m_cachedSpecPropValues != nullptr)
	{
		delete m_cachedSpecPropValues;
	}
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

int Signal::lowADC() const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::lowADCCaption));
}

void Signal::setLowADC(int lowADC)
{
	setSpecPropUInt(SignalProperties::lowADCCaption, static_cast<unsigned int>(lowADC));
}

int Signal::highADC() const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::highADCCaption));
}

void Signal::setHighADC(int highADC)
{
	setSpecPropUInt(SignalProperties::highADCCaption, static_cast<unsigned int>(highADC));
}

int Signal::lowDAC() const
{
	return getSpecPropInt(SignalProperties::lowDACCaption);
}

void Signal::setLowDAC(int lowDAC)
{
	setSpecPropInt(SignalProperties::lowDACCaption, lowDAC);
}

int Signal::highDAC() const
{
	return getSpecPropInt(SignalProperties::highDACCaption);
}

void Signal::setHighDAC(int highDAC)
{
	setSpecPropInt(SignalProperties::highDACCaption, highDAC);
}

double Signal::lowEngeneeringUnits() const
{
	return getSpecPropDouble(SignalProperties::lowEngeneeringUnitsCaption);
}

void Signal::setLowEngeneeringUnits(double lowEngeneeringUnits)
{
	setSpecPropDouble(SignalProperties::lowEngeneeringUnitsCaption, lowEngeneeringUnits);
}

double Signal::highEngeneeringUnits() const
{
	return getSpecPropDouble(SignalProperties::highEngeneeringUnitsCaption);
}

void Signal::setHighEngeneeringUnits(double highEngeneeringUnits)
{
	setSpecPropDouble(SignalProperties::highEngeneeringUnitsCaption, highEngeneeringUnits);
}

double Signal::lowValidRange() const
{
	return getSpecPropDouble(SignalProperties::lowValidRangeCaption);
}

void Signal::setLowValidRange(double lowValidRange)
{
	setSpecPropDouble(SignalProperties::lowValidRangeCaption, lowValidRange);
}

double Signal::highValidRange() const
{
	return getSpecPropDouble(SignalProperties::highValidRangeCaption);
}

void Signal::setHighValidRange(double highValidRange)
{
	setSpecPropDouble(SignalProperties::highValidRangeCaption, highValidRange);
}

double Signal::filteringTime() const
{
	return getSpecPropDouble(SignalProperties::filteringTimeCaption);
}

void Signal::setFilteringTime(double filteringTime)
{
	setSpecPropDouble(SignalProperties::filteringTimeCaption, filteringTime);
}

double Signal::spreadTolerance() const
{
	return getSpecPropDouble(SignalProperties::spreadToleranceCaption);
}

void Signal::setSpreadTolerance(double spreadTolerance)
{
	setSpecPropDouble(SignalProperties::spreadToleranceCaption, spreadTolerance);
}

double Signal::electricLowLimit() const
{
	return getSpecPropDouble(SignalProperties::electricLowLimitCaption);
}

void Signal::setElectricLowLimit(double electricLowLimit)
{
	setSpecPropDouble(SignalProperties::electricLowLimitCaption, electricLowLimit);
}

double Signal::electricHighLimit() const
{
	return getSpecPropDouble(SignalProperties::electricHighLimitCaption);
}

void Signal::setElectricHighLimit(double electricHighLimit)
{
	setSpecPropDouble(SignalProperties::electricHighLimitCaption, electricHighLimit);
}

E::ElectricUnit Signal::electricUnit() const
{
	return static_cast<E::ElectricUnit>(getSpecPropEnum(SignalProperties::electricUnitCaption));
}

void Signal::setElectricUnit(E::ElectricUnit electricUnit)
{
	setSpecPropEnum(SignalProperties::electricUnitCaption, static_cast<int>(electricUnit));
}

E::SensorType Signal::sensorType() const
{
	return static_cast<E::SensorType>(getSpecPropEnum(SignalProperties::sensorTypeCaption));
}

void Signal::setSensorType(E::SensorType sensorType)
{
	setSpecPropEnum(SignalProperties::sensorTypeCaption, static_cast<int>(sensorType));
}

E::OutputMode Signal::outputMode() const
{
	return static_cast<E::OutputMode>(getSpecPropEnum(SignalProperties::outputModeCaption));
}

void Signal::setOutputMode(E::OutputMode outputMode)
{
	setSpecPropEnum(SignalProperties::outputModeCaption, static_cast<int>(outputMode));
}

bool Signal::createSpecPropValues()
{
	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(m_specPropStruct);

	if (result.first == false)
	{
		assert(false);
		return false;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	SignalSpecPropValues spValues;

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		SignalSpecPropValue spValue;

		spValue.create(specificProperty);

		spValues.append(spValue);
	}

	spValues.serializeValuesToArray(&m_protoSpecPropValues);

	return true;
}

void Signal::cacheSpecPropValues()
{
	if (m_cachedSpecPropValues == nullptr)
	{
		m_cachedSpecPropValues = new SignalSpecPropValues;
	}

	m_cachedSpecPropValues->createFromSpecPropStruct(m_specPropStruct);
}


void Signal::saveProtoData(QByteArray* protoDataArray) const
{
	TEST_PTR_RETURN(protoDataArray);

	Proto::ProtoAppSignalData protoData;

	saveProtoData(&protoData);

	int size = protoData.ByteSize();

	protoDataArray->resize(size);

	protoData.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(protoDataArray->data()));
}

void Signal::saveProtoData(Proto::ProtoAppSignalData* protoData) const
{
	TEST_PTR_RETURN(protoData);

	protoData->Clear();

	protoData->set_bustypeid(m_busTypeID.toStdString());
	protoData->set_caption(m_caption.toStdString());
	protoData->set_channel(static_cast<int>(m_channel));

	protoData->set_datasize(m_dataSize);
	protoData->set_byteorder(static_cast<int>(m_byteOrder));
	protoData->set_analogsignalformat(static_cast<int>(m_analogSignalFormat));
	protoData->set_unit(m_unit.toStdString());

	protoData->set_enabletuning(m_enableTuning);
	m_tuningDefaultValue.save(protoData->mutable_tuningdefaultvalue());
	m_tuningLowBound.save(protoData->mutable_tuninglowbound());
	m_tuningHighBound.save(protoData->mutable_tuninghighbound());

	protoData->set_acquire(m_acquire);
	protoData->set_archive(m_archive);
	protoData->set_decimalplaces(m_decimalPlaces);
	protoData->set_coarseaperture(m_coarseAperture);
	protoData->set_fineaperture(m_fineAperture);
	protoData->set_adaptiveaperture(m_adaptiveAperture);
}


void Signal::loadProtoData(const QByteArray& protoDataArray)
{
	Proto::ProtoAppSignalData protoData;

	bool res = protoData.ParseFromArray(protoDataArray.constData(), protoDataArray.size());

	assert(res == true);

	loadProtoData(protoData);
}

void Signal::loadProtoData(const Proto::ProtoAppSignalData& protoData)
{
	m_busTypeID = QString::fromStdString(protoData.bustypeid());
	m_caption = QString::fromStdString(protoData.caption());
	m_channel = static_cast<E::Channel>(protoData.channel());

	m_dataSize = protoData.datasize();
	m_byteOrder = static_cast<E::ByteOrder>(protoData.byteorder());

	// Convert data format from E::DataFormat::UnsignedInt to E::AnalogAppSignalFormat::SignedInt32
	//
	int f = protoData.analogsignalformat();

	if (f == static_cast<int>(E::DataFormat::UnsignedInt))
	{
		f = TO_INT(E::AnalogAppSignalFormat::SignedInt32);
	}

	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(f);

	//

	m_unit = QString::fromStdString(protoData.unit());

	m_enableTuning = protoData.enabletuning();
	m_tuningDefaultValue.load(protoData.tuningdefaultvalue());
	m_tuningLowBound.load(protoData.tuninglowbound());
	m_tuningHighBound.load(protoData.tuninghighbound());

	m_acquire = protoData.acquire();
	m_archive = protoData.archive();
	m_decimalPlaces = protoData.decimalplaces();
	m_coarseAperture = protoData.coarseaperture();
	m_fineAperture = protoData.fineaperture();
	m_adaptiveAperture = protoData.adaptiveaperture();
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
	xml.writeIntAttribute("UnitID", 0);
	xml.writeDoubleAttribute("LowValidRange", lowValidRange());
	xml.writeDoubleAttribute("HighValidRange", highValidRange());
	xml.writeDoubleAttribute("UnbalanceLimit", 1);
	xml.writeDoubleAttribute("InputLowLimit", electricLowLimit());
	xml.writeDoubleAttribute("InputHighLimit", electricHighLimit());
	xml.writeIntAttribute("InputUnitID", TO_INT(electricUnit()));
	xml.writeIntAttribute("InputSensorID", TO_INT(sensorType()));
	xml.writeDoubleAttribute("OutputLowLimit", electricLowLimit());
	xml.writeDoubleAttribute("OutputHighLimit", electricHighLimit());
	xml.writeIntAttribute("OutputUnitID", TO_INT(electricUnit()));
	xml.writeIntAttribute("OutputMode", TO_INT(outputMode()));
	xml.writeIntAttribute("OutputSensorID", TO_INT(sensorType()));
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

	// write spec properties

	xml.writeStringAttribute("SpecPropStruct", specPropStruct());
	xml.writeStringAttribute("SpecPropValues", QString(protoSpecPropValues().toHex()));

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

	int intSpecPropValue = 0;
	double doubleSpecPropValue = 0;

	result &= xml.readIntAttribute("LowADC", &intSpecPropValue);
	result &= xml.readIntAttribute("HighADC", &intSpecPropValue);

	result &= xml.readDoubleAttribute("LowEngeneeringUnits", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("HighEngeneeringUnits", &doubleSpecPropValue);

	result &= xml.readIntAttribute("UnitID", &intValue);

	result &= xml.readDoubleAttribute("LowValidRange", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("HighValidRange", &doubleSpecPropValue);

	double unbalanceLimit = 0;
	result &= xml.readDoubleAttribute("UnbalanceLimit", &unbalanceLimit);

	result &= xml.readDoubleAttribute("InputLowLimit", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("InputHighLimit", &doubleSpecPropValue);

	result &= xml.readIntAttribute("InputUnitID", &intSpecPropValue);

	result &= xml.readIntAttribute("InputSensorID", &intSpecPropValue);

	result &= xml.readDoubleAttribute("OutputLowLimit", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("OutputHighLimit", &doubleSpecPropValue);

	result &= xml.readIntAttribute("OutputUnitID", &intSpecPropValue);
	result &= xml.readIntAttribute("OutputMode", &intSpecPropValue);
	result &= xml.readIntAttribute("OutputSensorID", &intSpecPropValue);

	result &= xml.readBoolAttribute("Acquire", &m_acquire);

	bool boolValue = false;
	result &= xml.readBoolAttribute("Calculated", &boolValue);
	result &= xml.readIntAttribute("NormalState", &intValue);
	result &= xml.readIntAttribute("DecimalPlaces", &m_decimalPlaces);
	result &= xml.readDoubleAttribute("Aperture", &m_coarseAperture);
	m_fineAperture = m_coarseAperture;

	result &= xml.readIntAttribute("InOutType", &intValue);
	m_inOutType = static_cast<E::SignalInOutType>(intValue);

	result &= xml.readDoubleAttribute("FilteringTime", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("SpreadTolerance", &doubleSpecPropValue);

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

	// read spec properties

	result &= xml.readStringAttribute("SpecPropStruct", &m_specPropStruct);

	QString hexArray;

	result &= xml.readStringAttribute("SpecPropValues", &hexArray);

	m_protoSpecPropValues = QByteArray(hexArray.toLatin1());

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

	// Signal specific properties

	s->set_specpropstruct(m_specPropStruct.toStdString());
	s->set_specpropvalues(m_protoSpecPropValues.constData(), m_protoSpecPropValues.size());

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

	// Signal specific properties

	m_specPropStruct = QString::fromStdString(s.specpropstruct());
	m_protoSpecPropValues.fromStdString(s.specpropvalues());

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


double Signal::getSpecPropDouble(const QString& name) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum);

	if (result == false)
	{
		return 0;
	}

	assert(qv.type() == QVariant::Double && isEnum == false);

	return qv.toDouble();
}

int Signal::getSpecPropInt(const QString& name) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum);

	if (result == false)
	{
		return 0;
	}

	assert(qv.type() == QVariant::Int && isEnum == false);

	return qv.toInt();
}

unsigned int Signal::getSpecPropUInt(const QString& name) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum);

	if (result == false)
	{
		return 0;
	}

	assert(qv.type() == QVariant::UInt && isEnum == false);

	return qv.toUInt();
}


int Signal::getSpecPropEnum(const QString& name) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum);

	if (result == false)
	{
		assert(false);
		return 0;
	}

	assert(qv.type() == QVariant::Int && isEnum == true);

	return qv.toInt();
}

bool Signal::getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum) const
{
	TEST_PTR_RETURN_FALSE(qv);
	TEST_PTR_RETURN_FALSE(isEnum);

	if (m_cachedSpecPropValues != nullptr)
	{
		return m_cachedSpecPropValues->getValue(name, qv, isEnum);
	}

	SignalSpecPropValues spv;

	bool result = spv.parseValuesFromArray(m_protoSpecPropValues);

	if (result == false)
	{
		assert(false);
		return false;
	}

	return spv.getValue(name, qv, isEnum);
}

bool Signal::setSpecPropDouble(const QString& name, double value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool Signal::setSpecPropInt(const QString& name, int value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool Signal::setSpecPropUInt(const QString& name, unsigned int value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool Signal::setSpecPropEnum(const QString& name, int enumValue)
{
	QVariant qv(enumValue);

	return setSpecPropValue(name, qv, true);
}

bool Signal::setSpecPropValue(const QString& name, const QVariant& qv, bool isEnum)
{
	SignalSpecPropValues spv;

	bool result = spv.parseValuesFromArray(m_protoSpecPropValues);

	if (result == false)
	{
		assert(false);
		return false;
	}

	if (isEnum == true)
	{
		return spv.setEnumValue(name, qv.toInt());
	}

	spv.setValue(name, qv);

	spv.serializeValuesToArray(&m_protoSpecPropValues);

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

bool SignalSet::serializeFromProtoFile(const QString& filePath)
{
	clear();

	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	QByteArray fileData = qUncompress(file.readAll());

	::Proto::AppSignalSet protoAppSignalSet;

	bool result = protoAppSignalSet.ParseFromArray(fileData.constData(), file.size());

	if (result == false)
	{
		return false;
	}

	int signalCount = protoAppSignalSet.appsignal_size();

	reserve(static_cast<int>(signalCount * 1.3));

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

		Signal* newSignal = new Signal;

		newSignal->serializeFrom(protoAppSignal);

		append(newSignal->ID(), newSignal);
	}

	buildID2IndexMap();

	return true;
}

