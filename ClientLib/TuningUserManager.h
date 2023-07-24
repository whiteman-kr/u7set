#pragma once

#include <QObject>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include "../lib/Tuning/ITuningAuthorization.h"

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

		// Operations
		//
		void setConfiguration(bool tuningLogin, const QStringList& tuningUserAccounts, bool loginPerOperation, int tuningSessionTimeout);

		bool login(QWidget* parent);
		void logout();
		void reLogin(QWidget* parent);

		// Properties
		//
		bool tuningLogin() const;
		const QStringList& tuningUserAccounts() const;
		bool loginPerOperation() const;
		int tuningSessionTimeout() const;

		// State
		//
		bool isLoggedIn() const;
		QString loggedInUser() const;

		int logoutPendingSeconds() const;

	signals:
		void loggedIn();
		void loggedOut();

	protected:
		virtual bool askForPassword(QString* userName, QString* password, QWidget* parent);
		virtual bool checkPassword(const QString& userName, const QString& password);

		// ITuningAuthorization imlementation
		//
		bool checkTuningAccess(QWidget* parent) override;

	private:
		bool requestPassword(QWidget* parent);

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
