#include "AppSignal.h"
#include "../UtilsLib/XmlHelper.h"

#include "../Proto/serialization.pb.h"

// -------------------------------------------------------------------------------------------------------------
//
// AppSignalStateFlags class implementation
//
// -------------------------------------------------------------------------------------------------------------

void AppSignalStateFlags::setFlag(E::AppSignalStateFlagType flagType, quint32 value)
{
	// set flagValue to corresponding flag
	//
	switch (flagType)
	{
	case E::AppSignalStateFlagType::Validity:
		valid = value;
		return;

	case E::AppSignalStateFlagType::StateAvailable:
		stateAvailable = value;
		return;

	case E::AppSignalStateFlagType::Simulated:
		simulated  = value;
		return;

	case E::AppSignalStateFlagType::Blocked:
		blocked = value;
		return;

	case E::AppSignalStateFlagType::Mismatch:
		mismatch = value;
		return;

	case E::AppSignalStateFlagType::AboveHighLimit:
		aboveHighLimit = value;
		return;

	case E::AppSignalStateFlagType::BelowLowLimit:
		belowLowLimit = value;
		return;

	case E::AppSignalStateFlagType::SwSimulated:
		swSimulated = value;
		return;
	}

	Q_ASSERT(false);
	return;
}

void AppSignalStateFlags::clear()
{
	all = 0;
}

void AppSignalStateFlags::clearReasonsFlags()
{
	all &= ~MASK_ALL_ARCHIVING_REASONS;
}

bool AppSignalStateFlags::hasArchivingReason() const
{
	return (all & MASK_ALL_ARCHIVING_REASONS) != 0;
}

bool AppSignalStateFlags::hasShortTermArchivingReasonOnly() const
{
	quint32 archivingReasons = all & MASK_ALL_ARCHIVING_REASONS;

	return	(archivingReasons & ~MASK_SHORT_TERM_ARCHIVING_REASONE) == 0 &&
			(archivingReasons & MASK_SHORT_TERM_ARCHIVING_REASONE) != 0;
}

bool AppSignalStateFlags::hasAutoPointReasonOnly() const
{
	quint32 archivingReasons = all & MASK_ALL_ARCHIVING_REASONS;

	return	(archivingReasons & ~MASK_AUTO_POINT_REASONE) == 0 &&
			(archivingReasons & MASK_AUTO_POINT_REASONE) != 0;
}

void AppSignalStateFlags::updateArchivingReasonFlags(const AppSignalStateFlags& prevFlags)
{
	quint32 changedFlags = all ^ prevFlags.all;

	validityChange = (changedFlags & MASK_VALIDITY_AND_AVAILABLE_FLAGS) == 0 ? 0 : 1;
	simBlockMismatchChange = (changedFlags & MASK_SIM_BLOCK_UNBL_FLAGS) == 0 ? 0 : 1;
	limitFlagsChange = (changedFlags & MASK_LIMITS_FLAGS) == 0 ? 0 : 1;
}

QString AppSignalStateFlags::print()
{
	return QString("Valid=%1 Avail=%2 Sim=%3 Blk=%4 Unbl=%5 HLim=%6 LLim=%7 SwSim=%8 "
				   "[Reasons: ValCh=%9 SBUCh=%10 Lim=%11 Auto=%12 Fine=%13 Coarse=%14]").
			arg(valid).arg(stateAvailable).arg(simulated).arg(blocked).
			arg(mismatch).arg(aboveHighLimit).arg(belowLowLimit).arg(swSimulated).
			arg(validityChange).arg(simBlockMismatchChange).arg(limitFlagsChange).
			arg(autoPoint).arg(fineAperture).arg(coarseAperture);
}

// -------------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValue class implementation
//
// -------------------------------------------------------------------------------------------------------------

AppSignalSpecPropValue::AppSignalSpecPropValue()
{
}

