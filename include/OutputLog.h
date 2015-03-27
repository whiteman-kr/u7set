#pragma once
#include <QObject>

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

class OutputLog : public QObject
{
	Q_OBJECT

public:
	OutputLog(void);
	virtual ~OutputLog(void);

public:
	Q_INVOKABLE void clear();

	Q_INVOKABLE void write(const QString& str, OutputMessageLevel level, bool bold);
	Q_INVOKABLE void writeMessage(const QString& str, bool bold = false);
	Q_INVOKABLE void writeSuccess(const QString& str, bool bold = false);
	Q_INVOKABLE void writeWarning(const QString& str, bool bold = false);
	Q_INVOKABLE void writeError(const QString& str, bool bold = false);

    Q_INVOKABLE void writeDump(const std::vector<quint8> &data);

	bool windowMessageListEmpty() const;
	OutputLogItem popWindowMessages();

private:
	std::list<OutputLogItem> windowMessageList;		// List buffer for writing messages to main window Log Widget
	std::list<OutputLogItem> fileMessageList;		// List buffer for writing messages to log file

	mutable QMutex mutex;							// access to windowMessageList, fileMessageList
};
