#include "Stable.h"
#include "UserManagementDialog.h"
#include "ui_UserManagementDialog.h"
#include "CreateUserDialogDialog.h"
#include "../lib/DbController.h"
#include "PasswordService.h"

UserManagementDialog::UserManagementDialog(QWidget* parent, DbController* dbController) :
	QDialog(parent),
	ui(new Ui::UserManagementDialog),
	m_dbController(dbController),
	m_userHasChages(false)
{
	ui->setupUi(this);
	assert(m_dbController);

	// --
	//
	m_currentUser = m_dbController->currentUser();
	m_dbController->getUserList(&m_users, this);

	// Fill user list
	//
	connect(ui->userList, &QListWidget::currentTextChanged, this, &UserManagementDialog::on_userChanged);

	fillUserList(m_currentUser.username());


	// Enable/disable Create new user button
	//
	ui->isAdministrator->setEnabled(false);

	return;
}

UserManagementDialog::~UserManagementDialog()
{
	delete ui;
}

void UserManagementDialog::clearUserControls()
{
	ui->usernameEdit->clear();
	ui->userIdEdit->clear();
	ui->firstNameEdit->clear();
	ui->lastNameEdit->clear();
	ui->oldPasswordEdit->clear();
	ui->newPasswordEdit->clear();
	ui->passwordConfirmationEdit->clear();
	ui->isAdministrator->setChecked(false);
	ui->isReadonly->setChecked(false);
	ui->isDisabled->setChecked(false);

	return;
}

void UserManagementDialog::enableUserControls(bool enable)
{
	ui->usernameEdit->setEnabled(enable);
	ui->userIdEdit->setEnabled(enable);
	ui->firstNameEdit->setEnabled(enable);
	ui->lastNameEdit->setEnabled(enable);
	ui->oldPasswordEdit->setEnabled(enable);
	ui->newPasswordEdit->setEnabled(enable);
	ui->passwordConfirmationEdit->setEnabled(enable);
	ui->isReadonly->setEnabled(enable);
	ui->isDisabled->setEnabled(enable);

	return;
}

void UserManagementDialog::setUserControls(const DbUser& user)
{
	disableApply();

	ui->usernameEdit->setText(user.username());
	ui->userIdEdit->setText(QString::number(user.userId()));
	ui->firstNameEdit->setText(user.firstName());
	ui->lastNameEdit->setText(user.lastName());
	ui->oldPasswordEdit->clear();
	ui->newPasswordEdit->clear();
	ui->passwordConfirmationEdit->clear();
	ui->isAdministrator->setChecked(user.isAdminstrator());
	ui->isReadonly->setChecked(user.isReadonly());
	ui->isDisabled->setChecked(user.isDisabled());

	return;
}

void UserManagementDialog::fillUserList(const QString& selectUsername)
{
	ui->userList->clear();

	for (unsigned int i = 0; i < m_users.size(); i++)
	{
		ui->userList->addItem(m_users[i].username());

		if (m_users[i].username() == selectUsername)
		{
			ui->userList->setCurrentRow(i);
		}
	}
}

