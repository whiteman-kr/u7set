#pragma once

#include "../CommonLib/Hash.h"
#include "../UtilsLib/WUtils.h"
#include "CircularLogger.h"
#include "OnlineDataSource.h"
#include "RupFramesReceiver.h"

#include <thread>

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

	bool isWorkable() const;

	CircularLoggerShared log();

	void updateDataSourcesStatistics500ms(bool oneSecond);

	bool pushRupFrame(	quint32 sourceIP,
						qint64 serverTime,
						bool isSimFrame,
						Rup::Frame& rupFrame,
						const QThread* thread);
protected:
	bool append(BaseOnlineDataSource* onlineSource,
				CircularLoggerShared logger);

	BaseOnlineDataSource* getSourceByIP(quint32 ip);
	BaseOnlineDataSource* getSignalSource(const QString& signalID);
	BaseOnlineDataSource* getSignalSource(Hash signalHash);

	std::vector<BaseOnlineDataSource*>::iterator begin();
	std::vector<BaseOnlineDataSource*>::const_iterator begin() const;

	std::vector<BaseOnlineDataSource*>::iterator end();
	std::vector<BaseOnlineDataSource*>::const_iterator end() const;

	void startProcessingThreads();
	void startRupFramesReceiver();
	virtual void startStatesDistribution() = 0;

	using ProcessingRequiredQueue = std::queue<std::pair<BaseOnlineDataSource*, bool>>;

	void processingRequired(BaseOnlineDataSource* source, bool parse);

	std::mutex& processingRequiredMutex() { return m_processigRequiredMutex; }
	std::condition_variable_any& processingRequiredCondition() { return m_processingRequiredCondition; }
	ProcessingRequiredQueue& processingRequiredQueue() { return m_processingRequiredQueue; }

	void processPackets(int threadNumber);

protected:
	CircularLoggerShared m_log;
	bool m_isWorkable = false;

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
	std::list<std::jthread> m_processingThreads;
};


// DATA_SOURCE - class derived from OnlineDataSource
template <typename DATA_SOURCE, typename SIGNAL_STATE>
class OnlineDataSources : public BaseOnlineDataSources
{
public:
	OnlineDataSources(const std::vector<OnlineLib::DataSource>& dataSourcesFromCfg,
					  const HostAddressPort& dataReceivingIP,
					  E::SoftwareRunMode swRunMode,
					  int parsingThreadsCount,
					  CircularLoggerShared log);
public:
	int count() const;
	DATA_SOURCE* getDataSource(int index);

private:
	void startStatesDistribution() override;

	void statesDistribution();

	void registerSignalStatesQueue(	std::shared_ptr<FastThreadSafeQueue<SIGNAL_STATE>> queue);
	void unregisterSignalStatesQueue(std::shared_ptr<FastThreadSafeQueue<SIGNAL_STATE>> queue);

private:
	std::mutex m_distributionRequiredMutex;
	std::condition_variable_any m_distributionRequiredCondition;
	std::queue<OnlineDataSource<SIGNAL_STATE>*> m_distributionRequiredSources;	//	queue of sources requires states queue processing

	SimpleMutex m_statesQueuesMutex;
	std::set<std::shared_ptr<FastThreadSafeQueue<SIGNAL_STATE>>> m_stateQueues;		// pairs <queue, description>
};

template <typename DATA_SOURCE, typename SIGNAL_STATE>
OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::OnlineDataSources(const std::vector<OnlineLib::DataSource>& dataSourcesFromCfg,
																const HostAddressPort& dataReceivingIP,
																E::SoftwareRunMode swRunMode,
																int parsingThreadsCount,
																CircularLoggerShared log) :
	BaseOnlineDataSources(dataReceivingIP, swRunMode, parsingThreadsCount, log)
{
	bool res = true;

	int acquiredSignalsCount = 0;

	for(const OnlineLib::DataSource& ds : dataSourcesFromCfg)
	{
		DATA_SOURCE* dataSource = new DATA_SOURCE(ds);

		res &= append(dataSource, m_log);

		acquiredSignalsCount += dataSource->acquiredSignalsCount();
	}

	m_isWorkable = res;
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
int OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::count() const
{
	return TO_INT(m_sources.size());
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
DATA_SOURCE* OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::getDataSource(int index)
{
	Q_ASSERT(index >=0 && index < TO_INT(m_sources.size()));

	return dynamic_cast<DATA_SOURCE*>(m_sources[index]);
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
void OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::startStatesDistribution()
{
	m_processingThreads.emplace_back(&OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::statesDistribution, this);
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
void OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::statesDistribution()
{
	std::stop_token stopToken = m_stopSource.get_token();

	DEBUG_LOG_MSG(m_log, QString("Signal states distribution thread started"));

	QThread* thisThread = QThread::currentThread();

	std::unique_lock ul(m_distributionRequiredMutex, std::defer_lock);

	const int SIGNAL_STATE_BUFFER_SIZE = 100;

	SIGNAL_STATE signalStatesBuffer[SIGNAL_STATE_BUFFER_SIZE];
	OnlineDataSource<SIGNAL_STATE>* sourceToDistribute = nullptr;

	while(true)
	{
		ul.lock();

		if (sourceToDistribute != nullptr)
		{
			// if sourceToDistribute is not null here, it means that source queue after processing is not empty!
			// push this source at the end of queue for repeated processing
			//
			m_distributionRequiredSources.push(sourceToDistribute);
		}

		m_distributionRequiredCondition.wait(ul, stopToken, [&]() -> bool
								{
									return	!m_distributionRequiredSources.empty();
								});

		if (stopToken.stop_requested() == true)
		{
			ul.unlock();
			break;
		}

		if (m_distributionRequiredSources.empty())
		{
			Q_ASSERT(false);
			ul.unlock();
			continue;
		}

		sourceToDistribute = m_distributionRequiredSources.front();

		m_distributionRequiredSources.pop();

		ul.unlock();

		int processingLoopCount = 0;
		int statesCount = 0;

		while(processingLoopCount < 20)
		{
			statesCount = sourceToDistribute->popStates(signalStatesBuffer, SIGNAL_STATE_BUFFER_SIZE, thisThread);

			if (statesCount == 0)
			{
				break;
			}

			m_statesQueuesMutex.lock();

			for(auto& queue : m_stateQueues)
			{
				queue->pushFromBuffer(signalStatesBuffer, statesCount, thisThread);
			}

			m_statesQueuesMutex.unlock();

			processingLoopCount++;
		}

		if (statesCount == 0)
		{
			sourceToDistribute = nullptr;		// states queue of source is empty
		}
	}

	DEBUG_LOG_MSG(m_log, QString("Signal states distribution thread finished"));
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
void OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::registerSignalStatesQueue(std::shared_ptr<FastThreadSafeQueue<SIGNAL_STATE>> queue)
{
	m_statesQueuesMutex.lock();
	auto [it, b] = m_stateQueues.insert(queue);
	Q_ASSERT(b == true);
	m_statesQueuesMutex.unlock();
}

template <typename DATA_SOURCE, typename SIGNAL_STATE>
void OnlineDataSources<DATA_SOURCE, SIGNAL_STATE>::unregisterSignalStatesQueue(std::shared_ptr<FastThreadSafeQueue<SIGNAL_STATE>> queue)
{
	m_statesQueuesMutex.lock();
	int erasedCount = m_stateQueues.erase(queue);
	Q_ASSERT(erasedCount == 1);
	m_statesQueuesMutex.unlock();
}

