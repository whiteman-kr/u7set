#include "Stable.h"
#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include "Settings.h"

LoginDialog::LoginDialog(const QStringList& loginCompleterList, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::LoginDialog)
{
	ui->setupUi(this);

	QString defaultUsername = theSettings.loginDialog_defaultUsername;
	ui->usernameEdit->setText(defaultUsername);

	m_completer = new QCompleter(loginCompleterList, this);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);

	ui->usernameEdit->setCompleter(m_completer);

	if (defaultUsername.isEmpty() == false)
	{
		ui->passwordEdit->setFocus();
	}
}

LoginDialog::~LoginDialog()
{
	delete ui;
}

const QString& LoginDialog::username() const
{
	return m_username;
}

const QString& LoginDialog::password() const
{
	return m_password;
}

void LoginDialog::on_buttonBox_accepted()
{
	m_username = ui->usernameEdit->text().trimmed();
	m_password = ui->passwordEdit->text();

	if (m_password.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Password cannon be empty"));
		ui->passwordEdit->setFocus();
		reject();
	}

	theSettings.loginDialog_defaultUsername = m_username;
	return;
}
