#pragma once

#include <ReportLib/ReportTemplate.h>
#include <TestSuiteLib/TestLog.h>


namespace Ui
{
	class DialogReport;
}

class DialogReport : public QDialog
{
	Q_OBJECT

public:
	explicit DialogReport(const ReportLib::ReportTemplateStorage& templates, const TestSuite::TestLog& testLog, QWidget* parent = nullptr);
	~DialogReport();

private:
	void fillReportsList();

private slots:
	void on_btnGenerate_clicked();

	void on_listReports_itemDoubleClicked(QListWidgetItem* item);

private:
	Ui::DialogReport* ui;
	const ReportLib::ReportTemplateStorage& m_templates;
	const TestSuite::TestLog& m_testLog;
};
