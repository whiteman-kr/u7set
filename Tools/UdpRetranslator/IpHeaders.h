#pragma once

#pragma pack(push, 1)

struct MacAddr
{
	quint8 byte1;
	quint8 byte2;
	quint8 byte3;
	quint8 byte4;
	quint8 byte5;
	quint8 byte6;
};

struct EthernetHeader
{
	MacAddr destMacAddr;
	MacAddr srcMacAddr;
	quint16 lenght;
};

// 4 bytes IP address
//
union IPv4Addr
{
	struct
	{
		quint8 byte1;
		quint8 byte2;
		quint8 byte3;
		quint8 byte4;
	};

	quint32 ip;
};

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
			quint8	ipHeaderLenght : 4;		// 4-bit header length (in 32-bit words)
			quint8  ipVersion : 4;			// 4-bit IPv4 version
		};

		quint8 ipVerLen;
	};

	quint8 typeOfService;					// requested type of service (mostly - datagram priority)
	quint16	ipTotalLength;					// IP datagram total length in Bytes
	quint16 id;								// unique identifier of IP datagram
											// with the 3 flag bits and the fragment offset values
											// are used in datagram fragmentation and reassembly
	quint16 fragmentOffset;					// fragment offset field
	quint8 timeToLive;						// TTL
	quint8 protocol;						// protocol ID (TCP - 6, UDP - 17, etc)
	quint16 headerChecksum;					// IP header checksum
	IPv4Addr srcIP;						// source IP address
	IPv4Addr desIP;					// destination IP address

	// Minimal IP header size - 20 bytes (ipHeaderLenght == 5)

	// Possible up to 40 bytes of optional data (real IP header size determined by ipHeaderLenght field)

	quint32 headerLenBytes() const { return ipHeaderLenght * 4; }

	void toHost()
	{
		ipTotalLength = ntohs(ipTotalLength);
		id = ntohs(id);
		fragmentOffset = ntohs(fragmentOffset);
		headerChecksum = ntohs(headerChecksum);
		srcIP.ip = ntohl(srcIP.ip);
		desIP.ip = ntohl(desIP.ip);
	}

	void toNetwork()
	{
		ipTotalLength = htons(ipTotalLength);
		id = htons(id);
		fragmentOffset = htons(fragmentOffset);
		headerChecksum = htons(headerChecksum);
		srcIP.ip = htonl(srcIP.ip);
		desIP.ip = htonl(desIP.ip);
	}
};

// RFC 768, User Datagram Protocol (UDP)
//
// All fields in Network (BigEndian) byte order
//
struct UdpHeader
{
	quint16 srcPort;
	quint16 destPort;
	quint16 udpLength;				// length of the UDP header and UDP data in bytes
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

// Compute IP checksum for count bytes starting at addr, using one's complement of one's complement sum
//
quint16 calcIpChecksum(quint8* rawHeaderPtr, int headerLenBytes);

// set IP checksum of a given ip header
//
void calcChecksum(IpHeader* ipHeader);




