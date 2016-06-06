#include "Stable.h"
#include "CreateUserDialogDialog.h"
#include "ui_CreateUserDialogDialog.h"
#include "PasswordService.h"

CreateUserDialogDialog::CreateUserDialogDialog(QWidget* parent, const std::vector<DbUser>& users) :
	QDialog(parent),
	ui(new Ui::CreateUserDialogDialog),
	m_users(users)
{
	ui->setupUi(this);
}

CreateUserDialogDialog::~CreateUserDialogDialog()
{
	delete ui;
}

const DbUser& CreateUserDialogDialog::user() const
{
	return m_user;
}

void CreateUserDialogDialog::done(int r)
{
	if (r == Accepted)
	{
		QString username = ui->usernameEdit->text().trimmed();
		QString password = ui->passwordEdit->text();
		QString passwordConfirmation = ui->passwordConfirmatioEdit->text();

		m_user.setUsername(username);
		m_user.setNewPassword(password);

		// Check username
		//
		if (username.isEmpty())
		{
			QMessageBox mb(this);
			mb.setText(tr("Wrong username."));
			mb.exec();

			ui->usernameEdit->setFocus();
			return;
		}

		// --
		//
		auto fu = std::find_if(m_users.begin(), m_users.end(),
			[&username](const DbUser& u)
			{
				return u.username() == username;
			});

		if (fu != m_users.end())
		{
			QMessageBox mb(this);
			mb.setText(tr("Username %1 is already exists.").arg(username));
			mb.setInformativeText(tr("Choose another username."));
			mb.exec();

			ui->usernameEdit->setFocus();

			return;
		}


		// Check password
		//
		bool ok = PasswordService::checkPassword(password, passwordConfirmation, true, this);

		if (ok == true)
		{
			QDialog::done(r);
		}
	}
	else
	{
		QDialog::done(r);
	}

	return;
}
