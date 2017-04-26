#ifndef DIALOGUSERS_H
#define DIALOGUSERS_H

#include <QDialog>

#include "MainWindow.h"
#include "DialogProperties.h"

namespace Ui {
	class DialogUsers;
}


class DialogUsers : public QDialog
{
	Q_OBJECT

public:
	explicit DialogUsers(const UserManager& userManager, QWidget* parent = 0);
	~DialogUsers();

public:
	UserManager m_userManager;

private slots:
	void on_m_add_clicked();

	void on_m_edit_clicked();

	void on_m_remove_clicked();

	void on_m_tree_doubleClicked(const QModelIndex& index);

	void on_DialogUsers_accepted();

private:
	void showUserData(QTreeWidgetItem* item, const User& user);

private:
	Ui::DialogUsers* ui;

};

#endif // DIALOGUSERS_H
