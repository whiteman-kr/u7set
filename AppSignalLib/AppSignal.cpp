#ifndef APP_SIGNAL_LIB_DOMAIN
#error Do not include this file in the project! Link AppSignalLib instead.
#endif

#include "AppSignal.h"
#include "AppSignalSpecPropValues.h"
#include "../UtilsLib/XmlHelper.h"
#include <CommonLib/ConstStrings.h>

template<typename ENUM_TYPE>
void writeEnumValueStrSpecPropAttribute(XmlWriteHelper& xml, const AppSignal& s,
									   const QString& propName, const QString& attributeName)
{
	QVariant v;
	bool isEnum = false;
	bool res = s.getSpecPropValue(propName, &v, &isEnum, nullptr);

	if (res == true)
	{
		ENUM_TYPE enumValue = static_cast<ENUM_TYPE>(v.toInt());

		xml.writeStringAttribute(attributeName.isEmpty() == true ? propName : attributeName, E::valueToString<ENUM_TYPE>(enumValue));
	}
	else
	{
		xml.writeStringAttribute(attributeName.isEmpty() == true ? propName : attributeName, QString());
	}
}

// --------------------------------------------------------------------------------------------------------
//
// AppSignal class implementation
//
// --------------------------------------------------------------------------------------------------------

const QString AppSignal::CAPTION_VALIDATOR("^.+$");
const QString AppSignal::IDENTIFICATORS_VALIDATOR("^[#]?[A-Za-z\\d_]*$");

AppSignal::AppSignal()
{
	updateTuningValuesType();
}

AppSignal::AppSignal(const AppSignal& s)
{
	*this = s;
}

AppSignal::AppSignal(const ID_AppSignalID& ids)
{
	m_ID = ids.ID;
	m_signalGroupID = ids.signalGroupID;
	m_appSignalID = ids.appSignalID;

	m_loaded = false;
}

AppSignal::AppSignal(const Proto::AppSignal& proto)
{
	loadFromProto(proto);
}

AppSignal::~AppSignal()
{
}

QString AppSignal::initFromDeviceSignal(const QString& deviceSignalEquipmentID,
										E::SignalType deviceSignalType,
										E::SignalFunction deviceSignalFunction,
										const QString& appSignalID,
										const QString& customAppSignalID,
										const QString& appSignalCaption,
										const QString& appSignalBusTypeID,
										E::AnalogAppSignalFormat analogAppSignalFormat,
										const QString& appSignalSpecPropsStruct,
										bool enableTuning,
										const QVariant& tuningLowBound,
										const QVariant& tuningHighBound,
										const QVariant& tuningDefaultValue)
{
	m_equipmentID = deviceSignalEquipmentID;
	m_appSignalID = appSignalID;
	m_customAppSignalID = customAppSignalID;
	m_caption = appSignalCaption;

	//

	m_signalType = deviceSignalType;
	m_analogSignalFormat = analogAppSignalFormat;

	switch(m_signalType)
	{
	case E::SignalType::Analog:

		switch(m_analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			m_dataSize = FLOAT32_SIZE;
			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			m_dataSize = SIGNED_INT32_SIZE;
			break;

		default:
			Q_ASSERT(false);
			return QString("Unknown E::AnalogAppSignalFormat");
		}

		initTuningValues();

		break;

	case E::SignalType::Discrete:

		m_dataSize = DISCRETE_SIZE;

		initTuningValues();

		break;

	case E::SignalType::Bus:

		m_busTypeID = appSignalBusTypeID;

		break;

	default:

		Q_ASSERT(false);
		return QString("Unknown device signal E::SignalType");
	}

	m_swCalcFunction = E::SoftwareCalcFunction::None;

	switch(deviceSignalFunction)
	{
	case E::SignalFunction::Input:
	case E::SignalFunction::Validity:
		m_inOutType = E::SignalInOutType::Input;
		break;

	case E::SignalFunction::Output:
		m_inOutType = E::SignalInOutType::Output;
		break;

	case E::SignalFunction::Diagnostics:

		Q_ASSERT(false);
		return QString("Can not create AppSignal from diagnostics device signal");

	case E::SignalFunction::SoftwareCalculated:
		{
			static const std::map<QString, E::SoftwareCalcFunction> suffixToFunction =
			{
				{ EquipmentPropNames::SC_FBLOCK_COUNT_SUFFIX, E::SoftwareCalcFunction::BlockFlagsCount },
				{ EquipmentPropNames::SC_FSIM_COUNT_SUFFIX, E::SoftwareCalcFunction::SimFlagsCount },
				{ EquipmentPropNames::SC_FMISMATCH_COUNT_SUFFIX, E::SoftwareCalcFunction::MismatchFlagsCount }
			};

			for(const auto& [suffix, func] : suffixToFunction)
			{
				if (deviceSignalEquipmentID.endsWith(suffix) == true)
				{
					m_swCalcFunction = func;
					break;
				}
			}

			if (m_swCalcFunction == E::SoftwareCalcFunction::None)
			{
				return QString("Unknown software calculetd function of signal %1").arg(appSignalID);
			}

			m_inOutType = E::SignalInOutType::SoftwareCalculated;

			switch(m_swCalcFunction)
			{
			case E::SoftwareCalcFunction::BlockFlagsCount:
			case E::SoftwareCalcFunction::SimFlagsCount:
			case E::SoftwareCalcFunction::MismatchFlagsCount:
				m_apertureType = E::ApertureType::AbsValue;
				m_coarseAperture = 1;
				m_fineAperture = 1;
				m_decimalPlaces = 0;
				break;

			default:
				Q_ASSERT(false);
			}

			break;
		}

	default:

		Q_ASSERT(false);
		return QString("Unknown device signal E::SignalFunction");
	}

	// specific properties processing
	//
	m_specPropStruct = appSignalSpecPropsStruct;

	if (m_specPropStruct.contains(AppSignalPropNames::MISPRINT_lowEngineeringUnitsCaption) ||
		m_specPropStruct.contains(AppSignalPropNames::MISPRINT_highEngineeringUnitsCaption))
	{
		return QString("Misprinted signal specific properties HighEngEneeringUnits/LowEngEneeringUnits has detected in device signal %1. \n\n"
					 "Update module preset first. \n\nApplication signal creation is aborted!").
										arg(deviceSignalEquipmentID);
	}

	AppSignalSpecPropValues spv;

	spv.createFromSpecPropStruct(m_specPropStruct);

	spv.serializeValuesToArray(&m_protoSpecPropValues);

	//

	m_enableTuning = enableTuning;

	m_tuningDefaultValue.setValue(	m_signalType,
									m_analogSignalFormat,
									tuningDefaultValue);

	m_tuningLowBound.setValue(	m_signalType,
								m_analogSignalFormat,
								tuningLowBound);

	m_tuningHighBound.setValue(	m_signalType,
								m_analogSignalFormat,
								tuningHighBound);
	return QString();
}

void AppSignal::clear()
{
	*this = AppSignal();
}

