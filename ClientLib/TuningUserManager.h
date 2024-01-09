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

		void setConfiguration(bool enabled,
							  const QStringList& tuningUserAccounts,
							  bool loginPerOperation,
							  int tuningSessionTimeout,
							  const std::vector<OnlineLib::MatsUser>& matsUsers);

		// Properties
		//
		bool enabled() const override;
		const QStringList& tuningUserAccounts() const;
		bool loginPerOperation() const;
		int tuningSessionTimeout() const;
		std::vector<OnlineLib::MatsUser> matsUsers() const;

		// Operations
		//
		bool login(QWidget* parent) override;
		bool login(const QString& userName, const QString& password);
		void logout();
		void reLogin(QWidget* parent);

		// State
		//
		bool isLoggedIn() const override;
		
		QString userName() const override;
		QStringList userTags() const override;
		
		QString password() const;
		int logoutPendingSeconds() const;

	signals:
		void loggedIn();
		void loggedOut();

	private:
		bool requestPassword(QWidget* parent);

	public:
		struct Config
		{
			bool enabled = false;
			
			QStringList tuningUserAccounts;
			bool loginPerOperation = false;
			int tuningSessionTimeout = 120;
			
			std::vector<OnlineLib::MatsUser> matsUsers;
		};

		struct State
		{
			bool loggedIn = false;
			
			QString loggedInUser;
			QString loggedInPassword;
			
			QStringList userTags;
			qint64 logoutSecsSinceEpoch = 0;
		};

	private:
		mutable QMutex m_mutex;
		Config m_config;
		State m_state;
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
