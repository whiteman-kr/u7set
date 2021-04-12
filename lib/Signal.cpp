#include <QXmlStreamAttributes>
#include <QFile>
#include <utility>

#include "Signal.h"
#include "DataSource.h"
#include "../UtilsLib/WUtils.h"
#include "SignalProperties.h"

#include "../Builder/IssueLogger.h"
#include "../Proto/serialization.pb.h"

// --------------------------------------------------------------------------------------------------------
//
// Signal class implementation
//
// --------------------------------------------------------------------------------------------------------

const QString Signal::MACRO_START_TOKEN("$(");
const QString Signal::MACRO_END_TOKEN(")");

Signal::Signal()
{
	updateTuningValuesType();
}

Signal::Signal(const Signal& s)
{
	*this = s;
}

Signal::Signal(const ID_AppSignalID& ids)
{
	m_ID = ids.ID;
	m_appSignalID = ids.appSignalID;

	m_isLoaded = false;
}

Signal::Signal(	const Hardware::DeviceAppSignal& deviceSignal,
				QString* errMsg)
{
	TEST_PTR_RETURN(errMsg);

	if (deviceSignal.isDiagSignal() == true)
	{
		Q_ASSERT(false);
		return;
	}

	//

	initIDsAndCaption(deviceSignal, errMsg);	// init AppSignalID, CustomAppSignalID, EquipmentID, Caption

	if (errMsg->isEmpty() == false)
	{
		return;
	}

	//

	m_signalType = deviceSignal.signalType();

	switch(m_signalType)
	{
	case E::SignalType::Analog:

		m_analogSignalFormat = deviceSignal.appSignalDataFormat();
		assert(m_analogSignalFormat == E::AnalogAppSignalFormat::Float32 || m_analogSignalFormat == E::AnalogAppSignalFormat::SignedInt32);

		setDataSize(m_signalType, m_analogSignalFormat);
		updateTuningValuesType();
		initTuningValues();

		break;

	case E::SignalType::Discrete:

		setDataSize(m_signalType, m_analogSignalFormat);
		updateTuningValuesType();
		initTuningValues();

		break;

	case E::SignalType::Bus:

		m_busTypeID = deviceSignal.appSignalBusTypeId();

		break;

	default:

		assert(false);
		*errMsg = "Unknown device signal E::SignalType";

		return;
	}

	switch(deviceSignal.function())
	{
	case E::SignalFunction::Input:
	case E::SignalFunction::Validity:

		m_inOutType = E::SignalInOutType::Input;

		break;

	case E::SignalFunction::Output:

		m_inOutType = E::SignalInOutType::Output;

		break;

	case E::SignalFunction::Diagnostics:

		assert(false);	// never will be here

		break;

	default:

		assert(false);
		*errMsg = "Unknown device signal E::SignalFunction";

		return;
	}

	checkAndInitTuningSettings(deviceSignal, errMsg);

	if (errMsg->isEmpty() == false)
	{
		return;
	}

	// specific properties processing
	//
	m_specPropStruct = deviceSignal.signalSpecPropsStruct();

	if (m_specPropStruct.contains(SignalProperties::MISPRINT_lowEngineeringUnitsCaption) ||
		m_specPropStruct.contains(SignalProperties::MISPRINT_highEngineeringUnitsCaption))
	{
		*errMsg = QString("Misprinted signal specific properties HighEngEneeringUnits/LowEngEneeringUnits has detected in device signal %1. \n\n"
					 "Update module preset first. \n\nApplication signal creation is aborted!").
										arg(deviceSignal.equipmentIdTemplate());
		return;
	}

	SignalSpecPropValues spv;

	spv.createFromSpecPropStruct(m_specPropStruct);

	spv.serializeValuesToArray(&m_protoSpecPropValues);
}

Signal::~Signal()
{
}

void Signal::clear()
{
	*this = Signal();
}

void Signal::initSpecificProperties()
{
	QString specPropStruct;

	switch(m_signalType)
	{
	case E::SignalType::Analog:

		switch(m_inOutType)
		{
		case E::SignalInOutType::Input:
			specPropStruct = SignalProperties::defaultInputAnalogSpecPropStruct;
			break;

		case E::SignalInOutType::Output:
			specPropStruct = SignalProperties::defaultOutputAnalogSpecPropStruct;
			break;

		case E::SignalInOutType::Internal:
			specPropStruct = SignalProperties::defaultInternalAnalogSpecPropStruct;
			break;

		default:
			assert(false);
		}

		break;
	case E::SignalType::Discrete:
	case E::SignalType::Bus:
		break;

	default:
		assert(false);
	}

	if (specPropStruct.isEmpty() == true)
	{
		setSpecPropStruct("");
		m_protoSpecPropValues.clear();
	}
	else
	{
		setSpecPropStruct(specPropStruct);
		createSpecPropValues();
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

	case E::SignalType::Bus:

		assert(false);						// function setDataSize should not be call for Bus signals
											// data size of bus signals is defined by BusTypeID
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

int Signal::lowADC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::lowADCCaption, err));
}

