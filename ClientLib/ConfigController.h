#pragma once

#include <memory>
#include <QObject>
#include <QSharedMemory>
#include <QDomDocument>
#include "../CommonLib/HostAddressPort.h"
#include "../UtilsLib/ILogFile.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CfgServerLoader.h"


namespace Client
{
	struct ConfigurationInfo
	{
		int buildNo = -1;
		QString softwareEquipmentId;
		QString project;
	};


	class ConfigController : public QObject
	{
		Q_OBJECT

	public:
		ConfigController() = delete;
		ConfigController(const ConfigController&) = delete;
		ConfigController(ConfigController&&) = delete;
		ConfigController& operator=(const ConfigController&) = delete;
		ConfigController& operator=(ConfigController&&) = delete;

		explicit ConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address, ILogFile* logFile);
		explicit ConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile);
		virtual ~ConfigController();

	public:
		void setConnectionParams(QString equipmentId, HostAddressPort address1, HostAddressPort address2);

		bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
		bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);

		[[nodiscard]] bool hasFileId(QString fileId) const;

		[[nodiscard]] Tcp::ConnectionState getConnectionState() const;
		[[nodiscard]] const SoftwareInfo& softwareInfo() const;

		[[nodiscard]] ILogFile* logFile();
		[[nodiscard]] ILogFile* logFile() const;

	protected:
		/// This function is called when the new configuarion arrives, reimplement it for specific XML pasing.
		/// Clinet must override an appropriate to settings class function.
		///
		virtual bool updateConfiguration(const Client::ConfigurationInfo& conf, const MonitorSettings& settings);
		virtual bool updateConfiguration(const Client::ConfigurationInfo& conf, const TuningClientSettings& settings);
		virtual bool updateConfiguration(const Client::ConfigurationInfo& conf, const TestClientSettings& settings);

	private:
		int acquireAppInstanceNo(const QString& programName);
		void releaseAppInstanceNo();

		bool xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigurationInfo* out);
		bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigurationInfo* out);

	public slots:
		void start();

	private slots:
		void slot_configurationReady(const QByteArray configurationXmlData,
									 const BuildFileInfoArray buildFileInfoArray,
									 SessionParams sessionParams,
									 std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	signals:
		/// Not all errors are send via this signal, only important for user communication.
		/// For GUI application show MessageBox on this signal.
		///
		void error(QString errorMessage);

	protected:
		mutable HasLogFile m_logFile;
		SoftwareInfo m_softwareInfo;

	private:
		std::unique_ptr<CfgLoaderThread> m_cfgLoaderThread;

		QSharedMemory m_appInstanceSharedMemory;
		int m_appInstanceNo = -1;
		static const int MaxInstanceCount = 512;
	};
}

