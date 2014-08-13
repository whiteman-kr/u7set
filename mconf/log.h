#pragma once

enum LogMessageLevel
{
	Message,
	Success,
	Warning,
	Error
};

class LogItem
{
public:
	LogItem();
	LogItem(const QString& m, LogMessageLevel l, bool b, const QDateTime& t);

public:
	QString message;
	LogMessageLevel level;
	bool bold;
	QDateTime time;
};

class Log
{
public:
	Log(void);
	virtual ~Log(void);

public:
	void write(const QString& str, LogMessageLevel level, bool bold);
	void writeMessage(const QString& str, bool bold = false);
	void writeSuccess(const QString& str, bool bold = false);
	void writeWarning(const QString& str, bool bold = false);
	void writeError(const QString& str, bool bold = false);

	void writeDump(const std::vector<uint8_t>& data);

	bool windowMessageListEmpty() const;
	LogItem popWindowMessages();

private:
	std::list<LogItem> windowMessageList;		// List buffer for writing messages to main window Log Widget
	std::list<LogItem> fileMessageList;			// List buffer for writing messages to log file

	mutable QMutex mutex;						// access to windowMessageList, fileMessageList
};

extern Log theLog;
