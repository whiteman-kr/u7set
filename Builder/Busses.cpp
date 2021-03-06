#include "Busses.h"
#include "../UtilsLib/WUtils.h"

namespace Builder
{
	bool BusSignal::conversionRequired() const
	{
		switch(signalType)
		{
		case E::SignalType::Analog:
			switch(analogFormat)
			{
			case E::AnalogAppSignalFormat::Float32:
				return !(inbusAnalogFormat == E::DataFormat::Float &&
					inbusSizeBits == SIZE_32BIT &&
					inbusAnalogByteOrder == E::ByteOrder::BigEndian &&
					busAnalogLowLimit == inbusAnalogLowLimit &&
					busAnalogHighLimit == inbusAnalogHighLimit);

			case E::AnalogAppSignalFormat::SignedInt32:
				return !(inbusAnalogFormat == E::DataFormat::SignedInt &&
					inbusSizeBits == SIZE_32BIT &&
					inbusAnalogByteOrder == E::ByteOrder::BigEndian &&
					busAnalogLowLimit == inbusAnalogLowLimit &&
					busAnalogHighLimit == inbusAnalogHighLimit);
			}

			// switch default

			assert(false);			// unknown E::AnalogAppSignalFormat
			return true;

		case E::SignalType::Bus:
			return false;

		case E::SignalType::Discrete:
			return false;
		}

		// switch default
		assert(false);			// unknown  E::SignalType

		return true;
	}

	bool BusSignal::is_SInt32_To_UInt16_BE_NoScale_conversion() const
	{
		return 	signalType == E::SignalType::Analog &&
				analogFormat == E::AnalogAppSignalFormat::SignedInt32 &&
				inbusAnalogFormat == E::DataFormat::UnsignedInt &&
				inbusSizeBits == SIZE_16BIT &&
				inbusAnalogByteOrder == E::ByteOrder::BigEndian &&
				busAnalogLowLimit == inbusAnalogLowLimit &&
				busAnalogHighLimit == inbusAnalogHighLimit;
	}

	bool BusSignal::is_SInt32_To_SInt16_BE_NoScale_conversion() const
	{
		return 	signalType == E::SignalType::Analog &&
				analogFormat == E::AnalogAppSignalFormat::SignedInt32 &&
				inbusAnalogFormat == E::DataFormat::SignedInt &&
				inbusSizeBits == SIZE_16BIT &&
				inbusAnalogByteOrder == E::ByteOrder::BigEndian &&
				busAnalogLowLimit == inbusAnalogLowLimit &&
				busAnalogHighLimit == inbusAnalogHighLimit;
	}

	void BusSignal::init(const Busses& busses, const VFrame30::BusSignal& bs)
	{
		signalID = bs.signalId();
		signalType = bs.type();
		caption = bs.caption();
		busTypeID = bs.busTypeId();
		units = bs.units();

		inbusAddr.set(0, 0);
		inbusAddr.addBit(bs.inbusOffset() * SIZE_8BIT + bs.inbusDiscreteBitNo());

		analogFormat = bs.analogFormat();

		switch(bs.type())
		{
		case E::SignalType::Analog:
			inbusSizeBits = bs.inbusAnalogSize();
			break;

		case E::SignalType::Discrete:
			inbusSizeBits = SIZE_1BIT;
			break;

		case E::SignalType::Bus:
			inbusSizeBits = busses.getBusSizeBits(bs.busTypeId());
			assert(inbusSizeBits != -1);
			break;

		default:
			assert(false);		// unknown signal type
		}

		inbusAnalogFormat  = bs.inbusAnalogFormat();
		inbusAnalogByteOrder = bs.inbusAnalogByteOrder();
		busAnalogLowLimit = bs.busAnalogLowLimit();
		busAnalogHighLimit = bs.busAnalogHighLimit();
		inbusAnalogLowLimit = bs.inbusAnalogLowLimit();
		inbusAnalogHighLimit = bs.inbusAnalogHighLimit();
	}