void AppSignal::initSpecificProperties()
{
	QString specPropStruct;

	switch(m_signalType)
	{
	case E::SignalType::Analog:

		switch(m_inOutType)
		{
		case E::SignalInOutType::Input:
			specPropStruct = AppSignalDefaultSpecPropStruct::INPUT_ANALOG;
			break;

		case E::SignalInOutType::Output:
			specPropStruct = AppSignalDefaultSpecPropStruct::OUTPUT_ANALOG;
			break;

		case E::SignalInOutType::Internal:
			specPropStruct = AppSignalDefaultSpecPropStruct::INTERNAL_ANALOG;
			break;

		case E::SignalInOutType::SoftwareCalculated:
			break;

		default:
			assert(false);
		}

		break;

	case E::SignalType::Discrete:
		break;

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

void AppSignal::setSignalType(E::SignalType type)
{
	m_signalType = type;
	updateTuningValuesType();
}

E::SoftwareCalcFunction AppSignal::swCalcFunction() const
{
	return m_swCalcFunction;
}

void AppSignal::setSwCalcFunction(E::SoftwareCalcFunction func)
{
	m_swCalcFunction = func;
}

void AppSignal::setDataSizeW(int sizeW)
{
	m_dataSize = sizeW * SIZE_16BIT;
}

void AppSignal::setDataSizeByType(E::SignalType type, E::AnalogAppSignalFormat analogFormat)
{
	switch(type)
	{
	case E::SignalType::Discrete:
		m_dataSize = SIZE_1BIT;
		break;

	case E::SignalType::Analog:

		switch(analogFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			m_dataSize = FLOAT32_SIZE;
			break;
		case E::AnalogAppSignalFormat::SignedInt32:
			m_dataSize = SIGNED_INT32_SIZE;
			break;

		default:
			Q_ASSERT(false);
		}

		break;

	case E::SignalType::Bus:
		m_dataSize = 0;					// size is defined by BusType
		break;

	default:
		Q_ASSERT(false);
	}
}

void AppSignal::setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat)
{
	m_analogSignalFormat = dataFormat;

	updateTuningValuesType();
}

E::DataFormat AppSignal::dataFormat() const
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

bool AppSignal::isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const
{
	if (signalType == E::SignalType::Bus)
	{
		assert(false);			// use isCompatibleFormat(signalType, busTtypeID)
		return false;
	}

	return isCompatibleFormatPrivate(signalType, dataFormat, size, byteOrder, "");
}

bool AppSignal::isCompatibleFormat(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, E::ByteOrder byteOrder) const
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

bool AppSignal::isCompatibleFormat(const SignalAddress16& sa16) const
{
	return isCompatibleFormatPrivate(sa16.signalType(), sa16.dataFormat(), sa16.dataSize(), sa16.byteOrder(), "");
}

bool AppSignal::isCompatibleFormat(const AppSignal& s) const
{
	if (s.signalType() == E::SignalType::Bus)
	{
		return isCompatibleFormat(E::SignalType::Bus, s.busTypeID());
	}

	return isCompatibleFormat(s.signalType(), s.analogSignalFormat(), s.byteOrder());
}

bool AppSignal::isCompatibleFormat(E::SignalType signalType, const QString& busTypeID) const
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

bool AppSignal::invertSignal() const
{
	return m_invertSignal;
}

void AppSignal::setInvertSignal(bool invert)
{
	m_invertSignal = invert;
}

bool AppSignal::reserved() const
{
	return m_reserved;
}

void AppSignal::setReserved(bool reserved)
{
	m_reserved = reserved;
}

int AppSignal::lowADC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(AppSignalPropNames::LOW_ADC, err));
}

void AppSignal::setLowADC(int lowADC)
{
	setSpecPropUInt(AppSignalPropNames::LOW_ADC, static_cast<unsigned int>(lowADC));
}

int AppSignal::highADC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(AppSignalPropNames::HIGH_ADC, err));
}

void AppSignal::setHighADC(int highADC)
{
	setSpecPropUInt(AppSignalPropNames::HIGH_ADC, static_cast<unsigned int>(highADC));
}

int AppSignal::lowDAC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(AppSignalPropNames::LOW_DAC, err));
}

void AppSignal::setLowDAC(int lowDAC)
{
	setSpecPropInt(AppSignalPropNames::LOW_DAC, lowDAC);
}

int AppSignal::highDAC(QString* err) const
{
	return static_cast<int>(getSpecPropUInt(AppSignalPropNames::HIGH_DAC, err));
}

void AppSignal::setHighDAC(int highDAC)
{
	setSpecPropInt(AppSignalPropNames::HIGH_DAC, highDAC);
}

double AppSignal::lowEngineeringUnits(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::LOW_ENGINEERING_UNITS, err);
}

void AppSignal::setLowEngineeringUnits(double lowEngineeringUnits)
{
	setSpecPropDouble(AppSignalPropNames::LOW_ENGINEERING_UNITS, lowEngineeringUnits);
}

double AppSignal::highEngineeringUnits(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::HIGH_ENGINEERING_UNITS, err);
}

void AppSignal::setHighEngineeringUnits(double highEngineeringUnits)
{
	setSpecPropDouble(AppSignalPropNames::HIGH_ENGINEERING_UNITS, highEngineeringUnits);
}

double AppSignal::lowPhysicalUnits(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::LOW_PHYSICAL_UNITS, err);
}

void AppSignal::setLowPhysicalUnits(double lowPhUnits)
{
	setSpecPropDouble(AppSignalPropNames::LOW_PHYSICAL_UNITS, lowPhUnits);
}

double AppSignal::highPhysicalUnits(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::HIGH_PHYSICAL_UNITS, err);
}

void AppSignal::setHighPhysicalUnits(double highPhUnits)
{
	setSpecPropDouble(AppSignalPropNames::HIGH_PHYSICAL_UNITS, highPhUnits);
}

bool AppSignal::isReverseEngineeringLimits() const
{
	return lowEngineeringUnits() > highEngineeringUnits();
}

double AppSignal::lowValidRange(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::LOW_VALID_RANGE, err);
}

void AppSignal::setLowValidRange(double lowValidRange)
{
	setSpecPropDouble(AppSignalPropNames::LOW_VALID_RANGE, lowValidRange);
}

double AppSignal::highValidRange(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::HIGH_VALID_RANGE, err);
}

void AppSignal::setHighValidRange(double highValidRange)
{
	setSpecPropDouble(AppSignalPropNames::HIGH_VALID_RANGE, highValidRange);
}

double AppSignal::filteringTime(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::FILTERING_TIME, err);
}

void AppSignal::setFilteringTime(double filteringTime)
{
	setSpecPropDouble(AppSignalPropNames::FILTERING_TIME, filteringTime);
}

double AppSignal::spreadTolerance(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::SPREAD_TOLERANCE, err);
}

void AppSignal::setSpreadTolerance(double spreadTolerance)
{
	setSpecPropDouble(AppSignalPropNames::SPREAD_TOLERANCE, spreadTolerance);
}

double AppSignal::electricLowLimit(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::ELECTRIC_LOW_LIMIT, err);
}

void AppSignal::setElectricLowLimit(double electricLowLimit)
{
	setSpecPropDouble(AppSignalPropNames::ELECTRIC_LOW_LIMIT, electricLowLimit);
}

double AppSignal::electricHighLimit(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::ELECTRIC_HIGH_LIMIT, err);
}

void AppSignal::setElectricHighLimit(double electricHighLimit)
{
	setSpecPropDouble(AppSignalPropNames::ELECTRIC_HIGH_LIMIT, electricHighLimit);
}

E::ElectricUnit AppSignal::electricUnit(QString* err) const
{
	return static_cast<E::ElectricUnit>(getSpecPropEnum(AppSignalPropNames::ELECTRIC_UNIT, err));
}

void AppSignal::setElectricUnit(E::ElectricUnit electricUnit)
{
	setSpecPropEnum(AppSignalPropNames::ELECTRIC_UNIT, static_cast<int>(electricUnit));
}

double AppSignal::rloadOhm(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::RLOAD_OHM, err);
}

void AppSignal::setRloadOhm(double rload_Ohm)
{
	setSpecPropDouble(AppSignalPropNames::RLOAD_OHM, rload_Ohm);
}

E::SensorType AppSignal::sensorType(QString* err) const
{
	return static_cast<E::SensorType>(getSpecPropEnum(AppSignalPropNames::SENSOR_TYPE, err));
}

void AppSignal::setSensorType(E::SensorType sensorType)
{
	setSpecPropEnum(AppSignalPropNames::SENSOR_TYPE, static_cast<int>(sensorType));
}

E::OutputMode AppSignal::outputMode(QString* err) const
{
	return static_cast<E::OutputMode>(getSpecPropEnum(AppSignalPropNames::OUTPUT_MODE, err));
}

void AppSignal::setOutputMode(E::OutputMode outputMode)
{
	setSpecPropEnum(AppSignalPropNames::OUTPUT_MODE, static_cast<int>(outputMode));
}

double AppSignal::r0_Ohm(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::R0_OHM, err);
}

void AppSignal::setR0_Ohm(double r0_Ohm)
{
	setSpecPropDouble(AppSignalPropNames::R0_OHM, r0_Ohm);
}

void AppSignal::setSpecPropStruct(const QString& specPropsStruct)
{
	m_specPropStruct = specPropsStruct;
	m_specPropStructHash = calcHash(m_specPropStruct);
}

Hash AppSignal::specPropStructHash() const
{
	if (m_specPropStructHash == 0)
	{
		m_specPropStructHash = calcHash(m_specPropStruct);
	}

	return m_specPropStructHash;
}

bool AppSignal::createSpecPropValues()
{
	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(m_specPropStruct);

	if (result.first == false)
	{
		assert(false);
		return false;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	AppSignalSpecPropValues spValues;

	for(const std::shared_ptr<Property>& specificProperty : specificProperties)
	{
		AppSignalSpecPropValue spValue;

		spValue.create(specificProperty);

		spValues.append(spValue);
	}

	spValues.serializeValuesToArray(&m_protoSpecPropValues);

	return true;
}

void AppSignal::cacheSpecPropValues() const
{
	if (m_cachedSpecPropValues == nullptr)
	{
		m_cachedSpecPropValues = std::make_shared<AppSignalSpecPropValues>();
	}

	m_cachedSpecPropValues->parseValuesFromArray(m_protoSpecPropValues);
}

void AppSignal::saveProtoData(QByteArray* protoDataArray) const
{
	TEST_PTR_RETURN(protoDataArray);

	Proto::ProtoAppSignalData protoData;

	saveProtoData(&protoData);

	protoDataArray->resize(static_cast<int>(protoData.ByteSizeLong()));

	protoData.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(protoDataArray->data()));
}

