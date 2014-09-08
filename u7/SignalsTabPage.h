#pragma once

#include "MainTabPage.h"
#include "../include/DeviceObject.h"

class DbController;


class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SignalsTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~SignalsTabPage();

protected:
	void CreateActions();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
private:
	//QAction* m_addSystemAction = nullptr;
	//QAction* m_addCaseAction = nullptr;

	//QTextEdit* m_propertyView = nullptr;
	//QSplitter* m_splitter = nullptr;
};


