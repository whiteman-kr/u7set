#include "./include/ClientLib/RtDataProvider.h"
#include "RtDataProviderPrivate.h"

namespace ClientLib
{
	//
	//
	//		RtDataProvider
	//
	//
	RtDataProvider::RtDataProvider(const ISignalDataServer& signalDataServer, ILogFile* logFile) :
		QObject{nullptr},
		m_pimpl{std::make_unique<RtDataProviderPrivate>(signalDataServer, logFile)}
	{
		connect(m_pimpl.get(), &RtDataProviderPrivate::dataReady, this, &RtDataProvider::dataReady);
		connect(m_pimpl.get(), &RtDataProviderPrivate::requestError, this, &RtDataProvider::requestError);
		connect(m_pimpl.get(), &RtDataProviderPrivate::connectionLost, this, &RtDataProvider::connectionLost);
		return;
	}

	RtDataProvider::~RtDataProvider() = default;

	void RtDataProvider::clear()
	{
		return m_pimpl->clear();
	}

	void RtDataProvider::createConnections(const SoftwareInfo& softwareInfo,
										   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		return m_pimpl->createConnections(softwareInfo, appDataServices);
	}

	void RtDataProvider::updateConnections(const SoftwareInfo& softwareInfo,
										   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		return m_pimpl->updateConnections(softwareInfo, appDataServices);
	}

	bool RtDataProvider::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
	{
		return m_pimpl->setData(samplePeriod, trendSignals);
	}

	void RtDataProvider::setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod)
	{
		return m_pimpl->setSamplePeriod(samplePeriod);
	}

	size_t RtDataProvider::size() const
	{
		return m_pimpl->size();
	}

	RtTrendConnectionStatistics RtDataProvider::statistics() const
	{
		return m_pimpl->statistics();
	}

	bool RtDataProvider::allConnected(std::chrono::milliseconds timeout) const
	{
		return m_pimpl->allConnected(timeout);
	}
} // namespace ClientLib
