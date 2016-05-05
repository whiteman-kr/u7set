#include "AppSignalState.h"


AppSignalStates::~AppSignalStates()
{
	clear();
}


void AppSignalStates::clear()
{
	if (m_appSignalState != nullptr)
	{
		delete [] m_appSignalState;
	}

	m_size = 0;
}


void AppSignalStates::setSize(int size)
{
	clear();

	if (size > 1000000)		// limit to 1 million of signals
	{
		assert(false);
		return;
	}

	m_appSignalState = new AppSignalState[size];
	m_size = size;
}


AppSignalState* AppSignalStates::operator [] (int index)
{
#ifdef QT_DEBUG

	if (m_appSignalState == nullptr ||
		index < 0  || index >= m_size)
	{
		assert(false);
		return nullptr;
	}

#endif

	return m_appSignalState + index;
}
