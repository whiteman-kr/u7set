#ifndef SIMULATORBASEPAGE_H
#define SIMULATORBASEPAGE_H

#include "SimIdeSimulator.h"

class SimControlPage;


class SimBasePage : public QWidget
{
	Q_OBJECT

public:
	explicit SimBasePage(SimIdeSimulator* simulator, QWidget* parent = nullptr);
	virtual ~SimBasePage();

public:
	static void deleteAllPages();
	static SimControlPage* controlPage(QString lmEquipmnetId, QWidget* parent);

protected:
	QAction* m_closeAction = nullptr;

private:
	static std::list<SimBasePage*> m_pages;

protected:
	SimIdeSimulator* m_simulator = nullptr;
};

#endif // SIMULATORBASEPAGE_H
