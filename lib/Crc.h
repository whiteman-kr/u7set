#pragma once

#include <QtGlobal>
#include <vector>

class Crc
{
private:
	Crc(void);
	~Crc(void);

public:
	static quint16 crc4(quint16 value);
	static quint64 crc64(const void* src, qint64 l);
	static quint64 setDataBlockCrc(quint16 frameIndex, void* datablock, int blockSize);
	static bool checkDataBlockCrc(quint16 frameIndex, const std::vector<quint8>& frame);


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
	Crc64();

	quint64 add(int val);
	quint64 add(double val);
	quint64 add(const QString& str);
	quint64 add(const QByteArray& bytes);

	quint64 result() const { return m_crc; }
	quint32 result32() const { return m_crc & 0xFFFFFFFF; }

	void reset() { m_crc = CRC64_INIT_VALUE; }
};

const quint32 CRC32_INITIAL_VALUE = 0xFFFFFFFF;

quint32 CRC32(quint32 initialValue, const char* buffer, int len, bool finishCalc);

quint32 CRC32(const char* buffer, int len);

quint32 CRC32(quint32 initialValue, const QString& str, bool finishCalc);

quint32 CRC32(quint32 initialValue, int value, bool finishCalc);

// CRC16 implementation acording to CCITT standards (x^16 + x^12 + x^2 + 1)
//
quint16 calcCrc16(const void* buf, int len);

