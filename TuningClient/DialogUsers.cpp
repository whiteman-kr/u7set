#include "DialogUsers.h"
#include "ui_DialogUsers.h"
#include "Stable.h"
#include "Settings.h"

#include <QCryptographicHash>

//
// DialogUsers
//

DialogUsers::DialogUsers(const UserManager &userManager, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogUsers),
	m_userManager(userManager)
{
	ui->setupUi(this);

	QStringList headerLabels;
	headerLabels<<"Name";
	headerLabels<<"Description";
	headerLabels<<"Administrator";

	ui->m_tree->setColumnCount(headerLabels.size());
	ui->m_tree->setHeaderLabels(headerLabels);
	ui->m_tree->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

	for (const User& user : m_userManager.m_users)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		showUserData(item, user);
		item->setData(0, Qt::UserRole, QVariant::fromValue(user));

		ui->m_tree->addTopLevelItem(item);
	}

	for (int c = 0; c < ui->m_tree->columnCount(); c++)
	{
		ui->m_tree->resizeColumnToContents(c);
	}

	ui->m_add->setEnabled(theSettings.admin() == true);
	ui->m_edit->setEnabled(theSettings.admin() == true);
	ui->m_remove->setEnabled(theSettings.admin() == true);
}

DialogUsers::~DialogUsers()
{
	delete ui;
}

void DialogUsers::on_m_add_clicked()
{
	User user("NewUserID", "NewUserDescription", "", false);

	QTreeWidgetItem* item = new QTreeWidgetItem();
	showUserData(item, user);
	item->setData(0, Qt::UserRole, QVariant::fromValue(user));

	ui->m_tree->addTopLevelItem(item);
}

void DialogUsers::on_m_edit_clicked()
{
	QTreeWidgetItem* item = ui->m_tree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	User user = item->data(0, Qt::UserRole).value<User>();

	QString oldPassword = user.password();

	QString passwordPrompt = tr("<Enter password>");
	user.setPassword(passwordPrompt);

	std::shared_ptr<User> userPtr = std::make_shared<User>();
	*userPtr = user;

	DialogProperties d(userPtr, this);

	if (d.exec() == QDialog::Accepted)
	{
		user = *userPtr;

		if (user.password() == passwordPrompt)
		{
			user.setPassword(oldPassword);
		}
		else
		{
			QCryptographicHash md5Generator(QCryptographicHash::Md5);
			md5Generator.addData(user.password().toUtf8());
			user.setPassword(md5Generator.result().toHex());
		}

		item->setData(0, Qt::UserRole, QVariant::fromValue(user));
		showUserData(item, user);
	}

}

void DialogUsers::on_m_remove_clicked()
{
	QTreeWidgetItem* item = ui->m_tree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	QTreeWidgetItem* deleteItem = ui->m_tree->takeTopLevelItem(ui->m_tree->indexOfTopLevelItem(item));
	if (deleteItem == nullptr)
	{
		assert(false);
		return;
	}
	delete deleteItem;
}

void DialogUsers::on_m_tree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	if (theSettings.admin() == true)
	{
		on_m_edit_clicked();
	}
}

void DialogUsers::on_DialogUsers_accepted()
{
	if (theSettings.admin() == true)
	{
		m_userManager.m_users.clear();

		for(int i = 0; i < ui->m_tree->topLevelItemCount(); i++)
		{
			const QTreeWidgetItem* item = ui->m_tree->topLevelItem(i);
			m_userManager.m_users.push_back(item->data(0, Qt::UserRole).value<User>());
		}
	}
}

void DialogUsers::showUserData(QTreeWidgetItem *item, const User& user)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QStringList l;

	l << user.name();
	l << user.description();
	l << (user.admin() ? "Yes" : "");

	int i = 0;
	for (auto s: l)
	{
		item->setText(i++, s);
	}
}