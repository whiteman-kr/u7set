#include "../lib/AppSignal.h"
#include "../Proto/serialization.pb.h"

const char* AppSignalParamMimeType::value ="application/x-appsignalparam";		// Data in format ::Proto::AppSiagnalParamSet


QDateTime Times::systemToDateTime() const
{
	return system.toDateTime();
}

QDateTime Times::localToDateTime() const
{
	return local.toDateTime();
}

QDateTime Times::plantToDateTime() const
{
	return plant.toDateTime();
}

void AppSignalStateFlags::clear()
{
	all = 0;
}

void AppSignalStateFlags::clearReasonsFlags()
{
	validityChange = 0;
	autoPoint = 0;
	roughAperture = 0;
	smoothAperture = 0;
}

bool AppSignalStateFlags::hasArchivingReason() const
{
	return	validityChange == 1 ||
			autoPoint == 1 ||
			roughAperture == 1 ||
			smoothAperture == 1;
}

bool AppSignalStateFlags::hasShortTermArchivingReasonOnly() const
{
	return	validityChange == 0 &&
			autoPoint == 0 &&
			roughAperture == 0 &&
			smoothAperture == 1;
}

Hash AppSignalState::hash() const
{
	return m_hash;
}

const Times& AppSignalState::time() const
{
	return m_time;
}

double AppSignalState::value() const
{
	return m_value;
}

bool AppSignalState::isValid() const
{
	return m_flags.valid;
}

//bool AppSignalState::isOverflow() const
//{
//	return flags.overflow;
//}

//bool AppSignalState::isUnderflow() const
//{
//	return flags.underflow;
//}

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

const AppSignalState& AppSignalState::operator = (const SimpleAppSignalState& smState)
{
	m_hash = smState.hash;
	m_time = smState.time;
	m_flags = smState.flags;
	m_value = smState.value;

	return *this;
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

AppSignalParam::AppSignalParam()
{
}

bool AppSignalParam::load(const ::Proto::AppSignal& message)
{
	m_hash = message.hash();
	m_appSignalId = QString::fromStdString(message.appsignalid());
	m_customSignalId = QString::fromStdString(message.customappsignalid());
	m_caption = QString::fromStdString(message.caption());
	m_equipmentId = QString::fromStdString(message.equipmentid());

	m_channel = static_cast<E::Channel>(message.channel());
	m_inOutType = static_cast<E::SignalInOutType>(message.inouttype());
	m_signalType = static_cast<E::SignalType>(message.signaltype());
	m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(message.analogsignalformat());
	m_byteOrder = static_cast<E::ByteOrder>(message.byteorder());

	m_unit = QString::fromStdString(message.unit());

	m_lowValidRange = message.lowvalidrange();
	m_highValidRange = message.highvalidrange();
	m_lowEngeneeringUnits = message.lowengeneeringunits();
	m_highEngeneeringUnits = message.highengeneeringunits();

	m_electricLowLimit = message.electriclowlimit();
	m_electricHighLimit = message.electrichighlimit();
	m_electricUnit = static_cast<E::ElectricUnit>(message.electricunit());
	m_sensorType = static_cast<E::SensorType>(message.sensortype());
	m_outputMode = static_cast<E::OutputMode>(message.outputmode());

	m_precision = message.decimalplaces();
	m_coarseAperture = message.coarseaperture();
	m_fineAperture = message.fineaperture();
	m_filteringTime = message.filteringtime();
	m_spreadTolerance = message.spreadtolerance();
	m_enableTuning = message.enabletuning();
	m_tuningDefaultValue = message.tuningdefaultvalue();

	return true;
}

void AppSignalParam::save(::Proto::AppSignal *message) const
{
	if (message == nullptr)
	{
		assert(message);
		return;
	}

	message->set_hash(m_hash);
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

	message->set_lowvalidrange(m_lowValidRange);
	message->set_highvalidrange(m_highValidRange);
	message->set_lowengeneeringunits(m_lowEngeneeringUnits);
	message->set_highengeneeringunits(m_highEngeneeringUnits);

	message->set_electriclowlimit(m_electricLowLimit);
	message->set_electrichighlimit(m_electricHighLimit);
	message->set_electricunit(m_electricUnit);
	message->set_sensortype(m_sensorType);
	message->set_outputmode(m_outputMode);

	message->set_decimalplaces(m_precision);
	message->set_coarseaperture(m_coarseAperture);
	message->set_fineaperture(m_fineAperture);
	message->set_filteringtime(m_filteringTime);
	message->set_spreadtolerance(m_spreadTolerance);
	message->set_enabletuning(m_enableTuning);
	message->set_tuningdefaultvalue(m_tuningDefaultValue);
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

double AppSignalParam::tuningDefaultValue() const
{
	return m_tuningDefaultValue;
}

void AppSignalParam::setTuningDefaultValue(double value)
{
	m_tuningDefaultValue = value;
}
