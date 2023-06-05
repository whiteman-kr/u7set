#include "DialogReport.h"
#include "ui_DialogReport.h"

#include "TestReport.h"

DialogReport::DialogReport(const TestSuite::TestSuiteConfigController& configController,
						   const TestSuite::TestLog& testLog,
						   QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogReport),
	m_configController(configController),
	m_testLog(testLog)
{
	ui->setupUi(this);
	fillReportsList();
}

DialogReport::~DialogReport()
{
	delete ui;
}

void DialogReport::fillReportsList()
{
	const std::vector<ReportLib::ReportTemplate>& templates = m_configController.reportTemplates().templates();

	for (const auto& t : templates)
	{
		ui->listReports->addItem(t.caption());
	}
}

void DialogReport::on_btnGenerate_clicked()
{
	QListWidgetItem* item = ui->listReports->currentItem();
	if (item == nullptr)
	{
		return;
	}

	TestSuite::TestReport::generateReport(m_configController.reportTemplates(), m_testLog, item->text(), this);
}


void DialogReport::on_listReports_itemDoubleClicked(QListWidgetItem* /*item*/)
{
	on_btnGenerate_clicked();
}

