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
	Q_PROPERTY(QVariant Value READ toVariant)
	Q_PROPERTY(QVariant LowBound READ lowBoundToVariant)
	Q_PROPERTY(QVariant HighBound READ highBoundToVariant)

	Q_PROPERTY(bool Valid READ valid)
	Q_PROPERTY(bool OutOfRange READ outOfRange)
	Q_PROPERTY(bool WriteInProgress READ writeInProgress)
	Q_PROPERTY(bool WriteFailed READ writeFailed)
	Q_PROPERTY(bool ControlIsEnabled READ controlIsEnabled)

public:
	TuningSignalState() = default;
	TuningSignalState(const ::Network::TuningSignalState& message);

	Hash hash() const;

	TuningValue value() const;

	QVariant toVariant() const;
	double toDouble() const;

	TuningValue modifiedValue() const;
	void setModifiedValue(const TuningValue& value);

	TuningValue lowBound() const;
	QVariant lowBoundToVariant() const;

	TuningValue highBound() const;
	QVariant highBoundToVariant() const;

	bool valid() const;
	bool outOfRange() const;
	bool writeInProgress() const;
	bool writeFailed() const;
	bool controlIsEnabled() const;

	int writeErrorCode() const;
	Hash writeClient() const;

	QDateTime successfulReadTime() const;
	QDateTime writeRequestTime() const;
	QDateTime successfulWriteTime() const;
	QDateTime unsuccessfulWriteTime() const;

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

	qint64 m_successfulReadTime = 0;
	qint64 m_writeRequestTime = 0;
	qint64 m_successfulWriteTime = 0;
	qint64 m_unsuccessfulWriteTime = 0;
};

Q_DECLARE_METATYPE(TuningSignalState)

