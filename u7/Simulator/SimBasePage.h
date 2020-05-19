#pragma once

#include "SimIdeSimulator.h"

class SimLogicModulePage;
class SimConnectionPage;
class SimAppLogicSchemasPage;

class SimBasePage : public QWidget
{
	Q_OBJECT

public:
	explicit SimBasePage(SimIdeSimulator* simulator, QWidget* parent = nullptr);
	virtual ~SimBasePage();

public:
	static void deleteAllPages();

	static SimLogicModulePage* logicModulePage(QString lmEquipmnetId, QWidget* parent);
	static SimConnectionPage* connectionPage(QString connectionId, QWidget* parent);
	static SimAppLogicSchemasPage* appLogicSchemasPage(QWidget* parent);

protected:
	QAction* m_closeAction = nullptr;

private:
	static std::list<SimBasePage*> m_pages;

protected:
	SimIdeSimulator* m_simulator = nullptr;
};

