#include "SignalsHeap.h"

namespace Builder
{
	SignalsHeap::SignalsHeap()
	{
	}

	SignalsHeap::~SignalsHeap()
	{
		for(const std::pair<QString, HeapItem*>& p : m_items)
		{
			delete p.second;
		}
	}

	void SignalsHeap::init(int startAddr, int size, int memCellSizeBits)
	{
		// memCellSizeBits should be == 1 or 16
		//
		assert(memCellSizeBits == 1 || memCellSizeBits == SIZE_16BIT);

		m_startAddr = startAddr;
		m_size = size;
		m_memCellSizeBits = memCellSizeBits;
	}

	void SignalsHeap::appendItem(const UalSignal& ualSignal, int expectedReadCount)
	{
		assert(expectedReadCount > 0);
		assert(ualSignal.isHeapPlaced() == true);

		QString appSignalID = ualSignal.appSignalID();
		int signalDataSize = ualSignal.dataSize();

		if (m_items.find(appSignalID) != m_items.end())
		{
			assert(false);		// duplicate appSignalID
			return;
		}

		if (m_memCellSizeBits == 1 &&  signalDataSize != 1)
		{
			assert(false);
			return;
		}

		if (m_memCellSizeBits == SIZE_16BIT && (signalDataSize % SIZE_16BIT) != 0)
		{
			assert(false);
			return;
		}

		HeapItem* item = new HeapItem;

		item->appSignalID = appSignalID;
		item->sizeBits = signalDataSize;
		item->readCount = expectedReadCount;

		std::pair<QString, HeapItem*> p;

		p.first = appSignalID;
		p.second = item;

		m_items.insert(p);
	}

	Address16 SignalsHeap::getAddressForWrite(const UalSignal& ualSignal)
	{
		std::map<QString, HeapItem*>::iterator p = m_items.find(ualSignal.appSignalID());

		Address16 addr;

		if (p == m_items.end())
		{
			assert(false);				// appSignalID is not found
			return addr;
		}

		HeapItem* heapItem = p->second;

		if (heapItem == nullptr)
		{
			assert(false);
			return addr;
		}

		if (heapItem->address != -1)
		{
			assert(false);				// repeated call of getAddressForWrite for appSignalID
			return addr;
		}

		for(const std::pair<int, int>& freedAddress : m_freedAddresses)
		{
			if (freedAddress.second == ualSignal.dataSize())
			{
				// use freed address
				//
				heapItem->address = freedAddress.first;

				m_itemsInHeap.insert(std::pair<int, HeapItem*>(heapItem->address, heapItem));
				m_freedAddresses.erase(freedAddress.first);

				addr.setBitAddress(heapItem->address);

				return addr;
			}
		}

		//

		int bitAddr = m_startAddr;

		if (m_itemsInHeap.size() > 0)
		{
			HeapItem* lastHeapItem = m_itemsInHeap.rbegin()->second;

			bitAddr = lastHeapItem->address + lastHeapItem->sizeBits;

			assert((bitAddr % m_memCellSizeBits) == 0);
		}

		heapItem->address = bitAddr;

		m_itemsInHeap.insert(std::pair<int, HeapItem*>(bitAddr, heapItem));

		addr.setBitAddress(bitAddr);

		return addr;
	}

}



