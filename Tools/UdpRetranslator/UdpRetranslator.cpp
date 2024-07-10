#include "UdpRetranslator.h"
#include <WinSock2.h>
#include <ws2tcpip.h>			// IP_HDRINCL is here!!!
#include <iostream>
#include <QString>
#include <QTextStream>
#include <QHostAddress>
#include <QCoreApplication>

#define DEBUG_STOP { int a = 0; a++; }


std::string ws2s(const WCHAR* wString)
{
	std::wstring ws(wString);
	return std::string(ws.begin(), ws.end());
}

QString ws2QString(const WCHAR* wString)
{
	std::wstring ws(wString);
	std::string s(ws.begin(), ws.end());

	return QString::fromStdString(s);
}

/*
void out(const QString& str)
{
	stdOut << str;
}

void out_nl()
{
	out("\n");
}

void out_nl(const QString& str)
{
	out(str);
	out_nl();
} */

#pragma pack(push, 1)

struct IpHeader
{
	unsigned char  ip_verlen;        // 4-bit IPv4 version
									 // 4-bit header length (in 32-bit words)
	unsigned char  ip_tos;           // IP type of service
	unsigned short ip_totallength;   // Total length
	unsigned short ip_id;            // Unique identifier
	unsigned short ip_offset;        // Fragment offset field
	unsigned char  ip_ttl;           // Time to live
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	unsigned short ip_checksum;      // IP checksum
	unsigned int   ip_srcaddr;       // Source address
	unsigned int   ip_destaddr;      // Source address

	void toHost()
	{
		ip_totallength = ntohs(ip_totallength);
		ip_id = ntohs(ip_id);
		ip_offset = ntohs(ip_offset);
		ip_checksum = ntohs(ip_checksum);
		ip_srcaddr = ntohl(ip_srcaddr);
		ip_destaddr = ntohl(ip_destaddr);
	}

	void toNetwork()
	{
		ip_totallength = htons(ip_totallength);
		ip_id = htons(ip_id);
		ip_offset = htons(ip_offset);
		ip_checksum = htons(ip_checksum);
		ip_srcaddr = htonl(ip_srcaddr);
		ip_destaddr = htonl(ip_destaddr);
	}
};

struct UdpHeader
{
	quint16 srcPort;
	quint16 destPort;
	quint16 udpLength;
	quint16 checksum;

	void toHost()
	{
		srcPort = ntohs(srcPort);
		destPort = ntohs(destPort);
		udpLength = ntohs(udpLength);
		checksum = ntohs(checksum);
	}

	void toNetwork()
	{
		srcPort = htons(srcPort);
		destPort = htons(destPort);
		udpLength = htons(udpLength);
		checksum = htons(checksum);
	}
};


#pragma pack(pop)

void enumProtocols()
{
	WSAPROTOCOL_INFO wsaProtocols[100];

	DWORD l = sizeof(wsaProtocols);

	int protocolsCount = WSAEnumProtocols(NULL, wsaProtocols, &l);

	QString qs(reinterpret_cast<QChar*>(wsaProtocols[0].szProtocol));

	for(int i = 0; i < protocolsCount; i++)
	{
//		out(ws2QString(wsaProtocols[i].szProtocol));
//		out_nl();
	}
}

void threadFunc()
{
	QTextStream out(stdout);

	int err = 0;
	WSADATA wsaData;

	enumProtocols();

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err != 0)
	{
		out << QString("WSAStartup error: %1\n").arg(WSAGetLastError());
		return;
	}

	std::cout << QString("WSAStartup Ok\n").toStdString();

	SOCKET rawSocket;

	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

	if (rawSocket == INVALID_SOCKET)

	{
		// to avoid WSAEACCES (10013) error create registry DWORD variable and set it to 1
		//
		// HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\AFD\Parameters\DisableRawSecurity
		//
		// To open RAW socket program must be run under user of Administrators Group
		//

		out << QString("RAW socket open error: %1\n").arg(WSAGetLastError());
		return;
	}

	out << "RAW socket open Ok\n";

	DWORD on = 1;

	err = setsockopt(rawSocket, IPPROTO_IP, IP_HDRINCL, reinterpret_cast<const char*>(&on), sizeof(on));

	if (err == SOCKET_ERROR)
	{
		out << QString("Error setsockopt(): %1\n").arg(WSAGetLastError());
		return;
	}

	out << "RAW socket setsockopt IP_HDRINCL Ok\n";

	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));

	sa.sin_port = htons((unsigned short)0);
	sa.sin_family = AF_INET;

	sa.sin_addr.S_un.S_addr = htonl(0);

	err = bind(rawSocket, (sockaddr*)&sa, sizeof(sa));

	if (err == SOCKET_ERROR)
	{
		out << QString("Error bind(): %1\n").arg(WSAGetLastError());
		return;
	}

	char recvBuf[2048];
	sockaddr fromAddr;
	int fromAddrLen = sizeof(fromAddr);

	while(1)
	{
		int recvLen = recv(rawSocket, recvBuf, 2048, 0);

		if (recvLen == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			out << QString("Error recv(): %1\n").arg(WSAGetLastError());
			return;
		}

		IpHeader iph = *reinterpret_cast<const IpHeader*>(recvBuf);
		UdpHeader udph = *reinterpret_cast<const UdpHeader*>(recvBuf + sizeof(IpHeader));

		iph.toHost();
		//udph.toHost();

		QHostAddress srcAddr(iph.ip_srcaddr);
		QHostAddress destAddr(iph.ip_destaddr);

		QString s = QString("from %1:%2 to %3:%4, len %5\n").
					arg(srcAddr.toString()).arg(udph.srcPort).
					arg(destAddr.toString()).arg(udph.destPort).
					arg(iph.ip_totallength);

		std::cout << s.toStdString();
	}

	err = closesocket(rawSocket);

	if (err == SOCKET_ERROR)
	{
		out << QString("Error closesocket(): %1\n").arg(WSAGetLastError());
		return;
	}

	out << "RAW socket close Ok\n";

	WSACleanup();
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::thread t = std::thread(threadFunc);

	return app.exec();
}
