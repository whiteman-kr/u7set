#pragma once

#include <QtGlobal>
#include <QDateTime>
#include <memory>

#include "Hash.h"
#include "Queue.h"
#include "TimeStamp.h"
#include "Tuning/TuningSignalState.h"
#include "Times.h"
#include "Types.h"
#include "AppSignalStateFlags.h"
#include "SimpleAppSignalState.h"

struct AppSignalParamMimeType
{
	static const char* value;	// = "application/x-appsignalparam";	Data in format ::Proto::AppSiagnalParamSet
};

namespace Proto
{
	class AppSignalState;
	class AppSignal;
}


/*! \class AppSignalState
	\ingroup groupParamsStates
	\brief Describes signal state in Monitor application

	AppSignalState class describes signal state in Monitor application.	This state is received from ApplicationDataService.
	\ref VFrame30::ScriptAppSignalController "ScriptAppSignalController" class accesed by global <b>signals</b> object is used for requesting signal states.

	\warning
	After requesting signal state it is highly recommended to check function return values, because errors can occur. For example,
	connection to ApplicationDataService can be down, or signal with specified identifier could not exist.

	<b>Example:</b>

	\code
	// Request signal state by identifier "#APPSIGNALID"
	//
	var state = signals.signalState("#APPSIGNALID");

	if (state == undefined)
	{
		// No state was received for this signal
		//
		return;
	}

	// Check signal validity
	//
	if (state.Valid == true)
	{
		// Put signal value to a schema item
		//
		schemaItemValue.Text = signalState.Value;
	}
	\endcode
*/
class AppSignalState
{
	Q_GADGET

	/// \brief Contains a unique 64-bit hash of a signal identifier
	Q_PROPERTY(Hash Hash READ hash)

	/// \brief Contains current signal value
	Q_PROPERTY(double Value READ value)

	/// \brief Contains signal validity flag
	Q_PROPERTY(bool Valid READ isValid)

	/// \brief Signal value is received from Application Data Service
	Q_PROPERTY(bool StateAvailable READ isStateAvailable)

	/// \brief Signal value simulated flag (see AFB simlock)
	Q_PROPERTY(bool Simulated READ isSimulated)

	/// \brief Signal value blocked flag (see AFB simlock)
	Q_PROPERTY(bool Blocked READ isBlocked)

	/// \brief Signal value mismatch flag (see AFB mismatch)
	Q_PROPERTY(bool Mismatch READ isMismatch)

	/// \brief Signal value is above high limit
	Q_PROPERTY(bool AboveHighLimit READ isAboveHighLimit)

	/// \brief Signal value is below low limit
	Q_PROPERTY(bool BelowLowLimit READ isBelowLowLimit)

	/// \brief Signal value is out of limits
	Q_PROPERTY(bool OutOfLimits READ isOutOfLimits)

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

	Hash hash() const;
	const Times& time() const;
	const TimeStamp& time(E::TimeType timeType) const;
	double value() const noexcept;

	bool isValid() const noexcept;
	bool isStateAvailable() const;
	bool isSimulated() const;
	bool isBlocked() const;
	bool isMismatch() const;
	bool isAboveHighLimit() const;
	bool isBelowLowLimit() const;
	bool isOutOfLimits() const;		//  isAboveHighLimit() || isBelowLowLimit()

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

	static QString toString(double value, E::ValueViewType viewType, int precision);

public:
	Hash m_hash = 0;					// == calcHash(AppSignalID)
	Times m_time;
	AppSignalStateFlags m_flags;
	double m_value = 0;
};

Q_DECLARE_METATYPE(AppSignalState)


/*! \class AppSignalParam
	\ingroup groupParamsStates
	\brief Describes signal parameters in Monitor and TuningClient applications.

	AppSignalParam class describes signal parmeters in Monitor and TuningClient applications. This state is received from ApplicationDataService by Monitor or
	from TuningService by Monitor or TuningClient.

	\ref VFrame30::ScriptAppSignalController "ScriptAppSignalController" class accesed by global <b>signals</b> object is used for
	requesting signal parameters from Application Data Service.

	\ref VFrame30::TuningController "TuningController" class accesed by global <b>tuning</b> object is used for
	requesting signal parameters from Tuning Service.

	\warning
	TuningController is always available in TuningClient. In Monitor it is available only in non-safety projects when Tuning function is enabled.

	\n
	\warning
	After requesting signal state it is highly recommended to check function return values, because errors can occur. For example,
	connection to a service can be down, or signal with specified identifier could not exist.

	<b>Example:</b>

	\code
	var appSignalIdA = "#APPSIGNAL01";	// Signal A
	var appSignalIdB = "#APPSIGNAL02";	// Signal B

	// Get parameters from Application Data Service
	//
	var param1 = signals.signalParam(appSignalIdA);

	// Get parameters from Tuning Service
	//
	var param2 = tuning.signalParam(appSignalIdB);

	if (param1 == undefined || param2 == undefined)
	{
		// Some signals do not exist or getting their parameters failed
		//
		return;
	}

	// Output parameters of signal A
	//
	schemaItemValue.Text = param1.Caption;

	// Check if signal B is analog and input
	//
	if (param2.IsAnalog)
	{
		if (param2.IsInput)
		{
			// Create a string with template "Caption, Units"
			//
			var units = param2.Unit;

			var text = param2.Caption + ", " + units;
			...
		}
	}
	\endcode
*/
class AppSignalParam
{
	Q_GADGET

