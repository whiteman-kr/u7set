#pragma once

#include <QtGlobal>
#include <QDateTime>
#include "../lib/Hash.h"
#include "../lib/Queue.h"
#include "../lib/TimeStamp.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "Types.cpp"
#include "Types.h"


struct AppSignalParamMimeType
{
	static const char* value;	// = "application/x-appsignalparam";	Data in format ::Proto::AppSiagnalParamSet
};


namespace Proto
{
	class AppSignalState;
	class AppSignal;
	class AppSignalSet;
}


struct Times
{
	TimeStamp system;
	TimeStamp local;
	TimeStamp plant;

	QDateTime systemToDateTime() const;
	QDateTime localToDateTime() const;
	QDateTime plantToDateTime() const;
};


union AppSignalStateFlags
{
	struct
	{
		quint32	valid : 1;

		// reasons to archiving
		//
		quint32 smoothAperture : 1;
		quint32 roughAperture : 1;
		quint32 autoPoint : 1;
		quint32 validityChange : 1;
	};

	quint32 all;

	void clear();

	void clearReasonsFlags();

	bool hasArchivingReason() const;
	bool hasShortTermArchivingReasonOnly() const;
};


struct SimpleAppSignalState;

class AppSignalState
{
	Q_GADGET

	Q_PROPERTY(Hash Hash READ hash)
	Q_PROPERTY(double Value READ value)

public:
	AppSignalState() = default;
	AppSignalState(const AppSignalState&) = default;
	AppSignalState(AppSignalState&&) = default;
	AppSignalState(const Proto::AppSignalState& protoState);
	~AppSignalState() = default;

	AppSignalState& operator= (const AppSignalState& state) = default;
	AppSignalState& operator= (const SimpleAppSignalState& smState);

	static const quint32 VALID = 1;
	static const quint32 INVALID = 0;

	Q_INVOKABLE Hash hash() const;
	const Times& time() const;
	const TimeStamp& time(E::TimeType timeType) const;
	Q_INVOKABLE double value() const;

	Q_INVOKABLE bool isValid() const;

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

public:
	Hash m_hash = 0;					// == calcHash(AppSignalID)
	Times m_time;
	AppSignalStateFlags m_flags;
	double m_value = 0;
};

Q_DECLARE_METATYPE(AppSignalState)


struct SimpleAppSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;					// == calcHash(AppSignalID)
	Times time;
	AppSignalStateFlags flags;
	double value = 0;

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);
};

typedef Queue<SimpleAppSignalState> AppSignalStatesQueue;


class AppSignalParam
{
	Q_GADGET

	Q_PROPERTY(Hash Hash READ hash)
	Q_PROPERTY(QString AppSignalID READ appSignalId)
	Q_PROPERTY(QString CustimSignalID READ customSignalId)
	Q_PROPERTY(QString Caption READ caption)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

	Q_PROPERTY(E::Channel Channel READ channel)

	Q_PROPERTY(bool IsInput READ isInput)
	Q_PROPERTY(bool IsOutput READ isOutput)
	Q_PROPERTY(bool IsInternal READ isInternal)

	Q_PROPERTY(bool IsAnalog READ isAnalog)
	Q_PROPERTY(bool IsDiscrete READ isDiscrete)

public:
	AppSignalParam();

	bool load(const Proto::AppSignal &message);
	void save(::Proto::AppSignal* message) const;

	// Properties
	//
public:
	Hash hash() const;
	void setHash(Hash value);

	Q_INVOKABLE QString appSignalId() const;
	void setAppSignalId(const QString& value);

	Q_INVOKABLE QString customSignalId() const;
	void setCustomSignalId(const QString& value);

	Q_INVOKABLE QString caption() const;
	void setCaption(const QString& value);

	Q_INVOKABLE QString equipmentId() const;
	void setEquipmentId(const QString& value);

	E::Channel channel() const;
	void setChannel(E::Channel value);

