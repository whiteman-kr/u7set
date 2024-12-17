#pragma once

#include <ReportLib/TaggedReportGenerator.h>
#include "TestLog.h"

namespace ReportLib
{
	class ReportTemplateStorage;
}

namespace TestSuite
{
	class TestLog;

	class TestReportDataProvider: public ReportLib::ITaggedReportDataProvider
	{
	public:
		TestReportDataProvider(const TestLog& testLog);


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
		static void generateReport(const ReportLib::ReportTemplateStorage& templates,
								   const TestSuite::TestLog& testLog,
								   const QString& caption,
								   QWidget* parent);

		static void generateReports(const ReportLib::ReportTemplateStorage& templates,
									const TestSuite::TestLog& testLog,
									const QString& captionMask,	// if empty - generate all
									const QString& path,
									ILogFile* appLog);
	};

}
