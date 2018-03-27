#ifndef SIMCONTROLTHREAD_H
#define SIMCONTROLTHREAD_H

#include <QThread>

class SimIdeSimulator;

enum class SimControlState
{
	Nop,
	RunNCycles,
	Stop,
	Pause,
	QuitThread
};

class SimControlThread : public QThread
{
	Q_OBJECT

public:
	explicit SimControlThread(SimIdeSimulator* simualtor, QObject* parent = nullptr);

	юююю увцуажцука рцукааджожлдк

protected:
	void run() override;

private:
	SimIdeSimulator* m_simualtor = nullptr;

	SimControlState m_state = SimControlState::Nop;
};

#endif // SIMCONTROLTHREAD_H
