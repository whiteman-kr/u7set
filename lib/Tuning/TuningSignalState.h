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
	Q_GADGET

public:
	// State methods

	float value() const;

	float readLowLimit() const;
	float readHighLimit() const;

	bool underflow() const;
	bool overflow() const;

	bool valid() const;
	bool writing() const;

	float editValue() const;

	// Functions used by model

	void onEditValue(float value);

	bool needRedraw();

	bool userModified() const;
	void clearUserModified();

	void copy(const TuningSignalState& source);

	// Functions used by signal manager

	void onReceiveValue(float readLowLimit, float readHighLimit, bool valid, float value, bool &writingFailed);

	void onSendValue(float value);

	void invalidate();

public:

	TuningSignalStateFlags m_flags;

	float m_value = 0;
	float m_editValue = 0;

	float m_readLowLimit = 0;
	float m_readHighLimit = 0;

	int m_writingCounter = 0;
};


#endif // TUNINGSIGNAL_H