void Signal::setLowADC(int lowADC)
{
	setSpecPropUInt(SignalProperties::lowADCCaption, static_cast<unsigned int>(lowADC));
}

int Signal::highADC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::highADCCaption, err));
}

void Signal::setHighADC(int highADC)
{
	setSpecPropUInt(SignalProperties::highADCCaption, static_cast<unsigned int>(highADC));
}

int Signal::lowDAC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::lowDACCaption, err));
}

void Signal::setLowDAC(int lowDAC)
{
	setSpecPropInt(SignalProperties::lowDACCaption, lowDAC);
}

int Signal::highDAC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(SignalProperties::highDACCaption, err));
}

void Signal::setHighDAC(int highDAC)
{
	setSpecPropInt(SignalProperties::highDACCaption, highDAC);
}

double Signal::lowEngineeringUnits(QString* err) const
{
	return getSpecPropDouble(SignalProperties::lowEngineeringUnitsCaption, err);
}

void Signal::setLowEngineeringUnits(double lowEngineeringUnits)
{
	setSpecPropDouble(SignalProperties::lowEngineeringUnitsCaption, lowEngineeringUnits);
}

double Signal::highEngineeringUnits(QString* err) const
{
	return getSpecPropDouble(SignalProperties::highEngineeringUnitsCaption, err);
}

void Signal::setHighEngineeringUnits(double highEngineeringUnits)
{
	setSpecPropDouble(SignalProperties::highEngineeringUnitsCaption, highEngineeringUnits);
}

double Signal::lowValidRange(QString* err) const
{
	return getSpecPropDouble(SignalProperties::lowValidRangeCaption, err);
}

void Signal::setLowValidRange(double lowValidRange)
{
	setSpecPropDouble(SignalProperties::lowValidRangeCaption, lowValidRange);
}

double Signal::highValidRange(QString* err) const
{
	return getSpecPropDouble(SignalProperties::highValidRangeCaption, err);
}

void Signal::setHighValidRange(double highValidRange)
{
	setSpecPropDouble(SignalProperties::highValidRangeCaption, highValidRange);
}

double Signal::filteringTime(QString* err) const
{
	return getSpecPropDouble(SignalProperties::filteringTimeCaption, err);
}

void Signal::setFilteringTime(double filteringTime)
{
	setSpecPropDouble(SignalProperties::filteringTimeCaption, filteringTime);
}

double Signal::spreadTolerance(QString* err) const
{
	return getSpecPropDouble(SignalProperties::spreadToleranceCaption, err);
}

void Signal::setSpreadTolerance(double spreadTolerance)
{
	setSpecPropDouble(SignalProperties::spreadToleranceCaption, spreadTolerance);
}

double Signal::electricLowLimit(QString* err) const
{
	return getSpecPropDouble(SignalProperties::electricLowLimitCaption, err);
}

void Signal::setElectricLowLimit(double electricLowLimit)
{
	setSpecPropDouble(SignalProperties::electricLowLimitCaption, electricLowLimit);
}

double Signal::electricHighLimit(QString* err) const
{
	return getSpecPropDouble(SignalProperties::electricHighLimitCaption, err);
}

void Signal::setElectricHighLimit(double electricHighLimit)
{
	setSpecPropDouble(SignalProperties::electricHighLimitCaption, electricHighLimit);
}

E::ElectricUnit Signal::electricUnit(QString* err) const
{
	return static_cast<E::ElectricUnit>(getSpecPropEnum(SignalProperties::electricUnitCaption, err));
}

void Signal::setElectricUnit(E::ElectricUnit electricUnit)
{
	setSpecPropEnum(SignalProperties::electricUnitCaption, static_cast<int>(electricUnit));
}

double Signal::rload_Ohm(QString* err) const
{
	return getSpecPropDouble(SignalProperties::rload_OhmCaption, err);
}

void Signal::setRload_Ohm(double rload_Ohm)
{
	setSpecPropDouble(SignalProperties::rload_OhmCaption, rload_Ohm);
}

E::SensorType Signal::sensorType(QString* err) const
{
	return static_cast<E::SensorType>(getSpecPropEnum(SignalProperties::sensorTypeCaption, err));
}

