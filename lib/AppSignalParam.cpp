#include "../lib/AppSignalParam.h"
#include "../Proto/serialization.pb.h"

const char* AppSignalParamMimeType::value ="application/x-appsignalparam";		// Data in format ::Proto::AppSiagnalParamSet

AppSignalState::AppSignalState(const Proto::AppSignalState& protoState)
{
	try
	{
		load(protoState);
	}
	catch(...)
	{
	}
}

AppSignalState& AppSignalState::operator= (const SimpleAppSignalState& smState)
{
	m_hash = smState.hash;
	m_time = smState.time;
	m_flags = smState.flags;
	m_value = smState.value;

	return *this;
}

Hash AppSignalState::hash() const
{
	return m_hash;
}

const Times& AppSignalState::time() const
{
	return m_time;
}

const TimeStamp& AppSignalState::time(E::TimeType timeType) const
{
	switch (timeType)
	{
	case E::TimeType::Plant:
		return m_time.plant;
	case E::TimeType::System:
		return m_time.system;
	case E::TimeType::Local:
		return m_time.local;
	default:
		{
			static const TimeStamp dummy;
			return dummy;
		}
	}
}

double AppSignalState::value() const  noexcept
{
	return m_value;
}

bool AppSignalState::isValid() const  noexcept
{
	return m_flags.valid;
}

bool AppSignalState::isStateAvailable() const
{
	return m_flags.stateAvailable;
}

bool AppSignalState::isSimulated() const
{
	return m_flags.simulated;
}

bool AppSignalState::isBlocked() const
{
	return m_flags.blocked;
}

bool AppSignalState::isMismatch() const
{
	return m_flags.mismatch;
}

bool AppSignalState::isAboveHighLimit() const
{
	return m_flags.aboveHighLimit;
}

bool AppSignalState::isBelowLowLimit() const
{
	return m_flags.belowLowLimit;
}

bool AppSignalState::isOutOfLimits() const
{
	return isAboveHighLimit() || isBelowLowLimit();
}

void AppSignalState::save(Proto::AppSignalState* protoState)
{
	if (protoState == nullptr)
	{
		assert(false);
		return;
	}

	assert(m_hash != 0);

	protoState->set_hash(m_hash);
	protoState->set_value(m_value);
	protoState->set_flags(m_flags.all);

	protoState->set_systemtime(m_time.system.timeStamp);
	protoState->set_localtime(m_time.local.timeStamp);
	protoState->set_planttime(m_time.plant.timeStamp);
}

Hash AppSignalState::load(const Proto::AppSignalState& protoState)
{
	m_hash = protoState.hash();
	assert(m_hash != 0);

	m_value = protoState.value();
	m_flags.all = protoState.flags();

	m_time.system.timeStamp = protoState.systemtime();
	m_time.local.timeStamp = protoState.localtime();
	m_time.plant.timeStamp = protoState.planttime();

	return m_hash;
}

bool AppSignalState::hasSameValue(const AppSignalState& b) const
{
	return m_flags.all == b.m_flags.all &&
		   m_value == b.m_value &&
		   m_hash == b.m_hash;
}

