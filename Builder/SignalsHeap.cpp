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

	//

	SignalsHeap::SignalsHeap(int memCellSizeBits, bool generateLog, IssueLogger* log) :
		m_memCellSizeBits(memCellSizeBits),
		m_generateLog(generateLog),
		m_log(log)
	{
		// memCellSizeBits should be == 1 or 16
		//
		Q_ASSERT(m_memCellSizeBits == SIZE_1BIT || m_memCellSizeBits == SIZE_16BIT);
	}

	SignalsHeap::~SignalsHeap()
	{
		for(const std::pair<QString, HeapItem*>& p : m_items)
		{
			delete p.second;
		}
	}

	void SignalsHeap::init(int heapStartAddrW, int heapSizeW)
	{
		m_heapStartAddrW = heapStartAddrW;
		m_heapSizeW = heapSizeW;

		m_heapHighBoundBits = m_heapStartAddrW * SIZE_16BIT;

		logInit();
	}

	void SignalsHeap::finalize()
	{
		if (m_itemsInHeap.size() != 0)
		{
			Q_ASSERT(false);

			LOG_INTERNAL_ERROR_MSG(m_log, "Heap is not clear on destruction!");
		}

		logFinalize();
	}

	void SignalsHeap::appendItem(const UalSignal& ualSignal, std::optional<int> expectedReadCount)
	{
		if (expectedReadCount.has_value() == false)
		{
			return;
		}

		Q_ASSERT(expectedReadCount.value() > 0);
		Q_ASSERT(ualSignal.isHeapPlaced() == true);

		QString appSignalID = ualSignal.appSignalID();
		int signalDataSize = ualSignal.dataSize();

		if (m_items.find(appSignalID) != m_items.end())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Duplicate appSignalID - %1").arg(appSignalID));
			return;
		}

		if (m_memCellSizeBits == SIZE_1BIT &&  signalDataSize != SIZE_1BIT)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Data size of signal %1 is not equal 1 bit").arg(appSignalID));
			return;
		}

		if (m_memCellSizeBits == SIZE_16BIT && (signalDataSize % SIZE_16BIT) != 0)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Data size of signal %1 is not multiple to 16 bit").arg(appSignalID));
			return;
		}

		HeapItem* item = new HeapItem;

		item->appSignalID = appSignalID;
		item->sizeBits = signalDataSize;
		item->readCount = expectedReadCount.value();

		std::pair<QString, HeapItem*> p;

		p.first = appSignalID;
		p.second = item;

		m_items.insert(p);
	}

	void SignalsHeap::removeItem(const UalSignal& ualSignal)
	{
		QString appSignalID = ualSignal.appSignalID();

		if (m_items.find(appSignalID) == m_items.end())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Heap item %1 is not exists").arg(appSignalID));
			return;
		}

		m_items.erase(appSignalID);
	}

	Address16 SignalsHeap::getAddressForWrite(const UalSignal& ualSignal)
	{
		if (m_firstGetWriteAddr == true)
		{
			logHeapItems();
			m_firstGetWriteAddr = false;
		}

		Address16 addrForWrite;

		QString appSignalID = ualSignal.appSignalID();

		std::map<QString, HeapItem*>::iterator p = m_items.find(appSignalID);

		if (p == m_items.end())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Heap item %1 is not exists").arg(appSignalID));
			return addrForWrite;
		}

		HeapItem* heapItem = p->second;

		if (heapItem == nullptr)
		{
			Q_ASSERT(false);
			LOG_NULLPTR_ERROR(m_log);
			return addrForWrite;
		}

		if (heapItem->address != BAD_ADDRESS)
		{
			if (ualSignal.isBus() == true)
			{
				// for bus signals multiple calls of getAddressForWrite is allowed due to mutistep bus processing
				//
				addrForWrite.setBitAddress(heapItem->address);

				return addrForWrite;
			}
			else
			{
				// for analog and discrete signals multiple calls of getAddressForWrite is not allowed!
				//
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Repeated call of getAddressForWrite for %1").arg(appSignalID));

				return addrForWrite;
			}
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
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Out of heap size"));
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

		QString appSignalID = ualSignal.appSignalID();

		std::map<QString, HeapItem*>::iterator p = m_items.find(appSignalID);

		if (p == m_items.end())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Heap item %1 is not exists").arg(appSignalID));
			return addrForRead;
		}

		HeapItem* heapItem = p->second;

		if (heapItem == nullptr)
		{
			Q_ASSERT(false);
			LOG_NULLPTR_ERROR(m_log);
			return addrForRead;
		}

		if (heapItem->address == BAD_ADDRESS)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Undefined heap item %1 address").arg(appSignalID));
			return addrForRead;
		}

		int plainBitAddr = heapItem->address;

		if (heapItem->sizeBits != ualSignal.dataSize())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Heap item data size and signal %1 data size are not equal").arg(appSignalID));
			return addrForRead;
		}

		if (heapItem->readCount <= 0)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Extra read of heap item %1").arg(appSignalID));
			return addrForRead;
		}

		if (m_itemsInHeap.find(plainBitAddr) == m_itemsInHeap.end())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Heap item %1 is not in m_itemsInHeap").arg(appSignalID));
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

		Q_ASSERT(heapUsedSizeW <= m_heapSizeW);

		return heapUsedSizeW;
	}

	const QStringList& SignalsHeap::getHeapLog() const
	{
		return m_heapLog;
	}

	const std::vector<std::tuple<QString, Address16, int>>& SignalsHeap::getHeapItemsLog() const
	{
		return m_heapItemsLog;
	}

	void SignalsHeap::logInit()
	{
		if (m_generateLog == false)
		{
			return;
		}

		m_heapLog.append(QString("Heap mem cell size:\t%1 bit(s)").arg(m_memCellSizeBits));
		m_heapLog.append(QString("Heap start addrW:\t%1").arg(m_heapStartAddrW));
		m_heapLog.append(QString("Max heap sizeW:\t\t%1").arg(m_heapSizeW));
	}

	void SignalsHeap::logHeapItems()
	{
		if (m_generateLog == false)
		{
			return;
		}

		m_heapLog.append(QString("Heap items count:\t%1").arg(m_items.size()));

		if (m_items.size() > 0)
		{
			m_heapLog.append(QString());
			m_heapLog.append(QString().fill('-', 80));
			m_heapLog.append(QString("Size\tExpRead"));
			m_heapLog.append(QString().fill('-', 80));

			for(const std::pair<QString, HeapItem*>& p : m_items)
			{
				 HeapItem* heapItem = p.second;

				 TEST_PTR_CONTINUE(heapItem);

				 m_heapLog.append(QString("%1\t%2\t%3").
									arg(heapItem->sizeBits).
									arg(heapItem->readCount).
									arg(heapItem->appSignalID));
			}

			m_heapLog.append(QString().fill('-', 80));
			m_heapLog.append(QString());
		}
	}

	void SignalsHeap::logAppendToHeap(const HeapItem& heapItem)
	{
		m_heapItemsLog.push_back({heapItem.appSignalID, heapItem.address16(), heapItem.sizeBits});

		if (m_generateLog == false)
		{
			return;
		}

		m_heapLog.append(QString("%1 append %2 (expRead = %3)").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID).
							arg(heapItem.readCount));
	}

	void SignalsHeap::logReadFromHeap(const HeapItem& heapItem, bool decrementReadCount)
	{
		if (m_generateLog == false)
		{
			return;
		}

		m_heapLog.append(QString("%1 read %2 (decReadCount = %3, expRead = %4)").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID).
							arg(decrementReadCount == true ? "TRUE" : "FALSE").
							arg(heapItem.readCount));
	}

	void SignalsHeap::logRemoveFromHeap(const HeapItem& heapItem)
	{
		if (m_generateLog == false)
		{
			return;
		}

		m_heapLog.append(QString("%1 remove %2").
							arg(heapItem.address16().toString()).
							arg(heapItem.appSignalID));
	}

	void SignalsHeap::logFinalize()
	{
		if (m_generateLog == false)
		{
			return;
		}

		if (m_items.size() > 0)
		{
			m_heapLog.insert(3, QString("Used heap sizeW:\t%1 (%2%)").
								arg(getHeapUsedSizeW()).
								arg(static_cast<double>((getHeapUsedSizeW() * 100.0) / m_heapSizeW), 0, 'g', 1));

			m_heapLog.append("");

			if (m_itemsInHeap.size() == 0)
			{
				m_heapLog.append(QString("Heap is clear before destruction - Ok."));
			}
			else
			{
				m_heapLog.append(QString("ERROR! Heap contains %1 item(s) before destruction.\n").arg(m_itemsInHeap.size()));
				m_heapLog.append(QString("Items in heap:\n"));

				for(const std::pair<int, HeapItem*>& p : m_itemsInHeap)
				{
					TEST_PTR_CONTINUE(p.second);

					m_heapLog.append(QString("%1\t%2").arg(p.first).arg(p.second->appSignalID));
				}
			}
		}
	}
}



