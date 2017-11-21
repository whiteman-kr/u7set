#pragma once

#include <map>

class MemWriteMap
{
public:
	enum class Error
	{
		Ok,
		OutOfRange,
		MemRewrite
	};

public:
	MemWriteMap(int startAddr, int size, bool checkRewrite);
	virtual ~MemWriteMap();

	Error write(int addr, int size);

private:
	bool addrInRange(int addr);
	bool addrInRange(int addr, int size);

private:
	int m_startAddr = 0;
	int m_size = 0;
	bool m_checkRewrite = false;

	typedef std::map<int, int> WriteMap;

	WriteMap m_writeMap;
};
