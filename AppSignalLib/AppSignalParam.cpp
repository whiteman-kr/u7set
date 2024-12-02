#ifndef APP_SIGNAL_LIB_DOMAIN
#error Do not include this file in the project! Link AppSignalLib instead.
#endif

#include "AppSignalParam.h"
#include "AppSignal.h"
#include "AppSignalSpecPropValues.h"

const char* AppSignalParamMimeType::value ="application/x-appsignalparam";		// Data in format ::Proto::AppSignalParamSet


// -------------------------------------------------------------------------------------------------
//
// AppSignalParam class implementation
//
// -------------------------------------------------------------------------------------------------

AppSignalParam::PrivateData::PrivateData() = default;
AppSignalParam::PrivateData::~PrivateData() = default;

AppSignalParam::AppSignalParam() = default;

AppSignalParam::AppSignalParam(const AppSignalParam&) = default;
AppSignalParam::AppSignalParam(AppSignalParam&&) noexcept = default;

AppSignalParam& AppSignalParam::operator=(const AppSignalParam&) = default;
AppSignalParam& AppSignalParam::operator=(AppSignalParam&&) noexcept = default;

AppSignalParam::AppSignalParam(const AppSignal& signal)
{
	load(signal);
}

bool AppSignalParam::load(const ::Proto::AppSignal& message)
{
	detach();
	return m_data->load(message);
}

void AppSignalParam::load(const AppSignal& s)
{
	detach();
	return m_data->load(s);
}

void AppSignalParam::save(::Proto::AppSignal* message) const
{
	return m_data->save(message);
}

AppSignalParam AppSignalParam::clone() const
{
	::Proto::AppSignal buffer;
	save(&buffer);

	AppSignalParam result;
	result.load(buffer);

	return result;
}

Hash AppSignalParam::hash() const
{
	return m_data->m_hash;
}
void AppSignalParam::setHash(Hash value)
{
	detach();
	m_data->m_hash = value;
}

QString AppSignalParam::appSignalId() const
{
	return m_data->m_appSignalId;
}

void AppSignalParam::setAppSignalId(const QString& value)
{
	detach();

	m_data->m_appSignalId = value;
	setHash(::calcHash(value));

	return;
}

QString AppSignalParam::customSignalId() const
{
	return m_data->m_customSignalId;
}

void AppSignalParam::setCustomSignalId(const QString& value)
{
	detach();
	m_data->m_customSignalId = value;
}

QString AppSignalParam::caption() const
{
	return m_data->m_caption;
}

void AppSignalParam::setCaption(const QString& value)
{
	detach();
	m_data->m_caption = value;
}


QString AppSignalParam::equipmentId() const
{
	return m_data->m_equipmentId;
}

void AppSignalParam::setEquipmentId(const QString& value)
{
	detach();
	m_data->m_equipmentId = value;
}

QString AppSignalParam::lmEquipmentId() const
{
	return m_data->m_lmEquipmentId;
}

void AppSignalParam::setLmEquipmentId(const QString& value)
{
	detach();
	m_data->m_lmEquipmentId = value;
}

E::Channel AppSignalParam::channel() const
{
	return m_data->m_channel;
}

void AppSignalParam::setChannel(E::Channel value)
{
	detach();
	m_data->m_channel = value;
}

bool AppSignalParam::isInput() const
{
	return m_data->m_inOutType == E::SignalInOutType::Input;
}

bool AppSignalParam::isOutput() const
{
	return m_data->m_inOutType == E::SignalInOutType::Output;
}

bool AppSignalParam::isInternal() const
{
	return m_data->m_inOutType == E::SignalInOutType::Internal;
}

bool AppSignalParam::isSwCalculated() const
{
	return m_data->m_inOutType == E::SignalInOutType::SoftwareCalculated;
}

E::SignalInOutType AppSignalParam::inOutType() const
{
	return m_data->m_inOutType;
}

void AppSignalParam::setInOutType(E::SignalInOutType value)
{
	detach();
	m_data->m_inOutType = value;
}

bool AppSignalParam::isAnalog() const
{
	return m_data->m_signalType == E::SignalType::Analog;
}

bool AppSignalParam::isDiscrete() const
{
	return m_data->m_signalType == E::SignalType::Discrete;
}

bool AppSignalParam::isBus() const
{
	return m_data->m_signalType == E::SignalType::Bus;
}

E::SignalType AppSignalParam::type() const
{
	return m_data->m_signalType;
}

void AppSignalParam::setType(E::SignalType value)
{
	detach();
	m_data->m_signalType = value;
}

