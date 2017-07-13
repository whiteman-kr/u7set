#include "../Builder/LmMemoryMap.h"
#include "../lib/WUtils.h"

namespace Builder
{

	// ---------------------------------------------------------------------------------
	//
	//	MemoryArea class implementation
	//
	// ---------------------------------------------------------------------------------

	void MemoryArea::setStartAddress(int startAddress)
	{
		assert(m_locked == false);
		m_startAddress = startAddress;
		m_nextSignalAddress.set(startAddress, 0);
	}


	MemoryArea& MemoryArea::operator = (const MemoryArea& ma)
	{
		assert(m_locked == false);

		m_startAddress = ma.m_startAddress;
		m_sizeW = ma.m_sizeW;

		return *this;
	}


	Address16 MemoryArea::appendSignal(const Signal& signal)
	{
		Address16 signalAddress;

		if (signal.isAnalog())
		{
			// do word-align
			//
			m_nextSignalAddress.wordAlign();
		}

		signalAddress = m_nextSignalAddress;

		switch(signal.signalType())
		{
		case E::SignalType::Analog:
			m_nextSignalAddress.addWord(signal.sizeW());
			break;

		case E::SignalType::Discrete:
			m_nextSignalAddress.add1Bit();
			break;

		case E::SignalType::Bus:
			m_nextSignalAddress.addWord(signal.sizeW());
			break;

		default:
			assert(false);
		}

		m_signals.append(SignalAddress16(signal.appSignalID(), signalAddress, signal.sizeW(), signal.isDiscrete()));

		m_sizeW = m_nextSignalAddress.offset() - m_startAddress;

		if (m_nextSignalAddress.bit() > 0)
		{
			m_sizeW += 1;
		}

		return signalAddress;
	}


	// ---------------------------------------------------------------------------------
	//
	//	LmMemoryMap class implementation
	//
	// ---------------------------------------------------------------------------------

	LmMemoryMap::LmMemoryMap(IssueLogger *log) :
		m_log(log)
	{
		assert(m_log != nullptr);

		m_memory.resize(APP_MEMORY_SIZE);
	}


	bool LmMemoryMap::init(	const MemoryArea& moduleData,
							const MemoryArea& optoInterfaceData,
							const MemoryArea& appLogicBitData,
							const MemoryArea& tuningData,
							const MemoryArea& appLogicWordData)
	{

		// init modules memory mapping
		//
		m_modules.memory.setStartAddress(moduleData.startAddress());
		m_modules.memory.setSizeW(moduleData.sizeW() * MODULES_COUNT);
		m_modules.memory.lock();

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			m_modules.module[i].setStartAddress(m_modules.memory.startAddress() + i * moduleData.sizeW());
			m_modules.module[i].setSizeW(moduleData.sizeW());
		}

		// init opto interface memory mapping
		//
		m_optoInterface.memory.setStartAddress(optoInterfaceData.startAddress());
		m_optoInterface.memory.setSizeW(optoInterfaceData.sizeW() * (OPTO_INTERFACE_COUNT + 1));
		m_optoInterface.memory.lock();

		for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
		{
			if (i == 0)
			{
				m_optoInterface.channel[i].setStartAddress(m_optoInterface.memory.startAddress());
			}
			else
			{
				m_optoInterface.channel[i].setStartAddress(m_optoInterface.channel[i-1].nextAddress());
			}

			m_optoInterface.channel[i].setSizeW(optoInterfaceData.sizeW());
		}

		m_optoInterface.reserv.setStartAddress(m_optoInterface.channel[OPTO_INTERFACE_COUNT -1].nextAddress());
		m_optoInterface.reserv.setSizeW(optoInterfaceData.sizeW());

		// init application bit-addressed memory mapping
		//
		m_appBitAdressed.memory = appLogicBitData;
		m_appBitAdressed.memory.lock();

		m_appBitAdressed.bitAccumulator.setStartAddress(appLogicBitData.startAddress());
		m_appBitAdressed.bitAccumulator.setSizeW(1);        // bit accumulator has 1 word (16 bit) size

		m_appBitAdressed.acquiredDiscreteOutputSignals.setStartAddress(appLogicBitData.startAddress());
		m_appBitAdressed.acquiredDiscreteInternalSignals.setStartAddress(appLogicBitData.startAddress());

