#include "../lib/AppSignal.h"
#include "../Proto/serialization.pb.h"
#include "../lib/Signal.h"

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
	}

	assert(false);
static const TimeStamp dummy;
	return dummy;
}

double AppSignalState::value() const
{
	return m_value;
}

bool AppSignalState::isValid() const
{
	return m_flags.valid;
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

void SimpleAppSignalState::save(Proto::AppSignalState* protoState)
{
	if (protoState == nullptr)
	{
		assert(false);
		return;
	}

	assert(hash != 0);

	protoState->set_hash(hash);
	protoState->set_value(value);
	protoState->set_flags(flags.all);
	protoState->set_systemtime(time.system.timeStamp);
	protoState->set_localtime(time.local.timeStamp);
	protoState->set_planttime(time.plant.timeStamp);
}

Hash SimpleAppSignalState::load(const Proto::AppSignalState& protoState)
{
	hash = protoState.hash();

	assert(hash != 0);

	value = protoState.value();
	flags.all = protoState.flags();

	time.system.timeStamp = protoState.systemtime();
	time.local.timeStamp = protoState.localtime();
	time.plant.timeStamp = protoState.planttime();

	return hash;
}

void SimpleAppSignalState::print() const
{
	qDebug() << "state" << QDateTime::fromMSecsSinceEpoch(time.system.timeStamp).toString("dd.MM.yyyy HH:mm:ss.zzz") <<
				"validity =" << flags.valid <<
				"value =" << value <<
				(flags.autoPoint == 1 ? " auto" : "");
}

// -------------------------------------------------------------------------------------------------
//
// AppSignalParam class implementation
//
// -------------------------------------------------------------------------------------------------

AppSignalParam::AppSignalParam()
{
}

bool AppSignalParam::load(const ::Proto::AppSignal& message)
{
	Signal s;

	s.serializeFrom(message);

	s.cacheSpecPropValues();

	m_hash = message.calcparam().hash();
	m_appSignalId = s.appSignalID();
	m_customSignalId = s.customAppSignalID();
	m_caption = s.caption();
	m_equipmentId = s.equipmentID();

	m_channel = s.channel();
	m_inOutType = s.inOutType();
	m_signalType = s.signalType();
	m_analogSignalFormat = s.analogSignalFormat();
	m_byteOrder = s.byteOrder();

	m_unit = s.unit();

	m_lowValidRange = s.lowValidRange();
	m_highValidRange = s.highValidRange();
	m_lowEngeneeringUnits = s.lowEngeneeringUnits();
	m_highEngeneeringUnits = s.highEngeneeringUnits();

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

	return true;
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
}

Hash AppSignalParam::hash() const
{
	return m_hash;
}
void AppSignalParam::setHash(Hash value)
{
	m_hash = value;
}

QString AppSignalParam::appSignalId() const
{
	return m_appSignalId;
}

void AppSignalParam::setAppSignalId(const QString& value)
{
	m_appSignalId = value;
}

QString AppSignalParam::customSignalId() const
{
	return m_customSignalId;
}

void AppSignalParam::setCustomSignalId(const QString& value)
{
	m_customSignalId = value;
}

QString AppSignalParam::caption() const
{
	return m_caption;
}

void AppSignalParam::setCaption(const QString& value)
{
	m_caption = value;
}


QString AppSignalParam::equipmentId() const
{
	return m_equipmentId;
}

void AppSignalParam::setEquipmentId(const QString& value)
{
	m_equipmentId = value;
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

TuningValueType AppSignalParam::toTuningType() const
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

QString AppSignalParam::unit() const
{
	return m_unit;
}

void AppSignalParam::setUnit(QString value)
{
	m_unit = value;
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
	return m_lowEngeneeringUnits;
}

void AppSignalParam::setLowEngineeringUnits(double value)
{
	m_lowEngeneeringUnits = value;
}

double AppSignalParam::highEngineeringUnits() const
{
	return m_highEngeneeringUnits;
}

void AppSignalParam::setHighEngineeringUnits(double value)
{
	m_highEngeneeringUnits = value;
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
