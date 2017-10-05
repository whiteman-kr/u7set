#include "Busses.h"
#include "../lib/WUtils.h"


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
					busAnalogLowLimit == inbusAnalogLowLimit &&
					busAnalogHighLimit == inbusAnalogHighLimit &&
					inbusAnalogByteOrder == E::ByteOrder::BigEndian);

			case E::AnalogAppSignalFormat::SignedInt32:
				return !(inbusAnalogFormat == E::DataFormat::SignedInt &&
					inbusSizeBits == SIZE_32BIT &&
					busAnalogLowLimit == inbusAnalogLowLimit &&
					busAnalogHighLimit == inbusAnalogHighLimit &&
					inbusAnalogByteOrder == E::ByteOrder::BigEndian);
			}

			// switch default

			assert(false);			// unknown E::AnalogAppSignalFormat
			return true;

		case E::SignalType::Discrete:
			return false;
		}

		// switch default
		assert(false);			// unknown  E::SignalType

		return true;
	}

	void BusSignal::init(const VFrame30::BusSignal& bs)
	{
		signalID = bs.signalId();
		signalType = bs.type();

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
			assert(false);		// bus inside bus is not allowed
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


	VFrame30::BusSignal Bus::m_invalidBusSignal;
	BusSignal Bus::m_invalidSignal;

	Bus::Bus(const VFrame30::Bus bus, IssueLogger* log) :
		m_srcBus(bus),
		m_log(log)
	{
		m_invalidBusSignal.setSignalId("InvalidInBusSignalID");

		m_invalidSignal.signalID = "InvalidInBusSignalID";
		m_invalidSignal.inbusAddr.reset();
	}

	bool Bus::init()
	{
		if (m_log == nullptr)
		{
			assert(false);
			return false;
		}

		m_signals.clear();

		if (m_srcBus.autoSignalPlacement() == false && (m_srcBus.manualBusSize() % WORD_SIZE_IN_BYTES) != 0)
		{
			// The bus size must be a multiple of 2 bytes (1 word)
			//
			m_log->errALC5095(m_srcBus.busTypeId());
			return false;
		}

		if (m_srcBus.busSignals().size() == 0)
		{
			if (m_srcBus.autoSignalPlacement() == false)
			{
				m_sizeW = m_srcBus.manualBusSize();
			}
			else
			{
				m_sizeW = 0;
			}

			return true;
		}

		bool result = buildInBusSignalsMap();

		if (result == false)
		{
			return false;
		}

		if (m_srcBus.autoSignalPlacement() == true)
		{
			result = placeSignals();
		}
		else
		{
			result = buildSignalsOrder();
		}

		if (result == false)
		{
			return false;
		}

		result = checkSignalsOffsets();

		if (result == false)
		{
			return false;
		}

		buildSignalIndexesArrays();

		return result;
	}

	void Bus::writeReport(QStringList& list)
	{
		QString separator("-------------------------------------------------------------------");

		list.append(separator);
		list.append(QString("BusTypeID:\t\t%1").arg(m_srcBus.busTypeId()));
		list.append(QString("AutoSignalPlacement:\t%1").arg(m_srcBus.autoSignalPlacement() == true ? "True" : "False"));
		list.append(QString("BusSizeW:\t\t%1").arg(m_sizeW));
		list.append(QString("Signals:\t\t%1").arg(m_signals.count()));
		list.append(separator);

		if (m_signals.count() == 0)
		{
			list.append("");
			return;
		}

		list.append(QString(" OffsetW:Bit   Size\tType\tConvert\t SignalID"));
		list.append(separator);

		for(const BusSignal& s : m_signals)
		{
			list.append(QString("   %1    %2\t%3\t%4\t %5").
							arg(s.inbusAddr.toString(true)).
							arg(s.inbusSizeBits).
							arg(s.signalType == E::SignalType::Analog ? "A" : "D").
							arg(s.conversionRequired() == true ? "Yes" : "No").
							arg(s.signalID));
		}

		list.append(separator);

		list.append("");
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

		for(int i = 0; i < busSignals.size(); i++)
		{
			const VFrame30::BusSignal& busSignal = busSignals[i];

			if (m_inBusSignalsMap.contains(busSignal.signalId()) == true)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			m_inBusSignalsMap.insert(busSignal.signalId(), i);
		}

		return true;
	}

	bool Bus::placeSignals()
	{
		assert(m_srcBus.autoSignalPlacement() == true);

		// calculate bus sizeW
		//
		int discreteSignalsCount = 0;

		int analogSignalsSizeBit = 0;

		QStringList analogSignals;
		QStringList discreteSignals;

		const std::vector<VFrame30::BusSignal>& busSignals = m_srcBus.busSignals();

		for(const VFrame30::BusSignal& busSignal : busSignals)
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

				analogSignalsSizeBit += (busSignal.inbusAnalogSize());

				analogSignals.append(busSignal.signalId());

				break;

			case E::SignalType::Discrete:
				discreteSignalsCount++;

				discreteSignals.append(busSignal.signalId());

				break;

			default:
				assert(false);			// unknown in bus signal type
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

		// bus size calculation
		//
		assert((analogSignalsSizeBit % SIZE_16BIT) == 0);

		m_sizeW = analogSignalsSizeBit / SIZE_16BIT;

		m_sizeW += discreteSignalsCount / SIZE_16BIT;

		if ((discreteSignalsCount % SIZE_16BIT) != 0)
		{
			m_sizeW++;
		}

		// signals ordering by alphabet
		//
		analogSignals.sort(Qt::CaseInsensitive);
		discreteSignals.sort(Qt::CaseInsensitive);

		QStringList signalsOrder = analogSignals;

		signalsOrder.append(discreteSignals);

		// calculate signals addresses in bus
		//
		Address16 inBusAddr(0, 0);

		for(const QString& signalID : signalsOrder)
		{
			VFrame30::BusSignal& srcBusSignal = getBusSignal(signalID);

			BusSignal busSignal;

			busSignal.init(srcBusSignal);
			busSignal.inbusAddr = inBusAddr;

			switch(srcBusSignal.type())
			{
			case E::SignalType::Analog:
				inBusAddr.addBit(busSignal.inbusSizeBits);
				break;

			case E::SignalType::Discrete:
				inBusAddr.addBit(SIZE_1BIT);
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

		if ((m_srcBus.manualBusSize() % WORD_SIZE_IN_BYTES) != 0)
		{
			// Bus size must be multiple of 2 bytes (bus type %1).
			//
			m_log->errALC5099(m_srcBus.busTypeId());
			return false;
		}

		m_sizeW = m_srcBus.manualBusSize() / WORD_SIZE_IN_BYTES;

		QVector<QPair<QString, int>> inBusSignals;		// QPair<busSignalID, signalBitAddressInBus>

		const std::vector<VFrame30::BusSignal>& busSignals = m_srcBus.busSignals();

		for(const VFrame30::BusSignal& busSignal : busSignals)
		{
			Address16 inBusAddr(0, 0);

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

			case E::SignalType::Discrete:
				inBusAddr.addBit(busSignal.inbusOffset() * SIZE_8BIT + busSignal.inbusDiscreteBitNo());
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

			busSignal.init(srcBusSignal);

			Address16 inBusAddr(0, 0);

			switch(srcBusSignal.type())
			{
			case E::SignalType::Analog:
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

	bool Bus::checkSignalsOffsets()
	{
		bool result = true;

		int count = m_signals.count();

		int maxBitAddress = m_sizeW * SIZE_16BIT;

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
		}

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

		return result;
	}

	void Bus::buildSignalIndexesArrays()
	{
		m_analogSignalIndexes.clear();
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
		assert(m_inBusSignalsMap.size() == m_srcBus.busSignals().size());

		int index = m_inBusSignalsMap.value(signalID, -1);

		if (index < 0 || index >= m_srcBus.busSignals().size())
		{
			return m_invalidBusSignal;
		}

		return m_srcBus.busSignals()[index];
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

			BusShared bus = std::make_shared<Bus>(srcBus, m_log);

			if (bus->init() == true)
			{
				m_busses.insert(srcBus.busTypeId(), bus);
			}
			else
			{
				result = false;
			}
		}

		return result;
	}

	bool Busses::writeReport(BuildResultWriter* resultWriter)
	{
		TEST_PTR_RETURN_FALSE(resultWriter);

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


}
