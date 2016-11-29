#pragma once

#include <QtCore>
#include <QUdpSocket>
#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataProtocols.h"
#include "../lib/Queue.h"
#include "../TuningService/TuningDataStorage.h"

namespace TuningIPEN
{

	class TuningIPENService;

	class TuningIPENSocketWorker : public SimpleThreadWorker
	{
		Q_OBJECT
	public:



	private:
		HostAddressPort m_tuningIP;
		TuningIPENService* m_tuningService = nullptr;

		QTimer m_timer;

		QUdpSocket* m_socket = nullptr;
		bool m_socketBound = false;

		RupFotipFrame m_ackFrame;
		RupFotipFrame m_reqFrame;

		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		virtual void onTimer();

		void createAndBindSocket();
		void closeSocket();
		void onSocketReadyRead();

		void clear();

		Queue<Tuning::SocketRequest> m_requests;
		Queue<Tuning::SocketReply> m_replies;

	private slots:
		void onSocketRequest();

	signals:
		void replyReady();

		void userRequest(FotipFrame fotipFrame);
		void replyWithNoZeroFlags(FotipFrame fotipFrame);

	public:
		TuningIPENSocketWorker(const HostAddressPort& tuningIP, TuningIPENService* tuningService);

		void sendRequest(const Tuning::SocketRequest& socketRequest);

		bool getReply(Tuning::SocketReply* reply);
	};

}
