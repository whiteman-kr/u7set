#include "LogonWorkspace.h"

//
// LogonWorkspace
//

LogonWorkspace::LogonWorkspace(TuningUserManager* userManager, QWidget* parent):
	QWidget(parent),
	m_userManager(userManager)
{
	QHBoxLayout* l = new QHBoxLayout(this);

	l->addStretch();

	m_loginButton = new QPushButton(tr("Login"));
	connect(m_loginButton, &QPushButton::clicked, this, &LogonWorkspace::onButtonLogin);
	l->addWidget(m_loginButton);
	m_loginButton->setEnabled(userManager->isLoggedIn() == false);

	m_logoutButton = new QPushButton(tr("Logout"));
	connect(m_logoutButton, &QPushButton::clicked, this, &LogonWorkspace::onButtonLogout);
	l->addWidget(m_logoutButton);
	m_logoutButton->setEnabled(userManager->isLoggedIn() == true);

	m_loginUserName = new QLabel(loggedOutString);
	m_loginUserName->setAlignment(Qt::AlignCenter);
	l->addWidget(m_loginUserName);

	// Adjust m_loginUserName width to have place for all usernames
	//
	int maxUsernameSpace = -1;

	QStringList userListStrings = m_userManager->tuningUserAccounts();
	userListStrings.push_back(loggedOutString);

	for (const QString& userName : userListStrings)
	{
		int space = m_loginUserName->fontMetrics().horizontalAdvance(userName);
		if (space > maxUsernameSpace)
		{
			maxUsernameSpace = space;
		}
	}
	m_loginUserName->setFixedWidth(maxUsernameSpace + 5);

	m_logoutPendingTime = new QLabel(zeroTimeString);
	l->addWidget(m_logoutPendingTime);

	QMargins m = l->contentsMargins();
	m.setBottom(0);
	l->setContentsMargins(m);

	connect(m_userManager, &TuningUserManager::loggedIn, this, &LogonWorkspace::onUserManagerLogin);
	connect(m_userManager, &TuningUserManager::loggedOut, this, &LogonWorkspace::onUserManagerLogout);
}

void LogonWorkspace::onButtonLogin()
{
	if (m_userManager->isLoggedIn() == true)
	{
		return;
	}

	m_userManager->login(this);
}

void LogonWorkspace::onButtonLogout()
{
	m_userManager->logout();
}

void LogonWorkspace::onUserManagerLogin()
{
	m_loginButton->setEnabled(false);
	m_logoutButton->setEnabled(true);

	m_loginUserName->setText(m_userManager->loggedInUser());
}

void LogonWorkspace::onUserManagerLogout()
{
	m_loginButton->setEnabled(true);
	m_logoutButton->setEnabled(false);

	m_loginUserName->setText(loggedOutString);
	m_logoutPendingTime->setText(zeroTimeString);
}

void LogonWorkspace::onTimer()
{
	if (m_userManager->isLoggedIn() == true)
	{
		if (m_userManager->tuningSessionTimeout() > 0)
		{
			int s = m_userManager->logoutPendingSeconds();

			QTime logoutTime(0, 0, 0);
			logoutTime = logoutTime.addSecs(s);
			m_logoutPendingTime->setText(logoutTime.toString("hh:mm:ss"));

			if (s <= 0)
			{
				m_userManager->logout();
			}
		}
	}
}
