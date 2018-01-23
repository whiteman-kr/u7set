#include "UserManager.h"
#include <QSettings>
#include "DialogPassword.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <lm.h>
#endif

#ifdef Q_OS_LINUX
#include <security/pam_appl.h>
#include <security/pam_misc.h>
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
    //m_users << "bv";
    //m_users << "operator";
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

	m_loggedInUser = d.userName();

	return result;
}

#ifdef Q_OS_LINUX

int UserManager::pamConverse(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
    UserManager* ob = static_cast<UserManager*>(data);

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
