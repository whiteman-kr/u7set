#ifndef TESTFILE_H
#define TESTFILE_H

#include <QObject>
#include <QFile>

#include "SignalBase.h"

class TestFile;

// ==============================================================================================

const char* const TestFileCmd[] =
{
				"",
				"test",
				"endtest",
				"schema",
				"compatible",
				"const",
				"var",
				"set",
				"check",
				"delay",
				"runsource",
				"stopsource",
				"exitps",
};

const int		TF_CMD_COUNT		= sizeof(TestFileCmd)/sizeof(TestFileCmd[0]);

const int		TF_CMD_UNKNOWN		= -1,
				TF_CMD_EMPTY		= 0,
				TF_CMD_TEST			= 1,
				TF_CMD_ENDTEST		= 2,
				TF_CMD_SCHEMA		= 3,
				TF_CMD_COMPATIBLE	= 4,
				TF_CMD_CONST		= 5,
				TF_CMD_VAR			= 6,
				TF_CMD_SET			= 7,
				TF_CMD_CHECK		= 8,
				TF_CMD_DELAY		= 9,
				TF_CMD_RUN_SOURCE	= 10,
				TF_CMD_STOP_SOURCE	= 11,
				TF_CMD_EXIT_PS		= 12;

// ==============================================================================================

enum class TestCmdParamType
{
	Undefined,
	Discrete,
	SignedInt32,
	SignedInt64,
	Float,
	Double,
	String
};

// ==============================================================================================

const int		TF_UNDEFINED_FLAG = -1;

// ==============================================================================================

#define			TF_SPACE			QString("    ")

// ==============================================================================================

class TestCmdParam
{

public:

	TestCmdParam() { clear(); }
	TestCmdParam(const TestCmdParam& from) { *this = from; }
	virtual ~TestCmdParam() {}

private:

	QString m_name;
	int m_flag = TF_UNDEFINED_FLAG;
	TestCmdParamType m_type = TestCmdParamType::Undefined;
	QVariant m_value;

public:

	bool isEmtpy();
	void clear();

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	int flag() const { return m_flag; }
	bool getFlag(QString& signalID);
	bool isFlag() { return m_flag != TF_UNDEFINED_FLAG; }

	TestCmdParamType type() const { return m_type; }
	void setType(TestCmdParamType type) { m_type = type; }

	QVariant value() const { return m_value; }
	QString valueStr(bool addParamName, int precise);
	void setValue(const QVariant& value) { m_value = value; }

	bool compare(const AppSignalState& state);

	TestCmdParam& operator=(const TestCmdParam& from);
};

// ==============================================================================================

class TestCmd
{

public:

	TestCmd();
	TestCmd(TestFile* pTestFile, SignalBase* pSignalBase);
	virtual ~TestCmd();

private:

	static const char* const PARAM_TEST_ID;
	static const char* const PARAM_TEST_DESCRIPTION;
	static const char* const PARAM_SCHEMA_ID;
	static const char* const PARAM_COMPATIBLE;
	static const char* const PARAM_SOURCE_ID;

	TestFile* m_pTestFile = nullptr;
	SignalBase* m_pSignalBase = nullptr;

	int m_lineIndex;
	bool m_foundEndOfTest;

	int m_type = TF_CMD_UNKNOWN;
	QVector<TestCmdParam> m_paramList;

	QString m_line;
	QString m_comment;
	QStringList m_errorList;

public:

	bool isEmpty() { return m_line.isEmpty() == true; }

	int lineIndex() { return m_lineIndex; }
	bool foundEndOfTest() { return m_foundEndOfTest; }

	int type() const { return m_type; }
	int getCmdType(const QString& line);

	QString comment() const { return m_comment; }
	void setComment(const QString& comment) { m_comment = comment; }

	bool parse(const QString& line);
	bool parseCmdTest();
	bool parseCmdEndtest();
	bool parseCmdSchema();
	bool parseCmdCompatible();
	bool parseCmdConst();
	bool parseCmdVar();
	bool parseCmdSet();
	bool parseCmdCheck();
	bool parseCmdDelay();
	bool parseCmdRunSource();
	bool parseCmdStopSource();
	bool parseCmdExitPS();

	bool isUniqueTestID(const QString& testID);
	bool isUniqueConstOrVarName(const QString& name, const QVector<TestCmdParam>& paramList);

	TestCmdParam paramFromConstOrVar(const QString& name, const QString& value, const Signal& signal);
	TestCmdParam paramFromSignal(const QString& name, const QString& value, const Signal& signal);

	const QVector<TestCmdParam>& paramList() const { return m_paramList; }
	const QStringList& errorList() const { return m_errorList; }
};

// ==============================================================================================

class TestItem
{

public:

	TestItem();
	TestItem(const TestItem& from);
	virtual		~TestItem();

private:

	mutable QMutex m_mutex;

	QString m_testID;
	QString m_name;
	QStringList m_compatibleList;
	int m_errorCount = 0;

	QVector<TestCmd> m_commandList;
	QStringList m_resultList;

public:


	QString testID() const { return m_testID; }
	void setTestID(const QString& testID) { m_testID = testID; }

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	QStringList& compatibleList() { return m_compatibleList; }

	int errorCount() const { return m_errorCount; }
	void incErrorCount() { m_errorCount ++; }

	int cmdCount();
	TestCmd cmd(int index);
	void appendCmd(const TestCmd& cmd);

	QStringList& resultList() { return m_resultList; }
	void appendResult(const QString& str, bool printDebugMsg);

	TestItem& operator=(const TestItem& from);
};

// ==============================================================================================

class TestFile : public QObject
{
	Q_OBJECT

public:

	explicit TestFile(QObject *parent = nullptr);
	virtual ~TestFile();

private:

	QString m_fileName;
	QFile m_file;

	SignalBase* m_pSignalBase = nullptr;

	QVector<TestItem> m_testList;			// all tests of file
	QVector<TestCmd> m_commandList;			// all commands of file
	QStringList m_errorList;				// errors of parsing

	void printErrorlist();

public:

	//
	//
	QString fileName() const { return m_fileName; }
	void setFileName(const QString& fileName) { m_fileName = fileName; }

	SignalBase* signalBase() const { return m_pSignalBase; }
	void setSignalBase(SignalBase* pSignalBase) { m_pSignalBase = pSignalBase; }

	const QVector<TestItem> testList() const { return m_testList; }
	const QVector<TestCmd> commandList() const  { return m_commandList; }
	const QStringList& errorList() const { return m_errorList; }

	//
	//
	bool parse(const QString& fileName, SignalBase* pSignalBase);
	void close();

	//
	//
	void createTestList();

signals:

public slots:

};

// ==============================================================================================

#endif // TESTFILE_H
