#pragma once

#include "../ReportLib/ReportGenerator.h"

class TestReportGenerator : public ReportLib::ReportGenerator
{
public:
    TestReportGenerator(const ReportLib::ReportTemplate& reportTemplate);


private:
	virtual int count(const QString& tag) const override;

	virtual QString text(const QString& tag, int index) const override;
	virtual QString tableText(const QString& tag, int index) const override;


};

class TestReport
{
public:
	TestReport();
};