bool AppSignalSpecPropValue::create(std::shared_ptr<Property> prop)
{
	if (prop == nullptr)
	{
		assert(false);
		return false;
	}

	return create(prop->caption(), prop->value(), prop->isEnum());
}

bool AppSignalSpecPropValue::create(const QString& name, const QVariant& value, bool isEnum)
{
	m_name = name;
	m_value = value;
	m_isEnum = isEnum;

	return true;
}

bool AppSignalSpecPropValue::setValue(const QString& name, const QVariant& value, bool isEnum)
{
	if (name != m_name)
	{
		assert(false);
		return false;
	}

	if (m_value.type() != value.type())
	{
		assert(false);
		return false;
	}

	if (m_isEnum != isEnum)
	{
		assert(false);
		return false;
	}

	m_value = value;

	return true;
}

bool AppSignalSpecPropValue::setAnyValue(const QString& name, const QVariant& value)
{
	return setValue(name, value, m_isEnum);
}

bool AppSignalSpecPropValue::save(Proto::SignalSpecPropValue* protoValue) const
{
	TEST_PTR_RETURN_FALSE(protoValue);

	protoValue->Clear();

	protoValue->set_name(m_name.toStdString());
	protoValue->set_type(static_cast<int>(m_value.type()));
	protoValue->set_isenum(m_isEnum);

	switch(m_value.type())
	{
	case QVariant::Invalid:
		assert(false);
		return true;

	case QVariant::Int:
		protoValue->set_int32val(m_value.toInt());
		return true;

	case QVariant::UInt:
		protoValue->set_uint32val(m_value.toUInt());
		return true;

	case QVariant::LongLong:
		protoValue->set_int64val(m_value.toLongLong());
		return true;

	case QVariant::ULongLong:
		protoValue->set_uint64val(m_value.toULongLong());
		return true;

	case QVariant::Double:
		protoValue->set_doubleval(m_value.toDouble());
		return true;

	case QVariant::Bool:
		protoValue->set_boolval(m_value.toBool());
		return true;

	case QVariant::String:
		protoValue->set_stringval(m_value.toString().toStdString());
		return true;

	default:
		assert(false);
	}

	return false;
}

bool AppSignalSpecPropValue::load(const Proto::SignalSpecPropValue& protoValue)
{
	m_name = QString::fromStdString(protoValue.name());

	QVariant::Type type = static_cast<QVariant::Type>(protoValue.type());

	m_isEnum = protoValue.isenum();

#ifdef QT_DEBUG
	if (m_isEnum == true && type != QVariant::Int)
	{
		assert(false);
	}
#endif

	switch(type)
	{
	case QVariant::Invalid:
		m_value = QVariant();
		return true;

	case QVariant::Int:
		assert(protoValue.has_int32val());
		m_value.setValue(protoValue.int32val());
		return true;

	case QVariant::UInt:
		assert(protoValue.has_uint32val());
		m_value.setValue(protoValue.uint32val());
		return true;

	case QVariant::LongLong:
		assert(protoValue.has_int64val());
		m_value.setValue(protoValue.int64val());
		return true;

	case QVariant::ULongLong:
		assert(protoValue.has_uint64val());
		m_value.setValue(protoValue.uint64val());
		return true;

	case QVariant::Double:
		assert(protoValue.has_doubleval());
		m_value.setValue(protoValue.doubleval());
		return true;

	case QVariant::Bool:
		assert(protoValue.has_boolval());
		m_value.setValue(protoValue.boolval());
		return true;

	case QVariant::String:
		assert(protoValue.has_stringval());
		m_value.setValue(QString::fromStdString(protoValue.stringval()));
		return true;

	default:
		assert(false);
	}

	return false;
}


// ----------------------------------------------------------------------------------------------------------
//
// SignalSpecPropValues class implementation
//
// ----------------------------------------------------------------------------------------------------------

AppSignalSpecPropValues::AppSignalSpecPropValues()
{
}