	bool BusSignal::isOverlaped(const BusSignal& bs)
	{
		int addr1 = inbusAddr.bitAddress();
		int size1 = inbusSizeBits;

		int addr2 = bs.inbusAddr.bitAddress();
		int size2 = bs.inbusSizeBits;

		return	(addr2 >= addr1 && addr2 < (addr1 + size1)) ||
				(addr1 >= addr2 && addr1 < (addr2 + size2));
	}

	bool BusSignal::isValid() const
	{
		return signalID.isEmpty() == false && signalID != Bus::INVALUD_BUS_SIGNAL_ID;
	}

	const QString Bus::INVALUD_BUS_SIGNAL_ID("##InvalidBusSignalID##");

	Bus::Bus(const Busses& busses, const VFrame30::Bus bus, IssueLogger* log) :
		m_busses(busses),
		m_srcBus(bus),
		m_log(log)
	{
		m_invalidBusSignal.setSignalId(INVALUD_BUS_SIGNAL_ID);

		m_invalidSignal.signalID = INVALUD_BUS_SIGNAL_ID;
		m_invalidSignal.inbusAddr.reset();
	}

	bool Bus::init()
	{
		TEST_PTR_RETURN_FALSE(m_log);

		m_signals.clear();

		if (m_srcBus.enableManualBusSize() == true && (m_srcBus.manualBusSize() % WORD_SIZE_IN_BYTES) != 0)
		{
			// The bus size must be a multiple of 2 bytes (1 word)
			//
			m_log->errALC5095(m_srcBus.busTypeId());
			return false;
		}

		if (m_srcBus.busSignals().size() == 0)
		{
			if (m_srcBus.enableManualBusSize() == true)
			{
				m_sizeW = m_srcBus.manualBusSize() / WORD_SIZE_IN_BYTES;
			}
			else
			{
				m_sizeW = 0;
			}

			m_isInitialized = true;

			return true;
		}

		bool result = buildInBusSignalsMap();

		RETURN_IF_FALSE(result);

		if (m_srcBus.autoSignalPlacement() == true)
		{
			result = autoPlaceSignals();
		}
		else
		{
			result = buildSignalsOrder();
		}

		RETURN_IF_FALSE(result);

		result = calcBusSizeW();

		RETURN_IF_FALSE(result);

		result = checkSignalsOffsets();

		RETURN_IF_FALSE(result);

		buildSignalIndexesArrays();

		m_isInitialized = true;

		return true;
	}

	void Bus::writeReport(QStringList& list)
	{
		QString busTypeIdStr("BusTypeID");

		int maxBusTypeIdLen = busTypeIdStr.length();

		for(const BusSignal& s : m_signals)
		{
			if (s.signalType == E::SignalType::Bus)
			{
				int len = s.busTypeID.length();

				if (maxBusTypeIdLen < len)
				{
					maxBusTypeIdLen = len;
				}
			}
		}

		QString separator("-------------------------------------------------------------------");

		list.append(separator);
		list.append(QString("BusTypeID:\t\t\t%1").arg(m_srcBus.busTypeId()));
		list.append(QString("AutoSignalPlacement:\t\t%1").arg(m_srcBus.autoSignalPlacement() == true ? "True" : "False"));
		list.append(QString("EnableManualBusSize:\t\t%1").arg(m_srcBus.enableManualBusSize() == true ? "True" : "False"));
		list.append(QString("ManualBusSize (in bytes):\t%1").arg(m_srcBus.manualBusSize()));
		list.append(QString("BusSizeW:\t\t\t%1").arg(m_sizeW));
		list.append(QString("Signals:\t\t\t%1").arg(m_signals.count()));
		list.append(separator);

		if (m_signals.count() == 0)
		{
			list.append("");
			return;
		}

		list.append(QString(" OffsetW:Bit   Size\tType\t%1\tConvert\t SignalID").arg(busTypeIdStr.leftJustified(maxBusTypeIdLen, ' ')));
		list.append(separator);

		for(const BusSignal& s : m_signals)
		{
			QString signalTypeStr;

			switch(s.signalType)
			{
			case E::SignalType::Analog:
				signalTypeStr = "A";
				break;

			case E::SignalType::Discrete:
				signalTypeStr = "D";
				break;

			case E::SignalType::Bus:
				signalTypeStr = "B";
				break;

			default:
				assert(false);
			}

			list.append(QString("   %1    %2\t%3\t%4\t%5\t %6").
							arg(s.inbusAddr.toString(true)).
							arg(s.inbusSizeBits).
							arg(signalTypeStr).
							arg(s.busTypeID.leftJustified(maxBusTypeIdLen, ' ')).
							arg(s.conversionRequired() == true ? "Yes" : "No").
							arg(s.signalID));
		}

		list.append(separator);

		list.append("");
	}

