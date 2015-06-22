#pragma once
#include <QObject>

#define LOG_STRING(PARAM) #PARAM

#define LOG_ERROR(logObject, message)	logObject->writeError(message, __FILE__, __LINE__, Q_FUNC_INFO);
#define LOG_WARNING(logObject, message)	logObject->writeWarning(message, __FILE__, __LINE__, Q_FUNC_INFO);
#define LOG_MESSAGE(logObject, message)	logObject->writeMessage(message, __FILE__, __LINE__, Q_FUNC_INFO);
#define LOG_SUCCESS(logObject, message)	logObject->writeSuccess(message, __FILE__, __LINE__, Q_FUNC_INFO);

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
	OutputLogItem(int messageNo);
	OutputLogItem(int messageNo,
				  QString message,
				  OutputMessageLevel level,
				  QDateTime time,
				  QString file,
				  int fileLine,
				  QString func);

	QString toText() const;
	QString toHtml() const;
	QString toCsv() const;

public:
	int m_no = 0;
	QString m_message;
	OutputMessageLevel m_level;
	QDateTime m_time;

	QString m_file;
	int m_fileLine = 0;
	QString m_func;
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

	Q_INVOKABLE void write(const QString& str, OutputMessageLevel level, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeMessage(const QString& str);
	Q_INVOKABLE void writeSuccess(const QString& str);
	Q_INVOKABLE void writeWarning(const QString& str);
	Q_INVOKABLE void writeError(const QString& str);

	Q_INVOKABLE void writeMessage(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeSuccess(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeWarning(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeError(const QString& str, QString file, int fileLine, QString func);

    Q_INVOKABLE void writeDump(const std::vector<quint8> &data);
	Q_INVOKABLE void writeEmptyLine();

	bool isEmpty() const;
	OutputLogItem popMessages();

	void startStrLogging();
	QString finishStrLogging();

	// Signals
	//
signals:
	void errorCountChanged(int oldValue, int newValue);
	void warningCountChanged(int oldValue, int newValue);

	void messageLogCleared();

	// Properties
	//
public:
	int errorCount() const;
	void setErrorCount(int value);
	void incErrorCount();
	void resetErrorCount();

	int warningCount() const;
	void setWarningCount(int value);
	void incWarningCount();
	void resetWarningCount();

private:
	std::list<OutputLogItem> m_messages;			// List buffer for writing messages to main window Log Widget

	bool m_strLogging = false;
	QString m_strFullLog;

	int m_errorCount = 0;
	int m_warningCount = 0;
	int m_messagesNo = 0;
	mutable QMutex m_mutex;							// access to windowMessageList, fileMessageList
};