QString AppSignalState::toString(double value, E::ValueViewType viewType, E::AnalogFormat analogFormat, int precision)
{
	QString result;
	result.reserve(32);

	int p = 4;

	switch (viewType)
	{
	case E::ValueViewType::Dec:
		result = QString::number(value, static_cast<char>(analogFormat), precision);
		break;

	case E::ValueViewType::Hex:
		result = /*tr("HEX:") + */QString::number((long)value, 16) + QStringLiteral("h");
		break;

	case E::ValueViewType::Exp:
		result = /*tr("EXP:") + */QString::number(value, 'e', precision);
		break;

	case E::ValueViewType::Bin16:
		{
			result = QString::number((quint16)value, 2);
			result = result.rightJustified(16, '0');
			for (int q = 0; q < 3; q++, p += 5)
			{
				result.insert(p, ' ');
			}
		}
		break;

	case E::ValueViewType::Bin32:
		result = QString::number((quint32)value, 2);
		result = result.rightJustified(32, '0');
		for (int q = 0; q < 7; q++, p += 5)
		{
			result.insert(p, ' ');
		}
		break;

	case E::ValueViewType::Bin64:
		result = QString::number((quint64)value, 2);
		result = result.rightJustified(64, '0');
		for (int q = 0; q < 15; q++, p += 5)
		{
			result.insert(p, ' ');
		}
		result.insert(40, QChar::LineFeed);
		break;

	default:
		assert(false);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------
//
// AppSignalParam class implementation
//
// -------------------------------------------------------------------------------------------------

AppSignalParam::AppSignalParam(const Signal& signal)
{
	load(signal);
}

bool AppSignalParam::load(const ::Proto::AppSignal& message)
{
	Signal s;

	s.serializeFrom(message);
	s.cacheSpecPropValues();

	m_hash = message.calcparam().hash();

	load(s);

	return true;
}

void AppSignalParam::load(const Signal& s)
{
	m_appSignalId = s.appSignalID();
	m_customSignalId = s.customAppSignalID();
	m_caption = s.caption();
	m_equipmentId = s.equipmentID();
	m_lmEquipmentId = s.lmEquipmentID();

	m_channel = s.channel();
	m_inOutType = s.inOutType();
	m_signalType = s.signalType();
	m_analogSignalFormat = s.analogSignalFormat();
	m_byteOrder = s.byteOrder();

	m_unit = s.unit();

	m_lowValidRange = s.lowValidRange();
	m_highValidRange = s.highValidRange();
	m_lowEngineeringUnits = s.lowEngineeringUnits();
	m_highEngineeringUnits = s.highEngineeringUnits();

	m_electricLowLimit = s.electricLowLimit();
	m_electricHighLimit = s.electricHighLimit();
	m_electricUnit = s.electricUnit();
	m_sensorType = s.sensorType();
	m_outputMode = s.outputMode();

	m_precision = s.decimalPlaces();
	m_coarseAperture = s.coarseAperture();
	m_fineAperture = s.fineAperture();
	m_filteringTime = s.filteringTime();
	m_spreadTolerance = s.spreadTolerance();
	m_enableTuning = s.enableTuning();

	m_tuningDefaultValue = s.tuningDefaultValue();
	m_tuningLowBound = s.tuningLowBound();
	m_tuningHighBound = s.tuningHighBound();

	m_specPropStruct = s.specPropStruct();
	m_specPropValues = s.protoSpecPropValues();

	m_tags = s.tagsSet();
}

void AppSignalParam::save(::Proto::AppSignal* message) const
{
	if (message == nullptr)
	{
		assert(message);
		return;
	}

	message->mutable_calcparam()->set_hash(m_hash);
	message->set_appsignalid(m_appSignalId.toStdString());
	message->set_customappsignalid(m_customSignalId.toStdString());
	message->set_caption(m_caption.toStdString());
	message->set_equipmentid(m_equipmentId.toStdString());
	message->set_lmequipmentid(m_lmEquipmentId.toStdString());

	message->set_channel(static_cast<int>(m_channel));
	message->set_inouttype(static_cast<int>(m_inOutType));
	message->set_signaltype(m_signalType);
	message->set_analogsignalformat(static_cast<int>(m_analogSignalFormat));
	message->set_byteorder(m_byteOrder);

	message->set_unit(m_unit.toStdString());

	message->set_decimalplaces(m_precision);
	message->set_coarseaperture(m_coarseAperture);
	message->set_fineaperture(m_fineAperture);
	message->set_enabletuning(m_enableTuning);

	m_tuningDefaultValue.save(message->mutable_tuningdefaultvalue());
	m_tuningLowBound.save(message->mutable_tuninglowbound());
	m_tuningHighBound.save(message->mutable_tuninghighbound());

	message->set_specpropstruct(m_specPropStruct.toStdString());
	message->set_specpropvalues(m_specPropValues.constData(), m_specPropValues.size());

	// Tags
	//
	message->clear_tags();
	for (const QString& t : m_tags)
	{
		message->add_tags(t.toStdString());
	}

	return;
}

Hash AppSignalParam::hash() const
{
	return m_hash;
}
void AppSignalParam::setHash(Hash value)
{
	m_hash = value;
}

const QString& AppSignalParam::appSignalId() const
{
	return m_appSignalId;
}

void AppSignalParam::setAppSignalId(const QString& value)
{
	m_appSignalId = value;
}

const QString& AppSignalParam::customSignalId() const
{
	return m_customSignalId;
}

void AppSignalParam::setCustomSignalId(const QString& value)
{
	m_customSignalId = value;
}

const QString& AppSignalParam::caption() const
{
	return m_caption;
}

void AppSignalParam::setCaption(const QString& value)
{
	m_caption = value;
}


const QString& AppSignalParam::equipmentId() const
{
	return m_equipmentId;
}

void AppSignalParam::setEquipmentId(const QString& value)
{
	m_equipmentId = value;
}

const QString& AppSignalParam::lmEquipmentId() const
{
	return m_lmEquipmentId;
}

void AppSignalParam::setLmEquipmentId(const QString& value)
{
	m_lmEquipmentId = value;
}

E::Channel AppSignalParam::channel() const
{
	return m_channel;
}

void AppSignalParam::setChannel(E::Channel value)
{
	m_channel = value;
}

bool AppSignalParam::isInput() const
{
	return m_inOutType == E::SignalInOutType::Input;
}

bool AppSignalParam::isOutput() const
{
	return m_inOutType == E::SignalInOutType::Output;
}

bool AppSignalParam::isInternal() const
{
	return m_inOutType == E::SignalInOutType::Internal;
}

E::SignalInOutType AppSignalParam::inOutType() const
{
	return m_inOutType;
}

void AppSignalParam::setInOutType(E::SignalInOutType value)
{
	m_inOutType = value;
}

bool AppSignalParam::isAnalog() const
{
	return m_signalType == E::SignalType::Analog;
}

bool AppSignalParam::isDiscrete() const
{
	return m_signalType == E::SignalType::Discrete;
}

E::SignalType AppSignalParam::type() const
{
	return m_signalType;
}

void AppSignalParam::setType(E::SignalType value)
{
	m_signalType = value;
}

TuningValueType AppSignalParam::tuningType() const
{
	switch (m_signalType)
	{
	case E::Analog:
		switch (m_analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			return TuningValueType::Float;
		case E::AnalogAppSignalFormat::SignedInt32:
			return TuningValueType::SignedInt32;
		default:
			assert(false);
			// Unsupported tuning signal type
			//
		}
		return TuningValueType::Discrete;

	case E::Discrete:
		return TuningValueType::Discrete;

	default:
		// Unsupported tuning signal type
		//
		assert(false);
	}

	return TuningValueType::Discrete;
}

E::AnalogAppSignalFormat AppSignalParam::analogSignalFormat() const
{
	return m_analogSignalFormat;
}

void AppSignalParam::setAnalogSignalFormat(E::AnalogAppSignalFormat value)
{
	m_analogSignalFormat = value;
}

E::ByteOrder AppSignalParam::byteOrder() const
{
	return m_byteOrder;
}

void AppSignalParam::AppSignalParam::setByteOrder(E::ByteOrder value)
{
	m_byteOrder = value;
}

const QString& AppSignalParam::unit() const
{
	return m_unit;
}

void AppSignalParam::setUnit(QString value)
{
	m_unit = std::move(value);
}

double AppSignalParam::lowValidRange() const
{
	return m_lowValidRange;
}

double AppSignalParam::highValidRange() const
{
	return m_highValidRange;
}

double AppSignalParam::lowEngineeringUnits() const
{
	return m_lowEngineeringUnits;
}

void AppSignalParam::setLowEngineeringUnits(double value)
{
	m_lowEngineeringUnits = value;
}

double AppSignalParam::highEngineeringUnits() const
{
	return m_highEngineeringUnits;
}

void AppSignalParam::setHighEngineeringUnits(double value)
{
	m_highEngineeringUnits = value;
}

double AppSignalParam::inputLowLimit() const
{
	return m_electricLowLimit;
}

double AppSignalParam::inputHighLimit() const
{
	return m_electricHighLimit;
}

E::ElectricUnit AppSignalParam::inputUnitId() const
{
	return m_electricUnit;
}

E::SensorType AppSignalParam::inputSensorType() const
{
	return m_sensorType;
}

double AppSignalParam::outputLowLimit() const
{
	return m_outputLowLimit;
}

double AppSignalParam::outputHighLimit() const
{
	return m_outputHighLimit;
}

int AppSignalParam::outputUnitId() const
{
	return m_outputUnitId;
}

E::OutputMode AppSignalParam::outputMode() const
{
	return m_outputMode;
}

E::SensorType AppSignalParam::outputSensorType() const
{
	return m_outputSensorType;
}

int AppSignalParam::precision() const
{
	return m_precision;
}

void AppSignalParam::setPrecision(int value)
{
	m_precision = value;
}

double AppSignalParam::aperture()
{
	return m_coarseAperture;
}

void AppSignalParam::setAperture(double value)
{
	m_coarseAperture = value;
}

double AppSignalParam::filteringTime()
{
	return m_filteringTime;
}

void AppSignalParam::setFilteringTime(double value)
{
	m_filteringTime = value;
}

double AppSignalParam::spreadTolerance()
{
	return m_spreadTolerance;
}

void AppSignalParam::setSpreadTolerance(double value)
{
	m_spreadTolerance = value;
}

bool AppSignalParam::enableTuning() const
{
	return m_enableTuning;
}

void AppSignalParam::setEnableTuning(bool value)
{
	m_enableTuning = value;
}

TuningValue AppSignalParam::tuningDefaultValue() const
{
	return m_tuningDefaultValue;
}

QVariant AppSignalParam::tuningDefaultValueToVariant() const
{
	return m_tuningDefaultValue.toVariant();
}

void AppSignalParam::setTuningDefaultValue(const TuningValue& value)
{
	m_tuningDefaultValue = value;
}

TuningValue AppSignalParam::tuningLowBound() const
{
	return m_tuningLowBound;
}

QVariant AppSignalParam::tuningLowBoundToVariant() const
{
	return m_tuningLowBound.toVariant();
}

void AppSignalParam::setTuningLowBound(const TuningValue& value)
{
	m_tuningLowBound = value;
}

TuningValue AppSignalParam::tuningHighBound() const
{
	return m_tuningHighBound;
}

QVariant AppSignalParam::tuningHighBoundToVariant() const
{
	return m_tuningHighBound.toVariant();
}

void AppSignalParam::setTuningHighBound(const TuningValue& value)
{
	m_tuningHighBound = value;
}

const std::set<QString>& AppSignalParam::tags() const
{
	return m_tags;
}

std::set<QString>& AppSignalParam::mutableTags()
{
	return m_tags;
}

QStringList AppSignalParam::tagStringList() const
{
	QStringList result;
	result.reserve(static_cast<int>(m_tags.size()));

	for (const QString& tag : m_tags)
	{
		result << tag;
	}

	return result;
}

void AppSignalParam::setTags(std::set<QString> tags)
{
	m_tags = std::move(tags);
}

bool AppSignalParam::hasTag(const QString& tag) const
{
	return m_tags.contains(tag);
}