		m_appBitAdressed.nonAcquiredDiscreteOutputSignals.setStartAddress(appLogicBitData.startAddress());
		m_appBitAdressed.nonAcquiredDiscreteInternalSignals.setStartAddress(appLogicBitData.startAddress());

		// init tuning interface memory mapping
		//
		m_tuningInterface.memory = tuningData;
		m_tuningInterface.memory.lock();

		// init application word-addressed memory mapping
		//
		m_appWordAdressed.memory = appLogicWordData;
		m_appWordAdressed.memory.lock();

		m_appWordAdressed.acquiredRawData.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.acquiredAnalogInputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogOutputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogInternalSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogTuningSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.acquiredBuses.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.acquiredDiscreteInputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteOutputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteInternalSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteTuningSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.nonAcquiredAnalogInputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.nonAcquiredAnalogOutputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.nonAcquiredAnalogInternalSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.nonAcquiredBuses.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.wordAccumulator.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.wordAccumulator.setSizeW(1);        // word accumulator has 1 word size

		return recalculateAddresses();
	}


	bool LmMemoryMap::recalculateAddresses()
	{
		// recalc application bit-addressed memory mapping
		//
		m_appBitAdressed.bitAccumulator.setStartAddress(m_appBitAdressed.memory.startAddress());
		m_appBitAdressed.bitAccumulator.setSizeW(1);

		m_appBitAdressed.acquiredDiscreteOutputSignals.setStartAddress(m_appBitAdressed.bitAccumulator.nextAddress());
		m_appBitAdressed.acquiredDiscreteInternalSignals.setStartAddress(m_appBitAdressed.acquiredDiscreteOutputSignals.nextAddress());

		m_appBitAdressed.nonAcquiredDiscreteOutputSignals.setStartAddress(m_appBitAdressed.acquiredDiscreteInternalSignals.nextAddress());
		m_appBitAdressed.nonAcquiredDiscreteInternalSignals.setStartAddress(m_appBitAdressed.nonAcquiredDiscreteOutputSignals.nextAddress());

		if (m_appBitAdressed.nonAcquiredDiscreteInternalSignals.nextAddress() > m_appBitAdressed.memory.nextAddress())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Out of bit-addressed memory range!"));

			return false;
		}

		// recalc application word-addressed memory mapping
		//
		m_appWordAdressed.acquiredRawData.setStartAddress(m_appWordAdressed.memory.startAddress());

		m_appWordAdressed.acquiredAnalogInputSignals.setStartAddress(m_appWordAdressed.acquiredRawData.nextAddress());
		m_appWordAdressed.acquiredAnalogOutputSignals.setStartAddress(m_appWordAdressed.acquiredAnalogInputSignals.nextAddress());
		m_appWordAdressed.acquiredAnalogInternalSignals.setStartAddress(m_appWordAdressed.acquiredAnalogOutputSignals.nextAddress());
		m_appWordAdressed.acquiredAnalogTuningSignals.setStartAddress(m_appWordAdressed.acquiredAnalogInternalSignals.nextAddress());

		m_appWordAdressed.acquiredBuses.setStartAddress(m_appWordAdressed.acquiredAnalogTuningSignals.nextAddress());

		m_appWordAdressed.acquiredDiscreteInputSignals.setStartAddress(m_appWordAdressed.acquiredBuses.nextAddress());
		m_appWordAdressed.acquiredDiscreteOutputSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteInputSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteInternalSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteOutputSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteTuningSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteInternalSignals.nextAddress());

		m_appWordAdressed.nonAcquiredAnalogInputSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteTuningSignals.nextAddress());
		m_appWordAdressed.nonAcquiredAnalogOutputSignals.setStartAddress(m_appWordAdressed.nonAcquiredAnalogInputSignals.nextAddress());
		m_appWordAdressed.nonAcquiredAnalogInternalSignals.setStartAddress(m_appWordAdressed.nonAcquiredAnalogOutputSignals.nextAddress());

		m_appWordAdressed.nonAcquiredBuses.setStartAddress(m_appWordAdressed.nonAcquiredAnalogInternalSignals.nextAddress());

		m_appWordAdressed.wordAccumulator.setStartAddress(m_appWordAdressed.nonAcquiredBuses.nextAddress());
		m_appWordAdressed.wordAccumulator.setSizeW(1);

		if (m_appWordAdressed.wordAccumulator.nextAddress() > m_appWordAdressed.memory.nextAddress())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Out of word-addressed memory range!"));

			return false;
		}

		return true;
	}


	int LmMemoryMap::getModuleDataOffset(int place) const
	{
		assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

		return m_modules.module[place - 1].startAddress();;
	}


	int LmMemoryMap::getRegBufStartAddr() const
	{
		return m_appWordAdressed.memory.startAddress();
	}


	void LmMemoryMap::getFile(QStringList& memFile)
	{
		memFile.append(QString(" LM's memory map"));
		memFile.append("");

		//

		addSection(memFile, m_modules.memory, "I/O modules controller memory");

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			addRecord(memFile, m_modules.module[i], QString().sprintf("I/O module %02d", i + 1));
		}

		memFile.append("");

		addSection(memFile, m_optoInterface.memory, "Opto interfaces memory");

		//

		for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
		{
			addRecord(memFile, m_optoInterface.channel[i], QString().sprintf("opto port %02d", i + 1));
		}

		memFile.append("");

		addRecord(memFile, m_optoInterface.reserv, "reserv");

		memFile.append("");

		//

		addSection(memFile, m_appBitAdressed.memory, "Application logic bit-addressed memory");

		addRecord(memFile, m_appBitAdressed.bitAccumulator, "bit accumulator");

		memFile.append("");

		addRecord(memFile, m_appBitAdressed.acquiredDiscreteOutputSignals, "acquired discrete output signals");
		memFile.append("");
		addSignals(memFile, m_appBitAdressed.acquiredDiscreteOutputSignals);

		addRecord(memFile, m_appBitAdressed.acquiredDiscreteInternalSignals, "acquired discrete internal signals");
		memFile.append("");
		addSignals(memFile, m_appBitAdressed.acquiredDiscreteInternalSignals);

		addRecord(memFile, m_appBitAdressed.nonAcquiredDiscreteOutputSignals, "non acquired discrete output signals");
		memFile.append("");
		addSignals(memFile, m_appBitAdressed.nonAcquiredDiscreteOutputSignals);

		addRecord(memFile, m_appBitAdressed.nonAcquiredDiscreteInternalSignals, "non acquired discrete internal signals");
		memFile.append("");
		addSignals(memFile, m_appBitAdressed.nonAcquiredDiscreteInternalSignals);

		//

		addSection(memFile, m_tuningInterface.memory, "Tuning interface memory");
		memFile.append("");

		//

		addSection(memFile, m_appWordAdressed.memory, "Application logic word-addressed memory", m_appWordAdressed.memory.startAddress());

		addRecord(memFile, m_appWordAdressed.acquiredRawData, "acquired raw data");
		memFile.append("");

		addRecord(memFile, m_appWordAdressed.acquiredAnalogInputSignals, "acquired analog input signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredAnalogInputSignals);

		addRecord(memFile, m_appWordAdressed.acquiredAnalogOutputSignals, "acquired analog output signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredAnalogOutputSignals);

		addRecord(memFile, m_appWordAdressed.acquiredAnalogInternalSignals, "acquired analog internal signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredAnalogInternalSignals);

		addRecord(memFile, m_appWordAdressed.acquiredAnalogTuningSignals, "acquired analog tuningable signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredAnalogTuningSignals);

		addRecord(memFile, m_appWordAdressed.acquiredBuses, "acquired buses");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredBuses);

		addRecord(memFile, m_appWordAdressed.acquiredDiscreteInputSignals, "acquired discrete input signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredDiscreteInputSignals);

		addRecord(memFile, m_appWordAdressed.acquiredDiscreteOutputSignals, "acquired discrete output signals (from bit memory)");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredDiscreteOutputSignals);

		addRecord(memFile, m_appWordAdressed.acquiredDiscreteInternalSignals, "acquired discrete internal signals (from bit memory)");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredDiscreteInternalSignals);

		addRecord(memFile, m_appWordAdressed.acquiredDiscreteTuningSignals, "acquired discrete tuningable signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.acquiredDiscreteTuningSignals);

		addRecord(memFile, m_appWordAdressed.nonAcquiredAnalogInputSignals, "non acquired analog input signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.nonAcquiredAnalogInputSignals);

		addRecord(memFile, m_appWordAdressed.nonAcquiredAnalogOutputSignals, "non acquired analog output signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.nonAcquiredAnalogOutputSignals);

		addRecord(memFile, m_appWordAdressed.nonAcquiredAnalogInternalSignals, "non acquired analog internal signals");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.nonAcquiredAnalogInternalSignals);

		addRecord(memFile, m_appWordAdressed.nonAcquiredBuses, "non acquired buses");
		memFile.append("");
		addSignals(memFile, m_appWordAdressed.nonAcquiredBuses);

		addRecord(memFile, m_appWordAdressed.wordAccumulator, "word accumulator");
		memFile.append("");
	}

	void LmMemoryMap::addSection(QStringList& memFile, MemoryArea& memArea, const QString& title, int sectionStartAddrW)
	{
		m_sectionStartAddrW = sectionStartAddrW;

		memFile.append(QString().rightJustified(80, '-'));
		memFile.append(QString(" Address   Offset    Size      Description"));
		memFile.append(QString().rightJustified(80, '-'));

		QString str;
		str.sprintf(" %05d               %05d     %s", memArea.startAddress(), memArea.sizeW(), C_STR(title));

		memFile.append(str);
		memFile.append(QString().rightJustified(80, '-'));
		memFile.append("");
	}


	void LmMemoryMap::addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title)
	{
		QString str;

		if (m_sectionStartAddrW == -1)
		{
			str.sprintf(" %05d               %05d     %s",
						memArea.startAddress(), memArea.sizeW(), C_STR(title));
		}
		else
		{
			str.sprintf(" %05d     %05d     %05d     %s",
						memArea.startAddress(),
						memArea.startAddress() - m_sectionStartAddrW,
						memArea.sizeW(), C_STR(title));
		}

		memFile.append(str);
	}


	void LmMemoryMap::addSignals(QStringList& memFile, MemoryArea& memArea)
	{
		if (memArea.hasSignals() == false)
		{
			return;
		}

		QVector<MemoryArea::SignalAddress16>& signalsArray = memArea.getSignals();

		for(MemoryArea::SignalAddress16& signal : signalsArray)
		{
			QString str;

			if (signal.isDiscrete())
			{
				if (m_sectionStartAddrW == -1)
				{
					str.sprintf(" %05d.%02d            00000.01  - %s",
								signal.address().offset(), signal.address().bit(),
								C_STR(signal.signalStrID()));
				}
				else
				{
					str.sprintf(" %05d.%02d  %05d.%02d  00000.01  - %s",
								signal.address().offset(),
								signal.address().bit(),
								signal.address().offset() - m_sectionStartAddrW,
								signal.address().bit(),
								C_STR(signal.signalStrID()));
				}
			}
			else
			{
				if (m_sectionStartAddrW == -1)
				{
					str.sprintf(" %05d               %05d     - %s",
								signal.address().offset(),
								signal.sizeW(),
								C_STR(signal.signalStrID()));
				}
				else
				{
					str.sprintf(" %05d     %05d     %05d     - %s",
								signal.address().offset(),
								signal.address().offset() - m_sectionStartAddrW,
								signal.sizeW(),
								C_STR(signal.signalStrID()));
				}
			}

			memFile.append(str);
		}

		memFile.append("");
	}

	Address16 LmMemoryMap::appendAcquiredDiscreteOutputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isOutput() == true);

		return m_appBitAdressed.acquiredDiscreteOutputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredDiscreteInternalSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isInternal() == true);

		return m_appBitAdressed.acquiredDiscreteInternalSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredDiscreteOutputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isDiscrete() == true &&
			   signal.isOutput() == true);

		return m_appBitAdressed.nonAcquiredDiscreteOutputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredDiscreteInternalSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isDiscrete() == true &&
			   signal.isInternal() == true);

		return m_appBitAdressed.nonAcquiredDiscreteInternalSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::setAcquiredRawDataSize(int sizeW)
	{
		m_appWordAdressed.acquiredRawData.setSizeW(sizeW);

		return Address16(m_appWordAdressed.acquiredRawData.startAddress(), 0);
	}

	Address16 LmMemoryMap::appendAcquiredAnalogInputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isAnalog() == true &&
			   signal.isInput() == true);

		return m_appWordAdressed.acquiredAnalogInputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredAnalogOutputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isAnalog() == true &&
			   signal.isOutput() == true);

		return m_appWordAdressed.acquiredAnalogOutputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredAnalogInternalSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isAnalog() == true &&
			   signal.isInternal() == true);

		return m_appWordAdressed.acquiredAnalogInternalSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredAnalogTuningSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isAnalog() == true &&
			   signal.isInternal() == true &&
			   signal.enableTuning() == true);

		return m_appWordAdressed.acquiredAnalogTuningSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredBus(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isBus() == true);

		return m_appWordAdressed.acquiredBuses.appendSignal(signal);
	}


	Address16 LmMemoryMap::appendAcquiredDiscreteInputSignalInRegBuf(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isInput() == true);

		return m_appWordAdressed.acquiredDiscreteInputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredDiscreteOutputSignalInRegBuf(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isOutput() == true);

		return m_appWordAdressed.acquiredDiscreteOutputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredDiscreteInternalSignalInRegBuf(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isInternal() == true);

		return m_appWordAdressed.acquiredDiscreteInternalSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendAcquiredDiscreteTuningSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == true &&
			   signal.isDiscrete() == true &&
			   signal.isInternal() == true &&
			   signal.enableTuning() == true);

		return m_appWordAdressed.acquiredDiscreteTuningSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredAnalogInputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isAnalog() == true &&
			   signal.isInput() == true);

		return m_appWordAdressed.nonAcquiredAnalogInputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredAnalogOutputSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isAnalog() == true &&
			   signal.isOutput() == true);

		return m_appWordAdressed.nonAcquiredAnalogOutputSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredAnalogInternalSignal(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isAnalog() == true &&
			   signal.isInternal() == true);

		return m_appWordAdressed.nonAcquiredAnalogInternalSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::appendNonAcquiredBus(const Signal& signal)
	{
		assert(signal.isAcquired() == false &&
			   signal.isBus() == true);

		return m_appWordAdressed.nonAcquiredBuses.appendSignal(signal);
	}

	double LmMemoryMap::bitAddressedMemoryUsed()
	{
		return double((m_appBitAdressed.nonAcquiredDiscreteInternalSignals.nextAddress() -
					   m_appBitAdressed.memory.startAddress()) * 100) /
					   double(m_appBitAdressed.memory.sizeW());
	}

	double LmMemoryMap::wordAddressedMemoryUsed()
	{
		return double((m_appWordAdressed.nonAcquiredBuses.nextAddress() -
					   m_appWordAdressed.memory.startAddress()) * 100) /
				double(m_appWordAdressed.memory.sizeW());
	}

	int LmMemoryMap::regBufSizeW() const
	{
		return m_appWordAdressed.acquiredDiscreteTuningSignals.nextAddress() - m_appWordAdressed.acquiredRawData.startAddress();
	}

	bool LmMemoryMap::read16(int address)
	{
		if (address < 0 || address >= APP_MEMORY_SIZE)
		{
			// Read address %1 of application memory is out of range 0..65535.
			//
			m_log->errALC5064(address);
			return false;
		}

		m_memory[address].readCount++;

		return true;
	}


	bool LmMemoryMap::read32(int address)
	{
		return readArea(address, 2);
	}


	bool LmMemoryMap::readArea(int startAddress, int size)
	{
		bool result = true;

		for(int i = 0; i < size ; i++)
		{
			result &= read16(startAddress + i);
		}

		return result;
	}


	bool LmMemoryMap::write16(int address)
	{
		if (address < 0 || address >= APP_MEMORY_SIZE)
		{
			// Write address %1 of application memory is out of range 0..65535.
			//
			m_log->errALC5065(address);
			return false;
		}

		m_memory[address].writeCount++;

		return true;
	}


	bool LmMemoryMap::write32(int address)
	{
		return writeArea(address, 2);
	}


	bool LmMemoryMap::writeArea(int startAddress, int size)
	{
		bool result = true;

		for(int i = 0; i < size ; i++)
		{
			result &= write16(startAddress + i);
		}

		return result;
	}


	int LmMemoryMap::getMemoryReadCount(int address) const
	{
		if (address < 0 || address >= APP_MEMORY_SIZE)
		{
			assert(false);
			return 0;
		}

		return m_memory[address].readCount;
	}


	int LmMemoryMap::getMemoryWriteCount(int address) const
	{
		if (address < 0 || address >= APP_MEMORY_SIZE)
		{
			assert(false);
			return 0;
		}

		return m_memory[address].writeCount;
	}
}

