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
}

SignalStatesProcessor::SignalStatesProcessor()
{

}







