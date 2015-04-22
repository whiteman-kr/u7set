#pragma once
#include <QObject>

enum class OutputMessageLevel
{
	Message,
	Success,
	Warning,
	Error
};

class OutputLogItem
{
public:
	OutputLogItem();
	OutputLogItem(const QString& m, OutputMessageLevel l, bool b, const QDateTime& t);

public:
	QString message;
	OutputMessageLevel level;
	bool bold;
	QDateTime time;
};

class OutputLog : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int ErrorCount READ errorCount WRITE setErrorCount RESET resetErrorCount NOTIFY errorCountChanged)
	Q_PROPERTY(int WarningCount READ warningCount WRITE setWarningCount RESET resetWarningCount NOTIFY warningCountChanged)

public:
	OutputLog(void);
	virtual ~OutputLog(void);

public:
	Q_INVOKABLE void clear();

	Q_INVOKABLE void write(const QString& str, OutputMessageLevel level, bool bold);
	Q_INVOKABLE void writeMessage(const QString& str, bool bold);
	Q_INVOKABLE void writeEmptyLine();
	Q_INVOKABLE void writeSuccess(const QString& str, bool bold);
	Q_INVOKABLE void writeWarning(const QString& str, bool bold, bool incWrnCounter);
	Q_INVOKABLE void writeError(const QString& str, bool bold, bool incErrCounter);

    Q_INVOKABLE void writeDump(const std::vector<quint8> &data);

	bool windowMessageListEmpty() const;
	OutputLogItem popWindowMessages();

	void startStrLogging();
	QString finishStrLogging();

	// Signals
	//
signals:
	void errorCountChanged(int oldValue, int newValue);
	void warningCountChanged(int oldValue, int newValue);

	// Properties
	//
public:

	int errorCount() const;
	void setErrorCount(int value);
	void resetErrorCount();

	int warningCount() const;
	void setWarningCount(int value);
	void resetWarningCount();

private:
	std::list<OutputLogItem> windowMessageList;		// List buffer for writing messages to main window Log Widget
	std::list<OutputLogItem> fileMessageList;		// List buffer for writing messages to log file

	bool m_strLogging = false;
	QString m_strFullLog;

	int m_errorCount = 0;
	int m_warningCount = 0;

	mutable QMutex mutex;							// access to windowMessageList, fileMessageList
};
