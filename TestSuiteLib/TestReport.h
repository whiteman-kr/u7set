#pragma once

#include "../ReportLib/ReportGenerator.h"
#include "TestLog.h"

namespace ReportLib
{
	class ReportTemplateStorage;
}

namespace TestSuite
{
	class TestLog;

	class TestReportGenerator : public ReportLib::ReportGenerator
	{
	public:
		TestReportGenerator(const ReportLib::ReportTemplate& reportTemplate, const TestLog& testLog);


	private:
		virtual int count(const QString& tag) const override;

		virtual QString text(const QString& tag, bool* found) override;

	private:
		std::vector<TestLogItem> m_items;

		QString m_lastTag;
		int m_lastIndex{0};
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
