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

	void TuningUserManager::setConfiguration(bool enabled,
											 const QStringList& tuningUserAccounts,
											 bool loginPerOperation,
											 int tuningSessionTimeout,
											 const std::vector<OnlineLib::MatsUser>& matsUsers)
	{
		QMutexLocker l(&m_mutex);
		m_config.enabled = enabled;
		m_config.tuningUserAccounts = tuningUserAccounts;
		m_config.loginPerOperation = loginPerOperation;
		m_config.tuningSessionTimeout = tuningSessionTimeout;
		m_config.matsUsers = matsUsers;
	}

	bool TuningUserManager::enabled() const
	{
		QMutexLocker l(&m_mutex);
		return m_config.enabled == true && m_config.tuningUserAccounts.empty() == false;
	}

	const QStringList& TuningUserManager::tuningUserAccounts() const
	{
		QMutexLocker l(&m_mutex);
		return m_config.tuningUserAccounts;
	}

	bool TuningUserManager::loginPerOperation() const
	{
		QMutexLocker l(&m_mutex);
		return m_config.loginPerOperation;
	}

	int TuningUserManager::tuningSessionTimeout() const
	{
		QMutexLocker l(&m_mutex);
		return m_config.tuningSessionTimeout;
	}

	std::vector<OnlineLib::MatsUser> TuningUserManager::matsUsers() const
	{
		QMutexLocker l(&m_mutex);
		return m_config.matsUsers;
	}

	bool TuningUserManager::login(QWidget* parent)
	{
		if (enabled() == false)
		{
			return true;
		}

		bool wasLoggedIn = false;

		{
			QMutexLocker l(&m_mutex);
			wasLoggedIn = m_state.loggedIn;
		}

		if (wasLoggedIn == false)
		{
			// Ask the password
			//
			if (requestPassword(parent) == false)
			{
				return false;
			}

			emit loggedIn();
		}

		{
			QMutexLocker l(&m_mutex);
		
			// Refresh pending time
			//
			m_state.logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_config.tuningSessionTimeout;

			// Refresh tags
			//
			for (const OnlineLib::MatsUser& user : m_config.matsUsers)
			{
				if (user.login() == m_state.loggedInUser)
				{
					m_state.userTags.clear();
					for (const QString& t : user.appSignalTags())
					{
						m_state.userTags.push_back(t);
					}
					break;
				}
			}

			// Refresh status
			//
			if (m_config.loginPerOperation == false)
			{
				m_state.loggedIn = true;
			}
		}

		return true;
	}

	bool TuningUserManager::login(const QString& userName, const QString& password)
	{
		if (enabled() == false)
		{
			return true;
		}

		bool wasLoggedIn = false;

		{
			QMutexLocker l(&m_mutex);
			wasLoggedIn = m_state.loggedIn;
		}

		if (wasLoggedIn == false)
		{
			if (checkPassword(userName, password) == false)
			{
				return false;
			}

			emit loggedIn();
		}

		{
			QMutexLocker l(&m_mutex);

			m_state.loggedInUser = userName;
			m_state.loggedInPassword = password;

			// Refresh pending time
			//
			m_state.logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_config.tuningSessionTimeout;

			// Refresh tags
			//
			for (const OnlineLib::MatsUser& user : m_config.matsUsers)
			{
				if (user.login() == m_state.loggedInUser)
				{
					m_state.userTags.clear();
					for (const QString& t : user.appSignalTags())
					{
						m_state.userTags.push_back(t);
					}
					break;
				}
			}

			// Refresh status
			//
			if (m_config.loginPerOperation == false)
			{
				m_state.loggedIn = true;
			}
		}

		return true;
	}

	void TuningUserManager::logout()
	{
		{
			QMutexLocker l(&m_mutex);
			m_state.loggedIn = false;
			m_state.loggedInUser.clear();
			m_state.loggedInPassword.clear();
			m_state.userTags.clear();
		}

		emit loggedOut();
	}

	void TuningUserManager::reLogin(QWidget* parent)
	{
		bool wasLoggedIn = false;
		{
			QMutexLocker l(&m_mutex);
			wasLoggedIn = m_state.loggedIn;
		}

		if (wasLoggedIn == true)
		{
			if (parent == nullptr)
			{
				QMutexLocker l(&m_mutex);
				m_state.logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_config.tuningSessionTimeout;
			}
			else
			{
				if (requestPassword(parent) == true)
				{
					QMutexLocker l(&m_mutex);
					m_state.logoutSecsSinceEpoch = QDateTime::currentSecsSinceEpoch() + m_config.tuningSessionTimeout;
				}
			}
		}
	}

	bool TuningUserManager::isLoggedIn() const
	{
		if (enabled() == false)
		{
			return true;
		}

		QMutexLocker l(&m_mutex);
		return m_state.loggedIn;
	}

	QString TuningUserManager::userName() const
	{
		QMutexLocker l(&m_mutex);
		return m_state.loggedInUser;
	}

	QStringList TuningUserManager::userTags() const
	{
		QMutexLocker l(&m_mutex);
		return m_state.userTags;
	}

	QString TuningUserManager::password() const
	{
		QMutexLocker l(&m_mutex);
		return m_state.loggedInPassword;
	}

	int TuningUserManager::logoutPendingSeconds() const
	{
		QMutexLocker l(&m_mutex);
		return static_cast<int>(m_state.logoutSecsSinceEpoch - QDateTime::currentSecsSinceEpoch());
	}

	bool TuningUserManager::requestPassword(QWidget* parent)
	{
		if (enabled() == false)
		{
			return true;
		}

		bool result = false;

		{
			QMutexLocker l(&m_mutex);
			m_state.loggedInUser.clear();
			m_state.loggedInPassword.clear();
			m_state.userTags.clear();
		}

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
				QMutexLocker l(&m_mutex);
				m_state.loggedInUser = userName;
				m_state.loggedInPassword = password;
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
