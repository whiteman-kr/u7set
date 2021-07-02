#include <QSettings>
#include "TuningUserManager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <lm.h>
#endif

#ifdef Q_OS_LINUX
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif

//
// DialogPassword
//

QString DialogTuningPassword::m_lastUser = "";

DialogTuningPassword::DialogTuningPassword(const TuningUserManager* userManager, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningUserManager(userManager)
{
	setWindowTitle(tr("Tuning Login"));

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

	for (const QString& user : m_tuningUserManager->tuningUserAccounts())
	{
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

DialogTuningPassword::~DialogTuningPassword()
{
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

	if (index < 0 || index >= m_tuningUserManager->tuningUserAccounts().size())
	{
		assert(false);
		return;
	}

	m_lastUser =  m_userCombo->currentText();
	m_password = m_passwordEdit->text();

	QDialog::accept();
}


//
// UserManager
//

TuningUserManager::TuningUserManager()
{
}

void TuningUserManager::setConfiguration(bool tuningLogin, const QStringList& tuningUserAccounts, bool loginPerOperation, int tuningSessionTimeout)
{
	m_tuningLogin = tuningLogin;
	m_tuningUserAccounts = tuningUserAccounts;
	m_loginPerOperation = loginPerOperation;
	m_tuningSessionTimeout = tuningSessionTimeout;

	m_loggedIn = false;
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

		if (requestPassword(parent) == false)
		{
			return false;
		}
	}

	// Refresh pending time

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

bool TuningUserManager::tuningLogin() const
{
	return m_tuningLogin;
}

const QStringList& TuningUserManager::tuningUserAccounts() const
{
	return m_tuningUserAccounts;
}

int TuningUserManager::tuningSessionTimeout() const
{
	return m_tuningSessionTimeout;
}

bool TuningUserManager::loginPerOperation() const
{
	return m_loginPerOperation;
}

bool TuningUserManager::isLoggedIn() const
{
	return m_loggedIn;
}

QString TuningUserManager::loggedInUser() const
{
	return m_loggedInUser;
}

int TuningUserManager::logoutPendingSeconds() const
{
	return static_cast<int>(m_logoutSecsSinceEpoch - QDateTime::currentSecsSinceEpoch());
}

bool TuningUserManager::requestPassword(QWidget* parent)
{
	if (m_tuningUserAccounts.empty() == true)
	{
		return true;
	}

	bool result = false;

	do
	{
		DialogTuningPassword d(this, parent);

		if (d.exec() != QDialog::Accepted)
		{
			return false;
		}

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

		QByteArray userNameData = d.userName().toLocal8Bit();
		char* userName = userNameData.data();

		conversePassword = d.password();

		pam_handle_t *pamh = nullptr;
		struct pam_conv pamc = {pamConverse, this};

		int res = pam_start("su", userName, &pamc, &pamh);

		if (res == PAM_SUCCESS)
		{
			res = pam_authenticate(pamh, 0);
		}

		if (res == PAM_SUCCESS)
		{
			res = pam_acct_mgmt(pamh, 0);
		}

		pam_end(pamh, res);

		result = res == PAM_SUCCESS ? true : false;

#endif

		if (result == false)
		{
			QMessageBox::critical(parent, qAppName(), QObject::tr("Wrong password!"));
		}
		else
		{
			m_loggedInUser = d.userName();
		}

	}while (result == false);

	return result;
}

#ifdef Q_OS_LINUX

int TuningUserManager::pamConverse(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
	TuningUserManager* ob = static_cast<TuningUserManager*>(data);

    QString strp = ob->conversePassword;
    QByteArray ba = strp.toLatin1();
    char *pcodec = ba.data();

    struct pam_response *aresp;
    char buf[PAM_MAX_RESP_SIZE];
    int i;

    aresp = new pam_response;

    if (n <= 0 || n > PAM_MAX_NUM_MSG)
        return (PAM_CONV_ERR);
    for (i = 0; i < n; ++i) {
        aresp[i].resp_retcode = 0;
        aresp[i].resp = NULL;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
            aresp[i].resp = strdup(pcodec);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_PROMPT_ECHO_ON:
            fputs(msg[i]->msg, stderr);
            if (fgets(buf, sizeof buf, stdin) == NULL)
                goto fail;
            aresp[i].resp = strdup(buf);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_ERROR_MSG:
            fputs(msg[i]->msg, stderr);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stderr);
            break;
        case PAM_TEXT_INFO:
            fputs(msg[i]->msg, stdout);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stdout);
            break;
        default:
            goto fail;
        }
    }
    *resp = aresp;
    return (PAM_SUCCESS);
 fail:
        for (i = 0; i < n; ++i) {
                if (aresp[i].resp != NULL) {
                        memset(aresp[i].resp, 0, strlen(aresp[i].resp));
                        free(aresp[i].resp);
                }
        }
        memset(aresp, 0, n * sizeof *aresp);
    *resp = NULL;
    return (PAM_CONV_ERR);
}

#endif
