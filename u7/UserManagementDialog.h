#pragma once

#include <QDialog>
#include "../include/DbStruct.h"

namespace Ui {
	class UserManagementDialog;
}

class DbController;

class UserManagementDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit UserManagementDialog(QWidget* parent, DbController* dbController);
	~UserManagementDialog();

private:
	void clearUserControls();
	void enableUserControls(bool enable);
	void setUserControls(const DbUser& user);

	void fillUserList(const QString& selectUsername);

	
private slots:
	void on_createUserButton_clicked();
	void on_userChanged(const QString& username);

	void on_userList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

	void on_firstNameEdit_textChanged(const QString &arg1);
	void on_lastNameEdit_textChanged(const QString &arg1);
	void on_newPasswordEdit_textChanged(const QString &arg1);
	void on_passwordConfirmationEdit_textChanged(const QString &arg1);
	void on_isAdministrator_stateChanged(int arg1);
	void on_isReadonly_stateChanged(int arg1);
	void on_isDisabled_stateChanged(int arg1);

	void enableApply();
	void disableApply();

	void applyChanges(const QString& username);

	void on_buttonBox_clicked(QAbstractButton *button);

private:
	DbController* dbController();

private:
	Ui::UserManagementDialog* ui;
	DbController* m_dbController;

	DbUser m_currentUser;
	std::vector<DbUser> m_users;

	bool m_userHasChages;
};