TuningValueType AppSignalParam::tuningType() const
{
	switch (m_data->m_signalType)
	{
	case E::Analog:
		switch (m_data->m_analogSignalFormat)
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
	return m_data->m_analogSignalFormat;
}

void AppSignalParam::setAnalogSignalFormat(E::AnalogAppSignalFormat value)
{
	detach();
	m_data->m_analogSignalFormat = value;
}

E::ByteOrder AppSignalParam::byteOrder() const
{
	return m_data->m_byteOrder;
}

void AppSignalParam::AppSignalParam::setByteOrder(E::ByteOrder value)
{
	detach();
	m_data->m_byteOrder = value;
}

QString AppSignalParam::units() const
{
	return m_data->m_units;
}

void AppSignalParam::setUnits(const QString& value)
{
	detach();
	m_data->m_units = value;
}

double AppSignalParam::lowValidRange() const
{
	return m_data->m_lowValidRange;
}

double AppSignalParam::highValidRange() const
{
	return m_data->m_highValidRange;
}

double AppSignalParam::lowEngineeringUnits() const
{
	return m_data->m_lowEngineeringUnits;
}

void AppSignalParam::setLowEngineeringUnits(double value)
{
	detach();
	m_data->m_lowEngineeringUnits = value;
}

double AppSignalParam::highEngineeringUnits() const
{
	return m_data->m_highEngineeringUnits;
}

void AppSignalParam::setHighEngineeringUnits(double value)
{
	detach();
	m_data->m_highEngineeringUnits = value;
}

double AppSignalParam::inputLowLimit() const
{
	return m_data->m_electricLowLimit;
}

double AppSignalParam::inputHighLimit() const
{
	return m_data->m_electricHighLimit;
}

E::ElectricUnit AppSignalParam::inputUnitId() const
{
	return m_data->m_electricUnit;
}

E::SensorType AppSignalParam::inputSensorType() const
{
	return m_data->m_sensorType;
}

double AppSignalParam::outputLowLimit() const
{
	return m_data->m_outputLowLimit;
}

double AppSignalParam::outputHighLimit() const
{
	return m_data->m_outputHighLimit;
}

int AppSignalParam::outputUnitId() const
{
	return m_data->m_outputUnitId;
}

E::OutputMode AppSignalParam::outputMode() const
{
	return m_data->m_outputMode;
}

E::SensorType AppSignalParam::outputSensorType() const
{
	return m_data->m_outputSensorType;
}

int AppSignalParam::precision() const
{
	return m_data->m_precision;
}

void AppSignalParam::setPrecision(int value)
{
	detach();
	m_data->m_precision = value;
}

double AppSignalParam::fineAperture() const
{
	return m_data->m_fineAperture;
}

void AppSignalParam::setFineAperture(double value)
{
	detach();
	m_data->m_fineAperture = value;
}

double AppSignalParam::coarseAperture() const
{
	return m_data->m_coarseAperture;
}

void AppSignalParam::setCoarseAperture(double value)
{
	detach();
	m_data->m_coarseAperture = value;
}

double AppSignalParam::filteringTime() const
{
	return m_data->m_filteringTime;
}

void AppSignalParam::setFilteringTime(double value)
{
	detach();
	m_data->m_filteringTime = value;
}

double AppSignalParam::spreadTolerance() const
{
	return m_data->m_spreadTolerance;
}

void AppSignalParam::setSpreadTolerance(double value)
{
	detach();
	m_data->m_spreadTolerance = value;
}

bool AppSignalParam::enableTuning() const
{
	return m_data->m_enableTuning;
}

void AppSignalParam::setEnableTuning(bool value)
{
	detach();
	m_data->m_enableTuning = value;
}

bool AppSignalParam::isEndpoint() const
{
	return m_data->m_endpoint;
}

void AppSignalParam::setEndpoint(bool value)
{
	detach();
	m_data->m_endpoint = value;
}

bool AppSignalParam::isInverted() const
{
	return m_data->m_inverted;
}

void AppSignalParam::setInverted(bool value)
{
	detach();
	m_data->m_inverted = value;
}

bool AppSignalParam::isReserved() const
{
	return m_data->m_reserved;
}

void AppSignalParam::setReserved(bool value)
{
	detach();
	m_data->m_reserved = value;
}

TuningValue AppSignalParam::tuningDefaultValue() const
{
	return m_data->m_tuningDefaultValue;
}

QVariant AppSignalParam::tuningDefaultValueToVariant() const
{
	return m_data->m_tuningDefaultValue.toVariant();
}

void AppSignalParam::setTuningDefaultValue(const TuningValue& value)
{
	detach();
	m_data->m_tuningDefaultValue = value;
}

TuningValue AppSignalParam::tuningLowBound() const
{
	return m_data->m_tuningLowBound;
}

QVariant AppSignalParam::tuningLowBoundToVariant() const
{
	return m_data->m_tuningLowBound.toVariant();
}

void AppSignalParam::setTuningLowBound(const TuningValue& value)
{
	detach();
	m_data->m_tuningLowBound = value;
}

TuningValue AppSignalParam::tuningHighBound() const
{
	return m_data->m_tuningHighBound;
}

QVariant AppSignalParam::tuningHighBoundToVariant() const
{
	return m_data->m_tuningHighBound.toVariant();
}

void AppSignalParam::setTuningHighBound(const TuningValue& value)
{
	detach();
	m_data->m_tuningHighBound = value;
}

std::set<QString> AppSignalParam::tags() const
{
	return m_data->m_tags;
}

QStringList AppSignalParam::tagStringList() const
{
	QStringList result;
	result.reserve(static_cast<int>(m_data->m_tags.size()));

	for (const QString& tag : m_data->m_tags)
	{
		result << tag;
	}

	return result;
}

void AppSignalParam::setTags(std::set<QString> tags)
{
	detach();
	m_data->m_tags = std::move(tags);
}

const QString& AppSignalParam::specificPropertyStruct() const
{
	return m_data->m_specPropStruct;
}

const QByteArray& AppSignalParam::protoSpecificPropertyValues() const
{
	return m_data->m_specPropValues;
}

const AppSignalSpecPropValues& AppSignalParam::specificPropertyValues() const
{
	if (m_data->m_specificPropertyValues == nullptr)
	{
		m_data->m_specificPropertyValues = std::make_unique<AppSignalSpecPropValues>();
		m_data->m_specificPropertyValues->create(*this);
	}

	return *m_data->m_specificPropertyValues;
}

bool AppSignalParam::hasTag(const QString& tag) const
{
	return m_data->m_tags.contains(tag);
}

QVariant AppSignalParam::specificPropertyValue(const QString& propertyName) const
{
	QVariant result;
	specificPropertyValues().getValue(propertyName, &result);

	return result;
}

bool AppSignalParam::specificPropertyExists(const QString& propertyName) const
{
	return specificPropertyValues().isExists(propertyName);
}

void AppSignalParam::detach()
{
	if (m_data.use_count() == 0)
	{
		m_data = std::make_shared<PrivateData>();
		return;
	}

	if (m_data.use_count() == 1)
	{
		return;
	}

	*this = clone();
	return;
}

bool AppSignalParam::PrivateData::load(const ::Proto::AppSignal& message)
{
	AppSignal s;

	s.loadFromProto(message);
	s.cacheSpecPropValues();

	m_hash = message.calcparam().hash();

	load(s);

	return true;
}

void AppSignalParam::PrivateData::load(const AppSignal& s)
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

	m_units = s.unit();

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
	m_endpoint = s.isEndpoint();
	m_inverted = s.invertSignal();
	m_reserved = s.reserved();

	m_tuningDefaultValue = s.tuningDefaultValue();
	m_tuningLowBound = s.tuningLowBound();
	m_tuningHighBound = s.tuningHighBound();

	m_specPropStruct = s.specPropStruct();
	m_specPropValues = s.protoSpecPropValues();

	m_tags = s.tagsSet();
}

void AppSignalParam::PrivateData::save(::Proto::AppSignal* message) const
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

	message->set_unit(m_units.toStdString());

	message->set_decimalplaces(m_precision);
	message->set_coarseaperture(m_coarseAperture);
	message->set_fineaperture(m_fineAperture);
	message->set_enabletuning(m_enableTuning);

	m_tuningDefaultValue.save(message->mutable_tuningdefaultvalue());
	m_tuningLowBound.save(message->mutable_tuninglowbound());
	m_tuningHighBound.save(message->mutable_tuninghighbound());

	message->set_specpropstruct(m_specPropStruct.toStdString());
	message->set_specpropvalues(m_specPropValues.constData(), m_specPropValues.size());

	message->set_invertsignal(m_inverted);
	message->set_reserved(m_reserved);

	// Signal properties calculated in compile-time

	Proto::AppSignalCalculatedParam* calcParam = message->mutable_calcparam();

	if (calcParam != nullptr)
	{
		calcParam->set_isendpoint(m_endpoint);
	}


	// Tags
	//
	message->clear_tags();
	for (const QString& t : m_tags)
	{
		message->add_tags(t.toStdString());
	}

	return;
}
