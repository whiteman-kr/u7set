#pragma once

#include <QObject>
#include <QTimer>
#include <QUdpSocket>


#pragma pack(push, 1)

const int PROTO_UDP_DATAGRAM_SIZE = 8192;


struct ProtoUdpFrameHeader
{
	quint32 clientID = 0;
	quint32 sessionID = 0;
	quint32 frameNumber = 0;
	quint32 totalFramesNumber = 0;
	quint32 frameDataSize = 0;
};


const int PROTO_UDP_FRAME_DATA_SIZE = PROTO_UDP_DATAGRAM_SIZE - sizeof(ProtoUdpFrameHeader);


union ProtoUdpFrame
{
	struct
	{
		ProtoUdpFrameHeader header;

		char data[PROTO_UDP_FRAME_DATA_SIZE];
	};

	char rawData[PROTO_UDP_DATAGRAM_SIZE];
};


#pragma pack(pop)


class ProtoUdpSocket : public QObject
{
/*private:
	static bool metaTypesRegistered;*/

protected:

	enum ProtoUdpState
	{
		Nothing,
		WaitingForAck,
	};

	QUdpSocket m_socket;
	QTimer m_timer;

	QByteArray m_requestData;
	QByteArray m_ackData;

	ProtoUdpFrame m_sentFrame;
	ProtoUdpFrame m_receivedFrame;

public:
	ProtoUdpSocket() {}
	~ProtoUdpSocket() {}
};


class ProtoUdpClient : public ProtoUdpSocket
{

};


class ProtoUdpServer : public ProtoUdpSocket
{

};
