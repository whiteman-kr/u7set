#ifndef SIMULATORBASEPAGE_H
#define SIMULATORBASEPAGE_H

#include <list>
#include <utility>
#include <QWidget>
#include <QAction>
#include "SimIdeSimulator.h"

class SimulatorControlPage;


class SimulatorBasePage : public QWidget
{
	Q_OBJECT

public:
	explicit SimulatorBasePage(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent = nullptr);
	virtual ~SimulatorBasePage();

public:
	static void deleteAllPages();
	static SimulatorControlPage* controlPage(QString lmEquipmnetId, QWidget* parent);

protected:
	QAction* m_closeAction = nullptr;

private:
	static std::list<SimulatorBasePage*> m_pages;

protected:
	std::shared_ptr<SimIdeSimulator> m_simulator;
};

#endif // SIMULATORBASEPAGE_H
