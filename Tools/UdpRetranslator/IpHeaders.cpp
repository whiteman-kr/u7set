#include "IpHeaders.h"

// Compute IP checksum for count bytes starting at addr, using one's complement of one's complement sum
//
quint16 calcIpChecksum(quint8* rawHeaderPtr, int headerLenBytes)
{
	unsigned short* addr = reinterpret_cast<unsigned short*>(rawHeaderPtr);

	unsigned int count = headerLenBytes;

	unsigned long sum = 0;

	while (count > 1)
	{
		sum += *addr++;
		count -= 2;
	}

	// if any bytes left, pad the bytes and add

	if(count > 0)
	{
		sum += ((*addr) & htons(0xFF00));
	}

	// Fold sum to 16 bits: add carrier to result

	while (sum>>16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	//one's complement

	sum = ~sum;

	return static_cast<quint16>(sum);
}

// set IP checksum of a given ip header
//
void calcIpHeaderChecksum(IpHeader* ipHeader)
{
	ipHeader->headerChecksum = 0;
	ipHeader->headerChecksum = htons(calcIpChecksum(reinterpret_cast<quint8*>(ipHeader), ipHeader->headerLenBytes()));
}

/* set tcp checksum: given IP header and UDP datagram */
void calcUdpHeaderChecksum(IpHeader* iph)
{
	unsigned long sum = 0;

	UdpHeader* uh = reinterpret_cast<UdpHeader*>(iph->payloadPtr());
	unsigned short udpLen = htons(uh->udpLength);

	//add the pseudo header

	// add the source IP
	//
	sum += (iph->srcIP.ip >> 16) & 0xFFFF;
	sum += (iph->srcIP.ip) & 0xFFFF;

	// add the dest IP
	//
	sum += (iph->destIP.ip >> 16) & 0xFFFF;
	sum += (iph->destIP.ip) & 0xFFFF;

	//protocol and reserved: 17
	//
	sum += htons(IPPROTO_UDP);

	//the length
	//
	sum += uh->udpLength;

	// initialize checksum to 0
	//
	uh->checksum = 0;

	// add the IP payload
	//
	quint16* ipPayloadPtr = reinterpret_cast<quint16*>(iph->payloadPtr());

	while (udpLen > 1)
	{
		sum += *ipPayloadPtr;

		ipPayloadPtr++;

		udpLen -= 2;
	}

	// if any bytes left, pad the bytes and add
	//
	if(udpLen > 0)
	{
		sum += ((*ipPayloadPtr) & htons(0xFF00));
	}

	// Fold sum to 16 bits: add carrier to result
	//
	while (sum>>16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// printf("one's complementn");
	//
	sum = ~sum;

	// set computation result
	//
	uh->checksum = htons((static_cast<quint16>(sum) == 0x0000) ? 0xFFFF : static_cast<quint16>(sum));
}
