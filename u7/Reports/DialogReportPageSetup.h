#pragma once
#include "../Builder/SchemasReportGenerator.h"

class DialogReportPageSetup : public QDialog
{
	Q_OBJECT

public:
	explicit DialogReportPageSetup(const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
											  const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
											  QWidget* parent);

	std::vector<Builder::SchemaTypesParams> schemaTypesParams() const;

private slots:
	void pageSetup();
	void setToDefault();

private:
	void fillTree();
	void saveOptions();

	void accept() override;

private:
	DbController* m_db = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	std::vector<Builder::SchemaTypesParams> m_schemaTypesParams;
	std::vector<Builder::SchemaTypesParams> m_defaultFileTypeParams;

	static const int m_noMarginsColumn = 4;
	
	static const int m_typeIndexColumn = 0;
	static const int m_layoutIndexColumn = 1;
};

