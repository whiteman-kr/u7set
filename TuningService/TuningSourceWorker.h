#pragma once

#include "TuningSource.h"


namespace Tuning
{

	class TuningSourceWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSourceWorker(const TuningSource& source);

		quint32 sourceIP() const;

		void pushReply(const RupFotipFrame& reply);


	signals:
		void replyReady();

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

		void processReply();

	private slots:
		void onTimer();
		void onReplyReady();

	private:
		const TuningSource& m_source;

		bool m_waitReply = false;

		int m_waitReplyCounter = 0;

		const int MAX_WAIT_REPLY_COUNTER = 3;

		QMutex m_waitReplyMutex;

		RupFotipFrame m_request;
		RupFotipFrame m_reply;

		// statisticts
		//
		qint64 m_requestCount = 0;
		qint64 m_replyCount = 0;

		qint64 m_errUntimelyReplay = 0;
	};

}
