#pragma once

#include <ServiceLib/Service.h>
#include "../OnlineLib/SoftwareSettings.h"
#include "TuningSource.h"
#include "TcpTuningServer.h"
#include "TuningSourceThread.h"
#include "TuningClientContext.h"

namespace Tuning
{
	class TcpTuningServerThread;

	class TuningServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	public:
		TuningServiceWorker(const SoftwareInfo& softwareInfo,
							const QString &serviceInstanceName,
							int argc,
							char** argv,
							CircularLoggerShared logger,
							CircularLoggerShared tuningLog);

		TuningServiceWorker(const TuningServiceWorker* worker);

		~TuningServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

		CircularLoggerShared tuningLog() const { return m_tuningLog; }

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

		TuningSourceThreadShared getTuningSourceThread(quint32 sourceIP);
		TuningSourceThreadShared getTuningSourceThread(const QString& sourceID);

		void getAllClientContexts(QVector<const TuningClientContext*>& clientContexts);

		bool singleLmControl() const;

		// called from TcpTuningServer thread!!!
		//
		E::NetworkError changeControlledTuningSource(const QString& tuningSourceEquipmentID,
													 bool activateControl,
													 QString* controlledTuningSource,
													 bool* controlIsActive);

		bool clientIsConnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool clientIsDisconnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool setActiveClient(const SoftwareInfo& softwareInfo, const QString& clientIP);

		QString activeClientID() const;
		QString activeClientIP() const;

		const TuningServiceSettings& tuningServiceSettings() const { return m_serviceSettings; }

		bool isControlled(const QString& lmEquipmentID, const QString& lanEquipmentID) const;

		void logTuningPacket(bool request, Fotip::OpCode opCode, quint16 rupNumerator, quint64 fotipNumerator);

		E::SecurityLevel securityLevel() const;

		void registerSignalsStateChangesQueue(const QString& clientEquipmentID, qint64 tcpConnectionID);
		void unregisterSignalsStateChangesQueue(const QString& clientEquipmentID, qint64 tcpConnectionID);

		void pushSignalStateChange(const TuningSignal::State& state);

		TuningSignalsChangesQueue* getSignalChangesQueue(const QString& clientEquipmentID, qint64 tcpConnectionID);

	private:
		virtual void initServiceSpecificCmdLineArgs() override;
		virtual void loadServiceSpecificSettings() override;

		void clear();

		virtual void initialize() override;
		virtual void shutdown() override;

		void clearConfiguration();
		void applyNewConfiguration(const TuningSources& newSources);

		void buildServiceMaps(const TuningSources& newSources);
		void clearServiceMaps();

		void fillControlledLans();

		bool readConfiguration(const QByteArray& cfgXmlData);
		bool loadConfigurationFromFile(const QString& fileName);
		bool readTuningSources(const QByteArray& fileData, const QString& profile, TuningSources* newSources);

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSourceThreads();
		bool runTuningSourceThread(bool runSingleSource,
								   const QString& tuningSourceEquipmentID);

		TuningSourceThreadShared createTuningSourceThread(const TuningSource& source);
		void stopTuningSourceThreads();

		void runSourcesListenerThreads();
		void stopSourcesListenerThreads();

		void setSourceThreadInTuningClientContexts(TuningSourceThreadShared thread);
		void removeSourceThreadFromTuningClientContexts(const QString& tuningSourceID);

		bool isSimulationMode() const;

		bool isSourceHandlerExistsForChannel(int channel) const;

	private slots:
		virtual void onConfigurationReady(const QByteArray configurationXmlData,
								  const BuildFileInfoArray buildFileInfoArray,
								  SessionParams sessionParams,
								  std::shared_ptr<const SoftwareSettings> curSettingsProfile) override;
	private:
		CircularLoggerShared m_tuningLog;
		CircularLoggerShared m_tuningPacketLog;

		TuningServiceSettings m_serviceSettings;

		std::map<QString, TuningSourceThreadShared> m_sourceThreads;	// module EquipmentID => TuningSourceThreadShared
		std::map<quint32, TuningSourceThreadShared> m_ip2sourceThread;	// TuningSource LANs ipV4 => TuningSourceThreadShared

		TuningSources m_tuningSources;

		std::set<std::pair<QString, QString>> m_controlledLans;			// pair: <LM EquipmentID, LAN EquipmentID>

		mutable QMutex m_startStopMutex;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		std::vector<TuningSocketListenerThread*> m_socketListenerThreads;

		TuningClientContextMap m_clientContextMap;

		mutable std::mutex m_activeClientInfoMutex;
		SoftwareInfo m_activeClientInfo;
		QString m_activeClientIP;
	};
}