void UserManagementDialog::enableApply()
{
	ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void UserManagementDialog::disableApply()
{
	ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

DbController* UserManagementDialog::dbController()
{
	assert(m_dbController);
	return m_dbController;
}

void UserManagementDialog::on_createUserButton_clicked()
{
	if (m_currentUser.isAdminstrator() == false)
	{
		assert(m_currentUser.isAdminstrator());
		return;
	}

	CreateUserDialogDialog dialog(this, m_users);
	if (dialog.exec() == QDialog::Rejected)
	{
		return;
	}

	// Create user
	//
	DbUser user = dialog.user();

	dbController()->createUser(user, this);

	// Refresh user list
	//
	m_users.clear();

	dbController()->getUserList(&m_users, this);
	fillUserList(user.username());

	disableApply();
	return;
}

void UserManagementDialog::on_userChanged(const QString & username)
{
	auto user = std::find_if(m_users.begin(), m_users.end(),
		[&username](const DbUser user) -> bool
		{
			return user.username() == username;
		});

	if (user == m_users.end())
	{
		clearUserControls();
		enableUserControls(false);
		return;
	}

	setUserControls(*user);
	enableUserControls(m_currentUser.username() == username || m_currentUser.isAdminstrator());

	// User with name "Administartor" can't change own Administartor field or disable himself
	//
	if (m_currentUser.isAdminstrator() && m_currentUser.userId() == 1 && username == m_currentUser.username())
	{
		ui->isReadonly->setEnabled(false);
		ui->isDisabled->setEnabled(false);
	}

	if (m_currentUser.isAdminstrator() == false)
	{
		ui->isDisabled->setEnabled(false);
	}

	if (username == m_currentUser.username())
	{
		ui->oldPasswordEdit->setEnabled(true);
	}
	else if (m_currentUser.isAdminstrator())
	{
		ui->oldPasswordEdit->setEnabled(false);
	}

	if (username == m_currentUser.username() || username == "Administrator")
	{
		ui->isReadonly->setEnabled(false);
		ui->isDisabled->setEnabled(false);
	}
	else
	{
		ui->isReadonly->setEnabled(m_currentUser.isAdminstrator());
		ui->isDisabled->setEnabled(m_currentUser.isAdminstrator());
	}

	if (username == "Administrator" && m_currentUser.username() != "Administrator")
	{
		ui->passwordConfirmationEdit->setEnabled(false);
		ui->newPasswordEdit->setEnabled(false);
		ui->firstNameEdit->setEnabled(false);
		ui->lastNameEdit->setEnabled(false);
	}
	else
	{
		ui->passwordConfirmationEdit->setEnabled(true);
		ui->newPasswordEdit->setEnabled(true);
		ui->firstNameEdit->setEnabled(true);
		ui->lastNameEdit->setEnabled(true);
	}

	disableApply();

	return;
}

void UserManagementDialog::on_userList_currentItemChanged(QListWidgetItem* /*current*/, QListWidgetItem* previous)
{
	if (previous == nullptr)
	{
		return;
	}

	QString previousUsername = previous->text().trimmed();

	if (ui->buttonBox->button(QDialogButtonBox::Apply)->isEnabled() == true)
	{
		// There are some changes, ask for apply them
		//
		QMessageBox msgBox(this);

		msgBox.setText(tr("User data have been modified."));
		msgBox.setInformativeText(tr("Do you want to save your changes?"));
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
		msgBox.setDefaultButton(QMessageBox::Save);

		if (msgBox.exec() == QMessageBox::Save)
		{
			// Find user in the user list, save changes
			//

			auto user = std::find_if(m_users.begin(), m_users.end(),
				[&previousUsername](const DbUser user)
				{
					return user.username() == previousUsername;
				});

			if (user == m_users.end())
			{
				assert(user != m_users.end());
				return;
			}

			// Update user data
			//
			applyChanges(previousUsername);

			return;
		}
	}

	return;
}

void UserManagementDialog::on_firstNameEdit_textChanged(const QString&)
{
	enableApply();
}

void UserManagementDialog::on_lastNameEdit_textChanged(const QString&)
{
	enableApply();
}

void UserManagementDialog::on_newPasswordEdit_textChanged(const QString&)
{
	enableApply();
}

void UserManagementDialog::on_passwordConfirmationEdit_textChanged(const QString&)
{
	enableApply();
}

void UserManagementDialog::on_isAdministrator_stateChanged(int)
{
	enableApply();
}

void UserManagementDialog::on_isReadonly_stateChanged(int)
{
	enableApply();
}

void UserManagementDialog::on_isDisabled_stateChanged(int)
{
	enableApply();
}

void UserManagementDialog::applyChanges(const QString& username)
{
	// Get data drom edit fields
	//
	QString firstName = ui->firstNameEdit->text().trimmed();
	QString lastName = ui->lastNameEdit->text().trimmed();
	QString oldPassword = ui->oldPasswordEdit->text();
	QString newPassword = ui->newPasswordEdit->text();
	QString passwordConfirmation = ui->passwordConfirmationEdit->text();
	bool isAdministrator = ui->isAdministrator->isChecked();
	bool isReadonly = ui->isReadonly->isChecked();
	bool isDisabled = ui->isDisabled->isChecked();

	// Check password
	//
	bool changePassword = false;

	if (oldPassword.isEmpty() == false ||
		newPassword.isEmpty() == false ||
		passwordConfirmation.isEmpty() == false)
	{
		changePassword = PasswordService::checkPassword(newPassword, passwordConfirmation, true, this);

		if (changePassword == false)
		{
			ui->oldPasswordEdit->setFocus();
			return;
		}

		ui->oldPasswordEdit->clear();
		ui->newPasswordEdit->clear();
		ui->passwordConfirmationEdit->clear();
	}

	// Check password if user who change record is administrator
	//
	if (m_currentUser.isAdminstrator() &&
		m_currentUser.username() != username &&
		oldPassword.isEmpty() == true &&
		newPassword.isEmpty() == false &&
		passwordConfirmation.isEmpty() == false)
	{
		changePassword = PasswordService::checkPassword(newPassword, passwordConfirmation, true, this);

		if (changePassword == false)
		{
			return;
		}
	}

	// --
	//
	DbUser user;

	user.setUsername(username);
	user.setFirstName(firstName);
	user.setLastName(lastName);

	if (changePassword == true)
	{
		user.setPassword(oldPassword);
		user.setNewPassword(newPassword);
	}

	user.setAdministrator(isAdministrator);
	user.setReadonly(isReadonly);
	user.setDisabled(isDisabled);

	// Save to Database;
	//
	dbController()->updateUser(user, this);
	dbController()->getUserList(&m_users, this);

	disableApply();
}

void UserManagementDialog::on_buttonBox_clicked(QAbstractButton* button)
{
	auto buttonRole = ui->buttonBox->buttonRole(button);

	if (buttonRole == QDialogButtonBox::ApplyRole ||
		buttonRole == QDialogButtonBox::AcceptRole)
	{
		QList<QListWidgetItem*> si = ui->userList->selectedItems();
		if (si.isEmpty())
		{
			return;
		}

		QString username = si[0]->text().trimmed();

		if (dbController()->currentUser().isAdminstrator() || dbController()->currentUser().username() == username)
		{
			applyChanges(username);
			//fillUserList(username);
		}

		if (buttonRole == QDialogButtonBox::AcceptRole)
		{
			done(QDialog::Accepted);
		}
		return;
	}

	if (buttonRole == QDialogButtonBox::RejectRole)
	{
		done(QDialog::Rejected);
		return;
	}
}
