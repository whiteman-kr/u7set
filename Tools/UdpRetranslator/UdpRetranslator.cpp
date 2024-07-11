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

// RFC 791, Internet Protocol (IP)
//
// All fields in Network (BigEndian) byte order
//
struct IpHeader
{

	//
	union
	{
		struct
		{
			quint8  ipVersion : 4;			// 4-bit IPv4 version
			quint8	ipHeaderLenght : 4;		// 4-bit header length (in 32-bit words)
		};

		quint8 ipVerLen;
	};

	quint8 typeOfService;					// requested type of service (mostly - datagram priority)
	quint16	ipTotalLength;					// IP datagram total length in Bytes
	quint16 id;								// unique identifier of IP datagram
											// with the 3 flag bits and the fragment offset values are used in datagram fragmentation and reassembly
	quint16 fragmentOffset;					// fragment offset field
	quint8 timeToLive;						// TTL
	quint8 protocol;						// protocol ID (TCP - 6, UDP - 17, etc)
	quint16 headerChecksum;					// IP header checksum
	quint32 sourceIP;						// source IP address
	quint32 destinationIP;					// destination IP address

	// Minimal IP header size - 20 bytes (ipHeaderLenght == 5)

	// Possible up to 40 bytes of optional data (real IP header size determined by ipHeaderLenght field)

	void toHost()
	{
		ipTotalLength = ntohs(ipTotalLength);
		id = ntohs(id);
		fragmentOffset = ntohs(fragmentOffset);
		headerChecksum = ntohs(headerChecksum);
		sourceIP = ntohl(sourceIP);
		destinationIP = ntohl(destinationIP);
	}

	void toNetwork()
	{
		ipTotalLength = htons(ipTotalLength);
		id = htons(id);
		fragmentOffset = htons(fragmentOffset);
		headerChecksum = htons(headerChecksum);
		sourceIP = htonl(sourceIP);
		destinationIP = htonl(destinationIP);
	}
};

// RFC 768, User Datagram Protocol (UDP)
//
// All fields in Network (BigEndian) byte order
//
struct UdpHeader
{
	quint16 sourcePort;
	quint16 destinationPort;
	quint16 udpLength;				// length of the UDP header and UDP data in bytes
	quint16 checksum;

	void toHost()
	{
		sourcePort = ntohs(sourcePort);
		destinationPort = ntohs(destinationPort);
		udpLength = ntohs(udpLength);
		checksum = ntohs(checksum);
	}

	void toNetwork()
	{
		sourcePort = htons(sourcePort);
		destinationPort = htons(destinationPort);
		udpLength = htons(udpLength);
		checksum = htons(checksum);
	}
};

#pragma pack(pop)

bool operator < (const QHostAddress& a1, const QHostAddress& a2)
{
	return a1.toIPv4Address() < a2.toIPv4Address();
}

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

