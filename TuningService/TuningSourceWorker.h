#pragma once

#include "TuningSource.h"


namespace Tuning
{

	class TuningSourceWorker : public SimpleThreadWorker
	{
	public:
		TuningSourceWorker(const TuningSource& source);

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

	private slots:
		void onTimer();

	private:
		const TuningSource& m_source;

		bool m_waitReply = false;

		int m_waitReplyCounter = 0;

		const int MAX_WAIT_REPLY_COUNTER = 3;

	};

}
