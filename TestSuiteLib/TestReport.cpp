#include "TestReport.h"

//
// TestReportGenerator
//
TestReportGenerator::TestReportGenerator(const ReportLib::ReportTemplate& reportTemplate):
	ReportLib::ReportGenerator(reportTemplate)
{

}

int TestReportGenerator::count(const QString& tag) const
{
	return 25;
}

QString TestReportGenerator::text(const QString& tag, int index) const
{
	return QObject::tr("Text %1 # %2\n").arg(tag).arg(index);
}

QString TestReportGenerator::tableText(const QString& tag, int index) const
{
	return QObject::tr("%1;%2").arg(tag).arg(index);
}

//
// TestReport
//
TestReport::TestReport()
{

}
