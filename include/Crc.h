#pragma once

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
