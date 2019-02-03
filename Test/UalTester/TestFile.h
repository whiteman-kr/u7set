#ifndef TESTFILE_H
#define TESTFILE_H

#include <QObject>
#include <QFile>

#include "SignalBase.h"

// ==============================================================================================

const char* const TestFileCmd[] =
{
				QT_TRANSLATE_NOOP("TestFile.h", ""),
				QT_TRANSLATE_NOOP("TestFile.h", "test"),
				QT_TRANSLATE_NOOP("TestFile.h", "endtest"),
				QT_TRANSLATE_NOOP("TestFile.h", "schema"),
				QT_TRANSLATE_NOOP("TestFile.h", "compatible"),
				QT_TRANSLATE_NOOP("TestFile.h", "const"),
				QT_TRANSLATE_NOOP("TestFile.h", "var"),
				QT_TRANSLATE_NOOP("TestFile.h", "set"),
				QT_TRANSLATE_NOOP("TestFile.h", "check"),
				QT_TRANSLATE_NOOP("TestFile.h", "apply"),
				QT_TRANSLATE_NOOP("TestFile.h", "delay"),
};

const int		TF_CMD_COUNT = sizeof(TestFileCmd)/sizeof(TestFileCmd[0]);

const int		TF_CMD_UNKNOWN = -1,
				TF_CMD_EMPTY = 0,
				TF_CMD_TEST = 1,
				TF_CMD_ENDTEST = 2,
				TF_CMD_SCHEMA = 3,
				TF_CMD_COMPATIBLE = 4,
				TF_CMD_CONST = 5,
				TF_CMD_VAR = 6,
				TF_CMD_SET = 7,
				TF_CMD_CHECK = 8,
				TF_CMD_APPLY = 9,
				TF_CMD_DELAY = 10;

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

class TestCmdParam
{

public:

	TestCmdParam() { clear(); }
	TestCmdParam(const TestCmdParam& from) { *this = from; }
	virtual ~TestCmdParam() {}

private:

	QString m_name;
	TestCmdParamType m_type = TestCmdParamType::Undefined;
	QVariant m_value;

public:

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	TestCmdParamType type() const { return m_type; }
	void setType(TestCmdParamType type) { m_type = type; }

	QVariant value() const { return m_value; }
	void setValue(const QVariant& value) { m_value = value; }

	void clear()
	{
		m_name.clear();
		m_type = TestCmdParamType::Undefined;
		m_value.clear();
	}

	TestCmdParam& operator=(const TestCmdParam& from)
	{
		m_name = from.m_name;
		m_type = from.m_type;
		m_value = from.m_value;

		return *this;
	}
};

// ==============================================================================================

class TestCommand
{

public:

	TestCommand();
	explicit TestCommand(SignalBase* pSignalBase);
	virtual ~TestCommand();

private:

	SignalBase* m_pSignalBase = nullptr;

	int m_lineIndex;
	bool m_foundEndOfTest;

	int m_cmdType = TF_CMD_UNKNOWN;
	QVector<TestCmdParam> m_paramList;

	QString m_line;
	QStringList m_errorList;

public:

	int lineIndex() { return m_lineIndex; }
	bool foundEndOfTest() { return m_foundEndOfTest; }

	int cmdType() const { return m_cmdType; }
	int getCmdType(const QString& line);

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

	const QVector<TestCmdParam>& paramList() const { return m_paramList; }
	const QStringList& errorList() const { return m_errorList; }
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

	int m_lineIndex = 0;

	QVector<TestCommand> m_commandList;
	QStringList m_errorList;

	void printErrorlist();

public:

	//
	//
	QString fileName() const { return m_fileName; }
	void setFileName(const QString& fileName) { m_fileName = fileName; }

	SignalBase* signalBase() const { return m_pSignalBase; }
	void setSignalBase(SignalBase* pSignalBase) { m_pSignalBase = pSignalBase; }

	//
	//
	bool open();
	bool parse();
	void close();

	void run();

signals:

public slots:

};

// ==============================================================================================

#endif // TESTFILE_H