bool AppSignalSpecPropValues::create(const AppSignal& s)
{
	bool result = true;

	result &= createFromSpecPropStruct(s.specPropStruct());
	result &= parseValuesFromArray(s.protoSpecPropValues());

	return result;
}


bool AppSignalSpecPropValues::createFromSpecPropStruct(const QString& specPropStruct, bool buildNamesMap)
{
	m_specPropValues.clear();
	m_propNamesMap.clear();

	if (specPropStruct.isEmpty() == true)
	{
		return true;
	}

	PropertyObject pob;

	std::pair<bool, QString> result = pob.parseSpecificPropertiesStruct(specPropStruct);

	if (result.first == false)
	{
		assert(false);
		return false;
	}

	std::vector<std::shared_ptr<Property>> properties = pob.properties();

	for(std::shared_ptr<Property> property : properties)
	{
		AppSignalSpecPropValue specPropValue;

		bool createResult = specPropValue.create(property);
		if (createResult == false)
		{
			Q_ASSERT(createResult);
			return false;
		}

		m_specPropValues.append(specPropValue);
	}

	if (buildNamesMap == true)
	{
		buildPropNamesMap();
	}

	return true;
}

bool AppSignalSpecPropValues::updateFromSpecPropStruct(const QString& specPropStruct)
{
	PropertyObject pob;

	std::pair<bool, QString> pobResult = pob.parseSpecificPropertiesStruct(specPropStruct);

	if (pobResult.first == false)
	{
		return false;
	}

	buildPropNamesMap();

	QStringList namesToDelete;
	QHash<QString, std::shared_ptr<Property>> namesToCreate;

	std::vector<std::shared_ptr<Property>> properties = pob.properties();

	for(std::shared_ptr<Property> property : properties)
	{
		QString propName = property->caption();

		if (isExists(propName) == false)
		{
			namesToCreate.insert(propName, property);
		}
		else
		{
			// value of property is exists
			//
			QVariant value;
			bool isEnum = false;

			getValue(propName, &value, &isEnum);

			// checking that property end value types are equal
			//
			if (property->value().type() == value.type() && property->isEnum() == isEnum)
			{
				// equal, update existing value if nessesery
				//
				if (property->updateFromPreset() == true)
				{
					setValue(propName, property->value(), property->isEnum());
				}
			}
			else
			{
				// property type has been changed, recreate value
				//
				namesToDelete.append(propName);
				namesToCreate.insert(propName, property);
			}
		}
	}

	buildPropNamesMap();

	for(const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (pob.propertyByCaption(specPropValue.name()) == nullptr)
		{
			namesToDelete.append(specPropValue.name());
		}
	}

	for(const QString& nameToDelete : namesToDelete)
	{
		int index = getPropertyIndex(nameToDelete);

		if (index != -1)
		{
			m_specPropValues.removeAt(index);
			buildPropNamesMap();
		}
		else
		{
			assert(false);
		}
	}

	// create new property value, set to default
	//
	for(std::shared_ptr<Property> property : namesToCreate)
	{
		AppSignalSpecPropValue specPropValue;

		specPropValue.create(property);

		m_specPropValues.append(specPropValue);
	}

	return true;
}

bool AppSignalSpecPropValues::setValue(const QString& name, const QVariant& value)
{
	return setValue(name, value, false);
}

bool AppSignalSpecPropValues::setAnyValue(const QString& name, const QVariant& value)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setAnyValue(name, value);
}

bool AppSignalSpecPropValues::setEnumValue(const QString& name, int enumItemValue)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setValue(name, QVariant(enumItemValue), true);
}

bool AppSignalSpecPropValues::setValue(const AppSignalSpecPropValue& propValue)
{
	return setValue(propValue.name(), propValue.value(), propValue.isEnum());
}

