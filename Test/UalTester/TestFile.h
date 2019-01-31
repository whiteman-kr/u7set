#ifndef TESTFILE_H
#define TESTFILE_H

#include <QObject>
#include <QFile>

// ==============================================================================================

const char* const TF_Cmd[] =
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

const int		TF_CMD_COUNT = sizeof(TF_Cmd)/sizeof(TF_Cmd[0]);

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

class TestFileLine
{

public:

	explicit TestFileLine(int number, const QString& line);
	virtual ~TestFileLine();

private:

	int m_number = -1;
	QString m_line;

	int m_cmdType = TF_CMD_UNKNOWN;

	QString m_testID;
	QString m_testDescription;
	QString m_schemaID;
	QStringList m_compatibleList;
	int m_delay_ms;

	QString m_errorStr;

public:

	//
	//
	int cmdType() const { return m_cmdType; }

	QString testID() const { return m_testID; }
	QString testDescription() const { return m_testDescription; }
	QString schemaID() const { return m_schemaID; }
	const QStringList& compatibleList() const { return m_compatibleList; }
	int delay_ms() const { return m_delay_ms; }

	//
	//
	int getCmdType(const QString& line);

	QString parse();
	QString parseCmdTest();
	QString parseCmdSchema();
	QString parseCmdCompatible();
	//QString parseCmdConst();
	//QString parseCmdVar();
	QString parseCmdSet();
	//QString parseCmdCheck();
	QString parseCmdDelay();
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

	int m_lineNumber = 0;

	bool m_foundEndOfTest = true;
	int m_foundEndOfComment = -1;


	QStringList m_commandList;
	QStringList m_errorList;
	QStringList m_testSignalList;

public:

	QString fileName() const { return m_fileName; }
	void setFileName(const QString& fileName) { m_fileName = fileName; }

	const QStringList& testSignalList() const { return m_testSignalList; }


	bool open();
	bool parse();
	void close();

signals:

public slots:

};

// ==============================================================================================

#endif // TESTFILE_H
