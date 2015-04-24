#include "../include/Crc.h"

#include <assert.h>
#include <QtEndian>

// Using normal poly 0x000000000000001B

qint64 Crc::crc64(const void* src, qint64 size)
{
	return Crc::crc64Normal(src, size);
	//return Crc::crc64Reverse(src, size);
}

qint64 Crc::setDataBlockCrc(uint16_t frameIndex, void* datablock, int blockSize)
{
// !!! ATTENTION !!!
// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN
//
//#pragma message (__FUNCTION__" !!! ATTENTION !!! HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN. SUPPOSED TO BE REMOVED IN THE FUTURE.")

	std::vector<uint8_t> buffer;
	buffer.resize(blockSize + sizeof(decltype(frameIndex)), 0);
	
	*reinterpret_cast<decltype(frameIndex)*>(buffer.data()) = qToBigEndian(frameIndex);
	memcpy(buffer.data() + sizeof(decltype(frameIndex)), datablock, blockSize);

	quint64 crc = Crc::crc64(buffer.data(), buffer.size() - sizeof(crc));
    *reinterpret_cast<qint64*>(buffer.data() + buffer.size() - sizeof(crc)) = qToBigEndian(crc);						// CONVERT CRC HERE

	// Check calculated data CRC
	//
    qint64 checkCrc = Crc::crc64(buffer.data(), buffer.size());
	assert(checkCrc == 0ULL);
	
	// Set crc as the last bytes in datablock
	//
    *reinterpret_cast<qint64*>(static_cast<char*>(datablock) + blockSize - sizeof(crc)) = qToBigEndian(crc);		// CONVERT CRC HERE
	
	return crc;
}

qint64 Crc::crc64Normal(const void* src, qint64 size)
{
	const unsigned char* p = static_cast<const unsigned char*>(src);
	unsigned long long crc = 0xFFFFFFFFFFFFFFFF;
	unsigned char i;

	while (size--)
	{
		crc ^= (unsigned long long)*p << 56;
		p++;

		for (i = 0; i < 8; i++)
			crc = crc & 0x8000000000000000ULL ? (crc << 1) ^ 0x000000000000001BULL : crc << 1;
	}

	return crc;
}

qint64 Crc::crc64Reverse(const void* src, qint64 size)
{
	const unsigned char* p = static_cast<const unsigned char*>(src);
	unsigned long long crc = 0xFFFFFFFFFFFFFFFF;
	unsigned char i;

	while (size--)
	{
		crc ^= (unsigned long long)*p;
		p++;

		for (i = 0; i < 8; i++)
			crc = crc & 0x0000000000000001ULL ? (crc >> 1) ^ 0xD800000000000000ULL : crc >> 1;
	}

	return crc;
}
