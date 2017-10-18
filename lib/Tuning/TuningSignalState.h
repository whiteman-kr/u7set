#ifndef TUNINGSIGNAL_H
#define TUNINGSIGNAL_H

#include "../Hash.h"
#include "../../Proto/network.pb.h"

struct TuningValue;
extern bool operator < (const TuningValue& l, const TuningValue& r);
extern bool operator > (const TuningValue& l, const TuningValue& r);
extern bool operator == (const TuningValue& l, const TuningValue& r);
extern bool operator != (const TuningValue& l, const TuningValue& r);

enum class TuningValueType
{
	Discrete,
	SignedInteger,
	Float,
	Double
};

struct TuningValue
{
	Q_GADGET
public:
	TuningValueType type = TuningValueType::Discrete;		// If type is Discrete or SignedInteger, then value is kept in intValue
	qint32 intValue = 0;
	float floatValue = 0.0;
	double doubleValue = 0.0;

	// Methods
	//
	TuningValue() = default;
	TuningValue(const Network::TuningValue& message);

	bool save(Network::TuningValue* message) const;
	bool load(const Network::TuningValue& message);

	friend bool operator < (const TuningValue& l, const TuningValue& r);
	friend bool operator > (const TuningValue& l, const TuningValue& r);
	friend bool operator == (const TuningValue& l, const TuningValue& r);
	friend bool operator != (const TuningValue& l, const TuningValue& r);
};

union TuningSignalStateFlags
{
	struct
	{
		quint32	valid : 1;
		quint32	outOfRange : 1;
		quint32	writeInProgress : 1;

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

	TuningValue newValue() const;
	void setNewValue(const TuningValue& value);

	Q_INVOKABLE TuningValue lowBound() const;
	Q_INVOKABLE TuningValue highBound() const;

	Q_INVOKABLE bool valid() const;
	Q_INVOKABLE bool outOfRange() const;
	Q_INVOKABLE bool writeInProgress() const;

	Q_INVOKABLE int writeErrorCode() const;

	bool userModified() const;
	void clearUserModified();

	bool setState(const ::Network::TuningSignalState& message);

	void invalidate();

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

Q_DECLARE_METATYPE(TuningSignalState);

#endif // TUNINGSIGNAL_H
