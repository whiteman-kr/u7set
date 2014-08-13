#include "CreateProjectDialog.h"
#include "ui_CreateProjectDialog.h"
#include "PasswordService.h"

CreateProjectDialog::CreateProjectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CreateProjectDialog)
{
	ui->setupUi(this);

	ui->projectNameEdit->setText("CUSTOMER_6USB1");

	connect(ui->cancelButton, &QAbstractButton::clicked, this, &CreateProjectDialog::reject);
}

CreateProjectDialog::~CreateProjectDialog()
{
	delete ui;
}

void CreateProjectDialog::on_okButton_clicked()
{
	projectName = ui->projectNameEdit->text().trimmed();
	adminstratorPassword = ui->passwordEdit->text();
	const QString& confirmedPassword = ui->confirmPasswordEdit->text();

	bool result = PasswordService::checkPassword(adminstratorPassword, confirmedPassword, true, this);

	if (result == false)
	{
		return;
	}

	accept();
}
