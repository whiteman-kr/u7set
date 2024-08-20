#pragma once

#include <TrendView/TrendSignalState.h>

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../UtilsLib/ILogFile.h"
#include "ISignalDataServer.h"
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
		RtDataProvider(RtDataProvider&&) = default;

		RtDataProvider& operator=(const RtDataProvider&) = delete;
		RtDataProvider& operator=(RtDataProvider&&) = default;

		~RtDataProvider() override;

	public:
		void clear();
		void createConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);
		void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);

		[[nodiscard]] size_t size() const; // Get number of connections
		[[nodiscard]] RtTrendConnectionStatistics statistics() const;

		[[nodiscard]] bool allConnected(std::chrono::milliseconds timeout) const;

	signals:
		void dataReady(QString sourceEquipmentId,
					   std::shared_ptr<TrendLib::RealtimeData> data,
					   E::RtTrendsSamplePeriod samplePeriod,
					   TrendLib::TrendStateItem minState,
					   TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

	private:
		std::unique_ptr<RtDataProviderPrivate> m_pimpl;
	};
} // namespace ClientLib
