#pragma once

#include <QtGlobal>
#include <QDateTime>
#include <memory>
#include <set>

#include "../UtilsLib/Hash.h"
#include "../UtilsLib/Queue.h"
#include "../CommonLib/Types.h"
#include "../OnlineLib/SimpleAppSignalState.h"

#include "TimeStamp.h"
#include "Tuning/TuningSignalState.h"
#include "Times.h"
#include "AppSignal.h"

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
	if (state.valid == true)
	{
		// Put signal value to a schema item
		//
		schemaItemValue.Text = signalState.value;
	}
	\endcode
*/
class AppSignalState
{
	Q_GADGET

	/// \brief Contains a unique 64-bit hash of a signal identifier
	Q_PROPERTY(Hash hash READ hash)
	Q_PROPERTY(Hash Hash READ hash)

	/*! \brief Contains current signal value

		Contains current signal value. For discrete signals <b>"False"</b> is equal to <b>0</b>, <b>"True"</b> is equal to <b>1</b>.

		\warning Be careful when comparing values. Remember that <b>double</b> can't be compared directly,
		because doubles and floats cannot express every numerical value. They are using approximations to represent the value.
		It is recommended to make comparsions as follows, especially analog values:

		\code
		var a = state.value;
		var b = 1.5;

		var threshold = 0.0000001;

		if (Math.abs(a - b) <= threshold)
		{
		...
		}
		\endcode
	*/
	Q_PROPERTY(double value READ value)
	Q_PROPERTY(double Value READ value)

	/// \brief Contains signal validity flag
	Q_PROPERTY(bool valid READ isValid)
	Q_PROPERTY(bool Valid READ isValid)

	/// \brief Signal value is received from Application Data Service
	Q_PROPERTY(bool stateAvailable READ isStateAvailable)
	Q_PROPERTY(bool StateAvailable READ isStateAvailable)

	/// \brief Signal value simulated flag (see AFB simlock)
	Q_PROPERTY(bool simulated READ isSimulated)
	Q_PROPERTY(bool Simulated READ isSimulated)

	/// \brief Signal value blocked flag (see AFB simlock)
	Q_PROPERTY(bool blocked READ isBlocked)
	Q_PROPERTY(bool Blocked READ isBlocked)

	/// \brief Signal value mismatch flag (see AFB mismatch)
	Q_PROPERTY(bool mismatch READ isMismatch)
	Q_PROPERTY(bool Mismatch READ isMismatch)

	/// \brief Signal value is above high limit
	Q_PROPERTY(bool aboveHighLimit READ isAboveHighLimit)
	Q_PROPERTY(bool AboveHighLimit READ isAboveHighLimit)

	/// \brief Signal value is below low limit
	Q_PROPERTY(bool belowLowLimit READ isBelowLowLimit)
	Q_PROPERTY(bool BelowLowLimit READ isBelowLowLimit)

	/// \brief Signal value is out of limits
	Q_PROPERTY(bool outOfLimits READ isOutOfLimits)
	Q_PROPERTY(bool OutOfLimits READ isOutOfLimits)

public:
	AppSignalState() = default;
	AppSignalState(const AppSignalState&) = default;
	AppSignalState(AppSignalState&&) = default;
	AppSignalState(const Proto::AppSignalState& protoState);
	~AppSignalState() = default;

	AppSignalState& operator= (const AppSignalState& state) = default;
	AppSignalState& operator= (const SimpleAppSignalState& smState);

	[[nodiscard]] Hash hash() const;
	[[nodiscard]] const Times& time() const;
	[[nodiscard]] const TimeStamp& time(E::TimeType timeType) const;
	[[nodiscard]] double value() const noexcept;

	[[nodiscard]] bool isValid() const noexcept;
	[[nodiscard]] bool isStateAvailable() const;
	[[nodiscard]] bool isSimulated() const;
	[[nodiscard]] bool isBlocked() const;
	[[nodiscard]] bool isMismatch() const;
	[[nodiscard]] bool isAboveHighLimit() const;
	[[nodiscard]] bool isBelowLowLimit() const;
	[[nodiscard]] bool isOutOfLimits() const;		//  isAboveHighLimit() || isBelowLowLimit()

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

	[[nodiscard]] bool hasSameValue(const AppSignalState& b) const;

	[[nodiscard]] static QString toString(double value, E::ValueViewType viewType, E::AnalogFormat analogFormat, int precision);

public:
	Hash m_hash = 0;					// == ::calcHash(AppSignalID)
	Times m_time;
	AppSignalStateFlags m_flags;
	double m_value = 0;

	static const quint32 VALID = 1;
	static const quint32 INVALID = 0;
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
	schemaItemValue.Text = param1.caption;

