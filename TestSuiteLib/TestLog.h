#pragma once
#include "../UtilsLib/ILogFile.h"
#include <vector>
#include <QReadWriteLock>

namespace TestSuite
{
	class ConstStrings : public QObject
	{
		Q_OBJECT
	public:
		static QString TEST_PASSED()
		{
			return tr("PASSED");
		}
		static QString TEST_FAILED()
		{
			return tr("FAILED");
		}
		static QString TEST_RUNNING()
		{
			return tr("RUNNING");
		}
		static QString TEST_ALLOWED()
		{
			return tr("Allowed");
		}
		static QString TEST_DENIED()
		{
			return tr("Denied");
		}
	};

	enum class TestLogItemType
	{
		Error = 0x0001,
		Warning = 0x0002,
		Message = 0x0004,
		Text = 0x0008,
		All = 0xffff
	};

	class TestLogItem
	{
	public:
		TestLogItem() = default;
		TestLogItem(int no, TestLogItemType type, const QString& text, const QString& tag);

		QString toText() const;
		QString toHtml() const;

		QStringList toStringList() const;
		static TestLogItem fromStringList(const QString& str, bool* ok);

		const QDateTime& dateTime() const;
		const QString& message() const;
		const QString& tag() const;

		TestLogItemType type() const;
		bool isError() const;
		bool isWarning() const;
		bool isMessage() const;

	private:
		int m_no{0};
		QDateTime m_dateTime;
		TestLogItemType m_type{TestLogItemType::Message};
		QString m_message;
		QString m_tag;

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

	class TestLog : public ILogFile
	{
	public:
		explicit TestLog(ITestLogOutput* logOutput);

		void clear();
		bool empty() const;

		bool writeAlert(const QString& text, const QString& tag) override;
		bool writeError(const QString& text, const QString& tag) override;
		bool writeWarning(const QString& text, const QString& tag) override;
		bool writeMessage(const QString& text, const QString& tag) override;
		bool writeText(const QString& text, const QString& tag) override;

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
