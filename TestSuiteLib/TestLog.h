#pragma once
#include <iostream>

namespace TestSuite
{
	class ITestLog
	{
	public:
		virtual ~ITestLog() = default;
		virtual void writeError(const QString& text, int level, int tag) = 0;
		virtual void writeWarning(const QString& text, int level, int tag) = 0;
		virtual void writeMessage(const QString& text, int level, int tag) = 0;
		virtual void writeText(const QString& text, int level, int tag) = 0;
	};

	enum class TestLogItemType
	{
		Error,
		Warning,
		Message,
		Text,
		All
	};

	class TestLogItem
	{
	public:
		TestLogItem() = default;
		TestLogItem(int no, TestLogItemType type, const QString& text, int level, int tag);

		QString toText() const;
		QString toHtml() const;

		QStringList toStringList() const;
		static TestLogItem fromStringList(const QString& str, bool* ok);

		TestLogItemType type() const;
		bool isError() const;
		bool isWarning() const;
		bool isMessage() const;

	private:
		int m_no{0};
		QDateTime m_dateTime;
		TestLogItemType m_type = TestLogItemType::Message;
		QString m_message;
		int m_level{0};
		int m_tag{0};

//		QString m_file;
//		int m_fileLine = 0;
//		QString m_func;
		QString m_htmlFont;

		static const int CsvVersion = 1;
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

		void writeError(const QString& text, int level, int tag) override;
		void writeWarning(const QString& text, int level, int tag) override;
		void writeMessage(const QString& text, int level, int tag) override;
		void writeText(const QString& text, int level, int tag) override;

		std::vector<TestLogItem> items() const;

		bool saveToCSV(const QString& fileName, QString* errorMsg) const;
		bool loadFromCSV(const QString& fileName, QString* errorMsg);

	private:
		int no = 0;

		mutable QReadWriteLock m_itemsLock;	// For access to m_items and m_items
		std::vector<TestLogItem> m_items;

		ITestLogOutput* m_logOutput = nullptr;
	};
}
