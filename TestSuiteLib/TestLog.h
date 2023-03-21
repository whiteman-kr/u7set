#pragma once


class IOutputLog
{
public:
	virtual void writeMessage(const QString& text) = 0;
	virtual void writeWarning(const QString& text) = 0;
	virtual void writeError(const QString& text) = 0;
};

enum class TestLogItemType
{
	Message,
	Warning,
	Error
};

class TestLogItem
{
public:

	TestLogItem(const QString& text, TestLogItemType type);

	QString toText() const;

private:
	QString m_message;
	TestLogItemType m_type = TestLogItemType::Message;
	QDateTime m_dateTime;

};
class TestLog
{
public:
	TestLog(IOutputLog* outputLog);

	void writeError(const QString& text);
	void writeWarning(const QString& text);
	void writeMessage(const QString& text);

private:
	QMutex m_itemsMutex;	// For access to m_itemsMutex
	std::vector<TestLogItem> m_items;

	IOutputLog* m_outputLog = nullptr;
};

