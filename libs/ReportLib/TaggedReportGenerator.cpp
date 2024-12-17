#include "TaggedReportPrivate.h"
#include <ReportLib/TaggedReportGenerator.h>

namespace ReportLib
{
	TaggedReportGenerator::TaggedReportGenerator(const ReportTemplate& reportTemplate, const ITaggedReportDataProvider& dataProvider) :
		m_impl(new TaggedReportPrivate(this, reportTemplate, dataProvider))
	{
	}

	bool TaggedReportGenerator::generate(QBuffer& buffer, std::atomic_bool& stop)
	{
		return m_impl->generate(buffer, stop);
	}

	Statistics TaggedReportGenerator::statistics() const
	{
		return m_impl->statistics();
	}

} // namespace ReportLib