void AppSignal::saveProtoData(Proto::ProtoAppSignalData* protoData) const
{
	TEST_PTR_RETURN(protoData);

	protoData->Clear();

	protoData->set_bustypeid(m_busTypeID.toStdString());
	protoData->set_caption(m_caption.toStdString());
	protoData->set_channel(TO_INT(m_channel));
	protoData->set_excludefrombuild(m_excludeFromBuild);

	protoData->set_datasize(m_dataSize);
	protoData->set_byteorder(TO_INT(m_byteOrder));

	protoData->set_analogsignalformat(TO_INT(m_analogSignalFormat));
	protoData->set_unit(m_unit.toStdString());

	protoData->set_enabletuning(m_enableTuning);
	m_tuningDefaultValue.save(protoData->mutable_tuningdefaultvalue());
	m_tuningLowBound.save(protoData->mutable_tuninglowbound());
	m_tuningHighBound.save(protoData->mutable_tuninghighbound());

	protoData->set_acquire(m_acquire);
	protoData->set_archive(m_archive);
	protoData->set_log(m_log);
	protoData->set_decimalplaces(m_decimalPlaces);
	protoData->set_coarseaperture(m_coarseAperture);
	protoData->set_fineaperture(m_fineAperture);
	protoData->set_aperturetype(TO_INT(m_apertureType));
	protoData->set_invertsignal(m_invertSignal);
	protoData->set_reserved(m_reserved);
	protoData->set_swcalcfunction(TO_INT(m_swCalcFunction));

	//

	protoData->set_tags(tagsStr().toStdString());
}

void AppSignal::loadProtoData(const char* protoDataPtr, int protoDataSize)
{
	Proto::ProtoAppSignalData protoData;

	bool res = protoData.ParseFromArray(protoDataPtr, protoDataSize);

	if (res == false)
	{
		Q_ASSERT(false);
		return;
	}

	//

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

	updateTuningValuesType();			// For correction of bug RPCT-3324 consequences.
										// In some databases in proto data were saved wrong tuning value types.

	m_acquire = protoData.acquire();
	m_archive = protoData.archive();
	m_log = protoData.log();
	m_decimalPlaces = protoData.decimalplaces();
	m_coarseAperture = protoData.coarseaperture();
	m_fineAperture = protoData.fineaperture();
	m_apertureType = static_cast<E::ApertureType>(protoData.aperturetype());
	m_invertSignal = protoData.invertsignal();
	m_reserved = protoData.reserved();
	m_swCalcFunction = static_cast<E::SoftwareCalcFunction>(protoData.swcalcfunction());

	//

	setTagsStr(QString::fromStdString(protoData.tags()));
}

void AppSignal::loadProtoData(const QByteArray& protoDataArray)
{
	loadProtoData(protoDataArray.constData(), static_cast<int>(protoDataArray.size()));
}

QDateTime AppSignal::created() const
{
	return QDateTime::fromMSecsSinceEpoch(m_createdMcs / 1000);;
}

QDateTime AppSignal::instanceCreated() const
{
	return QDateTime::fromMSecsSinceEpoch(m_instanceCreatedMcs / 1000);;
}

Hash AppSignal::hash() const
{
	if (m_hash == 0)
	{
		m_hash = calcHash(m_appSignalID);
	}

	return m_hash;
}

Address16 AppSignal::ioBufAddr() const
{
	return m_ioBufAddr;
}

void AppSignal::setIoBufAddr(const Address16& addr)
{
	m_ioBufAddr = addr;
}

Address16 AppSignal::actualAddr(E::LogicModuleRamAccess* lmRamAccess) const
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

void AppSignal::resetAddresses()
{
	m_ioBufAddr.reset();
	m_tuningAddr.reset();
	m_tuningAbsAddr.reset();
	m_ualAddr.reset();
	m_regValueAddr.reset();
	m_regValidityAddr.reset();
}

QString AppSignal::regValueAddrStr() const
{
	return QString("(reg %1:%2)").arg(regValueAddr().offset()).arg(regValueAddr().bit());
}


void AppSignal::writeToAzpzXml(XmlWriteHelper& xml) const
{
	//
	// Writing AppSignals.xml for old AZPZ software
	//

	xml.writeStartElement("Signal");	// <Signal>

	xml.writeIntAttribute("ID", ID());
	xml.writeIntAttribute("GroupID", signalGroupID());
	xml.writeIntAttribute("InstanceID", signalInstanceID());
	xml.writeIntAttribute("Channel", TO_INT(channel()));
	xml.writeIntAttribute("Type", TO_INT(signalType()));
	xml.writeStringAttribute("AppSignalID", appSignalID());
	xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
	xml.writeStringAttribute("Caption", caption());
	xml.writeStringAttribute("EquipmentID", equipmentID());
	xml.writeIntAttribute("DataFormat", TO_INT(analogSignalFormat()));
	xml.writeIntAttribute("DataSize", dataSize());

	writeIntSpecPropAttribute(xml, AppSignalPropNames::LOW_ADC);
	writeIntSpecPropAttribute(xml, AppSignalPropNames::HIGH_ADC);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::LOW_ENGINEERING_UNITS);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::HIGH_ENGINEERING_UNITS);
	xml.writeIntAttribute("UnitID", 0);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::LOW_VALID_RANGE);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::HIGH_VALID_RANGE);
	xml.writeDoubleAttribute("UnbalanceLimit", 1);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_LOW_LIMIT, "InputLowLimit");
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_HIGH_LIMIT, "InputHighLimit");
	writeIntSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_UNIT, "InputUnitID");
	writeIntSpecPropAttribute(xml, AppSignalPropNames::SENSOR_TYPE, "InputSensorID");
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_LOW_LIMIT, "OutputLowLimit");
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_HIGH_LIMIT, "OutputHighLimit");
	writeIntSpecPropAttribute(xml, AppSignalPropNames::ELECTRIC_UNIT, "OutputUnitID");

	writeIntSpecPropAttribute(xml, AppSignalPropNames::OUTPUT_MODE);

	writeIntSpecPropAttribute(xml, AppSignalPropNames::SENSOR_TYPE, "OutputSensorID");
	xml.writeBoolAttribute("Acquire", acquire());
	xml.writeBoolAttribute("Calculated", false);
	xml.writeIntAttribute("NormalState", 0);
	xml.writeIntAttribute("DecimalPlaces", decimalPlaces());
	xml.writeDoubleAttribute("Aperture", coarseAperture());
	xml.writeIntAttribute("InOutType", TO_INT(inOutType()));
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::FILTERING_TIME);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::SPREAD_TOLERANCE);
	xml.writeIntAttribute("ByteOrder", TO_INT(byteOrder()));

	xml.writeBoolAttribute("EnableTuning", enableTuning());
	xml.writeStringAttribute("TuningValueType", tuningDefaultValue().typeStr());
	xml.writeStringAttribute("TuningDefaultValue", tuningDefaultValue().toString());
	xml.writeStringAttribute("TuningLowBound", tuningLowBound().toString());
	xml.writeStringAttribute("TuningHighBound", tuningHighBound().toString());

	xml.writeStringAttribute("BusTypeID", busTypeID());
	xml.writeBoolAttribute("AdaptiveAperture", (apertureType() == E::ApertureType::ValuePercent));

	xml.writeIntAttribute("RamAddrOffset", ualAddr().offset());
	xml.writeIntAttribute("RamAddrBit", ualAddr().bit());
	xml.writeIntAttribute("ValueOffset", regValueAddr().offset());
	xml.writeIntAttribute("ValueBit", regValueAddr().bit());
	xml.writeIntAttribute("ValidityOffset", regValidityAddr().offset());
	xml.writeIntAttribute("ValidityBit", regValidityAddr().bit());

	xml.writeIntAttribute("TuningOffset", tuningAddr().offset());
	xml.writeIntAttribute("TuningBit", tuningAddr().bit());

	// write spec properties

	// xml.writeStringAttribute("SpecPropStruct", specPropStruct());
	// xml.writeStringAttribute("SpecPropValues", QString(protoSpecPropValues().toHex()));

	xml.writeEndElement();				// </Signal>
}

void AppSignal::writeDoubleSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName) const
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
		xml.writeStringAttribute(attributeName.isEmpty() == true ? propName : attributeName, QString());
	}
}