void Signal::setSensorType(E::SensorType sensorType)
{
	setSpecPropEnum(SignalProperties::sensorTypeCaption, static_cast<int>(sensorType));
}

E::OutputMode Signal::outputMode(QString* err) const
{
	return static_cast<E::OutputMode>(getSpecPropEnum(SignalProperties::outputModeCaption, err));
}

void Signal::setOutputMode(E::OutputMode outputMode)
{
	setSpecPropEnum(SignalProperties::outputModeCaption, static_cast<int>(outputMode));
}

double Signal::r0_Ohm(QString* err) const
{
	return getSpecPropDouble(SignalProperties::R0_OhmCaption, err);
}

void Signal::setR0_Ohm(double r0_Ohm)
{
	setSpecPropDouble(SignalProperties::R0_OhmCaption, r0_Ohm);
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
		m_cachedSpecPropValues = std::make_shared<SignalSpecPropValues>();
	}

	m_cachedSpecPropValues->parseValuesFromArray(m_protoSpecPropValues);
}

void Signal::saveProtoData(QByteArray* protoDataArray) const
{
	TEST_PTR_RETURN(protoDataArray);

	Proto::ProtoAppSignalData protoData;

	saveProtoData(&protoData);

	protoDataArray->resize(static_cast<int>(protoData.ByteSizeLong()));

	protoData.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(protoDataArray->data()));
}

void Signal::saveProtoData(Proto::ProtoAppSignalData* protoData) const
{
	TEST_PTR_RETURN(protoData);

	protoData->Clear();

	protoData->set_bustypeid(m_busTypeID.toStdString());
	protoData->set_caption(m_caption.toStdString());
	protoData->set_channel(static_cast<int>(m_channel));
	protoData->set_excludefrombuild(m_excludeFromBuild);

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

	//

	protoData->set_tags(tagsStr().toStdString());
}

void Signal::loadProtoData(const QByteArray& protoDataArray)
{
	Proto::ProtoAppSignalData protoData;

	bool res = protoData.ParseFromArray(protoDataArray.constData(), protoDataArray.size());

	assert(res == true);
	Q_UNUSED(res)

	loadProtoData(protoData);
}

void Signal::loadProtoData(const Proto::ProtoAppSignalData& protoData)
{
	m_busTypeID = QString::fromStdString(protoData.bustypeid());
	m_caption = QString::fromStdString(protoData.caption());
	m_channel = static_cast<E::Channel>(protoData.channel());
	m_excludeFromBuild = protoData.excludefrombuild();

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

	//

	setTagsStr(QString::fromStdString(protoData.tags()));
}

Address16 Signal::ioBufAddr() const
{
	return m_ioBufAddr;
}

void Signal::setIoBufAddr(const Address16& addr)
{
	m_ioBufAddr = addr;
}

Address16 Signal::actualAddr(E::LogicModuleRamAccess* lmRamAccess) const
{
	if (lmRamAccess != nullptr)
	{
		*lmRamAccess = m_lmRamAccess;
	}

	if (m_ualAddr.isValid() == true)
	{
		return m_ualAddr;
	}

	if ((isInput() == true || isOutput() == true) && m_ioBufAddr.isValid() == true)
	{
		return m_ioBufAddr;
	}

	if (isTunable() == true && m_tuningAbsAddr.isValid() == true)
	{
		return m_tuningAbsAddr;
	}

	return Address16();
}

