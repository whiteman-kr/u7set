#pragma once

#include "../DbLib/DbController.h"
#include "../ReportLib/Report.h"
#include "../Builder/SchemasReportGenerator.h"

class DialogSchemasReportTypePageSetup : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSchemasReportTypePageSetup(const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
										std::vector<Builder::SchemaTypesParams> defaultFileTypeParams,
										QWidget *parent);

	std::vector<Builder::SchemaTypesParams> schemaTypesParams() const;

private slots:
	void pageSetup();
	void setToDefault();

private:
	void fillTree();

private:
	DbController* m_db = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	std::vector<Builder::SchemaTypesParams> m_schemaTypesParams;
	std::vector<Builder::SchemaTypesParams> m_defaultFileTypeParams;
};


class DialogSchemasReport : public QDialog
{
	Q_OBJECT

public:
	DialogSchemasReport(const QString& path,
						const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
						const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
						const Builder::SchemasReportOptions& options,
						QWidget *parent);

	std::vector<Builder::SchemaTypesParams> schemaTypesParams() const;
	Builder::SchemasReportOptions options() const;

private slots:
	void okClicked();
	void browseClicked();
	void pageSetupClicked();
	void optionsClicked();

private:

	QTreeWidget* m_schemaTypesTree = nullptr;
	//QCheckBox* m_checkAddPageNumbers = nullptr;
	QCheckBox* m_checkInfoMode = nullptr;
	QCheckBox* m_checkAddSignalsSources = nullptr;
	QLineEdit* m_editReportPath = nullptr;

	QString m_reportPath;
	std::vector<Builder::SchemaTypesParams> m_schemaTypesParams;
	std::vector<Builder::SchemaTypesParams> m_defaultFileTypeParams;

	Builder::SchemasReportOptions m_options;

};

