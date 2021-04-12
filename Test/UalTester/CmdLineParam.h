#ifndef CMDLINEPARAM_H
#define CMDLINEPARAM_H

#include <QObject>

#include "../../ServiceLib/CommandLineParser.h"
#include "../../OnlineLib/HostAddressPort.h"

#include "TestFile.h"

// ==============================================================================================

class CmdLineParam
{

public:

	explicit CmdLineParam();
	virtual ~CmdLineParam();

protected:

	// main keys
	//
	static const char* const SETTING_CFG_SERVICE_IP1;
	static const char* const SETTING_CFG_SERVICE_IP2;
	static const char* const SETTING_EQUIPMENT_ID;
	static const char* const SETTING_TEST_FILE_NAME;

	// optional keys
	//
	static const char* const SETTING_PACKET_SOURCE_IP;
	static const char* const SETTING_ERROR_IGNORE;
	static const char* const SETTING_TEST_ID;
	static const char* const SETTING_FROM_TEST_ID;
	static const char* const SETTING_TRACE;
	static const char* const SETTING_REPORT_FILENAME;
	static const char* const SETTING_PRESET_LM;
	static const char* const SETTING_OPTION_FILENAME;

	static const char* const REQUIRED_OPTIONS;

private:

	CommandLineParser m_cmdLineParser;

	// main keys
	//
	QString m_cfgServiceIP1;
	QString m_cfgServiceIP2;
	QString m_equipmentID;
	QStringList m_testFileNameList;

	HostAddressPort m_cfgSocketAddress1;
	HostAddressPort m_cfgSocketAddress2;

	// optional keys
	//
	QString m_packetSourceIP;
	QString m_errorIngnoreStr;
	QString m_testID;
	QString m_fromTestID;
	QString m_traceStr;
	QString m_reportFileName;
	QString m_presetLM;
	QString m_optionFileName;

	HostAddressPort m_packetSourceAddress;

	bool m_errorIngnore = true;
	bool m_enableTrace = false;

	bool m_enableContinueTest = true;

	void updateTestFilesParam(QString& cmdLine);

	QString currentTimeStr();

public:

	HostAddressPort cfgSocketAddress1() const { return m_cfgSocketAddress1; }
	HostAddressPort cfgSocketAddress2() const { return m_cfgSocketAddress2; }
	QString equipmentID() const { return m_equipmentID; }
	const QStringList& testFileNameList() const { return m_testFileNameList; }

	HostAddressPort packetSourceAddress() const { return m_packetSourceAddress; }
	bool errorIngnore() const { return m_errorIngnore; }
	QString testID() const { return m_testID; }
	QString fromTestID() const { return m_fromTestID; }
	bool enableTrace() const { return m_enableTrace; }
	QString reportFileName() const { return m_reportFileName; }
	QString presetLM() const { return m_presetLM; }

	void getParams(int& argc, char** argv);
	bool parse();

	bool printToReportFile(const QStringList& msgList);							// print results to report file

	bool enableContinueTest() const { return m_enableContinueTest; }			// for cmd line param -errignore
	void setEnableContinueTest(bool enable) { m_enableContinueTest = enable; }

	int getStartTestIndex(const QVector<TestItem>& testList);					// check cmd line param -from
	bool enableExecuteTest(const QString& testID);								// check cmd line param -test
	bool enableExecuteTestForLM(TestItem test);									// check cmd line param -lm

signals:

public slots:

};

// ==============================================================================================

#endif // CMDLINEPARAM_H
