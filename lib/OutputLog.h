#pragma once
#include <set>
#include <QObject>
#include <QDateTime>
#include <QMutex>
#include "../CommonLib/Types.h"

#ifdef Q_CC_MSVC
	#define SHORT_FUNC_INFO __FUNCTION__
#else
	#ifdef Q_CC_GNU
		#define SHORT_FUNC_INFO __FUNCTION__
	#else
		#define SHORT_FUNC_INFO Q_FUNC_INFO
	#endif
#endif


#define LOG_STRING(PARAM) #PARAM

#define LOG_EMPTY_LINE(logObject)	logObject->writeEmptyLine();

#define LOG_ERROR_OBSOLETE(logObject, prefix, message)		logObject->writeError(message, __FILE__, __LINE__, SHORT_FUNC_INFO);
#define LOG_WARNING_OBSOLETE(logObject, prefix, message)	logObject->writeWarning0(message, __FILE__, __LINE__, SHORT_FUNC_INFO);
#define LOG_MESSAGE(logObject, message)	logObject->writeMessage(message, __FILE__, __LINE__, SHORT_FUNC_INFO);
#define LOG_SUCCESS(logObject, message)	logObject->writeSuccess(message, __FILE__, __LINE__, SHORT_FUNC_INFO);

#define LOG_OK(logObject) logObject->writeSuccess(QObject::tr("Ok"), __FILE__, __LINE__, SHORT_FUNC_INFO);

class OutputLogItem
{
public:
	OutputLogItem() = default;
	OutputLogItem(const OutputLogItem&) = default;
	OutputLogItem(int messageNo);
	OutputLogItem(int messageNo,
				  QString message,
				  OutputMessageLevel level,
				  QDateTime time,
				  QString file,
				  int fileLine,
				  QString func,
				  QString htmlFont);

	QString toText() const;
	QString toHtml() const;
	QString toCsv() const;

	bool isError() const;
	bool isWarning() const;
	bool isWarning0() const;
	bool isWarning1() const;
	bool isWarning2() const;
	bool isSuccess() const;
	bool isMessage() const;

public:
	int m_no = 0;
	QString m_message;
	OutputMessageLevel m_level;
	QDateTime m_time;

	QString m_file;
	int m_fileLine = 0;
	QString m_func;
	QString m_htmlFont;
};

Q_DECLARE_METATYPE(OutputLogItem)


class OutputLog : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int ErrorCount READ errorCount WRITE setErrorCount RESET resetErrorCount NOTIFY errorCountChanged)
	Q_PROPERTY(int WarningCount READ warningCount WRITE setWarningCount RESET resetWarningCount NOTIFY warningCountChanged)
	Q_PROPERTY(QString HtmlFont READ htmlFont WRITE setHtmlFont)

public:
	OutputLog(void);
	virtual ~OutputLog(void);

public:
	Q_INVOKABLE void clear();

	Q_INVOKABLE void write(const QString& str, OutputMessageLevel level, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeMessage(const QString& str);
	Q_INVOKABLE void writeSuccess(const QString& str);

	Q_INVOKABLE void writeWarning0(const QString& str);
	Q_INVOKABLE void writeWarning1(const QString& str);
	Q_INVOKABLE void writeWarning2(const QString& str);

	Q_INVOKABLE void writeWarning0(QString issueCategory, int issueCode, const QString& str);
	Q_INVOKABLE void writeWarning1(QString issueCategory, int issueCode, const QString& str);
	Q_INVOKABLE void writeWarning2(QString issueCategory, int issueCode, const QString& str);

	Q_INVOKABLE void writeError(const QString& str);
	Q_INVOKABLE void writeError(QString issueCategory, int issueCode, const QString& str);

	Q_INVOKABLE void writeMessage(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeSuccess(const QString& str, QString file, int fileLine, QString func);

	Q_INVOKABLE void writeWarning0(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeWarning1(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeWarning2(const QString& str, QString file, int fileLine, QString func);

	Q_INVOKABLE void writeWarning0(QString issueCategory, int issueCode, const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeWarning1(QString issueCategory, int issueCode, const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeWarning2(QString issueCategory, int issueCode, const QString& str, QString file, int fileLine, QString func);

	Q_INVOKABLE void writeError(const QString& str, QString file, int fileLine, QString func);
	Q_INVOKABLE void writeError(QString issueCategory, int issueCode, const QString& str, QString file, int fileLine, QString func);

    Q_INVOKABLE void writeDump(const std::vector<quint8> &data);
	Q_INVOKABLE void writeEmptyLine();

	void setSupressIssues(std::vector<int> warnings);
	bool isIssueSuppressed(int issueCode) const;

	bool isEmpty() const;
	OutputLogItem popMessages();
	void popMessages(std::vector<OutputLogItem>* out, int maxCount);

	void startStrLogging();
	QString finishStrLogging();

	// Signals
	//
signals:
	void newLogItem(OutputLogItem logItem);
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

	QString htmlFont() const;
	void setHtmlFont(QString fontName);

private:
	std::list<OutputLogItem> m_messages;			// List buffer for writing messages to main window Log Widget

	std::set<int> m_suppressedIssues;

	bool m_strLogging = false;
	QString m_strFullLog;

	int m_errorCount = 0;
	int m_warningCount = 0;
	int m_messagesNo = 0;
	mutable QMutex m_mutex;							// access to windowMessageList, fileMessageList

	QString m_htmlFont;
};
