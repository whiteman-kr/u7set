#include "TcpConnectionState.h"

namespace Tcp
{
	void ConnectionState::dump()
	{
		if (isConnected == false)
		{
			qDebug() << "\nTcp::ConnectionState - is not connected\n";
		}
		else
		{
			qDebug() << "\nTcp::ConnectionState - is connected";
			qDebug() << qPrintable(QString("Peer: %1").arg(peerAddr.addressPortStr()));
			qDebug() << qPrintable(QString("Start time: %1").arg(QDateTime::fromMSecsSinceEpoch(startTime).toString()));
			qDebug() << qPrintable(QString("Sent bytes: %1").arg(sentBytes));
			qDebug() << qPrintable(QString("Received bytes: %1").arg(receivedBytes));
			qDebug() << qPrintable(QString("Request count: %1").arg(requestCount));
			qDebug() << qPrintable(QString("Reply count: %1\n").arg(replyCount));
		}
	}
} // namespace Tcp