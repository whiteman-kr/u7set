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
		quint32 writeFailed: 1;
		quint32 controlIsEnabled: 1;

		quint32 userModified : 1;	// This flag is used by TuningClient's model
	};

	quint32 all = 0;
};

class TuningSignalState
{
	Q_GADGET

	Q_PROPERTY(Hash Hash READ hash)
	Q_PROPERTY(TuningValue Value READ value)
	Q_PROPERTY(TuningValue LowBound READ lowBound)
	Q_PROPERTY(TuningValue HighBound READ highBound)
	Q_PROPERTY(bool Valid READ valid)
	Q_PROPERTY(bool OutOfRange READ outOfRange)

public:
	TuningSignalState() = default;
	TuningSignalState(const ::Network::TuningSignalState& message);

	Q_INVOKABLE Hash hash() const;

	Q_INVOKABLE TuningValue value() const;

	double valueToDouble() const;

	TuningValue newValue() const;
	void setNewValue(const TuningValue& value);

	Q_INVOKABLE TuningValue lowBound() const;
	Q_INVOKABLE TuningValue highBound() const;

	Q_INVOKABLE bool valid() const;
	Q_INVOKABLE bool outOfRange() const;
	Q_INVOKABLE bool writeInProgress() const;
	Q_INVOKABLE bool writeFailed() const;
	Q_INVOKABLE bool controlIsEnabled() const;

	Q_INVOKABLE int writeErrorCode() const;

	bool userModified() const;
	void clearUserModified();

	bool setState(const ::Network::TuningSignalState& message);

	void invalidate();

	bool limitsUnbalance(const AppSignalParam& asp) const;

public:
	Hash m_hash = UNDEFINED_HASH;

	TuningSignalStateFlags m_flags;

	TuningValue m_value;
	TuningValue m_newValue;

	TuningValue m_lowBound;
	TuningValue m_highBound;

	int m_writeErrorCode = 0;
	Hash m_writeClient = 0;
};

Q_DECLARE_METATYPE(TuningSignalState)

