#ifndef CRC_H
#define CRC_H

#include <QtGlobal>

class Crc
{
private:
	Crc(void);
	~Crc(void);

public:
    static qint64 crc64(const void* src, qint64 l);
    static qint64 setDataBlockCrc(uint16_t frameIndex, void* datablock, int blockSize);

private:
    static qint64 crc64Normal(const void* src, qint64 l);
    static qint64 crc64Reverse(const void* src, qint64 l);
};

#endif // CRC_H