/* Compute checksum for count bytes starting at addr, using one's complement of one's complement sum*/
quint16 compute_checksum(quint8* rawHeaderPtr, int headerLenBytes)
{
	unsigned short* addr = reinterpret_cast<unsigned short*>(rawHeaderPtr);

	unsigned int count = headerLenBytes;

	unsigned long sum = 0;

	while (count > 1) {
		sum += * addr++;
		count -= 2;
	}
	//if any bytes left, pad the bytes and add
	if(count > 0) {
		sum += ((*addr)&htons(0xFF00));
	}
	//Fold sum to 16 bits: add carrier to result
	while (sum>>16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	//one's complement
	sum = ~sum;

	return ((quint16)sum);
}

/* set ip checksum of a given ip header*/
void compute_ip_checksum(IpHeader* iphdrp){
	iphdrp->headerChecksum = 0;
	iphdrp->headerChecksum = compute_checksum(reinterpret_cast<quint8*>(iphdrp), iphdrp->ipHeaderLenght << 2);
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

	SOCKET sendSocket;

	sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sendSocket == INVALID_SOCKET)

	{
		out << QString("SEND socket open error: %1\n").arg(WSAGetLastError());
		return;
	}

	out << "SEND socket open Ok\n";

	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));

	sa.sin_port = htons((unsigned short)0);
	sa.sin_family = AF_INET;

	sa.sin_addr.S_un.S_addr = htonl(QHostAddress("192.168.14.85").toIPv4Address());

	err = bind(sendSocket, (sockaddr*)&sa, sizeof(sa));

	if (err == SOCKET_ERROR)
	{
		out << QString("Error bind() SEND socket: %1\n").arg(WSAGetLastError());
		return;
	}

	out << "SEND socket bind Ok\n";

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

	ZeroMemory(&sa, sizeof(sa));

	sa.sin_port = htons((unsigned short)0);
	sa.sin_family = AF_INET;

	//sa.sin_addr.S_un.S_addr = htonl(QHostAddress("192.168.101.100").toIPv4Address());
	sa.sin_addr.S_un.S_addr = 0;

	err = bind(rawSocket, (sockaddr*)&sa, sizeof(sa));

	if (err == SOCKET_ERROR)
	{
		out << QString("Error bind(): %1\n").arg(WSAGetLastError());
		return;
	}

	char recvBuf[2048];

	std::map<QHostAddress, QHostAddress> fromTo =
	{
		{ QHostAddress("192.168.14.138"), QHostAddress("192.168.14.85") },
		{ QHostAddress("1192.168.14.224"), QHostAddress("192.168.14.85") },
		{ QHostAddress("192.168.14.193"), QHostAddress("192.168.14.85") },
		{ QHostAddress("192.168.14.222"), QHostAddress("192.168.14.85") },
		{ QHostAddress("192.168.14.197"), QHostAddress("192.168.14.85") },
	};

	sockaddr_in toAddr;
	ZeroMemory(&toAddr, sizeof(toAddr));

	toAddr.sin_family = AF_INET;

	while(1)
	{
		int recvLen = recv(rawSocket, recvBuf, 2048, 0);

		if (recvLen == SOCKET_ERROR)
		{
			out << QString("Error recv(): %1\n").arg(WSAGetLastError());
			return;
		}

		IpHeader iph = *reinterpret_cast<const IpHeader*>(recvBuf);
		UdpHeader udph = *reinterpret_cast<const UdpHeader*>(recvBuf + sizeof(IpHeader));

		iph.toHost();
		udph.toHost();

		QHostAddress srcAddr(iph.sourceIP);
		QHostAddress destAddr(iph.destinationIP);

		auto it = fromTo.find(srcAddr);

		QString s = QString("from %1:%2 to %3:%4, len %5 (recvLen %6)").
					arg(srcAddr.toString()).arg(udph.sourcePort).
					arg(destAddr.toString()).arg(udph.destinationPort).
					arg(iph.ipTotalLength).arg(recvLen);

		if (it == fromTo.end())
		{
			s += " => ignored\n";
		}
		else
		{
			QHostAddress newDestIP = it->second;

/*			IpHeader* correctedIph = reinterpret_cast<IpHeader*>(recvBuf);

			correctedIph->destinationIP = htonl(newDestIP.toIPv4Address());

			compute_ip_checksum(correctedIph);

			Q_ASSERT(compute_checksum(reinterpret_cast<quint8*>(correctedIph), correctedIph->ipHeaderLenght << 2) == 0);*/

			toAddr.sin_port = htons(13222);
			toAddr.sin_addr.S_un.S_addr = htonl(newDestIP.toIPv4Address());

			int offset = iph.ipHeaderLenght * 4 + sizeof(UdpHeader);

			int sendLen = sendto(sendSocket, recvBuf + offset, recvLen - offset, 0, reinterpret_cast<sockaddr*>(&toAddr), sizeof(toAddr));

			if (sendLen == SOCKET_ERROR)
			{
				s = QString("Error send(): %1\n").arg(WSAGetLastError());
			}
			else
			{
				//Q_ASSERT(sendLen == recvLen);
				s += QString(" => %1\n").arg(newDestIP.toString());
			}
		}

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
