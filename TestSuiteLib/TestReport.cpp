#include "TestReport.h"

//
// TestReportGenerator
//
TestReportGenerator::TestReportGenerator(const ReportLib::ReportTemplate& reportTemplate):
	ReportLib::ReportGenerator(reportTemplate)
{

}

QString TestReportGenerator::text(const QString& tag) const
{
    return QObject::tr("TestReportGenerator::text %1\n").arg(tag);

}

QString TestReportGenerator::tableText(const QString& tag, int column) const
{
    return QObject::tr("TestReportGenerator::tableText %1 %2").arg(tag).arg(column);
}


//
// TestReport
//
TestReport::TestReport()
{

}