void Signal::resetAddresses()
{
	m_ioBufAddr.reset();
	m_tuningAddr.reset();
	m_tuningAbsAddr.reset();
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

	writeIntSpecPropAttribute(xml, SignalProperties::lowADCCaption);
	writeIntSpecPropAttribute(xml, SignalProperties::highADCCaption);
	writeDoubleSpecPropAttribute(xml, SignalProperties::lowEngineeringUnitsCaption);
	writeDoubleSpecPropAttribute(xml, SignalProperties::highEngineeringUnitsCaption);
	xml.writeIntAttribute("UnitID", 0);
	writeDoubleSpecPropAttribute(xml, SignalProperties::lowValidRangeCaption);
	writeDoubleSpecPropAttribute(xml, SignalProperties::highValidRangeCaption);
	xml.writeDoubleAttribute("UnbalanceLimit", 1);
	writeDoubleSpecPropAttribute(xml, SignalProperties::electricLowLimitCaption, "InputLowLimit");
	writeDoubleSpecPropAttribute(xml, SignalProperties::electricHighLimitCaption, "InputHighLimit");
	writeIntSpecPropAttribute(xml, SignalProperties::electricUnitCaption, "InputUnitID");
	writeIntSpecPropAttribute(xml, SignalProperties::sensorTypeCaption, "InputSensorID");
	writeDoubleSpecPropAttribute(xml, SignalProperties::electricLowLimitCaption, "OutputLowLimit");
	writeDoubleSpecPropAttribute(xml, SignalProperties::electricHighLimitCaption, "OutputHighLimit");
	writeIntSpecPropAttribute(xml, SignalProperties::electricUnitCaption, "OutputUnitID");

	writeIntSpecPropAttribute(xml, SignalProperties::outputModeCaption);

	writeIntSpecPropAttribute(xml, SignalProperties::sensorTypeCaption, "OutputSensorID");
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeBoolAttribute("Calculated", false);
	xml.writeIntAttribute("NormalState", 0);
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("Aperture", coarseAperture());
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	writeDoubleSpecPropAttribute(xml, SignalProperties::filteringTimeCaption);
	writeDoubleSpecPropAttribute(xml, SignalProperties::spreadToleranceCaption);
	xml.writeIntAttribute("ByteOrder", byteOrderInt());

	writeTuningValuesToXml(xml);

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

void Signal::writeDoubleSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName)
{
	QVariant v;
	bool isEnum = false;
	bool res = getSpecPropValue(propName, &v, &isEnum, nullptr);

	if (res == true)
	{
		xml.writeDoubleAttribute(attributeName.isEmpty() == true ? propName : attributeName, v.toDouble());
	}
	else
	{
		xml.writeDoubleAttribute(attributeName.isEmpty() == true ? propName : attributeName, 0);
	}
}

void Signal::writeIntSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName)
{
	QVariant v;
	bool isEnum = false;
	bool res = getSpecPropValue(propName, &v, &isEnum, nullptr);

	if (res == true)
	{
		xml.writeIntAttribute(attributeName.isEmpty() == true ? propName : attributeName, v.toInt());
	}
	else
	{
		xml.writeIntAttribute(attributeName.isEmpty() == true ? propName : attributeName, 0);
	}
}

void Signal::writeTuningValuesToXml(XmlWriteHelper& xml)
{
	xml.writeBoolAttribute("EnableTuning", enableTuning());

	assert(tuningDefaultValue().type() == tuningLowBound().type());
	assert(tuningDefaultValue().type() == tuningHighBound().type());

	xml.writeStringAttribute("TuningValueType", tuningDefaultValue().typeStr());

	xml.writeStringAttribute("TuningDefaultValue", tuningDefaultValue().toString());
	xml.writeStringAttribute("TuningLowBound", tuningLowBound().toString());
	xml.writeStringAttribute("TuningHighBound", tuningHighBound().toString());
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

	result &= xml.readDoubleAttribute("LowEngineeringUnits", &doubleSpecPropValue);
	result &= xml.readDoubleAttribute("HighEngineeringUnits", &doubleSpecPropValue);

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

	result &= readTuningValuesFromXml(xml);

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

bool Signal::readTuningValuesFromXml(XmlReadHelper &xml)
{
	bool result = true;

	result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);

	QString tuningValueTypeStr;

	result &= xml.readStringAttribute("TuningValueType", &tuningValueTypeStr);

	TuningValueType tvType = TuningValue::typeFromStr(tuningValueTypeStr);

	if (tvType != TuningValue::getTuningValueType(m_signalType, m_analogSignalFormat))
	{
		assert(false);
		return false;
	}

	updateTuningValuesType();

	QString valueStr;

	bool conversionResult = true;

	result &= xml.readStringAttribute("TuningDefaultValue", &valueStr);
	m_tuningDefaultValue.fromString(valueStr, &conversionResult);
	result &= conversionResult;

	result &= xml.readStringAttribute("TuningLowBound", &valueStr);
	m_tuningLowBound.fromString(valueStr, &conversionResult);
	result &= conversionResult;

	result &= xml.readStringAttribute("TuningHighBound", &valueStr);
	m_tuningHighBound.fromString(valueStr, &conversionResult);
	result &= conversionResult;

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
	s->set_lmequipmentid(m_lmEquipmentID.toStdString());
	s->set_bustypeid(m_busTypeID.toStdString());
	s->set_channel(TO_INT(m_channel));
	s->set_excludefrombuild(m_excludeFromBuild);

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

		calcParam->set_lmramaccess(TO_INT(m_lmRamAccess));

		calcParam->set_isconst(m_isConst);
		calcParam->set_constvalue(m_constValue);

		// save state flags signals

		assert(calcParam->stateflagssignals_size() == 0);

		QList<E::AppSignalStateFlagType> flagTypes = m_stateFlagsSignals.keys();

		for(E::AppSignalStateFlagType flagType : flagTypes)
		{
			QString flagSignalID = m_stateFlagsSignals.value(flagType, QString());

			if (flagSignalID.isEmpty() == true)
			{
				assert(false);
				continue;
			}

			Proto::StateFlagSignal* protoStateFlagSignal = calcParam->add_stateflagssignals();

			if (protoStateFlagSignal == nullptr)
			{
				assert(false);
				continue;
			}

			protoStateFlagSignal->set_flagtype(TO_INT(flagType));
			protoStateFlagSignal->set_flagsignalid(flagSignalID.toStdString());
		}
	}
	else
	{
		assert(false);
	}

	// Tags
	//
	s->clear_tags();
	for (const QString& t : m_tags)
	{
		s->add_tags(t.toStdString());
	}
}

