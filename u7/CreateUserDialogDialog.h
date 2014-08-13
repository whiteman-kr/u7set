#ifndef CREATEUSERDIALOGDIALOG_H
#define CREATEUSERDIALOGDIALOG_H

#include <QDialog>
#include "../include/DbStruct.h"

namespace Ui {
	class CreateUserDialogDialog;
}

class CreateUserDialogDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit CreateUserDialogDialog(QWidget* parent, const std::vector<DbUser>& users);
	~CreateUserDialogDialog();

	const DbUser& user() const;
	
private slots:
	virtual void done(int r) override;

private:
	Ui::CreateUserDialogDialog* ui;
	std::vector<DbUser> m_users;

	DbUser m_user;
};

#endif // CREATEUSERDIALOGDIALOG_H