	bool isInput() const;
	bool isOutput() const;
	bool isInternal() const;
	E::SignalInOutType inOutType() const;
	void setInOutType(E::SignalInOutType value);

	Q_INVOKABLE bool isAnalog() const;
	Q_INVOKABLE bool isDiscrete() const;
	E::SignalType type() const;
	void setType(E::SignalType value);

	TuningValueType toTuningType() const;

	E::AnalogAppSignalFormat analogSignalFormat() const;
	void setAnalogSignalFormat(E::AnalogAppSignalFormat value);

	E::ByteOrder byteOrder() const;
	void setByteOrder(E::ByteOrder value);

	int unitId() const;
	void setUnitId(int value);

	Q_INVOKABLE QString unit() const;
	void setUnit(QString value);

	Q_INVOKABLE double lowValidRange() const;
	Q_INVOKABLE double highValidRange() const;

	Q_INVOKABLE double lowEngineeringUnits() const;
	void setLowEngineeringUnits(double value);

	Q_INVOKABLE double highEngineeringUnits() const;
	void setHighEngineeringUnits(double value);

	double inputLowLimit() const;
	double inputHighLimit() const;
	E::ElectricUnit inputUnitId() const;
	E::SensorType inputSensorType() const;

	double outputLowLimit() const;
	double outputHighLimit() const;
	int outputUnitId() const;
	E::OutputMode outputMode() const;
	E::SensorType outputSensorType() const;

	Q_INVOKABLE int precision() const;
	void setPrecision(int value);

	double aperture();
	void setAperture(double value);

	double filteringTime();
	void setFilteringTime(double value);

	double spreadTolerance();
	void setSpreadTolerance(double value);

	Q_INVOKABLE bool enableTuning() const;
	void setEnableTuning(bool value);

	TuningValue tuningDefaultValue() const;
	void setTuningDefaultValue(const TuningValue& value);

	TuningValue tuningLowBound() const;
	void setTuningLowBound(const TuningValue& value);

	TuningValue tuningHighBound() const;
	void setTuningHighBound(const TuningValue& value);

	Q_INVOKABLE double getTuningDefaultValue() const;
	Q_INVOKABLE double getTuingLowBound() const;
	Q_INVOKABLE double getTuningHighBound() const;

public:
	static const int NO_UNIT_ID = 1;

private:
	Hash m_hash = 0;					// Hash from m_appSignalId
	QString m_appSignalId;
	QString m_customSignalId;
	QString m_caption;
	QString m_equipmentId;

	E::Channel m_channel = E::Channel::A;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
	E::SignalType m_signalType = E::SignalType::Analog;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	QString m_unit;

	double m_lowValidRange = 0;
	double m_highValidRange = 100;
	double m_lowEngeneeringUnits = 0;
	double m_highEngeneeringUnits = 100;

	double m_electricLowLimit = 0;										// low electric value for input range
	double m_electricHighLimit = 0;									// high electric value for input range
	E::ElectricUnit m_electricUnit = E::ElectricUnit::NoUnit;			// electric unit for input range (mA, mV, Ohm, V ....)
	E::SensorType m_sensorType = E::SensorType::NoSensorType;	// electric sensor type for input range (was created for m_inputUnitID)

	double m_outputLowLimit = 0;									// low physical value for output range
	double m_outputHighLimit = 0;									// high physical value for output range
	int m_outputUnitId = NO_UNIT_ID;								// physical unit for output range (kg, mm, Pa ...)
	E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;		// output electric range (or mode ref. OutputModeStr[])
	E::SensorType m_outputSensorType = E::SensorType::NoSensorType;	// electric sensor type for output range (was created for m_outputMode)

	int m_precision = 2;
	double m_coarseAperture = 1;
	double m_fineAperture = 0.5;
	double m_filteringTime = 0.005;
	double m_spreadTolerance = 2;
	bool m_enableTuning = false;
	TuningValue m_tuningDefaultValue;
	TuningValue m_tuningLowBound;
	TuningValue m_tuningHighBound;
};

Q_DECLARE_METATYPE(AppSignalParam)
