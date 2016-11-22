#include "TuningSourceWorker.h"

namespace Tuning
{

	TuningSourceWorker::TuningSourceWorker(const TuningSource& source) :
		m_source(source)
	{
	}

	void TuningSourceWorker::onThreadStarted()
	{

	}


	void TuningSourceWorker::onThreadFinished()
	{

	}


	bool TuningSourceWorker::processWaitReply()
	{
		if (m_waitReply == true)
		{
			m_waitReplyCounter++;

			if (m_waitReplyCounter < MAX_WAIT_REPLY_COUNTER)
			{
				return true;
			}

			// fix replay timeout
			//
		}

		m_waitReply = false;

		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processCommandQueue()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		// get commands from queue and send FOTIP request
		//


		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processIdle()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		return false;
	}


	void TuningSourceWorker::onTimer()
	{
		do
		{
			if (processWaitReply() == true) break;
			if (processCommandQueue() == true) break;

			processIdle();

			break;
		}
		while(1);
	}


}
