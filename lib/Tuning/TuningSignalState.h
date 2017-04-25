#ifndef TUNINGSIGNAL_H
#define TUNINGSIGNAL_H

#include "Stable.h"
#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/Tuning./TuningSignalStorage.h"

union TuningSignalStateFlags
{
	struct
	{
		quint32	m_valid : 1;
		quint32	m_overflow : 1;
		quint32	m_underflow : 1;
		quint32	m_writing : 1;

		quint32	m_needRedraw : 1;	// This flag is used by TuningClient's model
		quint32 m_userModified : 1;	// This flag is used by TuningClient's model
	};

	quint32 all = 0;

};

class TuningSignalState
{

public:
	// State methods

	float value() const;
	void setValue(float value);

	float editValue() const;
	void onEditValue(float value);

	bool valid() const;
	void setValid(bool value);

	float readLowLimit() const;
	void setReadLowLimit(float value);

	float readHighLimit() const;
	void setReadHighLimit(float value);

	bool underflow() const;
	bool overflow() const;

	bool needRedraw();

	bool userModified() const;
	void clearUserModified();

	bool writing() const;
	void setWriting(bool value);

	void onReceiveValue(float readLowLimit, float readHighLimit, bool valid, float value, bool &writingFailed);
	void onSendValue(float value);

	TuningSignalState& operator = (const TuningSignalState& That)
	{
		setReadLowLimit(That.readLowLimit());
		setReadHighLimit(That.readHighLimit());
		setValue(That.value());

		setValid(That.valid());
		setWriting(That.writing());

		return *this;
	}

public:

	TuningSignalStateFlags m_flags;

	float m_value = 0;
	float m_editValue = 0;

	float m_readLowLimit = 0;
	float m_readHighLimit = 0;

	int m_writingCounter = 0;
};


#endif // TUNINGSIGNAL_H
