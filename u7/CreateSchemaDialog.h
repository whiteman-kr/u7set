#pragma once

#include <QDialog>
#include "../VFrame30/VFrame30.h"

namespace Ui {
	class CreateSchemaDialog;
}

class CreateSchemaDialog : public QDialog
{
	Q_OBJECT
	
public:
	CreateSchemaDialog(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent);
	~CreateSchemaDialog();

protected slots:
	virtual void accept();
	
private:
	Ui::CreateSchemaDialog *ui;

	std::shared_ptr<VFrame30::Schema> m_schema;
};

