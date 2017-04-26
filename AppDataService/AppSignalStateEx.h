#pragma once

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignal.h"


struct AppSignalStateEx
{
private:
	AppSignalState m_current;
	AppSignalState m_stored;

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
	AppSignalStateEx();

	void setSignalParams(int index, Signal* signal);
	void setState(Times time, quint32 validity, double value);

	void invalidate() { m_current.m_flags.all = 0; }

	Hash hash() const;

	QString appSignalID() const;

	friend class AppSignalStates;
};


class AppSignalStates
{
private:
	QMutex m_allMutex;

	AppSignalStateEx* m_appSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, const AppSignalStateEx*> m_hash2State;

public:
	~AppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	AppSignalStateEx* operator [] (int index);

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;
	bool getStoredState(Hash hash, AppSignalState& state) const;
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


