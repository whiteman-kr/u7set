#include "DialogPassword.h"
#include "ui_DialogPassword.h"

QString DialogPassword::m_lastUser = "";

DialogPassword::DialogPassword(const UserManager* userManager, bool adminNeeded, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogPassword),
	m_userManager(userManager)
{
	ui->setupUi(this);

	ui->m_passwordEdit->setEchoMode(QLineEdit::Password);

	int selectedIndex = -1;

	setWindowTitle(adminNeeded ? tr("Enter Administrator Password") : tr("Enter Password"));

	int i = 0;

	for (const User& user : m_userManager->users())
	{
		if (adminNeeded && user.m_admin == false)
		{
			continue;
		}

		ui->m_userCombo->addItem(user.m_name, i);

		if (user.m_name == m_lastUser)
		{
			selectedIndex = i;
		}

		i++;
	}

	if (selectedIndex != -1)
	{
		ui->m_userCombo->setCurrentIndex(selectedIndex);
	}
}

DialogPassword::~DialogPassword()
{
	delete ui;
}

QString DialogPassword::userName()
{
	return m_lastUser;
}

QString DialogPassword::password()
{
	return m_password;
}

void DialogPassword::accept()
{
	QVariant data = ui->m_userCombo->currentData();
	if (data.isValid() == false)
	{
		return;
	}

	int index = data.toInt();

	if (index < 0 || index >= m_userManager->users().size())
	{
		assert(false);
		return;
	}

	m_lastUser =  ui->m_userCombo->currentText();
	m_password = ui->m_passwordEdit->text();

	QDialog::accept();

}
