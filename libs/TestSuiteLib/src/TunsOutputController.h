#pragma once

#include <TestSuiteLib/IOutputController.h>

#include <ClientLib/ITuningLog.h>
#include <ClientLib/TuningConnection.h>
#include <ClientLib/TuningSignalManager.h>


namespace TestSuite
{

	// OutputControllerAuthorization is a ITuningAuthorization implementation which is always logged in and just returns user name
	//
	class OutputControllerAuthorization : public ITuningAuthorization
	{
	public:
		OutputControllerAuthorization() = default;

		explicit OutputControllerAuthorization(const QString& userName) :
			m_userName(userName)
		{
		}

	public:
		bool enabled() const override { return false; }

		bool login(QWidget* /*parent*/) override { return true; }
		bool isLoggedIn() const override { return true; }

		QString userName() const override 
		{ 
			QMutexLocker l{&m_mutex};
			return m_userName; 
		}
		
		void setUserName(const QString& value) 
		{
			QMutexLocker l{&m_mutex};
			m_userName = value; 
		}

		QStringList userTags() const override { return {}; }

	private:
		mutable QMutex m_mutex;
		QString m_userName;
	};

	class TunsOutputController : public IOutputController
	{
	public:
		explicit TunsOutputController(ILogFile* logFile);

		// IOutputController interface implementation
		//
	public:
		virtual bool init(qint64 timeoutMs) override;
		virtual bool shutdown() override;

		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) override;

		virtual bool waitForAllSignalsWritten(qint64 timeoutMs, qint64& timeElapsedMs) const override;

		bool tuningSourceIsActive(QString lmEquipmentId) const override;
		bool tuningSourceIsInactive(QString lmEquipmentId) const override;
		bool activateTuningSource(QString lmEquipmentId, bool activate) override;

		// End of IOutputController interface implementation

	public:
		void updateConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
							   const QByteArray& signalsFile,
							   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode);

		void setUserName(QString userName);

	private:
		SoftwareInfo m_softwareInfo;
		std::vector<SoftwareEndpoint::TuningService> m_tuningServices;
		TuningClientSettings::LmStatusFlagMode m_lmStatusFlagMode = TuningClientSettings::LmStatusFlagMode::None;

		ClientLib::TuningSignalManager m_signalManager;
		mutable HasLogFile m_appLog;

		OutputControllerAuthorization m_authorization;
		ClientLib::TuningLogStub m_tuningLogStub;
		ClientLib::TuningConnection m_connection;
	};
} // namespace TestSuite
