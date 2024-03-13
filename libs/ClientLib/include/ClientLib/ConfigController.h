#pragma once

#include "../OnlineLib/BuildInfo.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"

#include <QSharedMemory>
#include <memory>

class QDomNode;
class CfgLoaderThread;
class HostAddressPort;


namespace ClientLib
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

		explicit ConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address, ILogFile* logFile);
		explicit ConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address1, const HostAddressPort& address2, ILogFile* logFile);
		virtual ~ConfigController();

	public:
		void setConnectionParams(QString equipmentId, const HostAddressPort& address1, const HostAddressPort& address2);

		bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
		bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);

		[[nodiscard]] bool hasFileId(QString fileId) const;

		[[nodiscard]] Tcp::ConnectionState getConnectionState() const;
		[[nodiscard]] const SoftwareInfo& softwareInfo() const;

		[[nodiscard]] ILogFile* logFile();
		[[nodiscard]] ILogFile* logFile() const;

	protected:
		/// This function is called when the new configuration arrives, reimplement it for specific XML parsing.
		/// Client must override an appropriate to settings class function.
		///
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const MonitorSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files);
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const DiagnosticsSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files);
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files);
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TestClientSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files);
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TestSuiteSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files);

	private:
		int acquireAppInstanceNo(const QString& programName);
		void releaseAppInstanceNo();

		bool xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigurationInfo* out);
		bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigurationInfo* out);

	public slots:
		void start();

	private slots:
		void slot_configurationReady(const QByteArray configurationXmlData,
									 const std::vector<OnlineLib::BuildFileInfo> buildFileInfoArray,
									 SessionParams sessionParams,
									 std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	signals:
		/// Not all errors are send via this signal, only important for user communication.
		/// For GUI application show MessageBox on this signal.
		///
		void error(QString errorMessage);

	public:
		int appInstanceNo() const;

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

