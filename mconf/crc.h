#pragma once

class Crc
{
private:
	Crc(void);
	~Crc(void);

public:
	static uint64_t crc64(const void* src, uint64_t l);
	static uint64_t setDataBlockCrc(uint16_t frameIndex, void* datablock, int blockSize);

private:
	static uint64_t crc64Normal(const void* src, uint64_t l);
	static uint64_t crc64Reverse(const void* src, uint64_t l);
};