void AppSignal::writeIntSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName) const
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
		xml.writeStringAttribute(attributeName.isEmpty() == true ? propName : attributeName, QString());
	}
}

void AppSignal::writeToXml(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::SIGNAL_ELEM);	// <Signal>

	// Identification
	//
	xml.writeIntAttribute(AppSignalPropNames::ID, m_ID);

	xml.writeStringAttribute(AppSignalPropNames::APP_SIGNAL_ID, m_appSignalID);
	xml.writeStringAttribute(AppSignalPropNames::CUSTOM_APP_SIGNAL_ID, m_customAppSignalID);
	xml.writeStringAttribute(AppSignalPropNames::CAPTION, m_caption);
	xml.writeStringAttribute(AppSignalPropNames::EQUIPMENT_ID, m_equipmentID);
	xml.writeEnumKeyValueAttribute(AppSignalPropNames::CHANNEL, m_channel);

	xml.writeIntAttribute(AppSignalPropNames::SIGNAL_GROUP_ID, m_signalGroupID);
	xml.writeIntAttribute(AppSignalPropNames::SIGNAL_INSTANCE_ID, m_signalInstanceID);

	// Type
	//
	xml.writeEnumKeyValueAttribute(AppSignalPropNames::TYPE, m_signalType);
	xml.writeEnumKeyValueAttribute(AppSignalPropNames::IN_OUT_TYPE, m_inOutType);

	// Data format
	//
	xml.writeEnumKeyValueAttribute(AppSignalPropNames::BYTE_ORDER_PROP, m_byteOrder);
	xml.writeIntAttribute(AppSignalPropNames::DATA_SIZE, m_dataSize);
	xml.writeEnumKeyValueAttribute(AppSignalPropNames::ANALOG_SIGNAL_FORMAT, m_analogSignalFormat);
	xml.writeStringAttribute(AppSignalPropNames::BUS_TYPE_ID, m_busTypeID);

	// MATS and processing props
	//
	xml.writeBoolAttribute(AppSignalPropNames::INVERT_SIGNAL, m_invertSignal);
	xml.writeBoolAttribute(AppSignalPropNames::ACQUIRE, m_acquire);
	xml.writeBoolAttribute(AppSignalPropNames::ARCHIVE, m_archive);
	xml.writeBoolAttribute(AppSignalPropNames::LOG, m_log);
	xml.writeBoolAttribute(AppSignalPropNames::RESERVED, m_reserved);

	xml.writeEnumKeyValueAttribute(AppSignalPropNames::APERTURE_TYPE, m_apertureType);
	xml.writeDoubleAttribute(AppSignalPropNames::FINE_APERTURE, m_fineAperture);
	xml.writeDoubleAttribute(AppSignalPropNames::COARSE_APERTURE, m_coarseAperture);

	xml.writeStringAttribute(AppSignalPropNames::UNIT, m_unit);
	xml.writeIntAttribute(AppSignalPropNames::DECIMAL_PLACES, m_decimalPlaces);
	xml.writeStringAttribute(AppSignalPropNames::TAGS, tags().join(Separator::COMMA));

	// Addresses
	//
	xml.writeAddress16Attribute(AppSignalPropNames::UAL_ADDR, m_ualAddr);
	xml.writeAddress16Attribute(AppSignalPropNames::REG_VALUE_ADDR, m_regValueAddr);
	xml.writeAddress16Attribute(AppSignalPropNames::REG_VALIDITY_ADDR, m_regValidityAddr);

	// Tuning
	//
	xml.writeBoolAttribute(AppSignalPropNames::ENABLE_TUNING, m_enableTuning);

	if (m_enableTuning == true)
	{
		xml.writeIntAttribute(AppSignalPropNames::TUNING_VALUE_TYPE, TO_INT(m_tuningDefaultValue.type()));
		xml.writeStringAttribute(AppSignalPropNames::TUNING_VALUE_TYPE_STR, m_tuningDefaultValue.typeStr());

		xml.writeStringAttribute(AppSignalPropNames::TUNING_DEFAULT_VALUE, tuningDefaultValue().toString());
		xml.writeUInt64Attribute(AppSignalPropNames::TUNING_DEFAULT_VALUE_HEX, tuningDefaultValue().bitCastUint64Value(), true);

		xml.writeStringAttribute(AppSignalPropNames::TUNING_LOW_BOUND, tuningLowBound().toString());
		xml.writeUInt64Attribute(AppSignalPropNames::TUNING_LOW_BOUND_HEX, tuningLowBound().bitCastUint64Value(), true);

		xml.writeStringAttribute(AppSignalPropNames::TUNING_HIGH_BOUND, tuningHighBound().toString());
		xml.writeUInt64Attribute(AppSignalPropNames::TUNING_HIGH_BOUND_HEX, tuningHighBound().bitCastUint64Value(), true);

		xml.writeAddress16Attribute(AppSignalPropNames::TUNING_ADDR, m_tuningAddr);
		xml.writeAddress16Attribute(AppSignalPropNames::TUNING_ABS_ADDR, m_tuningAddr);
	}

	// Specific properties
	//
	xml.writeStringAttribute(AppSignalPropNames::SPEC_PROP_STRUCT, m_specPropStruct);

	cacheSpecPropValues();

	for(const AppSignalSpecPropValue& spv :  m_cachedSpecPropValues->values())
	{
		if (spv.isEnum() == false)
		{
			xml.writeQVariantAttribute(spv.name(), spv.value());
		}
		else
		{
			QString name = spv.name();

			if (name == AppSignalPropNames::ELECTRIC_UNIT)
			{
				E::ElectricUnit e = static_cast<E::ElectricUnit>(spv.value().toInt());
				xml.writeEnumKeyValueAttribute(name, e);
				continue;
			}

			if (name == AppSignalPropNames::SENSOR_TYPE)
			{
				E::SensorType e = static_cast<E::SensorType>(spv.value().toInt());
				xml.writeEnumKeyValueAttribute(name, e);
				continue;
			}

			if (name == AppSignalPropNames::OUTPUT_MODE)
			{
				E::OutputMode e = static_cast<E::OutputMode>(spv.value().toInt());
				xml.writeEnumKeyValueAttribute(name, e);
				continue;
			}

			xml.writeQVariantAttribute(spv.name(), spv.value());
		}
	}

	xml.writeEndElement();				// </Signal>
}

