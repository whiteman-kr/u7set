#pragma once

#include <ReportLib/TaggedReportGenerator.h>

#include <vector>

class ILogFile;

namespace ReportLib
{
	class ReportTemplateStorage;
}

namespace TestSuite
{
	class TestLog;
	class TestLogItem;

	class TestReportDataProvider: public ReportLib::ITaggedReportDataProvider
	{
	public:
		TestReportDataProvider(const TestLog& testLog);
		~TestReportDataProvider() override;

	private:
		virtual int count() const override;
		virtual int count(const QString& tag) const override;

		virtual QString text(int index, QString* tag) const override;
		virtual QString text(const QString& tag, bool* found) const override;

	private:
		std::vector<TestLogItem> m_items;

		mutable QString m_lastTag;
		mutable int m_lastIndex{0};
	};

	class TestReport
	{
	public:
		static bool generateReport(const ::ReportLib::ReportTemplateStorage& templates,
								   const ::TestSuite::TestLog& testLog,
								   const QString& caption,
								   QWidget* parent);

		static bool generateReport(const ::ReportLib::ReportTemplateStorage& templates,
								   const ::TestSuite::TestLog& testLog,
								   const QString& caption,
								   const QString& fileName,
								   QWidget* parent);

		static bool generateReports(const ReportLib::ReportTemplateStorage& templates,
									const ::TestSuite::TestLog& testLog,
									const QString& captionMask, // if empty - generate all
									const QString& path,
									ILogFile* appLog);
	};

} // namespace TestSuite