	int Bus::sizeW() const
	{
		assert(m_sizeW != -1);
		assert(m_isInitialized == true);

		return m_sizeW;
	}

	int Bus::sizeB() const
	{
		return sizeW() * WORD_SIZE_IN_BYTES;
	}

	int Bus::sizeBit() const
	{
		return sizeW() * SIZE_16BIT;
	}

	const BusSignal& Bus::signalByID(const QString& signalID) const
	{
		for(const BusSignal& busSignal : m_signals)
		{
			if (busSignal.signalID == signalID)
			{
				return busSignal;
			}
		}

		return m_invalidSignal;
	}

	const BusSignal& Bus::signalByIndex(int index) const
	{
		if (index < 0 || index >= m_signals.count())
		{
			assert(false);
			return m_invalidSignal;
		}

		return m_signals[index];
	}


	bool Bus::buildInBusSignalsMap()
	{
		// build in bus signalID => signal index in m_srcBus.signals map
		//
		m_inBusSignalsMap.clear();

		const std::vector<VFrame30::BusSignal>& busSignals = m_srcBus.busSignals();

		bool hasAnalogSignals = false;
		bool hasDiscreteSignals = false;

		for(int i = 0; i < static_cast<int>(busSignals.size()); i++)
		{
			const VFrame30::BusSignal& busSignal = busSignals[i];

			if (m_inBusSignalsMap.contains(busSignal.signalId()) == true)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			m_inBusSignalsMap.insert(busSignal.signalId(), i);

			switch(busSignal.type())
			{
			case E::SignalType::Analog:
				hasAnalogSignals = true;
				break;

			case E::SignalType::Discrete:
				hasDiscreteSignals = true;
				break;

			case E::SignalType::Bus:
				{
					BusShared childBus = m_busses.getBus(busSignal.busTypeId());

					if (childBus == nullptr)
					{
						// Bus type ID %1 of signal %2 is undefined.
						//
						m_log->errALC5092(busSignal.busTypeId(), QString("%1.%2").arg(busTypeID()).arg(busSignal.signalId()));
						return false;
					}

					if (childBus->isInitialized() == false)
					{
						// Bus type %1 has not initialized.
						//
						m_log->errALC5151(childBus->busTypeID());
						return false;
					}

					switch(childBus->busDataFormat())
					{
					case E::BusDataFormat::Discrete:
						hasDiscreteSignals = true;
						break;

					case E::BusDataFormat::Mixed:
						hasAnalogSignals = true;
						break;

					default:
						assert(false);			// unknown E::BusDataFormat
					}
				}
				break;

			default:
				assert(false);
			}
		}

		m_busDataFormat = E::BusDataFormat::Mixed;

		if (hasDiscreteSignals == true && hasAnalogSignals == false)
		{
			m_busDataFormat = E::BusDataFormat::Discrete;
		}

		return true;
	}

