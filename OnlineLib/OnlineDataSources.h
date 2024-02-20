#pragma once

#include "../CommonLib/Hash.h"
#include "../UtilsLib/WUtils.h"
#include "CircularLogger.h"
#include "OnlineDataSource.h"
#include "RupFramesReceiver.h"

class BaseOnlineDataSources
{
public:
	BaseOnlineDataSources(const HostAddressPort& dataReceivingIP,
						  E::SoftwareRunMode swRunMode,
						  int parsingThreadsCount,
						  CircularLoggerShared log);
	virtual ~BaseOnlineDataSources();

	void clear();

	void run();
	void stop();

	CircularLoggerShared log();

	bool append(BaseOnlineDataSource* onlineSource,
				CircularLoggerShared logger);

	bool pushRupFrame(	quint32 sourceIP,
						qint64 serverTime,
						bool isSimFrame,
						Rup::Frame& rupFrame,
						const QThread* thread);

	void updateDataSourcesStatistics500ms(bool oneSecond);

	BaseOnlineDataSource* getSourceByIP(quint32 ip);
	BaseOnlineDataSource* getSignalSource(const QString& signalID);
	BaseOnlineDataSource* getSignalSource(Hash signalHash);

	std::vector<BaseOnlineDataSource*>::iterator begin();
	std::vector<BaseOnlineDataSource*>::const_iterator begin() const;

	std::vector<BaseOnlineDataSource*>::iterator end();
	std::vector<BaseOnlineDataSource*>::const_iterator end() const;

private:
	void startProcessingThreads();
	void startRupFramesReceiver();

	using ProcessingRequiredQueue = std::queue<std::pair<BaseOnlineDataSource*, bool>>;

	void processingRequired(BaseOnlineDataSource* source, bool parse);

	std::mutex& processingRequiredMutex() { return m_processigRequiredMutex; }
	std::condition_variable_any& processingRequiredCondition() { return m_processingRequiredCondition; }
	ProcessingRequiredQueue& processingRequiredQueue() { return m_processingRequiredQueue; }

	void processPackets(int threadNumber);

protected:
	CircularLoggerShared m_log;

private:
	int m_parsingThreadsCount = 2;

	// owns BaseOnlineDataSource objects
	//
	std::vector<BaseOnlineDataSource*> m_sources;

	// module EquipmentID => BaseOnlineDataSource*
	//
	std::map<QString, BaseOnlineDataSource*> m_moduleToSource;

	// lan controller EquipmentID => BaseOnlineDataSource*
	//
	std::map<QString, BaseOnlineDataSource*> m_lanControllerToSource;

	// module ethernet adapter IP => BaseOnlineDataSource*
	//
	std::map<quint32, BaseOnlineDataSource*> m_ipToSource;

	// signal Hash => BaseOnlineDataSource*
	//
	std::map<Hash, BaseOnlineDataSource*> m_signalToSource;

	//

	RupFramesReceiver m_rupFramesReceiver;

	std::mutex m_processigRequiredMutex;
	std::condition_variable_any m_processingRequiredCondition;

	//	{ source, true	}	source require buffer parsing
	//	{ source, false }	source require signals invalidation
	//
	ProcessingRequiredQueue m_processingRequiredQueue;

	std::stop_source m_stopSource;
	std::vector<std::jthread> m_processingThreads;
};


// DATA_SOURCE - class derived from OnlineDataSource
template <typename DATA_SOURCE, typename SIGNAL_STATE>
class OnlineDataSources : public BaseOnlineDataSources
{
public:
	OnlineDataSources(const HostAddressPort& dataReceivingIP,
					  E::SoftwareRunMode swRunMode,
					  int parsingThreadsCount,
					  CircularLoggerShared log);
	bool init(const std::vector<DataSource>& dataSourcesFromCfg);
};

template <typename DATA_SOURCE, typename SIGNAL_STATE>
OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::OnlineDataSources(const HostAddressPort& dataReceivingIP,
																E::SoftwareRunMode swRunMode,
																int parsingThreadsCount,
																CircularLoggerShared log) :
	BaseOnlineDataSources(dataReceivingIP, swRunMode, parsingThreadsCount, log)
{
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
bool OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::init(const std::vector<DataSource>& dataSourcesFromCfg)
{
	bool result = true;

	for(const DataSource& ds : dataSourcesFromCfg)
	{
		DATA_SOURCE* dataSource = new DATA_SOURCE(ds);

		result &= append(dataSource, m_log);
	}

	return result;
}