bool AppSignal::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	if (xml.name() != XmlElement::SIGNAL_ELEM)
	{
		return false;
	}

	resetAddresses();

	// Identification
	//
	result &= xml.readIntAttribute(AppSignalPropNames::ID, &m_ID);

	result &= xml.readStringAttribute(AppSignalPropNames::APP_SIGNAL_ID, &m_appSignalID);
	result &= xml.readStringAttribute(AppSignalPropNames::CUSTOM_APP_SIGNAL_ID, &m_customAppSignalID);
	result &= xml.readStringAttribute(AppSignalPropNames::CAPTION, &m_caption);
	result &= xml.readStringAttribute(AppSignalPropNames::EQUIPMENT_ID, &m_equipmentID);
	result &= xml.readEnumValueAttribute(AppSignalPropNames::CHANNEL, &m_channel);

	result &= xml.readIntAttribute(AppSignalPropNames::SIGNAL_GROUP_ID, &m_signalGroupID);
	result &= xml.readIntAttribute(AppSignalPropNames::SIGNAL_INSTANCE_ID, &m_signalInstanceID);

	// Type
	//
	result &= xml.readEnumValueAttribute(AppSignalPropNames::TYPE, &m_signalType);
	result &= xml.readEnumValueAttribute(AppSignalPropNames::IN_OUT_TYPE, &m_inOutType);

	// Data format
	//
	result &= xml.readEnumValueAttribute(AppSignalPropNames::BYTE_ORDER_PROP, &m_byteOrder);
	result &= xml.readIntAttribute(AppSignalPropNames::DATA_SIZE, &m_dataSize);
	result &= xml.readEnumValueAttribute(AppSignalPropNames::ANALOG_SIGNAL_FORMAT, &m_analogSignalFormat);
	result &= xml.readStringAttribute(AppSignalPropNames::BUS_TYPE_ID, &m_busTypeID);

	// MATS and processing props
	//
	result &= xml.readBoolAttribute(AppSignalPropNames::INVERT_SIGNAL, &m_invertSignal);
	result &= xml.readBoolAttribute(AppSignalPropNames::ACQUIRE, &m_acquire);
	result &= xml.readBoolAttribute(AppSignalPropNames::ARCHIVE, &m_archive);
	result &= xml.readBoolAttribute(AppSignalPropNames::LOG, &m_log);
	result &= xml.readBoolAttribute(AppSignalPropNames::RESERVED, &m_reserved);

	m_apertureType = E::ApertureType::RangePercent;
	m_fineAperture = 0;
	m_coarseAperture = 0;
	m_decimalPlaces = 0;
	m_unit.clear();

	result &= xml.readEnumValueAttribute(AppSignalPropNames::APERTURE_TYPE, &m_apertureType);
	result &= xml.readDoubleAttribute(AppSignalPropNames::FINE_APERTURE, &m_fineAperture);
	result &= xml.readDoubleAttribute(AppSignalPropNames::COARSE_APERTURE, &m_coarseAperture);

	result &= xml.readStringAttribute(AppSignalPropNames::UNIT, &m_unit);
	result &= xml.readIntAttribute(AppSignalPropNames::DECIMAL_PLACES, &m_decimalPlaces);

	QString tagsStr;

	result &= xml.readStringAttribute(AppSignalPropNames::TAGS, &tagsStr);

	setTags(tagsStr.split(Separator::COMMA, Qt::SkipEmptyParts));

	// Addresses
	//
	result &= xml.readAddress16Attribute(AppSignalPropNames::UAL_ADDR, &m_ualAddr);
	result &= xml.readAddress16Attribute(AppSignalPropNames::REG_VALUE_ADDR, &m_regValueAddr);
	result &= xml.readAddress16Attribute(AppSignalPropNames::REG_VALIDITY_ADDR, &m_regValidityAddr);

	// Tuning
	//
	result &= xml.readBoolAttribute(AppSignalPropNames::ENABLE_TUNING, &m_enableTuning);

	if (m_enableTuning == true)
	{
		int v = 0;
		result &= xml.readIntAttribute(AppSignalPropNames::TUNING_VALUE_TYPE, &v);

		TuningValueType tvt = static_cast<TuningValueType>(v);

		updateTuningValuesType();

		Q_ASSERT(m_tuningDefaultValue.type() == tvt);

		quint64 v64;

		result &= xml.readUInt64Attribute(AppSignalPropNames::TUNING_DEFAULT_VALUE_HEX, &v64);
		m_tuningDefaultValue.setBitCastUint64Value(v64);

		result &= xml.readUInt64Attribute(AppSignalPropNames::TUNING_LOW_BOUND_HEX, &v64);
		m_tuningLowBound.setBitCastUint64Value(v64);

		result &= xml.readUInt64Attribute(AppSignalPropNames::TUNING_HIGH_BOUND_HEX, &v64);
		m_tuningHighBound.setBitCastUint64Value(v64);

		result &= xml.readAddress16Attribute(AppSignalPropNames::TUNING_ADDR, &m_tuningAddr);
		result &= xml.readAddress16Attribute(AppSignalPropNames::TUNING_ABS_ADDR, &m_tuningAbsAddr);
	}

	// Specific properties
	//
	result &= xml.readStringAttribute(AppSignalPropNames::SPEC_PROP_STRUCT, &m_specPropStruct);

	AppSignalSpecPropValues spvs;

	spvs.createFromSpecPropStruct(m_specPropStruct);

	QXmlStreamAttributes attrs = xml.attributes();

	for(AppSignalSpecPropValue& spv : spvs.values())
	{
		QString name = spv.name();

		if (attrs.hasAttribute(name) == false)
		{
			result = false;
			continue;
		}

		QVariant qv = spv.value();			// to set Type of qv equal to Type of spv.value()

		if (spv.isEnum() == false)
		{
			result &= xml.readQVariantAttribute(name, &qv);
			spv.setValue(name, qv, false);
		}
		else
		{
			name = spv.name();

			if (name == AppSignalPropNames::ELECTRIC_UNIT)
			{
				E::ElectricUnit e;
				result &= xml.readEnumValueAttribute(name, &e);
				spv.setValue(name, TO_INT(e), true);
				continue;
			}

			if (name == AppSignalPropNames::SENSOR_TYPE)
			{
				E::SensorType e;
				result &= xml.readEnumValueAttribute(name, &e);
				spv.setValue(name, TO_INT(e), true);
				continue;
			}

			if (name == AppSignalPropNames::OUTPUT_MODE)
			{
				E::OutputMode e;
				result &= xml.readEnumValueAttribute(name, &e);
				spv.setValue(name, TO_INT(e), true);
				continue;
			}

			result &= xml.readQVariantAttribute(name, &qv);
			spv.setValue(name, qv, true);
		}
	}

	spvs.serializeValuesToArray(&m_protoSpecPropValues);

	return result;
}

void AppSignal::saveToProto(Proto::AppSignal* s) const
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

	s->set_invertsignal(m_invertSignal);
	s->set_reserved(m_reserved);

	// Signal type

	s->set_signaltype(TO_INT(m_signalType));
	s->set_inouttype(TO_INT(m_inOutType));
	s->set_swcalcfunction(TO_INT(m_swCalcFunction));

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
	s->set_aperturetype(TO_INT(m_apertureType));
	s->set_log(m_log);

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
		dbField->set_created(m_createdMcs);
		dbField->set_deleted(m_deleted);
		dbField->set_instancecreated(m_instanceCreatedMcs);
		dbField->set_instanceaction(static_cast<int>(m_instanceAction));
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

		calcParam->set_isendpoint(m_isEndpoint);

		// save state flags signals

		assert(calcParam->stateflagssignals_size() == 0);

		for(auto const& [flagType, flagSignalID] :  m_stateFlagsSignals)
		{
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

void AppSignal::loadFromProto(const Proto::AppSignal& s)
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

	m_invertSignal = s.invertsignal();
	m_reserved = s.reserved();

	// Signal type

	m_signalType = static_cast<E::SignalType>(s.signaltype());
	m_inOutType = static_cast<E::SignalInOutType>(s.inouttype());
	m_swCalcFunction = static_cast<E::SoftwareCalcFunction>(s.swcalcfunction());

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
	m_apertureType = static_cast<E::ApertureType>(s.aperturetype());
	m_log = s.log();

	// Signal fields from database

	const Proto::AppSignalDbField& dbField = s.dbfield();

	m_ID = dbField.id();
	m_signalGroupID = dbField.signalgroupid();
	m_signalInstanceID = dbField.signalinstanceid();
	m_changesetID = dbField.changesetid();
	m_checkedOut = dbField.checkedout();
	m_userID = dbField.userid();
	m_createdMcs = dbField.created();
	m_deleted = dbField.deleted();
	m_instanceCreatedMcs = dbField.instancecreated();
	m_instanceAction = static_cast<E::VcsItemAction>(dbField.instanceaction());

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

	m_isEndpoint = calcParam.isendpoint();

	// load state flags signals

	m_stateFlagsSignals.clear();

	int flagSignalsCount = calcParam.stateflagssignals_size();

	for(int i = 0; i < flagSignalsCount; i++)
	{
		const Proto::StateFlagSignal& protoStateFlagSignal = calcParam.stateflagssignals(i);

		E::AppSignalStateFlagType flagType = static_cast<E::AppSignalStateFlagType>(protoStateFlagSignal.flagtype());

		assert(m_stateFlagsSignals.contains(flagType) == false);

		m_stateFlagsSignals.emplace(flagType, QString::fromStdString(protoStateFlagSignal.flagsignalid()));
	}

	// Tags
	//
	m_tags.clear();
	for (const auto& t : s.tags())
	{
		m_tags.insert(QString::fromStdString(t));
	}
}

