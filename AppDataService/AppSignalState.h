#pragma once

#include "../include/Signal.h"

struct AppSignalStateFlags
{
	quint32	valid : 1;
};


struct AppSignalState
{
public:
	qint64	serverTime = 0;
	qint64	pantTime = 0;

	AppSignalStateFlags flags;
	double value = 0;

	Signal* signal = nullptr;
	int index = 0;
};


class AppSignalStates
{
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

