#pragma once

#include <QDialog>

namespace Ui {
	class CreateSchemaDialog;
}

namespace VFrame30
{
	class Schema;
	class LogicSchema;
	class UfbSchema;
}

class DbController;
class DbFile;

class CreateSchemaDialog : public QDialog
{
	Q_OBJECT
	
public:
	CreateSchemaDialog(std::shared_ptr<VFrame30::Schema> schema, DbController* db, int tempateParentFileId, QString templateFileExtension, QWidget* parent);
	virtual ~CreateSchemaDialog();

protected slots:
	virtual void accept();
	void templateChanged(int index);

private:
	void setWidthHeight(VFrame30::Schema* schema);

	bool isLogicSchema() const;
	bool isUfbSchema() const;
	bool isMonitorSchema() const;
	bool isDiagSchema() const;

	std::shared_ptr<VFrame30::LogicSchema> logicSchema();
	std::shared_ptr<VFrame30::UfbSchema> ufbSchema();
	
private:
	Ui::CreateSchemaDialog *ui;

	DbController* m_db = nullptr;

	std::shared_ptr<VFrame30::Schema> m_schema;
	std::shared_ptr<VFrame30::Schema> m_templateSchema;

	std::vector<std::shared_ptr<DbFile>> m_templates;
};