bool AppSignalSpecPropValues::getValue(const QString& name, QVariant* qv) const
{
	TEST_PTR_RETURN_FALSE(qv);

	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	*qv = m_specPropValues[index].value();

	return true;
}

bool AppSignalSpecPropValues::getValue(const QString& name, QVariant* qv, bool* isEnum) const
{
	TEST_PTR_RETURN_FALSE(qv);
	TEST_PTR_RETURN_FALSE(isEnum);

	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	*qv = m_specPropValues[index].value();
	*isEnum = m_specPropValues[index].isEnum();

	return true;
}

bool AppSignalSpecPropValues::	serializeValuesToArray(QByteArray* protoData) const
{
	TEST_PTR_RETURN_FALSE(protoData);

	Proto::SignalSpecPropValues protoValues;

	for(const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		Proto::SignalSpecPropValue* protoValue = protoValues.add_value();
		specPropValue.save(protoValue);
	}

	protoData->resize(static_cast<int>(protoValues.ByteSizeLong()));

	protoValues.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(protoData->data()));

	return true;
}

bool AppSignalSpecPropValues::parseValuesFromArray(const QByteArray& protoData)
{
	m_specPropValues.clear();

	Proto::SignalSpecPropValues protoValues;

	bool result = protoValues.ParseFromArray(protoData.constData(), protoData.size());

	if (result == false)
	{
		return false;
	}

	int count = protoValues.value_size();

	for(int i = 0; i < count; i++)
	{
		AppSignalSpecPropValue specPropValue;

		specPropValue.load(protoValues.value(i));

		m_specPropValues.append(specPropValue);
	}

	buildPropNamesMap();

	return true;
}

void AppSignalSpecPropValues::append(const AppSignalSpecPropValue& value)
{
	m_specPropValues.append(value);
}

bool AppSignalSpecPropValues::replaceName(const QString& oldName, const QString& newName)
{
	bool replacingIsOccured = false;

	for(AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (specPropValue.name() == oldName)
		{
			specPropValue.setName(newName);
			replacingIsOccured = true;
			break;
		}
	}

	return replacingIsOccured;
}

void AppSignalSpecPropValues::buildPropNamesMap()
{
	m_propNamesMap.clear();

	m_propNamesMap.reserve(static_cast<int>(m_specPropValues.size() * 1.2));

	int index = 0;

	for(const AppSignalSpecPropValue& specPropValue : m_specPropValues)
	{
		if (m_propNamesMap.contains(specPropValue.name()) == false)
		{
			m_propNamesMap.insert(specPropValue.name(), index);
		}
		else
		{
			assert(false);			// duplicate property name
		}

		index++;
	}
}

bool AppSignalSpecPropValues::setValue(const QString& name, const QVariant& value, bool isEnum)
{
	int index = getPropertyIndex(name);

	if (index == -1)
	{
		return false;
	}

	return m_specPropValues[index].setValue(name, value, isEnum);
}

int AppSignalSpecPropValues::getPropertyIndex(const QString& name) const
{
	if (m_propNamesMap.isEmpty() == false)
	{
		return m_propNamesMap.value(name, -1);
	}

	int index = 0;

	for(const AppSignalSpecPropValue& propValue : m_specPropValues)
	{
		if (propValue.name() == name)
		{
			return index;
		}

		index++;
	}

	return -1;
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
	m_appSignalID = ids.appSignalID;

	m_isLoaded = false;
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
		if (m_analogSignalFormat != E::AnalogAppSignalFormat::Float32 &&
			m_analogSignalFormat != E::AnalogAppSignalFormat::SignedInt32)
		{
			Q_ASSERT(false);

			return QString("Unknown E::AnalogAppSignalFormat");
		}

		// no break, it is Ok!

	case E::SignalType::Discrete:

		setDataSize(m_signalType, m_analogSignalFormat);
		updateTuningValuesType();
		initTuningValues();

		break;

	case E::SignalType::Bus:

		m_busTypeID = appSignalBusTypeID;

		break;

	default:

		Q_ASSERT(false);
		return QString("Unknown device signal E::SignalType");
	}

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
		return QString("Can't create AppSignal from diagnostics device signal");

		break;

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