bool AppSignal::equalWithAppSignal(const AppSignal& s) const
{
	bool res = true;

	// Signal identificators

	res &= m_appSignalID == s.m_appSignalID;
	res &= m_customAppSignalID == s.m_customAppSignalID;
	res &= m_caption == s.m_caption;
	res &= m_equipmentID == s.m_equipmentID;
	res &= m_lmEquipmentID == s.m_lmEquipmentID;
	res &= m_busTypeID == s.m_busTypeID;
	res &= m_channel == s.m_channel;
	res &= m_excludeFromBuild == s.m_excludeFromBuild;

	res &= m_invertSignal == s.m_invertSignal;
	res &= m_reserved == s.m_reserved;

	// Signal type

	res &= m_signalType == s.m_signalType;
	res &= m_inOutType == s.m_inOutType;
	res &= m_swCalcFunction == s.m_swCalcFunction;

	// Signal format

	res &= m_dataSize == s.m_dataSize;
	res &= m_byteOrder == s.m_byteOrder;

	// Analog signal properties

	res &= m_analogSignalFormat == s.m_analogSignalFormat;
	res &= m_unit == s.m_unit;

	// Signal specific properties

	res &= m_specPropStruct == s.m_specPropStruct;
	res &= m_protoSpecPropValues == s.m_protoSpecPropValues;

	// Tuning signal properties

	res &= m_enableTuning == s.m_enableTuning;
	res &= m_tuningDefaultValue == s.m_tuningDefaultValue;
	res &= m_tuningLowBound == s.m_tuningLowBound;
	res &= m_tuningHighBound == s.m_tuningHighBound;

	//	Signal properties for MATS

	res &= m_acquire == s.m_acquire;
	res &= m_archive == s.m_archive;
	res &= m_decimalPlaces == s.m_decimalPlaces;
	res &= m_coarseAperture == s.m_coarseAperture;
	res &= m_fineAperture == s.m_fineAperture;
	res &= m_apertureType == s.m_apertureType;
	res &= m_log == s.m_log;

	// Signal fields from database

	res &= m_ID == s.m_ID;
	res &= m_signalGroupID == s.m_signalGroupID;
	res &= m_signalInstanceID == s.m_signalInstanceID;
	res &= m_changesetID == s.m_changesetID;
	res &= m_checkedOut == s.m_checkedOut;
	res &= m_userID == s.m_userID;
	res &= m_createdMcs == s.m_createdMcs;
	res &= m_deleted == s.m_deleted;
	res &= m_instanceCreatedMcs == s.m_instanceCreatedMcs;
	res &= m_instanceAction == s.m_instanceAction;

	// Signal properties calculated in compile-time

	res &= m_hash == s.m_hash;

	res &= m_ioBufAddr == s.m_ioBufAddr;
	res &= m_tuningAddr == s.m_tuningAddr;
	res &= m_ualAddr == s.m_ualAddr;
	res &= m_regBufAddr == s.m_regBufAddr;
	res &= m_regValueAddr == s.m_regValueAddr;
	res &= m_regValidityAddr == s.m_regValidityAddr;

	res &= m_lmRamAccess == s.m_lmRamAccess;

	res &= m_isConst == s.m_isConst;
	res &= m_constValue == s.m_constValue;

	res &= m_isEndpoint == s.m_isEndpoint;

	res &= m_stateFlagsSignals == s.m_stateFlagsSignals;

	res &= m_tags == s.m_tags;

	return res;
}

void AppSignal::initCalculatedProperties()
{
	m_hash = calcHash(m_appSignalID);
}

bool AppSignal::addFlagSignalID(E::AppSignalStateFlagType flagType, const QString& appSignalID)
{
	if (m_stateFlagsSignals.contains(flagType) == true)
	{
		return false;
	}

	m_stateFlagsSignals.emplace(flagType, appSignalID);

	return true;
}

QString AppSignal::getFlagSignalID(E::AppSignalStateFlagType flagType) const
{
	return getValueOrDefault(m_stateFlagsSignals, flagType, QString());
}

QStringList AppSignal::getFlagSignalsIDs() const
{
	QStringList result;

	for(const auto& [flagType, appSignalID] : m_stateFlagsSignals)
	{
		result.append(appSignalID);
	}

	return result;
}

bool AppSignal::hasFlagsSignals() const
{
	return !m_stateFlagsSignals.empty();
}

void AppSignal::initTuningValues()
{
	updateTuningValuesType();

	switch (signalType())
	{
	case E::SignalType::Analog:
		{
			double lowBound = lowEngineeringUnits(nullptr);
			double highBound = highEngineeringUnits(nullptr);

			if (lowBound == highBound )
			{
				lowBound = 0;
				highBound = 100;
			}

			m_tuningLowBound.setValue(m_tuningLowBound.type(), static_cast<qint64>(lowBound), lowBound);
			m_tuningHighBound.setValue(m_tuningHighBound.type(), static_cast<qint64>(highBound), highBound);
		}
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

AppSignal* AppSignal::createDiscreteSignal(	E::SignalInOutType inOutType,
											const QString& appSignalID,
											const QString& customAppSignalID,
											const QString& caption,
											const QString& equipmentID)
{
	//
	// Allocate and init new discrete AppSignal
	// Calling proc take on ownership of allocated AppSignal
	//
	AppSignal* newSignal = new AppSignal();

	newSignal->setSignalType(E::SignalType::Discrete);
	newSignal->setInOutType(inOutType);
	newSignal->setDataSize(DISCRETE_SIZE);

	newSignal->setAppSignalID(appSignalID);
	newSignal->setCustomAppSignalID(customAppSignalID);
	newSignal->setCaption(caption);
	newSignal->setEquipmentID(equipmentID);

	newSignal->initTuningValues();

	return newSignal;
}


QString AppSignal::removeNumberSign(const QString& appSignalID)
{
	if (appSignalID[0] == '#')
	{
		return appSignalID.mid(1);
	}

	return appSignalID;
}

void AppSignal::trimTextFields()
{
	m_appSignalID = m_appSignalID.trimmed();
	m_customAppSignalID = m_customAppSignalID.trimmed();
	m_equipmentID = m_equipmentID.trimmed();
	m_lmEquipmentID = m_lmEquipmentID.trimmed();
	m_busTypeID = m_busTypeID.trimmed();
	m_caption = m_caption.trimmed();
	m_unit = m_unit.trimmed();
}

void AppSignal::uppercaseAppSignalID(bool uppercase)
{
	if (uppercase)
	{
		m_appSignalID = m_appSignalID.toUpper();
	}
}

void AppSignal::initCreatedDates()
{
	m_createdMcs = QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000;
	m_instanceCreatedMcs = QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000;
}

bool AppSignal::isCompatibleFormatPrivate(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder, const QString& busTypeID) const
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

void AppSignal::updateTuningValuesType()
{
	TuningValueType tvType = TuningValue::getTuningValueType(m_signalType, m_analogSignalFormat);

	m_tuningDefaultValue.setType(tvType);
	m_tuningLowBound.setType(tvType);
	m_tuningHighBound.setType(tvType);
}

QString AppSignal::specPropNotExistErr(const QString& propName) const
{
	return QString("Specific property %1 is not exists in signal %2").arg(m_appSignalID).arg(propName);
}

bool AppSignal::getSpecPropBool(const QString& name, QString* err) const
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

	assert(qv.metaType().id() == QMetaType::Bool && isEnum == false);

	return qv.toBool();
}

double AppSignal::getSpecPropDouble(const QString& name, QString* err) const
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

	assert(qv.metaType().id() == QMetaType::Double && isEnum == false);

	return qv.toDouble();
}

int AppSignal::getSpecPropInt(const QString& name, QString* err) const
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

	assert(qv.metaType().id() == QMetaType::Int && isEnum == false);

	return qv.toInt();
}

unsigned int AppSignal::getSpecPropUInt(const QString& name, QString* err) const
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

	assert(qv.metaType().id() == QMetaType::UInt && isEnum == false);

	return qv.toUInt();
}


int AppSignal::getSpecPropEnum(const QString& name, QString* err) const
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

	assert(qv.metaType().id() == QMetaType::Int && isEnum == true);

	return qv.toInt();
}

bool AppSignal::getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum, QString* err) const
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
		AppSignalSpecPropValues spv;

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

bool AppSignal::isSpecPropExists(const QString& name) const
{
	if (m_cachedSpecPropValues != nullptr)
	{
		return m_cachedSpecPropValues->isExists(name);
	}

	AppSignalSpecPropValues spv;

	bool result = spv.parseValuesFromArray(m_protoSpecPropValues);

	if (result == false)
	{
		assert(false);
		return false;
	}

	return spv.isExists(name);
}

