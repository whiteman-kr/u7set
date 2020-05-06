#include "SignalsHeap.h"
#include "UalItems.h"

namespace Builder
{
	Address16 SignalsHeap::HeapItem::address16() const
	{
		Address16 addr;

		addr.setBitAddress(address);

		return addr;
	}

	SignalsHeap::SignalsHeap(int memCellSizeBits)
	{
		// memCellSizeBits should be == 1 or 16
		//
		Q_ASSERT(memCellSizeBits == 1 || memCellSizeBits == SIZE_16BIT);

		m_memCellSizeBits = memCellSizeBits;
	}

	SignalsHeap::~SignalsHeap()
	{
		Q_ASSERT(m_itemsInHeap.size() == 0);

		for(const std::pair<QString, HeapItem*>& p : m_items)
		{
			delete p.second;
		}
	}

	void SignalsHeap::init(int heapStartAddrW, int heapSizeW)
	{
		m_heapStartAddrW = heapStartAddrW;
		m_heapSizeW = heapSizeW;
	}

	void SignalsHeap::appendItem(const UalSignal& ualSignal, int expectedReadCount)
	{
		Q_ASSERT(expectedReadCount > 0);
		Q_ASSERT(ualSignal.isHeapPlaced() == true);

		QString appSignalID = ualSignal.appSignalID();
		int signalDataSize = ualSignal.dataSize();

		if (m_items.find(appSignalID) != m_items.end())
		{
			Q_ASSERT(false);		// duplicate appSignalID
			return;
		}

		if (m_memCellSizeBits == 1 &&  signalDataSize != 1)
		{
			Q_ASSERT(false);
			return;
		}

		if (m_memCellSizeBits == SIZE_16BIT && (signalDataSize % SIZE_16BIT) != 0)
		{
			Q_ASSERT(false);
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
		Address16 addrForWrite;

		std::map<QString, HeapItem*>::iterator p = m_items.find(ualSignal.appSignalID());

		if (p == m_items.end())
		{
			Q_ASSERT(false);				// appSignalID is not found
			return addrForWrite;
		}

		HeapItem* heapItem = p->second;

		if (heapItem == nullptr)
		{
			Q_ASSERT(false);
			return addrForWrite;
		}

		if (heapItem->address != BAD_ADDRESS)
		{
			Q_ASSERT(false);				// repeated call of getAddressForWrite for appSignalID
			return addrForWrite;
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

				addrForWrite.setBitAddress(heapItem->address);

				logAppendToHeap(*heapItem);

				return addrForWrite;
			}
		}

		//

		int plainBitAddr = m_heapStartAddrW * SIZE_16BIT;

		if (m_itemsInHeap.size() > 0)
		{
			HeapItem* lastHeapItem = m_itemsInHeap.rbegin()->second;

			plainBitAddr = lastHeapItem->address + lastHeapItem->sizeBits;

			Q_ASSERT((plainBitAddr % m_memCellSizeBits) == 0);
		}

		if (plainBitAddr + heapItem->sizeBits > (m_heapStartAddrW + m_heapSizeW) * SIZE_16BIT)
		{
			// out of heap size
			Q_ASSERT(false);
			return addrForWrite;
		}

		heapItem->address = plainBitAddr;

		m_itemsInHeap.insert(std::pair<int, HeapItem*>(plainBitAddr, heapItem));

		addrForWrite.setBitAddress(plainBitAddr);

		if (heapItem->address + heapItem->sizeBits > m_heapHighBoundBits)
		{
			m_heapHighBoundBits = heapItem->address + heapItem->sizeBits;
		}

		logAppendToHeap(*heapItem);

		return addrForWrite;
	}

	Address16 SignalsHeap::getAddressForRead(const UalSignal& ualSignal, bool decrementReadCount)
	{
		Address16 addrForRead;

		std::map<QString, HeapItem*>::iterator p = m_items.find(ualSignal.appSignalID());

		if (p == m_items.end())
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		HeapItem* heapItem = p->second;

		if (heapItem == nullptr)
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		if (heapItem->address == BAD_ADDRESS)
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		int plainBitAddr = heapItem->address;

		if (heapItem->sizeBits != ualSignal.dataSize())
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		if (heapItem->readCount <= 0)
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		if (m_itemsInHeap.find(plainBitAddr) == m_itemsInHeap.end())
		{
			Q_ASSERT(false);
			return addrForRead;
		}

		// All checks is OK

		addrForRead.setBitAddress(plainBitAddr);

		if (decrementReadCount == true)
		{
			heapItem->readCount--;
		}

		logReadFromHeap(*heapItem, decrementReadCount);

		if (heapItem->readCount != 0)
		{
			return addrForRead;
		}

		logRemoveFromHeap(*heapItem);

		m_itemsInHeap.erase(plainBitAddr);

		Q_ASSERT(m_freedAddresses.find(plainBitAddr) == m_freedAddresses.end());

		m_freedAddresses.insert(std::pair<int, int>(plainBitAddr, heapItem->sizeBits));

		return addrForRead;
	}

	int SignalsHeap::getHeapUsedSizeW() const
	{
		int heapUsedSizeBits = m_heapHighBoundBits - m_heapStartAddrW * SIZE_16BIT;

		int heapUsedSizeW = heapUsedSizeBits / SIZE_16BIT;

		if ((heapUsedSizeBits % SIZE_16BIT) != 0)
		{
			heapUsedSizeW++;
		}

		return heapUsedSizeW;
	}

	void SignalsHeap::logAppendToHeap(const HeapItem& heapItem)
	{
		m_heapLog.append(QString("%1 append %2 (expected read count = %3)").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID).
							arg(heapItem.readCount));
	}

	void SignalsHeap::logReadFromHeap(const HeapItem& heapItem, bool decrementReadCount)
	{
		m_heapLog.append(QString("%1 read %2 (decrementReadCount = %3, expected read count = %4)").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID).
							arg(decrementReadCount == true ? "TRUE" : "FALSE").
							arg(heapItem.readCount));
	}

	void SignalsHeap::logRemoveFromHeap(const HeapItem& heapItem)
	{
		m_heapLog.append(QString("%1 remove %2").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID));
	}
}



