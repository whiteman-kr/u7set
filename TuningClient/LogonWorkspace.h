#pragma once

#include "../lib/Tuning/TuningUserManager.h"

class LogonWorkspace : public QWidget
{
	Q_OBJECT
public:
	LogonWorkspace(TuningUserManager* userManager, QWidget* parent);

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

	TuningUserManager* m_userManager = nullptr;

	const QLatin1String zeroTimeString = QLatin1String("00:00:00");
	const QString loggedOutString = tr("Logged Out");
};
