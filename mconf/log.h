#pragma once

enum OutputMessageLevel
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

class OutputLog
{
public:
	OutputLog(void);
	virtual ~OutputLog(void);

public:
	void write(const QString& str, OutputMessageLevel level, bool bold);
	void writeMessage(const QString& str, bool bold = false);
	void writeSuccess(const QString& str, bool bold = false);
	void writeWarning(const QString& str, bool bold = false);
	void writeError(const QString& str, bool bold = false);

	void writeDump(const std::vector<uint8_t>& data);

	bool windowMessageListEmpty() const;
	OutputLogItem popWindowMessages();

private:
	std::list<OutputLogItem> windowMessageList;		// List buffer for writing messages to main window Log Widget
	std::list<OutputLogItem> fileMessageList;			// List buffer for writing messages to log file

	mutable QMutex mutex;						// access to windowMessageList, fileMessageList
};

extern OutputLog theLog;
