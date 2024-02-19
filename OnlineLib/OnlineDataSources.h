#pragma once

#include "../CommonLib/Hash.h"
#include "../UtilsLib/WUtils.h"
#include "CircularLogger.h"
#include "OnlineDataSource.h"

class BaseOnlineDataSources
{
public:
	BaseOnlineDataSources(int processingThreadsCount = -1, CircularLoggerShared log);
	virtual ~BaseOnlineDataSources();

	void clear();

	void startProcessingThreads();

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
	using ProcessingRequiredQueue = std::queue<std::pair<BaseOnlineDataSource*, bool>>;

	void processingRequired(BaseOnlineDataSource* source, bool parse);

	std::mutex& processingRequiredMutex() { return m_processigRequiredMutex; }
	std::condition_variable& processingRequiredCondition() { return m_processingRequiredCondition; }
	ProcessingRequiredQueue& rpcessingRequiredQueue() { return m_processingRequiredQueue; }

	static void processPackets(BaseOnlineDataSource& dataSources, int threadNumber);

private:
	int m_processingThreadsCount = 2;
	CircularLoggerShared m_log;

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

	std::mutex m_processigRequiredMutex;
	std::condition_variable m_processingRequiredCondition;

	//	{ source, true	}	source require buffer parsing
	//	{ source, false }	source require signals invalidation
	//
	ProcessingRequiredQueue m_processingRequiredQueue;

	std::vector<std::jthread> m_packetProcessingThreads;
};


// DATA_SOURCE - class derived from OnlineDataSource
template <typename DATA_SOURCE, typename SIGNAL_STATE>
class OnlineDataSources : public BaseOnlineDataSources
{
public:
	OnlineDataSources();
	bool init(const std::vector<DataSource>& dataSourcesFromCfg, CircularLoggerShared logger) {}
};

template <typename DATA_SOURCE, typename SIGNAL_STATE>
OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::OnlineDataSources()
{
}
/*
template <typename DATA_SOURCE, typename SIGNAL_STATE>
OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::init(const std::vector<DataSource>& dataSourcesFromCfg,
																CircularLoggerShared logger)
{
	bool result = true;

	for(const DataSource& ds : dataSourcesFromCfg)
	{
		DATA_SOURCE* dataSource = new DATA_SOURCE(ds);

		result &= append(dataSource, logger);
	}

	return result;
}*/

