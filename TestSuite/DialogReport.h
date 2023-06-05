#ifndef DIALOGREPORT_H
#define DIALOGREPORT_H

#include <QDialog>
#include <QListWidget>

#include "TestSuiteConfigController.h"

namespace Ui {
	class DialogReport;
}

class DialogReport : public QDialog
{
	Q_OBJECT

public:
	explicit DialogReport(const TestSuite::TestSuiteConfigController& configController,
						  const TestSuite::TestLog& testLog,
						  QWidget *parent = nullptr);
	~DialogReport();

private:
	void fillReportsList();

private slots:
	void on_btnGenerate_clicked();

	void on_listReports_itemDoubleClicked(QListWidgetItem* item);

private:
	Ui::DialogReport *ui;
	const TestSuite::TestSuiteConfigController& m_configController;
	const TestSuite::TestLog& m_testLog;
};

#endif // DIALOGREPORT_H
