#pragma once

class SignalStatesProcessor
{
private:
	class ThreadsContainer
	{
	public:
		ThreadsContainer();
		~ThreadsContainer();

		void append(std::thread& thread);

	private:
		std::map<std::size_t, std::thread> m_threads;
	};

public:
	SignalStatesProcessor();
};

