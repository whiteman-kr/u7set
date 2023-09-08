#pragma once

#include <QObject>
#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "AppSignalStateFlags.h"

namespace Proto
{
	class AppSignalState;
}


/*! \class AppSignalState
	\ingroup groupParamsStates
	\brief Describes signal state in Monitor application

	AppSignalState class describes signal state in Monitor application.	This state is received from ApplicationDataService.
	\ref VFrame30::ScriptAppSignalController "ScriptAppSignalController" class accessed by global <b>signals</b> object is used for requesting signal states.

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
		It is recommended to make comparisons as follows, especially analog values:

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

	/// \brief Tunable signal value is equal to tuningDefaultValue
	Q_PROPERTY(bool tuningDefault READ isTuningDefault)
	Q_PROPERTY(bool TuningDefault READ isTuningDefault)

  public:
	AppSignalState() = default;
	AppSignalState(const AppSignalState&) = default;
	AppSignalState(AppSignalState&&) = default;
	AppSignalState(const Proto::AppSignalState& protoState);
	~AppSignalState() = default;

	AppSignalState(Hash hash, Times times, double value, AppSignalStateFlags flags);

	AppSignalState& operator=(const AppSignalState& state) = default;

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
	[[nodiscard]] bool isOutOfLimits() const; //  isAboveHighLimit() || isBelowLowLimit()
	[[nodiscard]] bool isTuningDefault() const;

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

	[[nodiscard]] bool hasSameValue(const AppSignalState& b) const;

	[[nodiscard]] static QString toString(double value, E::ValueViewType viewType, E::AnalogFormat analogFormat, E::AnalogAppSignalFormat analogAppSignalFormat, int precision);

  public:
	Hash m_hash = {0};
	Times m_time{};
	double m_value{};
	AppSignalStateFlags m_flags{};

	static const quint32 VALID = 1;
	static const quint32 INVALID = 0;
};

Q_DECLARE_METATYPE(AppSignalState)
