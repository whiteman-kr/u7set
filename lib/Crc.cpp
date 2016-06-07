#include <QtEndian>
#include <QString>
#include "../lib/Crc.h"
#include <vector>
#include <cassert>


// Using normal poly 0x000000000000001B

quint64 Crc::crc64(const void* src, qint64 size)
{
	return Crc::crc64Normal(src, size);
	//return Crc::crc64Reverse(src, size);
}

quint64 Crc::setDataBlockCrc(quint16 frameIndex, void* datablock, int blockSize)
{
// !!! ATTENTION !!!
// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN
//
//#pragma message (__FUNCTION__" !!! ATTENTION !!! HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN. SUPPOSED TO BE REMOVED IN THE FUTURE.")

	std::vector<quint8> buffer;
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

quint16 Crc::crc4(quint16 value)
{
	const quint16 Polinom = 0x3;

	quint16 crc = 0xf000;
	crc ^= value;

	quint16 shifted_Polinom = Polinom << (3+8); //shift the polinom on 8 + 3 bits left to make top bit signed

	// make 16 cycles = 16 bits input
	//
	for (int j = 0; j < 16 ; j++)
	{
		if ((crc & (1 << 15)) == 0x8000)
		{
			crc ^= shifted_Polinom;
			crc = (crc << 1);
		}
		else
		{
			crc = crc << 1;
		}
	}

	crc = crc >> 8;
	crc = crc >> 4;

	return crc;
}
quint64 Crc::crc64Normal(const void* src, qint64 size)
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

quint64 Crc::crc64Reverse(const void* src, qint64 size)
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


// -------------------------------------------------------------------------------------
//
// Crc64 class implementation
//
// -------------------------------------------------------------------------------------

Crc64::Crc64()
{
}


quint64 Crc64::add(int val)
{
	return calc(reinterpret_cast<const quint8*>(&val), sizeof(int));
}


quint64 Crc64::add(double val)
{
	return calc(reinterpret_cast<const quint8*>(&val), sizeof(double));
}


quint64 Crc64::add(const QString& str)
{
	const QChar* data = str.constData();

	while(*data != 0)
	{
		ushort qchar = data->unicode();

		calc(reinterpret_cast<const quint8*>(&qchar), sizeof(ushort));

		data++;
	}

	return m_crc;
}


quint64 Crc64::add(const QByteArray& bytes)
{
	return calc(reinterpret_cast<const quint8*>(bytes.constData()), bytes.size());
}


quint64 Crc64::calc(const quint8* data, qint64 size)
{
	while (size > 0)
	{
		m_crc ^= (unsigned long long)*data << 56;
		data++;

		for (int i = 0; i < 8; i++)
		{
			m_crc = m_crc & 0x8000000000000000ULL ? (m_crc << 1) ^ 0x000000000000001BULL : m_crc << 1;
		}

		size--;
	}

	return m_crc;
}
