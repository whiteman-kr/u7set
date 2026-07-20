#pragma once

#include <CommonLib/HostAddressPort.h>
#include "../../UtilsLib/LogFile.h"

#include <TestSuiteLib/MatsTestSuite.h>
#include <TestSuiteLib/TestLog.h>
#include <TestSuiteLib/TestSuiteSettings.h>
#include <TestSuiteLib/TestSuiteConfigController.h>

#include "TO5Settings.h"

class PluginTestLog : public QObject,
					  public TestSuite::ITestLogOutput
{
	Q_OBJECT
public:
	virtual void logItemArrived(const TestSuite::TestLogItem& item) override
	{
		switch (item.type())
		{
		case TestSuite::TestLogItemType::Error:
			{
				emit logItem(item.toText(), item.type());
				m_errorCount++;
				break;
			}
		case TestSuite::TestLogItemType::Warning:
			{
				emit logItem(item.toText(), item.type());
				m_warningCount++;
				break;
			}
		case TestSuite::TestLogItemType::Message:
		case TestSuite::TestLogItemType::Text:
			{
				emit logItem(item.toText(), item.type());
				break;
			}
		}
	}
	std::pair<int,int> getLogIssuesCount() const
	{
		return std::pair<int,int>(m_errorCount, m_warningCount); 
	}

private:
	int m_errorCount = 0;
	int m_warningCount = 0;

signals:
	QString logItem(QString msg, TestSuite::TestLogItemType type);
};

class TO5Runner : public QObject
{
	Q_OBJECT
public:
	TO5Runner(Log::LogFile& logFile);

	bool isRunning() const;

	void setTestSuiteSettings(const PluginTO5Settings& settings);

	bool execute(const QString& testFilter);
	void stop();

	TestSuite::ConfigData configData() const;
	const TestSuite::TestLog& testLog();


	const Tcp::ConnectionState configConnectionState() const;
	const TestSuite::ConfigSettings configuration() const;
	const std::pair<int, int> getLogIssuesCount() const;

private:
	void saveTestLog(QString fileName, TestSuite::TestLog& testLog, Log::LogFile& appLog);

signals:
	void done(int result);
	QString logItem(QString msg, TestSuite::TestLogItemType type);

private:
	//QString m_instanceStrId = "TO5_WS00_TOSUITE";
	//HostAddressPort m_address = HostAddressPort("127.0.0.1", 13312);
	PluginTO5Settings m_settings;

	Log::LogFile& m_logFile;
	PluginTestLog m_testLogOutput;

	SoftwareInfo m_softwareInfo;
	TestSuite::TestSuiteConfigController m_configController;
	TestSuite::MatsTestSuite m_testSuite;
};