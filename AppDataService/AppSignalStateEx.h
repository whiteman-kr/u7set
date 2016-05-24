#pragma once

#include "../include/Hash.h"
#include "../include/Signal.h"
#include "../include/AppSignalState.h"


struct AppSignalStateEx
{
private:
	AppSignalState m_state;

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
	void setState(Times time, AppSignalStateFlags flags, double value);

	void invalidate() { m_state.flags.all = 0; }

	QString appSignalID();

	friend class AppSignalStates;
};


class AppSignalStates
{
private:
	QMutex m_allMutex;

	AppSignalStateEx* m_appSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, const AppSignalState*> m_hash2State;

public:
	~AppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	AppSignalStateEx* operator [] (int index);

	void buidlHash2State();

	bool getState(Hash hash, AppSignalState& state) const;
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