void Signal::serializeFrom(const Proto::AppSignal& s)
{
	// Signal identificators

	m_appSignalID = QString::fromStdString(s.appsignalid());
	m_customAppSignalID = QString::fromStdString(s.customappsignalid());
	m_caption = QString::fromStdString(s.caption());
	m_equipmentID = QString::fromStdString(s.equipmentid());
	m_lmEquipmentID = QString::fromStdString(s.lmequipmentid());
	m_busTypeID = QString::fromStdString(s.bustypeid());
	m_channel = static_cast<E::Channel>(s.channel());
	m_excludeFromBuild = s.excludefrombuild();

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
	m_protoSpecPropValues = QByteArray::fromStdString(s.specpropvalues());

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

	m_lmRamAccess = static_cast<E::LogicModuleRamAccess>(calcParam.lmramaccess());

	m_isConst = calcParam.isconst();
	m_constValue = calcParam.constvalue();

	// load state flags signals

	m_stateFlagsSignals.clear();

	int flagSignalsCount = calcParam.stateflagssignals_size();

	for(int i = 0; i < flagSignalsCount; i++)
	{
		const Proto::StateFlagSignal& protoStateFlagSignal = calcParam.stateflagssignals(i);

		E::AppSignalStateFlagType flagType = static_cast<E::AppSignalStateFlagType>(protoStateFlagSignal.flagtype());

		assert(m_stateFlagsSignals.contains(flagType) == false);

		m_stateFlagsSignals.insert(flagType, QString::fromStdString(protoStateFlagSignal.flagsignalid()));
	}

	// Tags
	//
	m_tags.clear();
	for (const auto& t : s.tags())
	{
		m_tags.insert(QString::fromStdString(t));
	}
}

void Signal::initCalculatedProperties()
{
	m_hash = calcHash(m_appSignalID);
}

bool Signal::addFlagSignalID(E::AppSignalStateFlagType flagType, const QString& appSignalID)
{
	if (m_stateFlagsSignals.contains(flagType) == true)
	{
		return false;
	}

	m_stateFlagsSignals.insert(flagType, appSignalID);

	return true;
}

void Signal::initTuningValues()
{
	switch (signalType())
	{
	case E::SignalType::Analog:
		m_tuningLowBound.setValue(m_tuningLowBound.type(), static_cast<qint64>(lowEngineeringUnits(nullptr)), lowEngineeringUnits(nullptr));
		m_tuningHighBound.setValue(m_tuningHighBound.type(), static_cast<qint64>(highEngineeringUnits(nullptr)), highEngineeringUnits(nullptr));
		break;

	case E::SignalType::Discrete:
		m_tuningLowBound.setValue(m_tuningLowBound.type(), 0, 0);
		m_tuningHighBound.setValue(m_tuningHighBound.type(), 1, 1);
		break;

	case E::SignalType::Bus:
		break;

	default:
		assert(false);
	}
}

QString Signal::expandDeviceSignalTemplate(	const Hardware::DeviceObject& startDeviceObject,
											 const QString& templateStr,
											 QString* errMsg)
{
	QString resultStr;

	int searchStartPos = 0;

	do
	{
		int macroStartPos = templateStr.indexOf(MACRO_START_TOKEN, searchStartPos);

		if (macroStartPos == -1)
		{
			// no more macroses
			//
			resultStr += templateStr.mid(searchStartPos);
			break;
		}

		resultStr += templateStr.mid(searchStartPos, macroStartPos - searchStartPos);

		int macroEndPos = templateStr.indexOf(MACRO_END_TOKEN, macroStartPos + 2);

		if (macroEndPos == -1)
		{
			*errMsg = QString("End of macro is not found in template %1 of device object %2. ").
						arg(templateStr).arg(startDeviceObject.equipmentIdTemplate());
			return QString();
		}

		QString macroStr = templateStr.mid(macroStartPos + 2, macroEndPos - (macroStartPos + 2));

		QString expandedMacroStr = expandDeviceObjectMacro(startDeviceObject, macroStr, errMsg);

		if (errMsg->isEmpty() == false)
		{
			return QString();
		}

		resultStr += expandedMacroStr;

		searchStartPos = macroEndPos + 1;
	}
	while(true);

	return resultStr;
}

