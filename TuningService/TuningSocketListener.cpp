#include "../lib/Types.h"
#include "TuningService.h"
#include "TuningSocketListener.h"

namespace Tuning
{

	// -------------------------------------------------------------------------
	//
	//	TuningSocketRequestQueue class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketRequestQueue::TuningSocketRequestQueue(quint32 tuningSourceIP) :
		Queue<TuningSocketRequest>(1000),
		m_tuningSourceIP(tuningSourceIP)
	{
	}


	bool TuningSocketRequestQueue::push(const TuningSocketRequest* ptr)
	{
		bool result = Queue<TuningSocketRequest>::push(ptr);

		if (result == true)
		{
			emit request(m_tuningSourceIP);
			return true;
		}

		return false;
	}


	void TuningSocketRequestQueue::stopWaiting()
	{
		m_waitingForAk = false;
		m_waitCount = 0;
	}


	void TuningSocketRequestQueue::requestIsSent()
	{
		assert(m_waitingForAk == false);

		m_waitingForAk = true;
		m_waitCount = 0;
	}


	void TuningSocketRequestQueue::incWaitCount()
	{
		assert(m_waitingForAk == true);

		m_waitCount++;
	}


	int TuningSocketRequestQueue::waitCount() const
	{
		return m_waitCount;
	}



}
