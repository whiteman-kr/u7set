#pragma once

#include <ClientLib/TuningUserManager.h>

class LogonWidget : public QWidget
{
	Q_OBJECT
public:
	LogonWidget(ClientLib::TuningUserManager& userManager, QWidget* parent);

private slots:
	void onButtonLogin();
	void onButtonRelogin();

	void onUserManagerLogin();
	void onUserManagerLogout();

public slots:
	void onTimer();

private:
	QPushButton* m_loginButton = nullptr;
	QLabel* m_loginUserName = nullptr;
	QPushButton* m_logoutPendingTime = nullptr;

	ClientLib::TuningUserManager& m_userManager;

	const QLatin1String zeroTimeString = QLatin1String("00:00:00");
	const QString loginString = tr("Login");
	const QString logoutString = tr("Logout");
	const QString loggedOutString = tr("Logged Out");
};