bool AppSignal::setSpecPropBool(const QString& name, bool value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool AppSignal::setSpecPropDouble(const QString& name, double value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool AppSignal::setSpecPropInt(const QString& name, int value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool AppSignal::setSpecPropUInt(const QString& name, unsigned int value)
{
	QVariant qv(value);

	return setSpecPropValue(name, qv, false);
}

bool AppSignal::setSpecPropEnum(const QString& name, int enumValue)
{
	QVariant qv(enumValue);

	return setSpecPropValue(name, qv, true);
}

bool AppSignal::setSpecPropValue(const QString& name, const QVariant& qv, bool isEnum)
{
	AppSignalSpecPropValues spv;

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

QStringList AppSignal::tags() const
{
	return QStringList(m_tags.begin(), m_tags.end());
}

void AppSignal::setTags(const QStringList& tags)
{
	clearTags();
	appendTags(tags);
}

void AppSignal::setTags(const std::set<QString>& tags) 
{
	m_tags = tags; 
}

void AppSignal::setTagsStr(const QString& tagsStr)
{
	static const auto re = QRegularExpression("\\W+");
	setTags(tagsStr.split(re, Qt::SkipEmptyParts));
}

void AppSignal::appendTag(const QString& tag)
{
	m_tags.insert(tag.toLower().trimmed());
}

void AppSignal::appendTags(const QStringList& tags)
{
	for(const QString& tag : tags)
	{
		appendTag(tag);
	}
}

void AppSignal::appendTags(const std::set<QString>& tags)
{
	for(const QString& tag : tags)
	{
		appendTag(tag);
	}
}

void AppSignal::removeTag(const QString& tag)
{
	m_tags.erase(tag.toLower().trimmed());
}

void AppSignal::removeTags(const QStringList& tags)
{
	for(const QString& tag : tags)
	{
		removeTag(tag);
	}
}

void AppSignal::removeTags(const std::set<QString>& tags)
{
	for(const QString& tag : tags)
	{
		removeTag(tag);
	}
}

void AppSignal::clearTags()
{
	m_tags.clear();
}

// --------------------------------------------------------------------------------------------------------
//
// AppSignalSet::SignalsGroups class implementation
//
// --------------------------------------------------------------------------------------------------------

void AppSignalSet::SignalsGroups::swap(SignalsGroups& signalGroups)
{
	m_groups.swap(signalGroups.m_groups);
}

void AppSignalSet::SignalsGroups::clear()
{
	m_groups.clear();
}

void AppSignalSet::SignalsGroups::insert(const AppSignal* appSignal)
{
	TEST_PTR_RETURN(appSignal);

	int groupID = appSignal->signalGroupID();

	if (groupID == SINGLE_CHANNEL)
	{
		return;
	}

	int signalID = appSignal->ID();

	auto it = m_groups.find(groupID);

	if (it == m_groups.end())
	{
		m_groups.emplace(groupID, std::vector<int>{ signalID });
	}
	else
	{
		it->second.push_back(signalID);
	}
}

void AppSignalSet::SignalsGroups::remove(const AppSignal* appSignal)
{
	remove(appSignal->signalGroupID(), appSignal->ID());
}

void AppSignalSet::SignalsGroups::remove(int groupID, int signalID)
{
	if (groupID == 0)
	{
		return;
	}

	auto it = m_groups.find(groupID);

	if (it == m_groups.end())
	{
		return;
	}

	std::vector<int>& ids = it->second;

	auto it2 = ids.begin();

	while(it2 != ids.end())
	{
		if (*it2 == signalID)
		{
			ids.erase(it2);
			break;
		}
	}
}

bool AppSignalSet::SignalsGroups::getGroupSignalIDs(int signalID, int groupID, std::vector<int>* signalsIDs) const
{
	TEST_PTR_RETURN_FALSE(signalsIDs);

	if (groupID == SINGLE_CHANNEL)
	{
		signalsIDs->clear();
		signalsIDs->push_back(signalID);
		return false;
	}

	auto it = m_groups.find(groupID);

	if (it != m_groups.end())
	{
		*signalsIDs = it->second;
		return true;
	}

	return false;
}

// --------------------------------------------------------------------------------------------------------
//
// AppSignalSet class implementation
//
// --------------------------------------------------------------------------------------------------------

AppSignalSet::AppSignalSet()
{
}

AppSignalSet::~AppSignalSet()
{
	clear();
}

void AppSignalSet::swap(AppSignalSet& appSignalSet)
{
	m_signals.swap(appSignalSet.m_signals);
	m_idToIndex.swap(appSignalSet.m_idToIndex);
	m_hashToIndex.swap(appSignalSet.m_hashToIndex);
	m_groups.swap(appSignalSet.m_groups);
}

void AppSignalSet::clear()
{
	for(AppSignal* s : m_signals)
	{
		if (s != nullptr)
		{
			delete s;
		}
	}

	m_signals.clear();
	m_idToIndex.clear();
	m_hashToIndex.clear();
	m_groups.clear();
}

void AppSignalSet::reserve(int n)
{
	Q_ASSERT(m_signals.size() == 0);
	m_signals.reserve(n);
}

std::pair<AppSignal*, int> AppSignalSet::append(AppSignal* newSignal)
{
	TEST_PTR_RETURN_VALUE(newSignal, (std::pair<AppSignal*, int>{nullptr, BAD_INDEX}));

	int signalID = newSignal->ID();

	if (signalID == 0)
	{
		if (m_enableIdGeneration == true)
		{
			signalID = m_idToIndex.empty() == true ? 1 : m_idToIndex.rbegin()->first + 1;
			newSignal->setID(signalID);
		}
		else
		{
			Q_ASSERT(false);				// assing signal->ID() before
											// or enable ID generation
			return {nullptr, BAD_INDEX};
		}
	}

	Hash hash = calcHash(newSignal->appSignalID());

	qsizetype index = m_signals.size();

	m_signals.push_back(newSignal);

	auto [it, inserted] = m_idToIndex.emplace(signalID, index);

	Q_ASSERT(inserted == true);

	auto [it2, inserted2] = m_hashToIndex.emplace(hash, index);

	Q_ASSERT(inserted2 == true);

	m_groups.insert(newSignal);

	return {newSignal, index};
}

std::pair<AppSignal*, int> AppSignalSet::append(const AppSignal& signal)
{
	auto newSignal = new AppSignal(signal);
	return append(newSignal);
}

std::pair<AppSignal*, int> AppSignalSet::append(const ID_AppSignalID& id)
{
	auto newSignal = new AppSignal(id);
	return append(newSignal);
}

void AppSignalSet::removeSignals(const std::vector<int> &signalToRemoveIDs)
{
	int removedCount = 0;

	for(int id : signalToRemoveIDs)
	{
		auto it = m_idToIndex.find(id);

		if (it == m_idToIndex.end())
		{
			Q_ASSERT(false);
			continue;
		}

		qsizetype index = it->second;

		AppSignal* s = m_signals[index];

		TEST_PTR_CONTINUE(s);

		m_groups.remove(s);
		delete s;
		m_signals[index] = nullptr;
		removedCount++;
	}

	std::vector<AppSignal*> tempSignals;

	tempSignals.reserve(m_signals.size() - removedCount);

	m_idToIndex.clear();
	m_hashToIndex.clear();

	qsizetype index = 0;

	for(AppSignal* s : m_signals)
	{
		if (s == nullptr)
		{
			continue;
		}

		m_idToIndex.emplace(s->ID(), index);
		m_hashToIndex.emplace(calcHash(s->appSignalID()), index);
		tempSignals.push_back(s);

		index++;
	}

	m_signals.swap(tempSignals);
}

bool AppSignalSet::contains(Hash appSignalHash) const
{
	return m_hashToIndex.contains(appSignalHash);
}

bool AppSignalSet::contains(const QString& appSignalID) const
{
	return contains(calcHash(appSignalID.trimmed()));
}

int AppSignalSet::count() const
{
	return static_cast<int>(m_signals.size());
}

int AppSignalSet::size() const
{
	return static_cast<int>(m_signals.size());
}

bool AppSignalSet::isEmpty() const
{
	return m_signals.empty();
}

void AppSignalSet::enableIdGeneration()
{
	m_enableIdGeneration = true;
}

const std::vector<AppSignal*>& AppSignalSet::signalsVector() const
{
	return m_signals;
}

std::vector<AppSignal*>::iterator AppSignalSet::begin()
{
	return m_signals.begin();
}

std::vector<AppSignal*>::const_iterator AppSignalSet::begin() const
{
	return m_signals.cbegin();
}

std::vector<AppSignal*>::iterator AppSignalSet::end()
{
	return m_signals.end();
}

std::vector<AppSignal*>::const_iterator AppSignalSet::end() const
{
	return m_signals.cend();
}

AppSignal* AppSignalSet::getSignal(const QString& appSignalID)
{
	return const_cast<AppSignal*>(privateGetSignal(appSignalID));
}

const AppSignal* AppSignalSet::getSignal(const QString& appSignalID) const
{
	return privateGetSignal(appSignalID);
}

AppSignal* AppSignalSet::getSignal(int signalID)
{
	return const_cast<AppSignal*>(privateGetSignalByID(signalID));
}

const AppSignal* AppSignalSet::getSignal(int signalID) const
{
	return privateGetSignalByID(signalID);
}

AppSignal* AppSignalSet::getSignalByHash(Hash appSignalIDHash)
{
	return const_cast<AppSignal*>(privateGetSignalByHash(appSignalIDHash));
}

const AppSignal* AppSignalSet::getSignalByHash(Hash appSignalIDHash) const
{
	return privateGetSignalByHash(appSignalIDHash);
}

AppSignal* AppSignalSet::at(int index)
{
	return const_cast<AppSignal*>(privateAt(index));
}

const AppSignal* AppSignalSet::at(int index) const
{
	return privateAt(index);
}

int AppSignalSet::signalIndex(int signalID) const
{
	auto it = m_idToIndex.find(signalID);

	if (it == m_idToIndex.end())
	{
		Q_ASSERT(false);
		return BAD_INDEX;
	}

	return it->second;
}

bool AppSignalSet::getChannelSignalsID(int signalID, std::vector<int>* channelSignalIDs) const
{
	const AppSignal* s = getSignal(signalID);

	TEST_PTR_RETURN_FALSE(s);

	return m_groups.getGroupSignalIDs(s->ID(), s->signalGroupID(), channelSignalIDs);
}

bool AppSignalSet::getChannelSignalsID(const AppSignal& signal, std::vector<int>* channelSignalIDs) const
{
	return m_groups.getGroupSignalIDs(signal.ID(), signal.signalGroupID(), channelSignalIDs);
}

bool AppSignalSet::getChannelSignalsID(int signalID, int groupID, std::vector<int>* channelSignalIDs) const
{
	return m_groups.getGroupSignalIDs(signalID, groupID, channelSignalIDs);
}

void AppSignalSet::appSignalIdsListSorted(bool removeNumberSign, QStringList* list) const
{
	TEST_PTR_RETURN(list);

	std::set<QString> ids;

	for (AppSignal* s : m_signals)
	{
		TEST_PTR_CONTINUE(s);

		QString appSignalId = s->appSignalID();

		if (removeNumberSign == true &&
			appSignalId.isEmpty() == false &&
			appSignalId.at(0) == QChar('#'))
		{
			ids.emplace(appSignalId.remove(0, 1));
		}
		else
		{
			ids.emplace(appSignalId);
		}
	}

	list->clear();
	list->resize(ids.size());

	for(const QString& id : ids)
	{
		list->append(id);
	}
}

std::pair<AppSignal*, int> AppSignalSet::updateSignal(const AppSignal& s)
{
	int signalID = s.ID();

	auto it = m_idToIndex.find(signalID);

	if (it == m_idToIndex.end())
	{
		return {nullptr, BAD_INDEX};
	}

	int signalIndex = it->second;

	AppSignal* existSignal = m_signals[signalIndex];

	QString oldAppSignalID = existSignal->appSignalID();

	*existSignal = s;

	if (oldAppSignalID != s.appSignalID())
	{
		auto oldHashIt = m_hashToIndex.find(calcHash(oldAppSignalID));

		if (oldHashIt == m_hashToIndex.end())
		{
			Q_ASSERT(false);
			return {nullptr, BAD_INDEX};
		}

		Q_ASSERT(oldHashIt->second == signalIndex);

		Hash newHash = calcHash(s.appSignalID());

		auto newHashIt = m_hashToIndex.find(newHash);

		if (newHashIt != m_hashToIndex.end())
		{
			Q_ASSERT(false);
			return {nullptr, BAD_INDEX};
		}

		m_hashToIndex.erase(oldHashIt);

		m_hashToIndex.emplace(newHash, signalIndex);
	}

	return {existSignal, signalIndex};
}

bool AppSignalSet::serializeFromProtoFile(const QString& filePath)
{
	clear();

	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	QByteArray fileData = qUncompress(file.readAll());

	::Proto::AppSignalSet protoAppSignalSet;

	bool result = protoAppSignalSet.ParseFromArray(fileData.constData(), static_cast<int>(fileData.size()));

	if (result == false)
	{
		return false;
	}

	int signalCount = protoAppSignalSet.appsignal_size();

	reserve(signalCount);

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

		AppSignal* newSignal = new AppSignal;

		newSignal->loadFromProto(protoAppSignal);

		append(newSignal);
	}

	return true;
}

const AppSignal* AppSignalSet::privateGetSignal(const QString& appSignalID) const
{
	Hash hash = calcHash(appSignalID);

	auto it = m_hashToIndex.find(hash);

	if (it == m_hashToIndex.end())
	{
		return nullptr;
	}

	qsizetype index = it->second;

	Q_ASSERT(index >= 0 && index < std::ssize(m_signals));

	return m_signals[index];
}

const AppSignal* AppSignalSet::privateGetSignalByID(int signalID) const
{
	auto it = m_idToIndex.find(signalID);

	if (it == m_idToIndex.end())
	{
		return nullptr;
	}

	qsizetype index = it->second;

	Q_ASSERT(index >= 0 && index < std::ssize(m_signals));

	return m_signals[index];
}

const AppSignal* AppSignalSet::privateGetSignalByHash(Hash appSignalIDHash) const
{
	auto it = m_hashToIndex.find(appSignalIDHash);

	if (it == m_hashToIndex.end())
	{
		return nullptr;
	}

	qsizetype index = it->second;

	Q_ASSERT(index >= 0 && index < std::ssize(m_signals));

	return m_signals[index];
}

const AppSignal* AppSignalSet::privateAt(int index) const
{
	if (index < 0 || index >= m_signals.size())
	{
		return nullptr;
	}

	return m_signals[index];
}

// -------------------------------------------------------------------------------
//
// AppSignals class implementation
//
// -------------------------------------------------------------------------------

AppSignals::AppSignals()
{
}

AppSignals::~AppSignals()
{
	clear();
}

void AppSignals::clear()
{
	m_hashToSignal.clear();
	m_signals.clear();
}

void AppSignals::reserve(int expectedSignalsCount)
{
	if (m_signals.size() > 0)
	{
		Q_ASSERT(false);
		return;
	}

	m_signals.reserve(expectedSignalsCount);
	m_hashToSignal.reserve(expectedSignalsCount);
}

void AppSignals::insert(const ::Proto::AppSignal& protoAppSignal)
{
	QString appSignalID = QString::fromStdString(protoAppSignal.appsignalid());

	Hash hash = calcHash(appSignalID);

	const AppSignal* existsAppSignal = getByHash(hash);

	if (existsAppSignal != nullptr)
	{
		if (existsAppSignal->appSignalID() == appSignalID)
		{
			qDebug() << C_STR(QString("Duplicate AppSignalID %1").arg(appSignalID));
			Q_ASSERT(false);
			return;
		}

		qDebug() << C_STR(QString("AppSignalIDs %1 and %2 hash %3 collision").
								arg(appSignalID).arg(existsAppSignal->appSignalID()).arg(hash, 16));
		Q_ASSERT(false);
		return;
	}

	int index = static_cast<int>(m_signals.size());
	m_signals.emplace_back(protoAppSignal);
	m_hashToSignal.insert({hash, index});
}

bool AppSignals::containsAppSignalID(const QString& appSignalID) const
{
	return m_hashToSignal.contains(calcHash(appSignalID));
}

bool AppSignals::containsHash(Hash hash) const
{
	return m_hashToSignal.contains(hash);
}

const AppSignal* AppSignals::getByAppSignalID(const QString& appSignalID) const
{
	return getByHash(calcHash(appSignalID));
}

const AppSignal* AppSignals::getByHash(Hash hash) const
{
	auto it = m_hashToSignal.find(hash);

	if (it == m_hashToSignal.end())
	{
		return nullptr;
	}

	return &m_signals[it->second];
}

const AppSignal* AppSignals::getByIndex(int index) const
{
	if (index < 0 || index >= static_cast<int>(m_signals.size()))
	{
		return nullptr;
	}

	return &m_signals[index];
}

const AppSignal* AppSignals::getSignalByIndex(size_t index) const
{
	if (index < 0 || index >= m_signals.size())
	{
		Q_ASSERT(false);
		return nullptr;
	}

	return &m_signals[index];
}

const AppSignal* AppSignals::getSignalByIndex(int index) const
{
	return getSignalByIndex(static_cast<size_t>(index));
}

bool AppSignals::isEmpty() const
{
	Q_ASSERT(m_signals.size() == m_hashToSignal.size());

	return m_signals.empty();
}

size_t AppSignals::count() const
{
	Q_ASSERT(m_signals.size() == m_hashToSignal.size());

	return m_signals.size();
}

std::vector<AppSignal>::iterator AppSignals::begin()
{
	return m_signals.begin();
}

std::vector<AppSignal>::const_iterator AppSignals::begin() const
{
	return m_signals.cbegin();
}

std::vector<AppSignal>::iterator AppSignals::end()
{
	return m_signals.end();
}

std::vector<AppSignal>::const_iterator AppSignals::end() const
{
	return m_signals.cend();
}

std::vector<Hash> AppSignals::getHashes() const
{
	std::vector<Hash> hashes;

	hashes.reserve(count());

	for(const AppSignal& s : m_signals)
	{
		hashes.push_back(s.hash());
	}

	return hashes;
}

