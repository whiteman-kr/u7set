#pragma once

#include "../DbLib/DbController.h"
#include "../ReportLib/Report.h"

class DialogReportFileTypeParams : public QDialog
{
	Q_OBJECT

public:
	explicit DialogReportFileTypeParams(const std::vector<ReportLib::ReportFileTypeParams>& fileTypeParams,
										std::vector<ReportLib::ReportFileTypeParams> defaultFileTypeParams,
										QWidget *parent);

	std::vector<ReportLib::ReportFileTypeParams> fileTypeParams() const;

private slots:
	void pageSetup();
	void setToDefault();

private:
	void fillTree();

private:
	DbController* m_db = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	std::vector<ReportLib::ReportFileTypeParams> m_fileTypeParams;

	std::vector<ReportLib::ReportFileTypeParams> m_defaultFileTypeParams;
};

