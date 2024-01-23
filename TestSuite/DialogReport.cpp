#include "DialogReport.h"
#include "ui_DialogReport.h"

#include "TestReport.h"

DialogReport::DialogReport(const ReportLib::ReportTemplateStorage& templates,
						   const TestSuite::TestLog& testLog,
						   QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogReport),
	m_templates(templates),
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
	for (const auto& t : m_templates.templates())
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

	TestSuite::TestReport::generateReport(m_templates, m_testLog, item->text(), this);
}


void DialogReport::on_listReports_itemDoubleClicked(QListWidgetItem* /*item*/)
{
	on_btnGenerate_clicked();
}

