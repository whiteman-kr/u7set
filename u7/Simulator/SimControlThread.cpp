#include "SimControlThread.h"

SimControlThread::SimControlThread(SimIdeSimulator* simualtor, QObject* parent) :
	QThread(parent),
	m_simualtor(simualtor)
{
	assert(m_simualtor);
}

void SimControlThread::run()
{

	while (isInterruptionRequested() == false &&
		   m_state != SimControlState::QuitThread)
	{

		usleep(500);
	}

	return;
}