void Signal::setLm(std::shared_ptr<Hardware::DeviceModule> lm)
{
	TEST_PTR_RETURN(lm);

	m_lm = lm;

	setLmEquipmentID(lm->equipmentIdTemplate());
}

QString Signal::expandDeviceObjectMacro(const Hardware::DeviceObject& startDeviceObject,
										const QString& macroStr,
										QString* errMsg)
{
	QStringList macroFields = macroStr.split(".");

	const Hardware::DeviceObject* deviceObject = nullptr;
	QString propertyCaption;

	switch(macroFields.count())
	{
	case 1:
		{
			// property only
			//
			deviceObject = &startDeviceObject;
			propertyCaption = macroFields.at(0);
		}
		break;

	case 2:
		{
			// parentObject.property
			//
			QString parentObjectType = macroFields.at(0);
			propertyCaption = macroFields.at(1);

			deviceObject = getParentDeviceObjectOfType(startDeviceObject, parentObjectType, errMsg);

			if (errMsg->isEmpty() == false)
			{
				return QString();
			}

			if (deviceObject == nullptr)
			{
				*errMsg = QString("Macro expand error! Parent device object of type '%1' is not found for device object %2").
								arg(parentObjectType).arg(startDeviceObject.equipmentIdTemplate());
				return QString();
			}

		}
		break;

	default:
		*errMsg = QString("Unknown format of macro %1 in template of device signal %2").
				arg(macroStr).arg(startDeviceObject.equipmentIdTemplate());
		return QString();
	}

	if (deviceObject->propertyExists(propertyCaption) == false)
	{
		*errMsg = QString("Device signal %1 macro expand error! Property '%2' is not found in device object %3.").
							arg(startDeviceObject.equipmentIdTemplate()).
							arg(propertyCaption).
							arg(deviceObject->equipmentIdTemplate());
		return QString();
	}

	QString propertyValue = deviceObject->propertyValue(propertyCaption).toString();

	return propertyValue;
}

const Hardware::DeviceObject* Signal::getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
																  const QString& parentObjectType,
																  QString* errMsg)
{
	static const std::map<QString, Hardware::DeviceType> objectTypes {
			std::make_pair(QString("root"), Hardware::DeviceType::Root),
			std::make_pair(QString("system"), Hardware::DeviceType::System),
			std::make_pair(QString("rack"), Hardware::DeviceType::Rack),
			std::make_pair(QString("chassis"), Hardware::DeviceType::Chassis),
			std::make_pair(QString("module"), Hardware::DeviceType::Module),
			std::make_pair(QString("workstation"), Hardware::DeviceType::Workstation),
			std::make_pair(QString("software"), Hardware::DeviceType::Software),
			std::make_pair(QString("controller"), Hardware::DeviceType::Controller),
			std::make_pair(QString("signal"), Hardware::DeviceType::AppSignal),
	};

	std::map<QString, Hardware::DeviceType>::const_iterator it = objectTypes.find(parentObjectType.toLower());

	if (it == objectTypes.end())
	{
		*errMsg = QString("Unknown object type '%1' in call of getParentObjectOfType(...) for device object %2").
						arg(parentObjectType).arg(startObject.equipmentIdTemplate());
		return nullptr;
	}

	Hardware::DeviceType requestedDeviceType = it->second;

	const Hardware::DeviceObject* parent = &startObject;

	do
	{
		if (parent == nullptr)
		{
			break;
		}

		if (parent->deviceType() == requestedDeviceType)
		{
			return parent;
		}

		parent = parent->parent().get();
	}
	while(true);

	return nullptr;
}

