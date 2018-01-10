#include "UserManager.h"
#include <QSettings>
#include "DialogPassword.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <lm.h>
#endif

//
// LogonWorkspace
//

LogonWorkspace::LogonWorkspace(UserManager* userManager, QWidget* parent):
	QWidget(parent),
	m_userManager(userManager)
{
	QHBoxLayout* l = new QHBoxLayout(this);

	m_loginButton = new QPushButton(tr("Login"));
	connect(m_loginButton, &QPushButton::clicked, this, &LogonWorkspace::onButtonLogin);
	l->addWidget(m_loginButton);

	m_logoutButton = new QPushButton(tr("Logout"));
	connect(m_logoutButton, &QPushButton::clicked, this, &LogonWorkspace::onButtonLogout);
	l->addWidget(m_logoutButton);
	m_logoutButton->setEnabled(false);

	l->addWidget(new QLabel(tr("User:")));

	m_loginUserName = new QLabel(tr("-"));
	l->addWidget(m_loginUserName);

	l->addWidget(new QLabel(tr("Logout Pending Time:")));

	m_logoutPendingTime = new QLabel(tr("-"));
	l->addWidget(m_logoutPendingTime);

	l->addStretch();

	QMargins m = l->contentsMargins();
	m.setBottom(0);
	l->setContentsMargins(m);

	connect(m_userManager, &UserManager::loggedOn, this, &LogonWorkspace::onUserManagerLogin);
	connect(m_userManager, &UserManager::loggedOut, this, &LogonWorkspace::onUserManagerLogout);
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

	m_loginUserName->setText(tr("-"));
	m_logoutPendingTime->setText("-");
}

void LogonWorkspace::onTimer()
{
	if (m_userManager->isLoggedIn() == true)
	{
		int s = QDateTime::currentDateTime().secsTo(m_userManager->logoutPendingTime());

		QTime logoutTime(0, 0, 0);
		logoutTime = logoutTime.addSecs(s);

		m_logoutPendingTime->setText(logoutTime.toString("hh:mm:ss"));

		if (s <= 0)
		{
			m_userManager->logout();
		}
	}
}

//
// UserManager
//

UserManager::UserManager()
{
}

void UserManager::setConfiguration(const QStringList& users, LogonMode logonMode, int sessionMaxLengthSeconds)
{
	m_users = users;
	m_logonMode = logonMode;
	m_sessionMaxLengthSeconds = sessionMaxLengthSeconds;

	m_loggedIn = false;
}

bool UserManager::login(QWidget* parent)
{
	if (m_loggedIn == false)
	{
		// Ask the password

		if (requestPassword(parent) == false)
		{
			return false;
		}

		m_logonTime = QDateTime::currentDateTime();
	}

	// Refresh pending time

	m_logoutPendingTime = QDateTime::currentDateTime().addSecs(m_sessionMaxLengthSeconds);

	if (m_loggedIn == false)
	{
		emit loggedOn();
	}

	if (m_logonMode == LogonMode::Permanent)
	{
		m_loggedIn = true;
	}

	return true;
}

void UserManager::logout()
{
	m_loggedIn = false;

	emit loggedOut();
}

LogonMode UserManager::logonMode() const
{
	return m_logonMode;
}

QStringList UserManager::users() const
{
	return m_users;
}

bool UserManager::isLoggedIn() const
{
	return m_loggedIn;
}

QString UserManager::loggedInUser() const
{
	return m_loggedInUser;
}

QDateTime UserManager::loginTime() const
{
	return m_logonTime;
}

QDateTime UserManager::logoutPendingTime() const
{
	return m_logoutPendingTime;
}

bool UserManager::requestPassword(QWidget* parent)
{
	if (m_users.empty() == true)
	{
		return true;
	}

	DialogPassword d(this, parent);

	if (d.exec() != QDialog::Accepted)
	{
		return false;
	}

	bool result = false;

#ifdef Q_OS_WIN
	HANDLE phToken=NULL;

	if (LogonUser(reinterpret_cast<LPCWSTR>(d.userName().data()),
				  0,
				  reinterpret_cast<LPCWSTR>(d.password().data()),
				  LOGON32_LOGON_INTERACTIVE,
				  LOGON32_PROVIDER_DEFAULT,
				  &phToken) == TRUE)
	{
		result = true;
	}

	if (phToken != nullptr)
	{
		CloseHandle (phToken);
	}
#endif

#ifdef Q_OS_LINUX
	assert(false);
	result = true;
#endif

	if (result == false)
	{
		QMessageBox::critical(parent, qAppName(), QObject::tr("Wrong password!"));
	}

	m_loggedInUser = d.userName();

	return result;
}
