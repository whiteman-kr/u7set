#include "SignalStatesProcessor.h"

SignalStatesProcessor::ThreadsContainer::ThreadsContainer()
{
}

SignalStatesProcessor::ThreadsContainer::~ThreadsContainer()
{
	for(auto& p : m_threads)
	{
		p.second.join();
	}
}

void SignalStatesProcessor::ThreadsContainer::append(std::thread& thread)
{
	Q_ASSERT(thread.joinable() == true);

	auto thread_id = std::hash<std::thread::id>{}(thread.get_id());

	if (m_threads.contains(thread_id))
	{
		Q_ASSERT(false);
		return;
	}

	auto p = m_threads.insert({thread_id, std::move(thread)});

	Q_ASSERT(p.first->second.joinable() == true);
	Q_ASSERT(thread.joinable() == false);
}


SignalStatesProcessor::SignalStatesProcessor()
{

}



//void RupFramesReceiver::startProcessingThreads(StdThreadsGuard& stg)
//{
//	int poolSize = m_processingThreadsCountFromSettings;

//	int idealThreadCount = QThread::idealThreadCount();

//	if (poolSize <= 0 || poolSize > idealThreadCount)
//	{
//		poolSize = idealThreadCount;
//	}

//	for(int i = 0; i < poolSize; i++)
//	{
//		std::thread t(&processPackets, std::ref(*this), i + 1);

//		stg.append(t);
//	}

//	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2").
//							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : ""));

//	std::thread t(&SignalStatesProcessingThread::processStates,
//				  &m_statesProcessingThread, std::ref(*this));

//	stg.append(t);
//}

//void RupFramesReceiver::wakeupAllProcessingThreads()
//{
//	std::lock_guard lg(m_packetProcessigRequiredMutex);
//	m_packetProcessingRequiredCondition.notify_all();
//	m_statesProcessingRequiredCondition.notify_all();
//}

//void processPackets(RupFramesReceiver& receiver, int threadNumber)
//{
//	CircularLoggerShared log = receiver.log();

//	DEBUG_LOG_MSG(log, QString("DiagDataProcessingThread #%1 is started").arg(threadNumber));

//	QThread* thisThread = QThread::currentThread();

//	auto& waitConditionMutex = receiver.m_packetProcessigRequiredMutex;
//	auto& waitCondition = receiver.m_packetProcessingRequiredCondition;
//	auto& requireProcessing = receiver.m_packetProcessingRequired;

//	std::unique_lock ul(waitConditionMutex, std::defer_lock);

//	while(true)
//	{
//		ul.lock();

//		waitCondition.wait(ul, [&receiver, &requireProcessing]() -> bool
//								{
//									return	receiver.isQuitRequested() ||
//											!requireProcessing.empty();
//								});

//		// here ul is LOCKED!

//		if (receiver.isQuitRequested() == true)
//		{
//			ul.unlock();
//			break;
//		}

//		auto it = requireProcessing.begin();

//		if (it == requireProcessing.end())
//		{
//			ul.unlock();
//			continue;
//		}

//		OnlineDataSource* source = it->first;;
//		bool requireBufferProcessing = it->second;

//		requireProcessing.erase(it);

//		ul.unlock();

//		if (source->takeProcessingOwnership(thisThread) == true)
//		{
//			if (requireBufferProcessing == true)
//			{
//				source->parseNextBuffer(thisThread);
//			}
//			else
//			{
//				source->invalidateAllSignals(thisThread);
//			}

//			source->releaseProcessingOwnership(thisThread);
//		}
//		else
//		{
//			// another thread already processing this source
//		}
//	}

//	DEBUG_LOG_MSG(log, QString("DiagDataProcessingThread #%1 finished").arg(threadNumber));
//}




