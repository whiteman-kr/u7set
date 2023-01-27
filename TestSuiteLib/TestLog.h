#pragma once

enum class TestLogItemType
{
	Message,
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

class TestLog : public QObject
{
	Q_OBJECT
public:
	TestLog();

	void addMessage(const QString& text);
	void addError(const QString& text);

signals:
	void newLogItem(const TestLogItem& item);

private:
	QMutex m_itemsMutex;	// For access to m_itemsMutex
	std::vector<TestLogItem> m_items;
};

