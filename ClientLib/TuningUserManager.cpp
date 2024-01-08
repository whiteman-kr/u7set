#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TuningUserManager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <lm.h>
#endif

namespace ClientLib
{
	//
	// TuningUserManager
	//

	bool TuningUserManager::checkPassword(const QString& userName, const QString& password)
	{
		bool result = false;

#ifdef Q_OS_WIN
			HANDLE phToken=NULL;

			if (LogonUser(reinterpret_cast<LPCWSTR>(userName.data()),
						  0,
						  reinterpret_cast<LPCWSTR>(password.data()),
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
			QString command = QString("echo %1 | /bin/su - %2 >/dev/null 2>/dev/null").arg(password).arg(userName);
			result = system(command.toLocal8Bit()) == 0;
#endif

		return result;
	}

	void TuningUserManager::setConfiguration(bool tuningLogin,
											 const QStringList& tuningUserAccounts,
											 bool loginPerOperation,
											 int tuningSessionTimeout,
											 const std::vector<OnlineLib::MatsUser>& matsUsers)
	{
		m_tuningLogin = tuningLogin;
		m_tuningUserAccounts = tuningUserAccounts;
		m_loginPerOperation = loginPerOperation;
		m_tuningSessionTimeout = tuningSessionTimeout;
		m_matsUsers = matsUsers;

		m_loggedIn = false;
	}

	bool TuningUserManager::tuningLogin() const
	{
		return m_tuningLogin;
	}

	const QStringList& TuningUserManager::tuningUserAccounts() const
	{
		return m_tuningUserAccounts;
	}

	bool TuningUserManager::loginPerOperation() const
	{
		return m_loginPerOperation;
	}

	int TuningUserManager::tuningSessionTimeout() const
	{
		return m_tuningSessionTimeout;
	}

	const std::vector<OnlineLib::MatsUser>& TuningUserManager::matsUsers() const
	{
		return m_matsUsers;
	}

	bool TuningUserManager::login(QWidget* parent)
	{
		if (m_tuningLogin == false)
		{
			return true;
		}

		if (m_loggedIn == false)
		{
			// Ask the password
			//
			if (requestPassword(parent) == false)
			{
				return false;
			}
		}

		// Refresh pending time
		//
		m_logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_tuningSessionTimeout;

		if (m_loggedIn == false)
		{
			emit loggedIn();
		}

		if (m_loginPerOperation == false)
		{
			m_loggedIn = true;
		}

		return true;
	}

	bool TuningUserManager::login(const QString& userName, const QString& password)
	{
		if (m_tuningLogin == false)
		{
			return true;
		}

		if (m_tuningUserAccounts.empty() == true)
		{
			return true;
		}

		if (m_loggedIn == false)
		{
			if (checkPassword(userName, password) == false)
			{
				return false;
			}
		}

		// Refresh pending time
		//
		m_logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_tuningSessionTimeout;

		if (m_loggedIn == false)
		{
			emit loggedIn();
		}

		if (m_loginPerOperation == false)
		{
			m_loggedIn = true;
		}

		return true;
	}

	void TuningUserManager::logout()
	{
		m_loggedInUser.clear();
		m_loggedInPassword.clear();

		m_loggedIn = false;

		emit loggedOut();
	}

	void TuningUserManager::reLogin(QWidget* parent)
	{
		if (m_loggedIn == true)
		{
			if (requestPassword(parent) == true)
			{
				m_logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_tuningSessionTimeout;
			}
		}
	}

	bool TuningUserManager::isLoggedIn() const
	{
		if (m_tuningLogin == false || m_tuningUserAccounts.empty() == true)
		{
			return true;
		}

		return m_loggedIn;
	}

	QString TuningUserManager::loggedInUser() const
	{
		return m_loggedInUser;
	}

	QString TuningUserManager::loggedInPassword() const
	{
		return m_loggedInPassword;
	}

	int TuningUserManager::logoutPendingSeconds() const
	{
		return static_cast<int>(m_logoutSecsSinceEpoch - QDateTime::currentSecsSinceEpoch());
	}

	bool TuningUserManager::requestPassword(QWidget* parent)
	{
		if (m_tuningLogin == false || m_tuningUserAccounts.empty() == true)
		{
			return true;
		}

		bool result = false;

		m_loggedInUser.clear();
		m_loggedInPassword.clear();

		for (int i = 0; i < 3; i++)
		{
			ClientLib::DialogTuningPassword d(*this, parent);
			if (d.exec() != QDialog::Accepted)
			{
				break;
			}

			QString userName = d.userName();
			QString password = d.password();

			result = checkPassword(userName, password);

			if (result == false)
			{
				if (parent == nullptr)
                {
					break;
                }

				if (i < 2)
				{
					QMessageBox::critical(parent, qAppName(), tr("Wrong password! Please try again."));
				}
			}
			else
			{
				m_loggedInUser = userName;
				m_loggedInPassword = password;
				break;
			}
		}

		return result;
	}

	//
	// DialogTuningPassword
	//
	DialogTuningPassword::DialogTuningPassword(const ClientLib::TuningUserManager& userManager, QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		m_tuningUserManager(userManager)
	{
		setWindowTitle(tr("User Authorization"));

		setMinimumSize(400, 150);

		// Setup UI

		m_userCombo = new QComboBox();

		m_passwordEdit = new QLineEdit();
		m_passwordEdit->setEchoMode(QLineEdit::Password);

		QVBoxLayout* mainLayout = new QVBoxLayout();
		mainLayout->addWidget(new QLabel(tr("Login:")));
		mainLayout->addWidget(m_userCombo);
		mainLayout->addStretch();
		mainLayout->addWidget(new QLabel(tr("Password:")));
		mainLayout->addWidget(m_passwordEdit);
		mainLayout->addStretch();

		QHBoxLayout* buttonsLayout = new QHBoxLayout();
		buttonsLayout->addStretch();

		QPushButton* b = new QPushButton(tr("OK"));
		connect(b, &QPushButton::clicked, this, &DialogTuningPassword::accept);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Cancel"));
		connect(b, &QPushButton::clicked, this, &DialogTuningPassword::reject);
		buttonsLayout->addWidget(b);

		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);

		m_passwordEdit->setFocus();

		// Fill user list

		int selectedIndex = -1;

		int i = 0;

		for (const QString& user : m_tuningUserManager.tuningUserAccounts())
		{

			bool userIsEnabled = true;
			for (const auto& matsUser : m_tuningUserManager.matsUsers())
			{
				if (matsUser.login() == user)
				{
					userIsEnabled = matsUser.enabled();
					break;
				}
			}
			if (userIsEnabled == false)
			{
				continue;
			}

			m_userCombo->addItem(user, i);

			if (user == m_lastUser)
			{
				selectedIndex = i;
			}

			i++;
		}

		if (selectedIndex != -1)
		{
			m_userCombo->setCurrentIndex(selectedIndex);
		}
	}

	QString DialogTuningPassword::userName() const
	{
		return m_lastUser;
	}

	QString DialogTuningPassword::password() const
	{
		return m_password;
	}

	void DialogTuningPassword::accept()
	{
		QVariant data = m_userCombo->currentData();
		if (data.isValid() == false)
		{
			return;
		}

		int index = data.toInt();

		if (index < 0 || index >= m_tuningUserManager.tuningUserAccounts().size())
		{
			assert(false);
			return;
		}

		m_lastUser =  m_userCombo->currentText();
		m_password = m_passwordEdit->text();

		QDialog::accept();
	}
}
