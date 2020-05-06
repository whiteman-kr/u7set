#pragma once

#include "../lib/Address16.h"

namespace Builder
{
	class UalSignal;

	class SignalsHeap
	{
	public:
		struct HeapItem
		{
			QString appSignalID;
			int sizeBits = -1;
			int readCount = 0;

			int address = BAD_ADDRESS;			// plain bit address

			Address16 address16() const;
		};

	public:
		SignalsHeap(int memCellSizeBits);
		virtual ~SignalsHeap();

		void init(int heapStartAddrW, int heapSizeW);

		void appendItem(const UalSignal &ualSignal, int expectedReadCount);

		Address16 getAddressForWrite(const UalSignal& ualSignal);		// should be called only once for each appSignalID
		Address16 getAddressForRead(const UalSignal& ualSignal, bool decrementReadCount);

		int getHeapUsedSizeW() const;

		const QStringList& getHeapLog() const { return m_heapLog; }

	private:

		void logAppendToHeap(const HeapItem& heapItem);
		void logReadFromHeap(const HeapItem& heapItem, bool decrementReadCount);
		void logRemoveFromHeap(const HeapItem& heapItem);

	private:
		int m_heapStartAddrW = -1;
		int m_heapSizeW = -1;
		int m_memCellSizeBits = -1;

		std::map<QString, HeapItem*> m_items;				// all signals will be placed in heap: appSignalID -> HeapItem
		std::map<int, HeapItem*> m_itemsInHeap;				// current signals in heap: address -> HeapItem
		std::map<int, int> m_freedAddresses;				// freed addresses: address -> sizeBits

		int m_heapHighBoundBits = 0;

		QStringList m_heapLog;
	};
}