void Signal::initCreatedDates()
{
	m_created = QDateTime::currentDateTime();
	m_instanceCreated = QDateTime::currentDateTime();
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

void Signal::initIDsAndCaption(	const Hardware::DeviceAppSignal& deviceSignal,
								QString* errMsg)
{
	TEST_PTR_RETURN(errMsg);

	QString deviceSignalEquipmentID = deviceSignal.equipmentIdTemplate();

	m_equipmentID = deviceSignal.equipmentIdTemplate();

	if (deviceSignal.propertyExists(SignalProperties::appSignalIDTemplateCaption) == true)
	{
		m_appSignalID = expandDeviceSignalTemplate( deviceSignal,
													deviceSignal.propertyValue((SignalProperties::appSignalIDTemplateCaption)).toString(),
													errMsg);
		if (errMsg->isEmpty() == false)
		{
			return;
		}
	}
	else
	{
		m_appSignalID = QString("#%1").arg(deviceSignalEquipmentID);
	}

	if (deviceSignal.propertyExists(SignalProperties::customAppSignalIDTemplateCaption) == true)
	{
		// customSignalIDTemplate will be expand in compile time
		//
		m_customAppSignalID = deviceSignal.propertyValue(SignalProperties::customAppSignalIDTemplateCaption).toString();
	}
	else
	{
		m_customAppSignalID = deviceSignalEquipmentID;
	}

	if (deviceSignal.propertyExists(SignalProperties::appSignalCaptionTemplateCaption) == true)
	{
		m_caption = deviceSignal.propertyValue(SignalProperties::appSignalCaptionTemplateCaption).toString();
	}
	else
	{
		int pos = deviceSignalEquipmentID.lastIndexOf(QChar('_'));

		if (pos != -1)
		{
			deviceSignalEquipmentID = deviceSignalEquipmentID.mid(pos + 1);
		}

		m_caption = QString("Signal #%1").arg(deviceSignalEquipmentID);
	}
}

void Signal::checkAndInitTuningSettings(const Hardware::DeviceAppSignal& deviceSignal, QString* errMsg)
{
	if (deviceSignal.propertyExists(SignalProperties::enableTuningCaption) == false)
	{
		return;
	}

	if (deviceSignal.propertyExists(SignalProperties::tuningDefaultValueCaption) == false ||
		deviceSignal.propertyExists(SignalProperties::tuningLowBoundCaption) == false ||
		deviceSignal.propertyExists(SignalProperties::tuningHighBoundCaption) == false)
	{
		*errMsg = QString("Not all required properties for tuning settings initialization is exists in device signal %1").
						arg(deviceSignal.equipmentIdTemplate());
		return;
	}

	switch(deviceSignal.signalType())
	{
	case E::SignalType::Analog:
	case E::SignalType::Discrete:
		break;

	default:
		*errMsg = QString("Device signal %1 is not Analog or Discrete. Tuning is no allowed.").
						arg(deviceSignal.equipmentIdTemplate());
		return;
	}

	switch(deviceSignal.function())
	{
	case E::SignalFunction::Output:
		break;

	default:
		*errMsg = QString("Device signal %1 is not Output. Tuning is no allowed.").
						arg(deviceSignal.equipmentIdTemplate());
		return;
	}

	m_enableTuning = deviceSignal.propertyValue(SignalProperties::enableTuningCaption).toBool();

	m_tuningDefaultValue.setValue(	m_signalType,
									m_analogSignalFormat,
									deviceSignal.propertyValue(SignalProperties::tuningDefaultValueCaption));

	m_tuningLowBound.setValue(	m_signalType,
								m_analogSignalFormat,
								deviceSignal.propertyValue(SignalProperties::tuningLowBoundCaption));

	m_tuningHighBound.setValue(	m_signalType,
								m_analogSignalFormat,
								deviceSignal.propertyValue(SignalProperties::tuningHighBoundCaption));
}

QString Signal::specPropNotExistErr(const QString& propName) const
{
	return QString("Specific property %1 is not exists in signal %2").arg(m_appSignalID).arg(propName);
}

double Signal::getSpecPropDouble(const QString& name, QString* err) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum, err);

	if (result == false)
	{
		if (err != nullptr)
		{
			*err = specPropNotExistErr(name);
		}

		return 0;
	}

	assert(qv.type() == QVariant::Double && isEnum == false);

	return qv.toDouble();
}

int Signal::getSpecPropInt(const QString& name, QString* err) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum, err);

	if (result == false)
	{
		if (err != nullptr)
		{
			*err = specPropNotExistErr(name);
		}

		return 0;
	}

	assert(qv.type() == QVariant::Int && isEnum == false);

	return qv.toInt();
}

unsigned int Signal::getSpecPropUInt(const QString& name, QString* err) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum, err);

	if (result == false)
	{
		if (err != nullptr)
		{
			*err = specPropNotExistErr(name);
		}

		return 0;
	}

	assert(qv.type() == QVariant::UInt && isEnum == false);

	return qv.toUInt();
}


int Signal::getSpecPropEnum(const QString& name, QString* err) const
{
	QVariant qv;
	bool isEnum = false;

	bool result = getSpecPropValue(name, &qv, &isEnum, err);

	if (result == false)
	{
		if (err != nullptr)
		{
			*err = specPropNotExistErr(name);
		}

		return 0;
	}

	assert(qv.type() == QVariant::Int && isEnum == true);

	return qv.toInt();
}

