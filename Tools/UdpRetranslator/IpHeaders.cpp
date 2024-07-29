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
void calcChecksum(IpHeader* ipHeader)
{
	ipHeader->headerChecksum = 0;
	ipHeader->headerChecksum = calcIpChecksum(reinterpret_cast<quint8*>(ipHeader), ipHeader->ipHeaderLenght * 4);
}
