#include "TestReport.h"

namespace TestSuite
{
	//
	// TestReportGenerator
	//
	TestReportGenerator::TestReportGenerator(const ReportLib::ReportTemplate& reportTemplate, const TestLog& testLog):
		ReportLib::ReportGenerator(reportTemplate),
		m_items(testLog.items())
	{

	}

	int TestReportGenerator::count(const QString& /*tag*/) const
	{
		return 0;
	}

	QString TestReportGenerator::text(const QString& tag, bool* found)
	{
		if (found == nullptr)
		{
			Q_ASSERT(found);
			return QString();
		}

		if (m_lastTag != tag)
		{
			m_lastTag = tag;
			m_lastIndex = 0;
		}
		else
		{
			m_lastIndex++;
		}

		if (m_items.empty() == true)
		{
			*found = false;
			return QString();
		}

		int count = static_cast<int>(m_items.size());
		for (; m_lastIndex < count; m_lastIndex++)
		{
			if (m_items[m_lastIndex].tag() == tag)
			{
				*found = true;
				return m_items[m_lastIndex].message();
			}
		}

		*found = false;
		return QString();
	}

	//
	// TestReport
	//
	TestReport::TestReport()
	{

	}
}
