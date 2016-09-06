#pragma once

#include <QtCore>
#include <QUdpSocket>
#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataProtocols.h"
#include "../lib/Queue.h"

namespace Tuning
{
	enum OperationCode
	{
		Read = 1200,
		Write = 1400,
	};

	enum DataType
	{
		AnalogSignedInt = 1300,
		AnalogFloat = 1500,
		Discrete = 1700
	};

	struct SocketRequest
	{
		quint32 lmIP;
		int lmPort;
		int lmNumber;
		int lmSubsystemID;
		quint64 uniqueID;
		quint16 numerator;
		OperationCode operation;
		int startAddressW;
		int frameSizeW;
		DataType dataType;
		int romSizeW;
		bool userRequest;			// true - user request, false - atomatic periodic request

		char fotipData[FOTIP_TX_RX_DATA_SIZE];
	};


	struct SocketReply
	{
		quint32 lmIP;

		FotipHeader fotipHeader;

		char fotipData[FOTIP_TX_RX_DATA_SIZE];
		char fotipComparisonResult[FOTIP_COMPARISON_RESULT_SIZE];

		int frameNo;
	};


	struct RequestHeader
	{
		quint16 version;				// current version 1
		quint64 tuningID;				// unique connection ID generated by RPCT
		quint16 subsystemKey;			// key of Subsystem assigned in RPCT Subsystems List Editor
		quint16 operationCode;			// OperationCode enum values
		quint16 flags;
		quint32 startAddressW;			//
		quint16 requestSizeB;			// UDP frame size = 1432 bytes
		quint32 romSizeB;				// = ROM_SIZE_B
		quint16 romFrameSizeB;			// = ROM_FRAME_SIZE_B
		quint16 dataType;				// DataType enum values
	};


	class TuningService;


	class TuningSocketWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	private:
		HostAddressPort m_tuningIP;
		TuningService* m_tuningService = nullptr;

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

		Queue<SocketRequest> m_requests;
		Queue<SocketReply> m_replies;

	private slots:
		void onSocketRequest();

	signals:
		void replyReady();

		void userRequest(FotipFrame fotipFrame);
		void replyWithNoZeroFlags(FotipFrame fotipFrame);

	public:
		TuningSocketWorker(const HostAddressPort& tuningIP, TuningService* tuningService);

		void sendRequest(const SocketRequest& socketRequest);

		bool getReply(SocketReply* reply);
	};

}
