#pragma once

#include <QDialog>
#include "../VFrame30/VFrame30.h"

namespace Ui {
	class CreateSchemeDialog;
}

class CreateSchemeDialog : public QDialog
{
	Q_OBJECT
	
public:
	CreateSchemeDialog(std::shared_ptr<VFrame30::Scheme> scheme, QWidget* parent);
	~CreateSchemeDialog();

protected slots:
	virtual void accept();
	
private:
	Ui::CreateSchemeDialog *ui;

	std::shared_ptr<VFrame30::Scheme> m_scheme;
};

