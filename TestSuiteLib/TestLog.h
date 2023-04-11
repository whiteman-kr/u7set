#pragma once
#include <iostream>

namespace TestSuite
{
	class ITestLog
	{
	public:
		virtual ~ITestLog() = default;
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
		explicit TestLogItem(const QString& text, TestLogItemType type);

		QString toText() const;

	private:
		QString m_message;
		TestLogItemType m_type = TestLogItemType::Message;
		QDateTime m_dateTime;
	};

	class TestLog
	{
	public:
		explicit TestLog(ITestLog* outputLog);

		void writeError(const QString& text);
		void writeWarning(const QString& text);
		void writeMessage(const QString& text);

	private:
		QMutex m_itemsMutex;	// For access to m_itemsMutex
		std::vector<TestLogItem> m_items;

		ITestLog* m_outputLog = nullptr;
	};

	class ConsoleTestLog : public TestSuite::ITestLog
	{
	public:
		void writeMessage(const QString& text) override	{	qInfo() << text;		}
		void writeWarning(const QString& text) override	{	qWarning() << text;		}
		void writeError(const QString& text) override	{	qCritical() << text;	}
	};
}
