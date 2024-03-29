#pragma once

#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../TrendView/TrendSignalState.h"
#include "../UtilsLib/ILogFile.h"
#include "../lib/ISignalDataServer.h"
#include "RtTrendConnectionStatistics.h"


namespace ClientLib
{
	class RtDataProviderPrivate;

	//
	// Real time trends data provdider - connects to all real time sources (app data service)
	//
	class RtDataProvider : public QObject
	{
		Q_OBJECT

	public:
		explicit RtDataProvider(const ISignalDataServer& signalDataServer, ILogFile* logFile);

		RtDataProvider(const RtDataProvider&) = delete;
		RtDataProvider(RtDataProvider&&) = delete;
	
		~RtDataProvider() override;

		RtDataProvider& operator=(const RtDataProvider&) = delete;
		RtDataProvider& operator=(RtDataProvider&&) = delete;

	public:
		void clear();
		void createConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);
		void updateConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);
		void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] RtTrendConnectionStatistics statistics() const;

		[[nodiscard]] bool allConnected(std::chrono::milliseconds timeout) const;

	signals:
		void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

	private:
		std::unique_ptr<RtDataProviderPrivate> m_pimpl;
	};
} // namespace ClientLib
