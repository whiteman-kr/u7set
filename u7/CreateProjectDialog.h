#pragma once

#include <QDialog>

namespace Ui {
	class CreateProjectDialog;
}

class CreateProjectDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit CreateProjectDialog(QWidget *parent = 0);
	~CreateProjectDialog();

	// Result
public:
	QString projectName;
	QString adminstratorPassword;
	
private slots:
	void on_okButton_clicked();

private:
	Ui::CreateProjectDialog *ui;
};

