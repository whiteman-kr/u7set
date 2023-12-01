#pragma once

#include "../ReportLib/Report.h"
#include "../Builder/SchemasReportGenerator.h"
#include "../DbLib/DbController.h"


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


class VariablesWidget : public QWidget
{
	Q_OBJECT
public:
	VariablesWidget(const std::map<QString, QString>& variables);
	std::map<QString, QString> getVariables() const;

private slots:
	void onAddVariableClicked();
	void onRemoveVariableClicked();

private:
	QTreeWidget* m_variablesTree = nullptr;

	const std::map<QString, QString>& m_variables;
};


class DialogSchemasReport : public QDialog
{
	Q_OBJECT

public:
	DialogSchemasReport(const QString& path,
						const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
						const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
						const Builder::SchemasReportOptions& options,
						DbController* db,
						QWidget *parent);

	std::vector<Builder::SchemaTypesParams> schemaTypesParams() const;
	Builder::SchemasReportOptions options() const;
	QString path() const;

private slots:
	void applyClicked();
	void okClicked();
	void browseClicked();
	void pageSetupClicked();

private:
	bool applyOptions();

private:
	QLineEdit* m_editReportPath = nullptr;

	QTreeWidget* m_schemaTypesTree = nullptr;
	QTreeWidget* m_schemaTagsTree = nullptr;

	QCheckBox* m_checkAddTableOfContents = nullptr;
	QCheckBox* m_checkAddFolders = nullptr;
	QCheckBox* m_checkAddFooters = nullptr;
	QCheckBox* m_checkSignalsDetails = nullptr;
	QCheckBox* m_checkItemsLabels = nullptr;

	VariablesWidget* m_userVariables = nullptr;
	VariablesWidget* m_projectVariables = nullptr;

	QString m_reportPath;
	std::vector<Builder::SchemaTypesParams> m_schemaTypesParams;
	std::vector<Builder::SchemaTypesParams> m_defaultFileTypeParams;

	DbController* m_db;

	Builder::SchemasReportOptions m_options;

};