	/// \brief Contains a 64-bit hash of a signal
	Q_PROPERTY(Hash Hash READ hash)

	/// \brief Application Signal Identifier
	Q_PROPERTY(QString AppSignalID READ appSignalId)

	/// \brief Custom Application Signal Identifier
	Q_PROPERTY(QString CustomSignalID READ customSignalId)

	/// \brief Signal Caption
	Q_PROPERTY(QString Caption READ caption)

	/// \brief Signal Equipment ID
	Q_PROPERTY(QString EquipmentID READ equipmentId)

	/// \brief Signal Measure Units
	Q_PROPERTY(QString Unit READ unit)

	/// \brief Signal precision (digits after point)
	Q_PROPERTY(int Precision READ precision)

	/// \brief Signal channel
	Q_PROPERTY(E::Channel Channel READ channel)

	/// \brief Signal is tunable
	Q_PROPERTY(bool EnableTuning READ enableTuning)

	/// \brief Default value of tunable signal
	Q_PROPERTY(QVariant TuningDefaultValue READ tuningDefaultValueToVariant)

	/// \brief Low tuning limit of the signal
	Q_PROPERTY(QVariant TuningLowBound READ tuningLowBoundToVariant)

	/// \brief High tuning limit of the signal
	Q_PROPERTY(QVariant TuningHighBound READ tuningHighBoundToVariant)

	/// \brief Signal is input
	Q_PROPERTY(bool IsInput READ isInput)

	/// \brief Signal is output
	Q_PROPERTY(bool IsOutput READ isOutput)

	/// \brief Signal is internal
	Q_PROPERTY(bool IsInternal READ isInternal)

	/// \brief Signal is analog
	Q_PROPERTY(bool IsAnalog READ isAnalog)

	/// \brief Signal is discrete
	Q_PROPERTY(bool IsDiscrete READ isDiscrete)

public:
	AppSignalParam();
	AppSignalParam(const AppSignalParam&) = default;

	bool load(const Proto::AppSignal& message);
	void save(::Proto::AppSignal* message) const;

	// Properties
	//
public:
	Hash hash() const;
	void setHash(Hash value);

	QString appSignalId() const;
	void setAppSignalId(const QString& value);

	QString customSignalId() const;
	void setCustomSignalId(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	QString equipmentId() const;
	void setEquipmentId(const QString& value);

	E::Channel channel() const;
	void setChannel(E::Channel value);

	bool isInput() const;
	bool isOutput() const;
	bool isInternal() const;
	E::SignalInOutType inOutType() const;
	void setInOutType(E::SignalInOutType value);

	bool isAnalog() const;
	bool isDiscrete() const;
	E::SignalType type() const;
	void setType(E::SignalType value);

	TuningValueType toTuningType() const;

	E::AnalogAppSignalFormat analogSignalFormat() const;
	void setAnalogSignalFormat(E::AnalogAppSignalFormat value);

	E::ByteOrder byteOrder() const;
	void setByteOrder(E::ByteOrder value);

	int unitId() const;
	void setUnitId(int value);

	QString unit() const;
	void setUnit(QString value);

	double lowValidRange() const;
	double highValidRange() const;

	double lowEngineeringUnits() const;
	void setLowEngineeringUnits(double value);

	double highEngineeringUnits() const;
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

	int precision() const;
	void setPrecision(int value);

	double aperture();
	void setAperture(double value);

	double filteringTime();
	void setFilteringTime(double value);

	double spreadTolerance();
	void setSpreadTolerance(double value);

	bool enableTuning() const;
	void setEnableTuning(bool value);

	TuningValue tuningDefaultValue() const;
	QVariant tuningDefaultValueToVariant() const;
	void setTuningDefaultValue(const TuningValue& value);

	TuningValue tuningLowBound() const;
	QVariant tuningLowBoundToVariant() const;
	void setTuningLowBound(const TuningValue& value);

	TuningValue tuningHighBound() const;
	QVariant tuningHighBoundToVariant() const;
	void setTuningHighBound(const TuningValue& value);

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

	double m_electricLowLimit = 0;									// low electric value for input range
	double m_electricHighLimit = 0;									// high electric value for input range
	E::ElectricUnit m_electricUnit = E::ElectricUnit::NoUnit;		// electric unit for input range (mA, mV, Ohm, V ....)
	E::SensorType m_sensorType = E::SensorType::NoSensor;			// electric sensor type for input range (was created for m_inputUnitID)

	double m_outputLowLimit = 0;									// low physical value for output range
	double m_outputHighLimit = 0;									// high physical value for output range
	int m_outputUnitId = NO_UNIT_ID;								// physical unit for output range (kg, mm, Pa ...)
	E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;		// output electric range (or mode ref. OutputModeStr[])
	E::SensorType m_outputSensorType = E::SensorType::NoSensor;		// electric sensor type for output range (was created for m_outputMode)

	int m_precision = 2;
	double m_coarseAperture = 1;
	double m_fineAperture = 0.5;
	double m_filteringTime = 0.005;
	double m_spreadTolerance = 2;
	bool m_enableTuning = false;
	TuningValue m_tuningDefaultValue;
	TuningValue m_tuningLowBound;
	TuningValue m_tuningHighBound;

	QString m_specPropStruct;
	QByteArray m_specPropValues;
};

Q_DECLARE_METATYPE(AppSignalParam)
