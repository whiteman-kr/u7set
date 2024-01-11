#include "LogonWorkspace.h"

//
// LogonWorkspace
//

LogonWidget::LogonWidget(ClientLib::TuningUserManager& userManager, QWidget* parent):
	QWidget(parent),
	m_userManager(userManager)
{
	QHBoxLayout* l = new QHBoxLayout(this);
	l->setContentsMargins(0, 0, 10, 0);

	l->addStretch();

	m_loginButton = new QPushButton(loginString);
	connect(m_loginButton, &QPushButton::clicked, this, &LogonWidget::onButtonLogin);
	l->addWidget(m_loginButton);

	m_loginUserName = new QLabel(loggedOutString);
	m_loginUserName->setAlignment(Qt::AlignCenter);
	l->addWidget(m_loginUserName);

	// Adjust m_loginUserName width to have place for all usernames
	//
	int maxUsernameSpace = -1;

	QStringList userListStrings = m_userManager.tuningUserAccounts();
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

	connect(&m_userManager, &ClientLib::TuningUserManager::loggedIn, this, &LogonWidget::onUserManagerLogin);
	connect(&m_userManager, &ClientLib::TuningUserManager::loggedOut, this, &LogonWidget::onUserManagerLogout);
}

void LogonWidget::onButtonLogin()
{
	if (m_userManager.isLoggedIn() == true)
	{
		m_userManager.logout();
	}
	else
	{
		m_userManager.login(this);
	}
}

void LogonWidget::onUserManagerLogin()
{
	m_loginButton->setText(logoutString);

	m_loginUserName->setStyleSheet("QLabel {background-color:blue; color: white;}");
	m_loginUserName->setText(m_userManager.userName());
}

void LogonWidget::onUserManagerLogout()
{
	m_loginButton->setText(loginString);

	m_loginUserName->setStyleSheet(QString());
	m_loginUserName->setText(loggedOutString);

	m_logoutPendingTime->setText(zeroTimeString);
}

void LogonWidget::onTimer()
{
	if (m_userManager.isLoggedIn() == true)
	{
		if (m_userManager.tuningSessionTimeout() > 0)
		{
			int s = m_userManager.logoutPendingSeconds();

			QTime logoutTime(0, 0, 0);
			logoutTime = logoutTime.addSecs(s);
			m_logoutPendingTime->setText(logoutTime.toString("hh:mm:ss"));

			if (s <= 0)
			{
				m_userManager.logout();
			}
		}
	}
}
