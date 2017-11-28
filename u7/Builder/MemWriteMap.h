#pragma once

#include <map>
#include <QPair>
#include <QHash>

class MemWriteMap
{
public:
	enum class Error
	{
		Ok,
		OutOfRange,
		MemRewrite
	};

	typedef QPair<int, int> Area;				// memory area - <startAddr, sizeW>
	typedef QList<QPair<int, int>> AreaList;

public:
	MemWriteMap(int startAddr, int size, bool checkRewrite);
	virtual ~MemWriteMap();

	Error write(int addr, int size);

	Error write16(int addr) { return write(addr, 1); }
	Error write32(int addr) { return write(addr, 2); }

	void getNonWrittenAreas(AreaList* areaList);

private:
	bool addrInRange(int addr);
	bool addrInRange(int addr, int size);

private:
	int m_startAddr = 0;
	int m_size = 0;
	bool m_checkRewrite = false;

	typedef QHash<int, int> WriteMap;

	WriteMap m_writeMap;
};
