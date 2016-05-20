#pragma once

#include "../include/Hash.h"
#include "../include/Signal.h"

#pragma pack(push, 1)

union AppSignalStateFlags
{
	struct
	{
		quint32	valid : 1;
		quint32	overflow : 1;
		quint32	underflow : 1;
	};

	quint32 allFlags;

	inline void reset() { allFlags = 0; }
};

#pragma pack(pop)


struct AppSignalState
{
private:
	// signal state
	//
	Times m_time;

	AppSignalStateFlags m_flags;
	double m_value = 0;

	// paramters needed to update state
	//
	bool m_initialized = false;
	bool m_isDiscreteSignal = false;
	double m_aperture = 0;
	double m_lowLimit = 0;
	double m_highLimit = 0;
	double m_absAperture = 0;

	Signal* m_signal = nullptr;
	int m_index = 0;

public:
	AppSignalState();

	void setSignalParams(int index, Signal* signal);
	void setState(Times time, AppSignalStateFlags flags, double value);
};


class AppSignalStates
{
private:
	QMutex m_allMutex;

	AppSignalState* m_appSignalState = nullptr;
	int m_size = 0;

public:
	~AppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	AppSignalState* operator [] (int index);
};


class AppSignals : public HashedVector<QString, Signal*>
{
private:
	QHash<Hash, Signal*> m_hash2Signal;

public:
	~AppSignals();

	void clear();
	void buildHash2Signal();

	const Signal* getSignal(Hash hash) const;
};


