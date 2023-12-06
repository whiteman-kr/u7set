#pragma once

#include "../ReportLib/Report.h"
#include "../Builder/SchemasReportGenerator.h"
#include "../DbLib/DbController.h"

class VariablesWidget : public QWidget
{
	Q_OBJECT
public:
	VariablesWidget(const std::map<QString, QString>& variables, bool readOnly);
	std::map<QString, QString> getVariables() const;

private:
	void onCopyVariableClicked(int column);
	void onEditVariableClicked(int column);

private slots:
	void onAddVariableClicked();
	void onRemoveVariableClicked();
	void onCustomContextMenuRequested(const QPoint &pos);

private:
	QTreeWidget* m_variablesTree = nullptr;
	
	const std::map<QString, QString>& m_variables;
	bool m_readOnly = false;
};


class DialogSchemasReport : public QDialog
{
	Q_OBJECT

public:
	DialogSchemasReport(const QString& path,
						const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
						const Builder::SchemasReportOptions& options,
						DbController* db,
						QWidget *parent);
	virtual ~DialogSchemasReport();

	std::vector<Builder::SchemaTypesParams> schemaTypesParams() const;
	Builder::SchemasReportOptions options() const;
	QString path() const;

	bool optionsApplied() const;

private slots:
	void applyClicked();
	void okClicked();
	void browseClicked();
	void pageSetupClicked();

private:
	[[nodiscard]] bool applyOptions();

private:
	QLineEdit* m_editReportPath = nullptr;

	QTreeWidget* m_schemaTypesTree = nullptr;
	QTreeWidget* m_schemaTagsTree = nullptr;
	QSplitter* m_schemasTabSplitter = nullptr;

	QCheckBox* m_checkSignleFile = nullptr;

	QCheckBox* m_checkAddTableOfContents = nullptr;
	QCheckBox* m_checkAddFolders = nullptr;
	QCheckBox* m_checkAddFooters = nullptr;
	QCheckBox* m_checkSignalsDetails = nullptr;
	QCheckBox* m_checkItemsLabels = nullptr;

	QLineEdit* m_editStartPageNumber = nullptr;
	
	QLineEdit* m_editContentsTextFontSize = nullptr;
	QLineEdit* m_editContentsTableFontSize = nullptr;
	
	QLineEdit* m_editTextFontSize = nullptr;
	QLineEdit* m_editTableFontSize = nullptr;

	VariablesWidget* m_userVariables = nullptr;
	VariablesWidget* m_projectVariables = nullptr;
	QSplitter* m_variablesTabSplitter = nullptr;

	QString m_reportPath;
	std::vector<Builder::SchemaTypesParams> m_schemaTypesParams;
	std::vector<Builder::SchemaTypesParams> m_defaultFileTypeParams;

	DbController* m_db;

	Builder::SchemasReportOptions m_options;

	bool m_optionsApplied = false;

};

