#pragma once

#include "../Hash.h"
#include "../TuningValue.h"
#include "../../Proto/network.pb.h"

class AppSignalParam;

union TuningSignalStateFlags
{
	struct
	{
		quint32	valid : 1;
		quint32	outOfRange : 1;
		quint32	writeInProgress : 1;
		quint32 controlIsEnabled: 1;
		quint32 writingIsEnabled: 1;

	};

	quint32 all = 0;
};

/*! \class TuningSignalState
	\ingroup groupParamsStates
	\brief Describes tuning signal state in the Monitor and TuningClient applications

	TuningSignalState class describes tunable signal state in Monitor and TuningClient applications.
	This state is received from Tuning Service.

	\ref VFrame30::TuningController "TuningController" class accesed by <b>tuning</b> object is used for requesting tunable signal states.

	\warning
	After requesting signal state it is highly recommended to check function return values, because errors can occur. For example,
	connection to Application Data Service can be down, or signal with specified identifier could not exist.

	\n
	\warning It can be used by Monitor only in non-safety projects when Tuning feature is turned on.

	<b>Example:</b>

	\code
	// Request signal state by identifier "#APPSIGNALID"
	//
	var state = tuning.signalState("#APPSIGNALID");

	if (state == undefined)
	{
		// No state was received for this signal, print an error message
		//
		view.errorMessageBox("Signal does not exist!");
		return;
	}

	// Check signal validity
	//
	if (state.Valid == false)
	{
		view.errorMessageBox("Signal is not valid!");
		return;
	}

	//Increase signal value to 10
	//
	var newValue = state.Value;

	newValue =+ 10;

	// Write new value to logic module
	//
	if (tuning.writeValue("#APPSIGNALID", newValue) == false)
	{
		view.errorMessageBox("Value set error!");
		return;
	}
	\endcode
*/
class TuningSignalState
{
	Q_GADGET

	/// \brief Contains a 64-bit hash of a signal
	Q_PROPERTY(Hash Hash READ hash)

	/*! \brief Contains current signal value

		Contains current signal value. For discrete signals <b>"False"</b> is equal to <b>0</b>, <b>"True"</b> is equal to <b>1</b>.

		\warning Be careful when comparing values. Remember that <b>double</b> can't be compared directly,
		because doubles and floats cannot express every numerical value. They are using approximations to represent the value.
		It is recommended to make comparsions as follows, especially analog values:

		\code
		var a = state.Value;
		var b = 1.5;

		var threshold = 0.0000001;

		if (Math.abs(a - b) <= threshold)
		{
		...
		}
		\endcode
	*/
	Q_PROPERTY(QVariant Value READ toVariant)

	/// \brief Contains low bound of signal value
	Q_PROPERTY(QVariant LowBound READ lowBoundToVariant)

	/// \brief Contains high bound of signal value.
	Q_PROPERTY(QVariant HighBound READ highBoundToVariant)

	/// \brief Contains signal validity flag.
	Q_PROPERTY(bool Valid READ valid)

	/// \brief This flag is set to true when signal value is out of range
	Q_PROPERTY(bool OutOfRange READ outOfRange)

	/// \brief This flag is set to true when a new value is being written to a logic module
	Q_PROPERTY(bool WriteInProgress READ writeInProgress)

	/// \brief This flag is set to true when logic module control is enabled
	Q_PROPERTY(bool ControlIsEnabled READ controlIsEnabled)

	/// \brief This flag is set to true when signal has access for tuning.\n
	Q_PROPERTY(bool WritingIsEnabled READ writingIsEnabled)

public:
	TuningSignalState() = default;
	TuningSignalState(const ::Network::TuningSignalState& message);

	Hash hash() const;

	TuningValue value() const;

	QVariant toVariant() const;
	double toDouble() const;

	TuningValue lowBound() const;
	QVariant lowBoundToVariant() const;

	TuningValue highBound() const;
	QVariant highBoundToVariant() const;

	bool valid() const;
	bool outOfRange() const;
	bool writeInProgress() const;
	bool controlIsEnabled() const;
	bool writingIsEnabled() const;

	int writeErrorCode() const;
	Hash writeClient() const;

	QDateTime successfulReadTime() const;
	QDateTime writeRequestTime() const;
	QDateTime successfulWriteTime() const;
	QDateTime unsuccessfulWriteTime() const;

	bool setState(const ::Network::TuningSignalState& message);

	void invalidate();

	bool limitsUnbalance(const AppSignalParam& asp) const;

public:
	Hash m_hash = UNDEFINED_HASH;

	TuningSignalStateFlags m_flags;

	TuningValue m_value;

	TuningValue m_lowBound;
	TuningValue m_highBound;

	int m_writeErrorCode = 0;
	Hash m_writeClient = 0;

	qint64 m_successfulReadTime = 0;
	qint64 m_writeRequestTime = 0;
	qint64 m_successfulWriteTime = 0;
	qint64 m_unsuccessfulWriteTime = 0;
};

Q_DECLARE_METATYPE(TuningSignalState)

