#pragma once

#include "../Builder/ReportGenerator.h"

using namespace Builder;

class DialogReportFileTypeParams : public QDialog
{
	Q_OBJECT

public:
	explicit DialogReportFileTypeParams(const std::vector<ReportFileTypeParams>& fileTypeParams, std::vector<ReportFileTypeParams> defaultFileTypeParams, QWidget *parent);

	std::vector<ReportFileTypeParams> fileTypeParams() const;

private slots:
	void pageSetup();
	void setToDefault();

private:
	void fillTree();

private:
	DbController* m_db = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	std::vector<ReportFileTypeParams> m_fileTypeParams;

	std::vector<ReportFileTypeParams> m_defaultFileTypeParams;
};

