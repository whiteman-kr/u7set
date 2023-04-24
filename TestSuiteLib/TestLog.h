#pragma once
#include <iostream>

namespace TestSuite
{
	class ITestLog
	{
	public:
		virtual ~ITestLog() = default;
		virtual void writeError(const QString& text) = 0;
		virtual void writeWarning(const QString& text) = 0;
		virtual void writeMessage(const QString& text) = 0;
	};

	enum class TestLogItemType
	{
		Error,
		Warning,
		Message
	};

	class TestLogItem
	{
	public:
		explicit TestLogItem(int no, const QString& text, TestLogItemType type);

		QString toText() const;
		QString toHtml() const;

		TestLogItemType type() const;
		bool isError() const;
		bool isWarning() const;
		bool isMessage() const;

	private:
		int m_no = 0;
		QString m_message;
		TestLogItemType m_type = TestLogItemType::Message;
		QDateTime m_dateTime;

//		QString m_file;
//		int m_fileLine = 0;
//		QString m_func;
		QString m_htmlFont;

	};

	class ITestLogOutput
	{
	public:
		virtual ~ITestLogOutput() = default;
		virtual void logItemArrived(const TestLogItem& item) = 0;
	};

	class TestLog : public ITestLog
	{
	public:
		explicit TestLog(ITestLogOutput* logOutput);

		void clear();

		void writeError(const QString& text) override;
		void writeWarning(const QString& text) override;
		void writeMessage(const QString& text) override;

	private:
		int no = 0;

		QReadWriteLock m_itemsLock;	// For access to m_items and m_items
		std::vector<TestLogItem> m_items;

		ITestLogOutput* m_logOutput = nullptr;
	};
}
