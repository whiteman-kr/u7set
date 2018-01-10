#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QDateTime>

enum class LogonMode
{
	Permanent = 0,
	PerOperation
};

class UserManager;

class LogonWorkspace : public QWidget
{
	Q_OBJECT
public:
	LogonWorkspace(UserManager* userManager, QWidget* parent);

private slots:
	void onButtonLogin();
	void onButtonLogout();

	void onUserManagerLogin();
	void onUserManagerLogout();

public slots:
	void onTimer();

private:
	QPushButton* m_loginButton = nullptr;
	QPushButton* m_logoutButton = nullptr;
	QLabel* m_loginUserName = nullptr;
	QLabel* m_logoutPendingTime = nullptr;

	UserManager* m_userManager = nullptr;
};

class UserManager : public QObject
{
	Q_OBJECT
public:
	UserManager();

	// Operations

	void setConfiguration(const QStringList& users, LogonMode logonMode, int sessionMaxLengthSeconds);

	bool login(QWidget* parent);
	void logout();

	// State

	LogonMode logonMode() const;

	QStringList users() const;

	bool isLoggedIn() const;

	QString loggedInUser() const;

	QDateTime loginTime() const;
	QDateTime logoutPendingTime() const;

signals:
	void loggedOn();
	void loggedOut();

private:
	bool requestPassword(QWidget* parent);

private:
	LogonMode m_logonMode = LogonMode::Permanent;
	int m_sessionMaxLengthSeconds = 120;

	QStringList m_users;

	bool m_loggedIn = false;
	QString m_loggedInUser;

	QDateTime m_logonTime;
	QDateTime m_logoutPendingTime;
};

#endif // USERMANAGER_H
