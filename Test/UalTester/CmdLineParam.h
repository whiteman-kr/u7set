#ifndef CMDLINEPARAM_H
#define CMDLINEPARAM_H

#include <QObject>

#include "../../lib/HostAddressPort.h"

#include "TestFile.h"

// ==============================================================================================

class CmdLineParam
{

public:

	explicit CmdLineParam();
	virtual ~CmdLineParam();

protected:

	static const char* const SETTING_CFG_SERVICE_IP1;
	static const char* const SETTING_CFG_SERVICE_IP2;
	static const char* const SETTING_EQUIPMENT_ID;
	static const char* const SETTING_TEST_FILE_NAME;

	static const char* const SETTING_ERROR_IGNORE;
	static const char* const SETTING_TEST_ID;
	static const char* const SETTING_FROM_TEST_ID;
	static const char* const SETTING_TRACE;
	static const char* const SETTING_REPORT;
	static const char* const SETTING_PRESET_LM;

private:

	HostAddressPort m_cfgSocketAddress1;
	HostAddressPort m_cfgSocketAddress2;

	QString m_cfgServiceIP1;
	QString m_cfgServiceIP2;
	QString m_equipmentID;
	QString m_testFileName;

	QString m_errorIngnoreStr;
	QString m_testID;
	QString m_fromTestID;
	QString m_traceStr;
	QString m_reportFileName;
	QString m_presetLM;

	bool m_errorIngnore = true;
	bool m_enableTrace = false;

	bool m_enableContinueTest = true;

	QString currentTimeStr();

public:

	HostAddressPort cfgSocketAddress1() const { return m_cfgSocketAddress1; }
	HostAddressPort cfgSocketAddress2() const { return m_cfgSocketAddress2; }
	QString equipmentID() const { return m_equipmentID; }
	QString testFileName() const { return m_testFileName; }

	bool errorIngnore() const { return m_errorIngnore; }
	QString testID() const { return m_testID; }
	QString fromTestID() const { return m_fromTestID; }
	bool enableTrace() const { return m_enableTrace; }
	QString reportFileName() const { return m_reportFileName; }
	QString presetLM() const { return m_presetLM; }

	void getParams(int& argc, char** argv);
	bool paramIsValid();

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
