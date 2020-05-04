#pragma once

#include "../lib/Address16.h"
#include "UalItems.h"

namespace Builder
{
	class SignalsHeap
	{
	public:
		struct HeapItem
		{
			QString appSignalID;
			int sizeBits = -1;
			int readCount = 0;

			int address = -1;			// plain bit address
		};

	public:
		SignalsHeap();
		virtual ~SignalsHeap();

		void init(int startAddr, int size, int memCellSizeBits);

		void appendItem(const UalSignal &ualSignal, int expectedReadCount);

		Address16 getAddressForWrite(const UalSignal& ualSignal);			// should be called only once for each appSignalID

	private:
		int m_startAddr = -1;
		int m_size = -1;
		int m_memCellSizeBits = -1;

		std::map<QString, HeapItem*> m_items;				// all signals will be placed in heap, appSignalID -> HeapItem
		std::map<int, HeapItem*> m_itemsInHeap;				// current signals in heap
		std::map<int, int> m_freedAddresses;				// freed addresses address -> sizeBits
	};
}

