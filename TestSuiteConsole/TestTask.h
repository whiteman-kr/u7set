#pragma once

#include <QObject>
#include <QDebug>
#include "../UtilsLib/LogFile.h"
#include "../TestSuiteLib/TestEngine.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"

class TestTask : public QObject
{
	Q_OBJECT

public:
	TestTask(const SoftwareInfo &softwareInfo, HostAddressPort address1, HostAddressPort address2, QObject* parent = nullptr);

public slots:
	void start();
	void stop();

public:
	bool isRunning() const;

	TestSuiteConfigController& configController();
	const TestSuiteConfigController& configController() const;

private slots:
	void slot_configurationArrived(ConfigSettings configuration);
	void slot_unknownClient(QString errMsg);
	void slot_wrongClientHostname(QString errMsg);

	void newLogItem(const TestLogItem& logItem);

signals:
	void finished(int result);

	// Properties
	//
public:
/*	void setDatabaseAddress(QString value);
	void setDatabasePort(int value);
	void setDatabaseUserName(QString value);
	void setDatabasePassword(QString value);
	void setProjectName(QString value);
	void setProjectUserName(QString value);
	void setProjectUserPassword(QString value);
	void setBuildOutputPath(QString value);*/

	// Data
	//
private:

	TestEngine m_testEngine;
	Log::LogFile m_LogFile;						// Must be initialized first

	TestSuiteConfigController m_configController;

	/*Builder::Builder m_builder{nullptr, this};

	QString m_databaseAddress{"127.0.0.1"};
	int m_databasePort{5432};
	QString m_databaseUserName{"u7"};
	QString m_databasePassword{"Password"};
	QString m_projectName;
	QString m_projectUserName{"Administrator"};
	QString m_projectUserPassword{"Password"};
	QString m_buildOutputPath{"."};*/
};