void AppSignal::setSignalType(E::SignalType type)
{
	m_signalType = type;
	updateTuningValuesType();
}

void AppSignal::setDataSize(E::SignalType signalType, E::AnalogAppSignalFormat dataFormat)
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

void AppSignal::setDataSizeW(int sizeW)
{
	m_dataSize = sizeW * SIZE_16BIT;
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

double AppSignal::rload_Ohm(QString* err) const
{
	return getSpecPropDouble(AppSignalPropNames::RLOAD_OHM, err);
}

void AppSignal::setRload_Ohm(double rload_Ohm)
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

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		AppSignalSpecPropValue spValue;

		spValue.create(specificProperty);

		spValues.append(spValue);
	}

	spValues.serializeValuesToArray(&m_protoSpecPropValues);

	return true;
}

void AppSignal::cacheSpecPropValues()
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

void AppSignal::loadProtoData(const QByteArray& protoDataArray)
{
	Proto::ProtoAppSignalData protoData;

	bool res = protoData.ParseFromArray(protoDataArray.constData(), protoDataArray.size());

	assert(res == true);
	Q_UNUSED(res)

	loadProtoData(protoData);
}

void AppSignal::loadProtoData(const Proto::ProtoAppSignalData& protoData)
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

void AppSignal::writeToXml(XmlWriteHelper& xml)
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
	xml.writeIntAttribute("InOutType", inOutTypeInt());
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::FILTERING_TIME);
	writeDoubleSpecPropAttribute(xml, AppSignalPropNames::SPREAD_TOLERANCE);
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

void AppSignal::writeDoubleSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName)
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

void AppSignal::writeIntSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName)
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

void AppSignal::writeTuningValuesToXml(XmlWriteHelper& xml)
{
	xml.writeBoolAttribute("EnableTuning", enableTuning());

	assert(tuningDefaultValue().type() == tuningLowBound().type());
	assert(tuningDefaultValue().type() == tuningHighBound().type());

	xml.writeStringAttribute("TuningValueType", tuningDefaultValue().typeStr());

	xml.writeStringAttribute("TuningDefaultValue", tuningDefaultValue().toString());
	xml.writeStringAttribute("TuningLowBound", tuningLowBound().toString());
	xml.writeStringAttribute("TuningHighBound", tuningHighBound().toString());
}

bool AppSignal::readFromXml(XmlReadHelper& xml)
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

bool AppSignal::readTuningValuesFromXml(XmlReadHelper &xml)
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

void AppSignal::serializeTo(Proto::AppSignal* s) const
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

void AppSignal::serializeFrom(const Proto::AppSignal& s)
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
	m_instanceAction = static_cast<E::VcsItemAction>(dbFiled.instanceaction());

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

	m_stateFlagsSignals.insert(flagType, appSignalID);

	return true;
}

void AppSignal::initTuningValues()
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

	newSignal->updateTuningValuesType();
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

void AppSignal::initCreatedDates()
{
	m_created = QDateTime::currentDateTime();
	m_instanceCreated = QDateTime::currentDateTime();
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

	assert(qv.type() == QVariant::Double && isEnum == false);

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

	assert(qv.type() == QVariant::Int && isEnum == false);

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

	assert(qv.type() == QVariant::UInt && isEnum == false);

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

	assert(qv.type() == QVariant::Int && isEnum == true);

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
	QStringList list;

	for(const QString& tag : m_tags)
	{
		list.append(tag);
	}

	return list;
}