	bool Bus::autoPlaceSignals()
	{
		assert(m_srcBus.autoSignalPlacement() == true);

		QStringList analogSignals;
		QStringList busSignals;
		QStringList discreteSignals;

		for(const VFrame30::BusSignal& busSignal : m_srcBus.busSignals())
		{
			switch(busSignal.type())
			{
			case E::SignalType::Analog:

				if ((busSignal.inbusAnalogSize() % SIZE_16BIT) != 0)
				{
					// Size of in bus analog signal '%1' is not multiple 16 bits (bus type '%2').
					//
					m_log->errALC5094(busSignal.signalId(), m_srcBus.busTypeId());
					return false;
				}

				analogSignals.append(busSignal.signalId());

				break;

			case E::SignalType::Bus:
				{
					int busSizeBits = m_busses.getBusSizeBits(busSignal.busTypeId());

					if (busSizeBits < 0)
					{
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					if ((busSizeBits % SIZE_16BIT) != 0)
					{
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					busSignals.append(busSignal.signalId());
				}
				break;

			case E::SignalType::Discrete:

				discreteSignals.append(busSignal.signalId());

				break;

			default:
				assert(false);			// unknown in bus signal type
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

		// signals ordering by alphabet
		//
		analogSignals.sort(Qt::CaseInsensitive);
		busSignals.sort(Qt::CaseInsensitive);
		discreteSignals.sort(Qt::CaseInsensitive);

		QStringList signalsOrder = analogSignals;

		signalsOrder.append(busSignals);
		signalsOrder.append(discreteSignals);

		// calculate signals addresses in bus
		//
		Address16 inBusAddr(0, 0);

		for(const QString& signalID : signalsOrder)
		{
			VFrame30::BusSignal& srcBusSignal = getBusSignal(signalID);

			BusSignal busSignal;

			busSignal.init(m_busses, srcBusSignal);
			busSignal.inbusAddr = inBusAddr;

			switch(srcBusSignal.type())
			{
			case E::SignalType::Analog:
			case E::SignalType::Discrete:
			case E::SignalType::Bus:

				assert(busSignal.inbusSizeBits != -1);
				inBusAddr.addBit(busSignal.inbusSizeBits);

				break;

			default:
				assert(false);			// unknown in bus signal type
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			m_signals.append(busSignal);
		}

		return true;
	}

	bool Bus::buildSignalsOrder()
	{
		assert(m_srcBus.autoSignalPlacement() == false);

		QVector<QPair<QString, int>> inBusSignals;		// QPair<busSignalID, signalBitAddressInBus>

		const std::vector<VFrame30::BusSignal>& busSignals = m_srcBus.busSignals();

		for(const VFrame30::BusSignal& busSignal : busSignals)
		{
			Address16 inBusAddr(0, 0);

			int signalSizeBits = 0;

			switch(busSignal.type())
			{
			case E::SignalType::Analog:
				if ((busSignal.inbusAnalogSize() % SIZE_16BIT) != 0)
				{
					// Size of in bus analog signal '%1' is not multiple 16 bits (bus type '%2').
					//
					m_log->errALC5094(busSignal.signalId(), m_srcBus.busTypeId());
					return false;
				}

				signalSizeBits = busSignal.inbusAnalogSize();

				if ((busSignal.inbusOffset() % WORD_SIZE_IN_BYTES) != 0)
				{
					// Offset of in bus analog signal '%' is not multiple of 2 bytes (1 word) (bus type '%2')
					//
					m_log->errALC5096(busSignal.signalId(), m_srcBus.busTypeId());
					return false;
				}

				inBusAddr.setOffset(busSignal.inbusOffset() / WORD_SIZE_IN_BYTES);
				inBusAddr.setBit(0);

				break;

			case E::SignalType::Bus:
				{
					BusShared childBus = m_busses.getBus(busSignal.busTypeId());

					if (childBus == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
					}
					else
					{
						if ((childBus->sizeBit() % SIZE_16BIT) != 0)
						{
							// The bus size must be a multiple of 2 bytes (1 word) (bus type '%1').
							//
							m_log->errALC5095(childBus->busTypeID());
							return false;
						}

						signalSizeBits = childBus->sizeBit();

						if ((busSignal.inbusOffset() % WORD_SIZE_IN_BYTES) != 0)
						{
							// Offset of in bus analog (or bus) signal '%' is not multiple of 2 bytes (1 word) (bus type '%2')
							//
							m_log->errALC5096(busSignal.signalId(), m_srcBus.busTypeId());
							return false;
						}
					}

					inBusAddr.setOffset(busSignal.inbusOffset() / WORD_SIZE_IN_BYTES);
					inBusAddr.setBit(0);
				}
				break;

			case E::SignalType::Discrete:
				inBusAddr.addBit(busSignal.inbusOffset() * SIZE_8BIT + busSignal.inbusDiscreteBitNo());
				signalSizeBits = 1;
				break;

			default:
				assert(false);			// unknown in bus signal type
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			inBusSignals.append(QPair<QString, int>(busSignal.signalId(), inBusAddr.bitAddress()));
		}

		// sort inBusSignals by bitAddress
		//
		int count = inBusSignals.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (inBusSignals[i].second > inBusSignals[k].second)
				{
					QPair<QString, int> tmp = inBusSignals[i];
					inBusSignals[i] = inBusSignals[k];
					inBusSignals[k] = tmp;
				}
			}
		}

		// calculate signals addresses in bus
		//
		for(const QPair<QString, int> inBusSignal : inBusSignals)
		{
			VFrame30::BusSignal& srcBusSignal = getBusSignal(inBusSignal.first);

			BusSignal busSignal;

			busSignal.init(m_busses, srcBusSignal);

			Address16 inBusAddr(0, 0);

			switch(srcBusSignal.type())
			{
			case E::SignalType::Analog:
			case E::SignalType::Bus:
				inBusAddr.addBit(srcBusSignal.inbusOffset() * SIZE_8BIT);
				break;

			case E::SignalType::Discrete:
				inBusAddr.addBit(srcBusSignal.inbusOffset() * SIZE_8BIT + srcBusSignal.inbusDiscreteBitNo());
				break;

			default:
				assert(false);			// unknown in bus signal type
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			busSignal.inbusAddr = inBusAddr;

			m_signals.append(busSignal);
		}

		return true;
	}

	bool Bus::calcBusSizeW()
	{
		if (m_srcBus.enableManualBusSize() == true)
		{
			m_sizeW = m_srcBus.manualBusSize() / WORD_SIZE_IN_BYTES;
			return true;
		}

		const BusSignal& lastBusSignal = m_signals.last();

		int busSizeBits = lastBusSignal.inbusAddr.bitAddress() + lastBusSignal.inbusSizeBits;

		m_sizeW = busSizeBits / SIZE_16BIT + ((busSizeBits % SIZE_16BIT) == 0 ? 0 : 1);

		return true;
	}

	bool Bus::checkSignalsOffsets()
	{
		bool result = true;

		int count = m_signals.count();

		int maxBitAddress = m_sizeW * SIZE_16BIT;

		// check signals overlapping
		//
		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if(m_signals[i].isOverlaped(m_signals[k]) == true)
				{
					// Bus signals '%1' and '%2' are overlapped (bus type '%3').
					//
					m_log->errALC5097(m_signals[i].signalID, m_signals[k].signalID, m_srcBus.busTypeId());
					result = false;
				}
			}
		}

		// check signals offsets
		//
		for(int i = 0; i < count; i++)
		{
			const BusSignal& s = m_signals[i];

			int signalBitAddr = s.inbusAddr.bitAddress();

			if (signalBitAddr < 0 || signalBitAddr >= maxBitAddress)
			{
				// Bus signal '%1' offset out of range (bus type '%2').
				//
				m_log->errALC5098(s.signalID, m_srcBus.busTypeId());
				result = false;
			}

			if (signalBitAddr + s.inbusSizeBits > maxBitAddress)
			{
				// Bus signal %1 placement is out of bus size (bus type %2).
				//
				m_log->errALC5152(s.signalID, m_srcBus.busTypeId());
				result = false;
			}
		}

		return result;
	}

	void Bus::buildSignalIndexesArrays()
	{
		m_analogSignalIndexes.clear();
		m_busSignalIndexes.clear();
		m_discreteSignalIndexes.clear();

		int count = m_signals.count();

		for(int i = 0; i < count; i++)
		{
			const BusSignal& s = m_signals[i];

			switch(s.signalType)
			{
			case E::SignalType::Analog:
				m_analogSignalIndexes.push_back(i);
				break;

			case E::SignalType::Bus:
				m_busSignalIndexes.push_back(i);
				break;

			case E::SignalType::Discrete:
				m_discreteSignalIndexes[s.inbusAddr.offset()].push_back(i);
				break;

			default:
				assert(false);
			}
		}
	}

	VFrame30::BusSignal& Bus::getBusSignal(const QString& signalID)
	{
		assert(m_inBusSignalsMap.size() == static_cast<int>(m_srcBus.busSignals().size()));

		int index = m_inBusSignalsMap.value(signalID, -1);

		if (index < 0 || index >= static_cast<int>(m_srcBus.busSignals().size()))
		{
			return m_invalidBusSignal;
		}

		return m_srcBus.busSignals()[index];
	}

	QStringList Bus::getChildBussesIDs()
	{
		QStringList childBussesIDs;

		for(const VFrame30::BusSignal& busSignal : m_srcBus.busSignals())
		{
			if (busSignal.type() != E::SignalType::Bus)
			{
				continue;
			}

			childBussesIDs.append(busSignal.busTypeId());
		}

		return childBussesIDs;
	}

	Busses::Busses(VFrame30::BusSet* busSet, IssueLogger* log) :
		m_busSet(busSet),
		m_log(log)
	{
	}

	Busses::~Busses()
	{
		m_busses.clear();
	}


	bool Busses::prepare()
	{
		TEST_PTR_RETURN_FALSE(m_busSet);

		const std::vector<VFrame30::Bus>& busses = m_busSet->busses();

		if (busses.size() == 0)
		{
			return true;
		}

		bool result = true;

		for(const VFrame30::Bus& srcBus : busses)
		{
			if (m_busses.contains(srcBus.busTypeId()) == true)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			BusShared bus = std::make_shared<Bus>(*this, srcBus, m_log);

			m_busses.insert(srcBus.busTypeId(), bus);
		}

		QVector<BusShared> busInitOrder;

		result = getBusInitOrder(&busInitOrder);

		if (result == false)
		{
			return false;
		}

		for(BusShared bus : busInitOrder)
		{
			result &= bus->init();
		}

		return result;
	}

	bool Busses::writeReport(BuildResultWriter* resultWriter)
	{
		TEST_PTR_RETURN_FALSE(resultWriter);

		if (m_busses.count() == 0)
		{
			return true;
		}

		QStringList busTypeIDs = m_busses.keys();

		busTypeIDs.sort();

		QStringList report;

		for(const QString& busTypeID : busTypeIDs)
		{
			BusShared bus = m_busses.value(busTypeID, nullptr);

			TEST_PTR_CONTINUE(bus);

			bus->writeReport(report);
		}

		resultWriter->addFile("Reports", "Busses.txt", "", "", report);

		return true;
	}

	BusShared Busses::getBus(const QString& busTypeID) const
	{
		return m_busses.value(busTypeID, nullptr);
	}

	int Busses::getBusSizeBits(const QString& busTypeID) const
	{
		BusShared bus = getBus(busTypeID);

		if (bus == nullptr)
		{
			assert(false);
			return -1;
		}

		return bus->sizeBit();
	}

	bool Busses::getBusInitOrder(QVector<BusShared>* busInitOrder)
	{
		if (busInitOrder == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		busInitOrder->clear();

		QHash<QString, BusShared> nonOrderedBusses = m_busses;
		QHash<QString, BusShared> orderedBusses;

		do
		{
			int orderedCount = 0;

			for(BusShared bus : m_busses)
			{
				if (nonOrderedBusses.contains(bus->busTypeID()) == false)
				{
					continue;
				}

				QStringList childBussesIDs = bus->getChildBussesIDs();

				bool hasUnresolvedChildBus = false;

				for(const QString childBusID : childBussesIDs)
				{
					if (orderedBusses.contains(childBusID) == true)
					{
						continue;
					}

					hasUnresolvedChildBus = true;
					break;
				}

				if (hasUnresolvedChildBus == true)
				{
					continue;
				}

				busInitOrder->append(bus);

				orderedBusses.insert(bus->busTypeID(), bus);

				nonOrderedBusses.remove(bus->busTypeID());

				orderedCount++;

				if (nonOrderedBusses.count() == 0)
				{
					break;
				}
			}

			if (orderedCount == 0)
			{
				// Can't resolve bus interdependencies: %1
				//
				QStringList unresolvedBussesIDs = nonOrderedBusses.keys();

				m_log->errALC5132(unresolvedBussesIDs.join(", "));

				return false;
			}
		}
		while(busInitOrder->count() < m_busses.count());

		return true;
	}

}
