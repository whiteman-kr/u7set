#pragma once

#include <QObject>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include "../lib/Tuning/ITuningAuthorization.h"
#include "../OnlineLib/MatsUsers.h"

#ifdef Q_OS_LINUX
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif


namespace ClientLib
{
	class TuningUserManager : public QObject, public ITuningAuthorization
	{
		Q_OBJECT

	public:
		TuningUserManager() = default;

	public:
		static bool checkPassword(const QString& userName, const QString& password);

		void setConfiguration(bool tuningLogin,
							  const QStringList& tuningUserAccounts,
							  bool loginPerOperation,
							  int tuningSessionTimeout,
							  const std::vector<OnlineLib::MatsUser>& matsUsers);

		// Properties
		//
		bool tuningLogin() const override;
		const QStringList& tuningUserAccounts() const;
		bool loginPerOperation() const;
		int tuningSessionTimeout() const;
		const std::vector<OnlineLib::MatsUser>& matsUsers() const;

		// Operations
		//
		bool login(QWidget* parent) override;
		bool login(const QString& userName, const QString& password);
		void logout();
		void reLogin(QWidget* parent);

		// State
		//
		bool isLoggedIn() const override;
		QString loggedInUser() const override;
		QString loggedInPassword() const;
		int logoutPendingSeconds() const;

	signals:
		void loggedIn();
		void loggedOut();

	private:
		bool requestPassword(QWidget* parent);

	private:
		bool m_tuningLogin = false;
		QStringList m_tuningUserAccounts;
		int m_tuningSessionTimeout = 120;
		bool m_loginPerOperation = false;
		std::vector<OnlineLib::MatsUser> m_matsUsers;

		bool m_loggedIn = false;
		QString m_loggedInUser;
		QString m_loggedInPassword;

		qint64 m_logoutSecsSinceEpoch = 0;

#ifdef Q_OS_LINUX
		QString conversePassword;
#endif
	};

	//
	// DialogTuningPassword
	//

	class DialogTuningPassword : public QDialog
	{
		Q_OBJECT
	public:
		explicit DialogTuningPassword(const ClientLib::TuningUserManager& userManager, QWidget* parent);
		~DialogTuningPassword() = default;

		[[nodiscard]] QString userName() const;
		[[nodiscard]] QString password() const;

	private:
		virtual void accept();

	private:
		const ClientLib::TuningUserManager& m_tuningUserManager;

		QString m_password;
		static inline QString m_lastUser;

		QComboBox* m_userCombo = nullptr;
		QLineEdit* m_passwordEdit = nullptr;
	};
}
