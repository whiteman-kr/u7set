#pragma once

#include "IOutputController.h"
#include <ClientLib/TuningSignalManager.h>
#include <ClientLib/ITuningLog.h>
#include <ClientLib/TuningConnection.h>


namespace TestSuite
{

	// OutputControllerAuthorization is a ITuningAuthorization implementation which is always logged in and just returns user name
	//
	class OutputControllerAuthorization : public ITuningAuthorization
	{
	public:
		OutputControllerAuthorization(const QString& userName) :
			m_userName(userName)
		{
		}

	private:
		bool enabled() const  override { return false; }
		
		bool login(QWidget* /*parent*/) override { return true; }
		bool isLoggedIn() const override { return true; }
		
		QString userName() const override { return m_userName; }
		QStringList userTags() const override { return {}; }

	private:
		QString m_userName;
	};

	class TunsOutputController : public IOutputController
	{
	public:
		TunsOutputController(const SoftwareInfo& softwareInfo,
							 const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
							 const QString& userName,
							 const QByteArray& signalsFile,
							 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
							 ILogFile* logFile);

	public:
		virtual bool waitForConnection(qint64 timeoutMs) const override;
		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) override;

		virtual bool waitForAllSignalsWritten(qint64 timeoutMs, qint64& timeElapsedMs) const override;

		bool tuningSourceIsActive(QString lmEquipmentId) const override;
		bool tuningSourceIsInactive(QString lmEquipmentId) const override;
		bool activateTuningSource(QString lmEquipmentId, bool activate) override;


	private:
		ClientLib::TuningSignalManager m_signalManager;
		mutable HasLogFile m_appLog;

		OutputControllerAuthorization m_authorization;
		ClientLib::TuningLogStub m_tuningLogStub;
		ClientLib::TuningConnection m_connection;
	};
}

