#include "CreateProjectDialog.h"
#include "ui_CreateProjectDialog.h"
#include "PasswordService.h"
#include <QMessageBox>

CreateProjectDialog::CreateProjectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CreateProjectDialog)
{
	ui->setupUi(this);

	ui->projectNameEdit->setText("CUSTOMER_6USB1");
	ui->projectDescriptionEdit->setText("Description");

	connect(ui->cancelButton, &QAbstractButton::clicked, this, &CreateProjectDialog::reject);
}

CreateProjectDialog::~CreateProjectDialog()
{
	delete ui;
}

void CreateProjectDialog::on_okButton_clicked()
{
	projectName = ui->projectNameEdit->text().trimmed();
	projectDescription = ui->projectDescriptionEdit->text().trimmed();

	adminstratorPassword = ui->passwordEdit->text();
	const QString& confirmedPassword = ui->confirmPasswordEdit->text();

	bool result = PasswordService::checkPassword(adminstratorPassword, confirmedPassword, "Administrator", true, this);
	if (result == false)
	{
		return;
	}

	if (projectName.size() > 40)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("The project name is limited to 40 characters."));
		return;
	}

	if (projectName.count(QRegExp("[A-Za-z_0-9]")) != projectName.size())
	{
		QMessageBox::critical(this, qApp->applicationName(), QString("The project name contains illegal characters: %1").arg(projectName.remove(QRegExp("[A-Za-z_0-9]"))));
		return;
	}

	accept();
}
