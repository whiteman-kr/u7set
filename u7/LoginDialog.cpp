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

	return;
}

LoginDialog::~LoginDialog()
{
	delete ui;
}

QString LoginDialog::username() const
{
	return m_username;
}

QString LoginDialog::password() const
{
	return m_password;
}

void LoginDialog::showEvent(QShowEvent* event)
{
	if (event->spontaneous() == true)
	{
		// Resize depends on monitor size, DPI, resolution
		//
		QSize resizeTo = size();
		QRect screen = QDesktopWidget().availableGeometry(this);
		resizeTo.setWidth(screen.size().width() * 0.12);

		resize(resizeTo);
		move(screen.center() - rect().center());
	}

	return;
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
