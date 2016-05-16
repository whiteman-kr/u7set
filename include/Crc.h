#pragma once

#include <QtGlobal>

class Crc
{
private:
	Crc(void);
	~Crc(void);

public:
	static quint16 crc4(quint16 value);
	static quint64 crc64(const void* src, qint64 l);
	static quint64 setDataBlockCrc(quint16 frameIndex, void* datablock, int blockSize);

private:
	static quint64 crc64Normal(const void* src, qint64 l);
	static quint64 crc64Reverse(const void* src, qint64 l);
};


class Crc64
{
private:
	static const quint64 CRC64_INIT_VALUE = 0xFFFFFFFFFFFFFFFF;

	quint64 m_crc = CRC64_INIT_VALUE;

	quint64 calc(const quint8* data, qint64 size);

public:
	quint64 add(double val);
	quint64 add(const QString& str);

	quint64 result() const { return m_crc; }

	void reset() { m_crc = CRC64_INIT_VALUE; }
};