	// Check if signal B is analog and input
	//
	if (param2.isAnalog)
	{
		if (param2.isInput)
		{
			// Create a string with template "caption, units"
			//
			var units = param2.unit;

			var text = param2.caption + ", " + units;
			...
		}
	}
	\endcode
*/
class AppSignalParam
{
	Q_GADGET

	/// \brief Contains a 64-bit hash of a signal
	Q_PROPERTY(Hash hash READ hash)
	Q_PROPERTY(Hash Hash READ hash)

	/// \brief Application Signal Identifier
	Q_PROPERTY(QString appSignalID READ appSignalId)
	Q_PROPERTY(QString AppSignalID READ appSignalId)

	/// \brief Custom Application Signal Identifier
	Q_PROPERTY(QString customAppSignalID READ customSignalId)
	Q_PROPERTY(QString CustomAppSignalID READ customSignalId)

	Q_PROPERTY(QString customSignalID READ customSignalId)	// same as CustomAppSignalID, for compatibility
	Q_PROPERTY(QString CustomSignalID READ customSignalId)	// same as CustomAppSignalID, for compatibility

	/// \brief Signal Caption
	Q_PROPERTY(QString caption READ caption)
	Q_PROPERTY(QString Caption READ caption)

	/// \brief Signal EquipmentID, can be port EquipmentID for inputs/ouputs or LmEquipmentID for internal signals.
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

	/// \brief Signal Equipment ID
	Q_PROPERTY(QString lmEquipmentID READ lmEquipmentId)
	Q_PROPERTY(QString LmEquipmentID READ lmEquipmentId)

	/// \brief Signal Measure Units
	Q_PROPERTY(QString unit READ unit)
	Q_PROPERTY(QString Unit READ unit)

	/// \brief Signal precision (digits after point)
	Q_PROPERTY(int precision READ precision)
	Q_PROPERTY(int Precision READ precision)

	/// \brief Signal channel
	Q_PROPERTY(E::Channel channel READ channel)
	Q_PROPERTY(E::Channel Channel READ channel)

	/// \brief Low Valid Range
	Q_PROPERTY(double lowValidRange READ lowValidRange)
	Q_PROPERTY(double LowValidRange READ lowValidRange)

	/// \brief High Valid Range
	Q_PROPERTY(double highValidRange READ highValidRange)
	Q_PROPERTY(double HighValidRange READ highValidRange)

	/// \brief Low Engineering Units
	Q_PROPERTY(double lowEngineeringUnits READ lowEngineeringUnits)
	Q_PROPERTY(double LowEngineeringUnits READ lowEngineeringUnits)

	/// \brief High Engineering Units
	Q_PROPERTY(double highEngineeringUnits READ highEngineeringUnits)
	Q_PROPERTY(double HighEngineeringUnits READ highEngineeringUnits)

	/// \brief Signal is tunable
	Q_PROPERTY(bool enableTuning READ enableTuning)
	Q_PROPERTY(bool EnableTuning READ enableTuning)

	/// \brief Default value of tunable signal
	Q_PROPERTY(QVariant tuningDefaultValue READ tuningDefaultValueToVariant)
	Q_PROPERTY(QVariant TuningDefaultValue READ tuningDefaultValueToVariant)

	/// \brief Low tuning limit of the signal
	Q_PROPERTY(QVariant tuningLowBound READ tuningLowBoundToVariant)
	Q_PROPERTY(QVariant TuningLowBound READ tuningLowBoundToVariant)

	/// \brief High tuning limit of the signal
	Q_PROPERTY(QVariant tuningHighBound READ tuningHighBoundToVariant)
	Q_PROPERTY(QVariant TuningHighBound READ tuningHighBoundToVariant)

	/// \brief Signal is input
	Q_PROPERTY(bool isInput READ isInput)
	Q_PROPERTY(bool IsInput READ isInput)

	/// \brief Signal is output
	Q_PROPERTY(bool isOutput READ isOutput)
	Q_PROPERTY(bool IsOutput READ isOutput)

	/// \brief Signal is internal
	Q_PROPERTY(bool isInternal READ isInternal)
	Q_PROPERTY(bool IsInternal READ isInternal)

	/// \brief Signal is analog
	Q_PROPERTY(bool isAnalog READ isAnalog)
	Q_PROPERTY(bool IsAnalog READ isAnalog)

	/// \brief Signal is discrete
	Q_PROPERTY(bool isDiscrete READ isDiscrete)
	Q_PROPERTY(bool IsDiscrete READ isDiscrete)

public:
	AppSignalParam() = default;
	AppSignalParam(const AppSignalParam&) = default;
	AppSignalParam(const AppSignal& signal);

