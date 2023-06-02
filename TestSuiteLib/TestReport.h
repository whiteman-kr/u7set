#pragma once

#include "../ReportLib/ReportGenerator.h"
#include "TestLog.h"

namespace TestSuite
{
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
		TestReport();
	};
}