bool Signal::getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum, QString* err) const
{
	TEST_PTR_RETURN_FALSE(qv);
	TEST_PTR_RETURN_FALSE(isEnum);

	bool result = false;

	if (m_cachedSpecPropValues != nullptr)
	{
		result = m_cachedSpecPropValues->getValue(name, qv, isEnum);
	}
	else
	{
		SignalSpecPropValues spv;

		bool res = spv.parseValuesFromArray(m_protoSpecPropValues);

		if (res == false)
		{
			if (err != nullptr)
			{
				*err = QString("Signal %1 specific properties values parsing error").arg(m_appSignalID);
			}

			result = false;
		}
		else
		{
			result = spv.getValue(name, qv, isEnum);
		}
	}

	return result;
}

bool Signal::isSpecPropExists(const QString& name) const
{
	if (m_cachedSpecPropValues != nullptr)
	{
		return m_cachedSpecPropValues->isExists(name);
	}

	SignalSpecPropValues spv;

	bool result = spv.parseValuesFromArray(m_protoSpecPropValues);

	if (result == false)
	{
		assert(false);
		return false;
	}

	return spv.isExists(name);
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
		result = spv.setEnumValue(name, qv.toInt());
	}
	else
	{
		result = spv.setValue(name, qv);
	}

	if (result == false)
	{
		assert(false);
		return false;
	}

	spv.serializeValuesToArray(&m_protoSpecPropValues);

	return true;
}

QStringList Signal::tags() const
{
	QStringList list;

	for(const QString& tag : m_tags)
	{
		list.append(tag);
	}

	return list;
}

void Signal::setTags(const QStringList& tags)
{
	clearTags();
	appendTags(tags);
}

void Signal::appendTag(const QString& tag)
{
	m_tags.insert(tag.toLower().trimmed());
}

void Signal::appendTags(const QStringList& tags)
{
	for(const QString& tag : tags)
	{
		appendTag(tag);
	}
}

void Signal::appendTags(const std::set<QString>& tags)
{
	for(const QString& tag : tags)
	{
		appendTag(tag);
	}
}

void Signal::removeTag(const QString& tag)
{
	m_tags.erase(tag.toLower().trimmed());
}

void Signal::removeTags(const QStringList& tags)
{
	for(const QString& tag : tags)
	{
		removeTag(tag);
	}
}

void Signal::removeTags(const std::set<QString>& tags)
{
	for(const QString& tag : tags)
	{
		removeTag(tag);
	}
}

void Signal::clearTags()
{
	m_tags.clear();
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
			updateID2IndexInMap(s.appSignalID(), i);
		}
	}
}

void SignalSet::updateID2IndexInMap(const QString& appSignalId, int index)
{
	m_strID2IndexMap.insert(appSignalId, index);
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
	if (signalID > m_maxID)
	{
		m_maxID = signalID;
	}

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

void SignalSet::append(Signal* signal)
{
	int newID = getMaxID() + 1;

	append(newID, signal);
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

	bool result = protoAppSignalSet.ParseFromArray(fileData.constData(), fileData.size());

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

int SignalSet::getMaxID()
{
	if (m_maxID >= 0)
	{
		return m_maxID;
	}

	int count = SignalPtrOrderedHash::count();

	m_maxID = -1;

	for(int i = 0; i < count; i++)
	{
		int keyI = key(i);

		if (keyI > m_maxID)
		{
			m_maxID = keyI;
		}
	}

	return m_maxID;
}

QStringList SignalSet::appSignalIdsList(bool removeNumberSign, bool sort) const
{
	QStringList result;
	result.reserve(count());

	for (int i = 0; i < count(); i++)
	{
		const Signal& signal = operator[](i);
		const QString& appSignalId = signal.appSignalID();

		if (removeNumberSign == false ||
			appSignalId.isEmpty() == true ||
			appSignalId.at(0) != QChar('#'))
		{
			result.push_back(appSignalId);
		}
		else
		{
			QString chooped = appSignalId;
			result.push_back(chooped.remove(0, 1));
		}
	}

	if (sort == true)
	{
		std::sort(result.begin(), result.end());
	}

	return result;
}

void SignalSet::replaceOrAppendIfNotExists(int signalID, const Signal& s)
{
	Signal* existsSignal = valuePtr(signalID);

	if (existsSignal != nullptr)
	{
		*existsSignal = s;
	}
	else
	{
		append(signalID, new Signal(s));
	}

	m_strID2IndexMap.insert(s.appSignalID(), keyIndex(signalID));
}