	bool load(const Proto::AppSignal& message);
	void load(const AppSignal& signal);
	void save(::Proto::AppSignal* message) const;

	// Properties
	//
public:
	[[nodiscard]] Hash hash() const;
	void setHash(Hash value);

	[[nodiscard]] const QString& appSignalId() const;
	void setAppSignalId(const QString& value);

	[[nodiscard]] const QString& customSignalId() const;
	void setCustomSignalId(const QString& value);

	[[nodiscard]] const QString& caption() const;
	void setCaption(const QString& value);

	[[nodiscard]] const QString& equipmentId() const;
	void setEquipmentId(const QString& value);

	[[nodiscard]] const QString& lmEquipmentId() const;
	void setLmEquipmentId(const QString& value);

	[[nodiscard]] E::Channel channel() const;
	void setChannel(E::Channel value);

	[[nodiscard]] bool isInput() const;
	[[nodiscard]] bool isOutput() const;
	[[nodiscard]] bool isInternal() const;
	[[nodiscard]] E::SignalInOutType inOutType() const;
	void setInOutType(E::SignalInOutType value);

	[[nodiscard]] bool isAnalog() const;
	[[nodiscard]] bool isDiscrete() const;
	[[nodiscard]] E::SignalType type() const;
	void setType(E::SignalType value);

	[[nodiscard]] TuningValueType tuningType() const;

	[[nodiscard]] E::AnalogAppSignalFormat analogSignalFormat() const;
	void setAnalogSignalFormat(E::AnalogAppSignalFormat value);

	[[nodiscard]] E::ByteOrder byteOrder() const;
	void setByteOrder(E::ByteOrder value);

	[[nodiscard]] int unitId() const;
	void setUnitId(int value);

	[[nodiscard]] const QString& unit() const;
	void setUnit(QString value);

	[[nodiscard]] double lowValidRange() const;
	[[nodiscard]] double highValidRange() const;

	[[nodiscard]] double lowEngineeringUnits() const;
	void setLowEngineeringUnits(double value);

	[[nodiscard]] double highEngineeringUnits() const;
	void setHighEngineeringUnits(double value);

	[[nodiscard]] double inputLowLimit() const;
	[[nodiscard]] double inputHighLimit() const;
	[[nodiscard]] E::ElectricUnit inputUnitId() const;
	[[nodiscard]] E::SensorType inputSensorType() const;

	[[nodiscard]] double outputLowLimit() const;
	[[nodiscard]] double outputHighLimit() const;
	[[nodiscard]] int outputUnitId() const;
	[[nodiscard]] E::OutputMode outputMode() const;
	[[nodiscard]] E::SensorType outputSensorType() const;

	[[nodiscard]] int precision() const;
	void setPrecision(int value);

	[[nodiscard]] double aperture();
	void setAperture(double value);

	[[nodiscard]] double filteringTime();
	void setFilteringTime(double value);

	[[nodiscard]] double spreadTolerance();
	void setSpreadTolerance(double value);

	[[nodiscard]] bool enableTuning() const;
	void setEnableTuning(bool value);

	[[nodiscard]] TuningValue tuningDefaultValue() const;
	[[nodiscard]] QVariant tuningDefaultValueToVariant() const;
	void setTuningDefaultValue(const TuningValue& value);

	[[nodiscard]] TuningValue tuningLowBound() const;
	[[nodiscard]] QVariant tuningLowBoundToVariant() const;
	void setTuningLowBound(const TuningValue& value);

	[[nodiscard]] TuningValue tuningHighBound() const;
	[[nodiscard]] QVariant tuningHighBoundToVariant() const;
	void setTuningHighBound(const TuningValue& value);

	[[nodiscard]] const std::set<QString>& tags() const;
	[[nodiscard]] std::set<QString>& mutableTags();
	[[nodiscard]] QStringList tagStringList() const;

	void setTags(std::set<QString> tags);

public slots:
	/// \brief Check if signal has specified tag
	[[nodiscard]] bool hasTag(const QString& tag) const;

public:
	static const int NO_UNIT_ID = 1;

private:
	Hash m_hash = 0;					// Hash from m_appSignalId
	QString m_appSignalId;
	QString m_customSignalId;
	QString m_caption;
	QString m_equipmentId;
	QString m_lmEquipmentId;

	E::Channel m_channel = E::Channel::A;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
	E::SignalType m_signalType = E::SignalType::Analog;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	QString m_unit;

	double m_lowValidRange = 0;
	double m_highValidRange = 100;
	double m_lowEngineeringUnits = 0;
	double m_highEngineeringUnits = 100;

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

	std::set<QString> m_tags;
};

Q_DECLARE_METATYPE(AppSignalParam)