void AppSignal::setTags(const QStringList& tags)
{
	clearTags();
	appendTags(tags);
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
// SignalSet class implementation
//
// --------------------------------------------------------------------------------------------------------

AppSignalSet::AppSignalSet()
{
}

AppSignalSet::~AppSignalSet()
{
	clear();
}

void AppSignalSet::clear()
{
	SignalPtrOrderedHash::clear();

	m_groupSignals.clear();
	m_strID2IndexMap.clear();
}

void AppSignalSet::reserve(int n)
{
	SignalPtrOrderedHash::reserve(n);
	m_groupSignals.reserve(n);
}

void AppSignalSet::buildID2IndexMap()
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
		AppSignal& s = (*this)[i];

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

void AppSignalSet::updateID2IndexInMap(const QString& appSignalId, int index)
{
	m_strID2IndexMap.insert(appSignalId, index);
}

void AppSignalSet::updateID2IndexInMap(const AppSignal* appSignal)
{
	TEST_PTR_RETURN(appSignal);

	int index = keyIndex(appSignal->ID());

	updateID2IndexInMap(appSignal->appSignalID(), index);
}

bool AppSignalSet::ID2IndexMapIsEmpty()
{
	return m_strID2IndexMap.isEmpty();
}

bool AppSignalSet::contains(const QString& appSignalID) const
{
	if (count() > 0 && m_strID2IndexMap.isEmpty() == true)
	{
		assert(false);		//call buildStrID2IndexMap() before
		return false;
	}

	return m_strID2IndexMap.contains(appSignalID.trimmed());
}

AppSignal* AppSignalSet::getSignal(const QString& appSignalID)
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

const AppSignal* AppSignalSet::getSignal(const QString& appSignalID) const
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

void AppSignalSet::append(const int& signalID, AppSignal* signal)
{
	if (signalID > m_maxID)
	{
		m_maxID = signalID;
	}

	SignalPtrOrderedHash::append(signalID, signal);

	m_groupSignals.insert(signal->signalGroupID(), signalID);
}

void AppSignalSet::append(AppSignal* signal)
{
	int newID = getMaxID() + 1;

	append(newID, signal);
}

void AppSignalSet::remove(const int& signalID)
{
	AppSignal signal = value(signalID);

	SignalPtrOrderedHash::remove(signalID);

	m_groupSignals.remove(signal.signalGroupID(), signalID);
}

void AppSignalSet::removeAt(const int index)
{
	const AppSignal& signal = SignalPtrOrderedHash::operator [](index);

	int signalGroupID = signal.signalGroupID();
	int signalID = signal.ID();

	SignalPtrOrderedHash::removeAt(index);

	m_groupSignals.remove(signalGroupID, signalID);
}

QVector<int> AppSignalSet::getChannelSignalsID(const AppSignal& signal) const
{
	return getChannelSignalsID(signal.signalGroupID());
}

QVector<int> AppSignalSet::getChannelSignalsID(int signalGroupID) const
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

void AppSignalSet::resetAddresses()
{
	int signalCount = count();

	for(int i = 0; i < signalCount; i++)
	{
		(*this)[i].resetAddresses();
	}
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

		AppSignal* newSignal = new AppSignal;

		newSignal->serializeFrom(protoAppSignal);

		append(newSignal->ID(), newSignal);
	}

	buildID2IndexMap();

	return true;
}

int AppSignalSet::getMaxID()
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

QStringList AppSignalSet::appSignalIdsList(bool removeNumberSign, bool sort) const
{
	QStringList result;
	result.reserve(count());

	for (int i = 0; i < count(); i++)
	{
		const AppSignal& signal = operator[](i);
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

void AppSignalSet::replaceOrAppendIfNotExists(int signalID, const AppSignal& s)
{
	AppSignal* existsSignal = valuePtr(signalID);

	if (existsSignal != nullptr)
	{
		*existsSignal = s;
	}
	else
	{
		append(signalID, new AppSignal(s));
	}

	m_strID2IndexMap.insert(s.appSignalID(), keyIndex(signalID));
}
