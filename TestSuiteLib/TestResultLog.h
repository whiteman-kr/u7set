#pragma once

class TestLogItem
{
public:

	TestLogItem(const QString& test);

	QString toText() const;

private:
	QString m_message;

};

class TestResultLog : public QObject
{
	Q_OBJECT
public:
	TestResultLog();

	void addMessage(const QString& text);

signals:
	void newLogItem(const TestLogItem& item);

private:
	std::vector<TestLogItem> m_items;
};

