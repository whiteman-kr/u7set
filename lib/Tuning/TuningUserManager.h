#pragma once

#include <QDateTime>

class TuningUserManager;

class DialogTuningPassword : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningPassword(const TuningUserManager* userManager, QWidget* parent);
	~DialogTuningPassword();

	QString userName() const;
	QString password() const;

private:
	virtual void accept();

private:
	QComboBox* m_userCombo = nullptr;

	QLineEdit* m_passwordEdit = nullptr;

	const TuningUserManager* m_tuningUserManager = nullptr;

	static QString m_lastUser;

	QString m_password;
};

class TuningUserManager : public QObject
{
	Q_OBJECT
public:
	TuningUserManager();

	// Operations

	void setConfiguration(bool tuningLogin, const QStringList& tuningUserAccounts, bool loginPerOperation, int tuningSessionTimeout);

	bool login(QWidget* parent);
	void logout();
	void reLogin(QWidget* parent);

	// Properties

	bool tuningLogin() const;
	const QStringList& tuningUserAccounts() const;
	int tuningSessionTimeout() const;
	bool loginPerOperation() const;

	// State

	bool isLoggedIn() const;
	QString loggedInUser() const;

	int logoutPendingSeconds() const;

signals:
	void loggedIn();
	void loggedOut();

private:
	bool requestPassword(QWidget* parent);

#ifdef Q_OS_LINUX
    static int pamConverse(int n, const struct pam_message **msg,
        struct pam_response **resp, void *data);
#endif

private:
	bool m_tuningLogin = false;
	QStringList m_tuningUserAccounts;
	int m_tuningSessionTimeout = 120;
	bool m_loginPerOperation = false;

	bool m_loggedIn = false;
	QString m_loggedInUser;

	qint64 m_logoutSecsSinceEpoch = 0;

#ifdef Q_OS_LINUX
    QString conversePassword;
#endif
};

