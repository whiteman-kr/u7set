#include "../Builder/LmMemoryMap.h"
#include "../lib/WUtils.h"

#include "UalItems.h"


namespace Builder
{

	void MemoryArea::SignalAddress16::appendSignalID(const QString& signalID)
	{
		if (m_signalID.isEmpty() == true)
		{
			m_signalID = signalID;
		}
		else
		{
			m_signalID += ", " + signalID;
		}
	}

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

	Address16 MemoryArea::appendSignal(const UalSignal* ualSignal, bool appendAcquiredOnly)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return Address16();
		}

		Address16 signalAddress;

		if (ualSignal->isAnalog())
		{
			// do word-align
			//
			m_nextSignalAddress.wordAlign();
		}

		signalAddress = m_nextSignalAddress;

		switch(ualSignal->signalType())
		{
		case E::SignalType::Analog:
			m_nextSignalAddress.addWord(ualSignal->sizeW());
			break;

		case E::SignalType::Discrete:
			m_nextSignalAddress.add1Bit();
			break;

		case E::SignalType::Bus:
			m_nextSignalAddress.addWord(ualSignal->sizeW());
			break;

		default:
			assert(false);
		}

		appendUalRefSignals(signalAddress, ualSignal, appendAcquiredOnly);

		m_sizeW = m_nextSignalAddress.offset() - m_startAddress;

		if (m_nextSignalAddress.bit() > 0)
		{
			m_sizeW += 1;
		}

		return signalAddress;
	}

	QVector<MemoryArea::SignalAddress16> MemoryArea::getSignalsJoined() const
	{
		QVector<SignalAddress16> array;

		SignalAddress16 joined;

		for(const SignalAddress16& sa : m_signals)
		{
			if (joined.address() != sa.address() || joined.isDiscrete() != sa.isDiscrete())
			{
				if (joined.address().isValid() == true)
				{
					array.append(joined);
				}

				joined.setSignalStrID(sa.signalStrID());
				joined.setAddress(sa.address());
				joined.setDiscrete(sa.isDiscrete());
			}
			else
			{
				joined.appendSignalID(sa.signalStrID());
			}
		}

		if (joined.address().isValid())
		{
			array.append(joined);
		}

		return array;
	}

	void MemoryArea::appendUalRefSignals(const Address16& addr16, const UalSignal* ualSignal, bool appendAcquiredOnly)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return;
		}

		const QVector<Signal*>& refSignals = ualSignal->refSignals();

		for(const Signal* s : refSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			if (appendAcquiredOnly == true)
			{
				if (s->isAcquired() == true)
				{
					m_signals.append(SignalAddress16(s->appSignalID(), addr16, s->sizeW(), s->isDiscrete()));
				}
			}
			else
			{
				m_signals.append(SignalAddress16(s->appSignalID(), addr16, s->sizeW(), s->isDiscrete()));
			}
		}
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
	}


	bool LmMemoryMap::init(	int appMemorySize,
							const MemoryArea& moduleData,
							const MemoryArea& optoInterfaceData,
							const MemoryArea& appLogicBitData,
							const MemoryArea& tuningData,
							const MemoryArea& appLogicWordData)
	{
		m_appMemorySize = appMemorySize;

		m_memory.resize(m_appMemorySize);

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
		m_appBitAdressed.bitAccumulator.setSizeW(2);        // bit accumulator has 2 word (32bit) size

		m_appBitAdressed.constBits.setStartAddress(appLogicBitData.startAddress());
		m_appBitAdressed.constBits.setSizeW(2);				// const bits: bit 0 == 0, bit 1 == 1

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
		m_appWordAdressed.acquiredAnalogOptoSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogBusChildSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogTuningSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredAnalogConstSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.acquiredBuses.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.acquiredDiscreteInputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteOutputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteInternalSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteOptoAndBusChildSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteTuningSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.acquiredDiscreteConstSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.nonAcquiredAnalogInputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.nonAcquiredAnalogOutputSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.nonAcquiredAnalogInternalSignals.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.nonAcquiredBuses.setStartAddress(appLogicWordData.startAddress());

		m_appWordAdressed.wordAccumulator.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.wordAccumulator.setSizeW(2);        // word accumulator has 2 word size

		return recalculateAddresses();
	}


	bool LmMemoryMap::recalculateAddresses()
	{
		// recalc application bit-addressed memory mapping
		//
		m_appBitAdressed.bitAccumulator.setStartAddress(m_appBitAdressed.memory.startAddress());
		m_appBitAdressed.bitAccumulator.setSizeW(2);			// bit accumulator 32 bit size

		m_appBitAdressed.constBits.setStartAddress(m_appBitAdressed.bitAccumulator.nextAddress());
		m_appBitAdressed.constBits.setSizeW(1);					// const bits: bit 0 == 0, bit 1 == 1

		m_appBitAdressed.acquiredDiscreteOutputSignals.setStartAddress(m_appBitAdressed.constBits.nextAddress());
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
		m_appWordAdressed.acquiredAnalogOptoSignals.setStartAddress(m_appWordAdressed.acquiredAnalogInternalSignals.nextAddress());
		m_appWordAdressed.acquiredAnalogBusChildSignals.setStartAddress(m_appWordAdressed.acquiredAnalogOptoSignals.nextAddress());
		m_appWordAdressed.acquiredAnalogTuningSignals.setStartAddress(m_appWordAdressed.acquiredAnalogBusChildSignals.nextAddress());
		m_appWordAdressed.acquiredAnalogConstSignals.setStartAddress(m_appWordAdressed.acquiredAnalogTuningSignals.nextAddress());

		m_appWordAdressed.acquiredBuses.setStartAddress(m_appWordAdressed.acquiredAnalogConstSignals.nextAddress());

		m_appWordAdressed.acquiredDiscreteInputSignals.setStartAddress(m_appWordAdressed.acquiredBuses.nextAddress());
		m_appWordAdressed.acquiredDiscreteOutputSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteInputSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteInternalSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteOutputSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteOptoAndBusChildSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteInternalSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteTuningSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteOptoAndBusChildSignals.nextAddress());
		m_appWordAdressed.acquiredDiscreteConstSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteTuningSignals.nextAddress());

		m_appWordAdressed.nonAcquiredAnalogInputSignals.setStartAddress(m_appWordAdressed.acquiredDiscreteConstSignals.nextAddress());
		m_appWordAdressed.nonAcquiredAnalogOutputSignals.setStartAddress(m_appWordAdressed.nonAcquiredAnalogInputSignals.nextAddress());
		m_appWordAdressed.nonAcquiredAnalogInternalSignals.setStartAddress(m_appWordAdressed.nonAcquiredAnalogOutputSignals.nextAddress());

		m_appWordAdressed.nonAcquiredBuses.setStartAddress(m_appWordAdressed.nonAcquiredAnalogInternalSignals.nextAddress());

		m_appWordAdressed.wordAccumulator.setStartAddress(m_appWordAdressed.nonAcquiredBuses.nextAddress());
		m_appWordAdressed.wordAccumulator.setSizeW(2);

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
		addRecord(memFile, m_appBitAdressed.constBits, "const bits");

		memFile.append("");

		addRecordSignals(memFile, m_appBitAdressed.acquiredDiscreteOutputSignals, "acquired discrete output signals");
		addRecordSignals(memFile, m_appBitAdressed.acquiredDiscreteInternalSignals, "acquired discrete internal signals");
		addRecordSignals(memFile, m_appBitAdressed.nonAcquiredDiscreteOutputSignals, "non acquired discrete output signals");
		addRecordSignals(memFile, m_appBitAdressed.nonAcquiredDiscreteInternalSignals, "non acquired discrete internal signals");

		//

		addSection(memFile, m_tuningInterface.memory, "Tuning interface memory");
		memFile.append("");

		//

		addSection(memFile, m_appWordAdressed.memory, "Application logic word-addressed memory", m_appWordAdressed.memory.startAddress());

		if (m_appWordAdressed.acquiredRawData.sizeW() > 0)
		{
			addRecord(memFile, m_appWordAdressed.acquiredRawData, "acquired raw data");
			memFile.append("");
		}

		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogInputSignals, "acquired analog input signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogOutputSignals, "acquired analog output signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogInternalSignals, "acquired analog internal signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogOptoSignals, "acquired analog opto signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogBusChildSignals, "acquired analog bus child signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogTuningSignals, "acquired analog tunable signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredAnalogConstSignals, "acquired analog const signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredBuses, "acquired buses");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteInputSignals, "acquired discrete input signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteOutputSignals, "acquired discrete output signals (from bit memory)");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteInternalSignals, "acquired discrete internal signals (from bit memory)");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteOptoAndBusChildSignals, "acquired discrete opto and bus child signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteTuningSignals, "acquired discrete tunable signals");
		addRecordSignals(memFile, m_appWordAdressed.acquiredDiscreteConstSignals, "acquired discrete const signals");
		addRecordSignals(memFile, m_appWordAdressed.nonAcquiredAnalogInputSignals, "non acquired analog input signals");
		addRecordSignals(memFile, m_appWordAdressed.nonAcquiredAnalogOutputSignals, "non acquired analog output signals");
		addRecordSignals(memFile, m_appWordAdressed.nonAcquiredAnalogInternalSignals, "non acquired analog internal signals");
		addRecordSignals(memFile, m_appWordAdressed.nonAcquiredBuses, "non acquired buses");

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

	void LmMemoryMap::addRecordSignals(QStringList& memFile, MemoryArea& memArea, const QString& title)
	{
		if (memArea.sizeW() == 0)
		{
			return;
		}

		addRecord(memFile, memArea, title);
		memFile.append("");
		addSignals(memFile, memArea);
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

		QVector<MemoryArea::SignalAddress16> signalsArray = memArea.getSignalsJoined();

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

	bool LmMemoryMap::appendUalSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		for(UalSignal* ualSignal : ualSignals)
		{
			if (ualSignal == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			Address16 addr = memArea.appendSignal(ualSignal, false);

			ualSignal->setUalAddr(addr);
		}

		return result;
	}

	bool LmMemoryMap::appendRegSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals, bool setUalAddrEqualToRegBufAddr)
	{
		bool result = true;

		for(UalSignal* ualSignal : ualSignals)
		{
			if (ualSignal == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			Address16 addr = memArea.appendSignal(ualSignal, true);

			if (setUalAddrEqualToRegBufAddr == true)
			{
				assert(ualSignal->ualAddr().isValid() == false);			//	checking that ualAddr is not set early
				ualSignal->setUalAddr(addr);
			}

			bool res = ualSignal->setRegBufAddr(addr);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}

			addr.addWord(-m_appWordAdressed.memory.startAddress());			// minus is OK!

			ualSignal->setRegValueAddr(addr);
		}

		return result;
	}

	bool LmMemoryMap::appendRegAnalogConstSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		bool first = true;

		Address16 addrOfConst;

		for(UalSignal* ualSignal : ualSignals)
		{
			if (ualSignal == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			if (first == true)
			{
				addrOfConst = memArea.appendSignal(ualSignal, true);			// allocate memory for const value
				first = false;
			}
			else
			{
				memArea.appendUalRefSignals(addrOfConst, ualSignal, true);		// append ref only
			}

			Address16 addr = addrOfConst;

			ualSignal->setRegBufAddr(addr);

			addr.addWord(-m_appWordAdressed.memory.startAddress());			// minus is OK!

			ualSignal->setRegValueAddr(addr);
		}

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appBitAdressed.acquiredDiscreteOutputSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appBitAdressed.acquiredDiscreteInternalSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appBitAdressed.nonAcquiredDiscreteOutputSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appBitAdressed.nonAcquiredDiscreteInternalSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredDiscreteInputSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredDiscreteOutputSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredDiscreteInternalSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteOptoAndBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredDiscreteOptoAndBusChildSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredDiscreteTuningSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogTuningSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredDiscreteConstSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		Address16 const0Addr(m_appWordAdressed.acquiredDiscreteConstSignals.startAddress(), 0);
		Address16 const1Addr(m_appWordAdressed.acquiredDiscreteConstSignals.startAddress(), 1);

		for(UalSignal* ualSignal : ualSignals)
		{
			if (ualSignal == nullptr || ualSignal->isConst() == false || ualSignal->isDiscrete() == false)
			{
				assert(false);
				result = false;
				continue;
			}

			if (m_appWordAdressed.acquiredDiscreteConstSignals.sizeW() == 0)
			{
				m_appWordAdressed.acquiredDiscreteConstSignals.setSizeW(1);			// always 1 word!
			}

			Address16 addr;

			if (ualSignal->constDiscreteValue() == 0)
			{
				addr = const0Addr;
			}
			else
			{
				addr = const1Addr;
			}

			m_appWordAdressed.acquiredDiscreteConstSignals.appendUalRefSignals(addr, ualSignal, true);

			ualSignal->setRegBufAddr(addr);

			addr.addWord(-m_appWordAdressed.memory.startAddress());			// minus is OK!

			ualSignal->setRegValueAddr(addr);
		}

		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogInputSignals, ualSignals, true);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogOutputSignals, ualSignals, true);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogInternalSignals, ualSignals, true);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogOptoSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogOptoSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredAnalogBusChildSignals, ualSignals, false);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredAnalogConstSignalsInRegBuf(const QHash<int, UalSignal*>& acquiredAnalogConstIntSignals,
															   const QHash<float, UalSignal*>& acquiredAnalogConstFloatSignals)
	{
		bool result = true;

		QVector<int> sortedIntConsts = QVector<int>::fromList(acquiredAnalogConstIntSignals.uniqueKeys());

		qSort(sortedIntConsts);

		for(int intConst : sortedIntConsts)
		{
			result &= appendRegAnalogConstSignals(m_appWordAdressed.acquiredAnalogConstSignals,
									   QVector<UalSignal*>::fromList(acquiredAnalogConstIntSignals.values(intConst)));
		}

		QVector<float> sortedFloatConsts = QVector<float>::fromList(acquiredAnalogConstFloatSignals.uniqueKeys());

		qSort(sortedFloatConsts);

		for(float floatConst : sortedFloatConsts)
		{
			result &= appendRegAnalogConstSignals(m_appWordAdressed.acquiredAnalogConstSignals,
									   QVector<UalSignal*>::fromList(acquiredAnalogConstFloatSignals.values(floatConst)));
		}

		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendAcquiredBussesInRegBuf(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendRegSignals(m_appWordAdressed.acquiredBuses, ualSignals, true);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredAnalogInputSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appWordAdressed.nonAcquiredAnalogInputSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredAnalogStrictOutputSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appWordAdressed.nonAcquiredAnalogOutputSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredAnalogInternalSignals(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appWordAdressed.nonAcquiredAnalogInternalSignals, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	bool LmMemoryMap::appendNonAcquiredBusses(const QVector<UalSignal*>& ualSignals)
	{
		bool result = true;

		result &= appendUalSignals(m_appWordAdressed.nonAcquiredBuses, ualSignals);
		result &= recalculateAddresses();

		return result;
	}

	Address16 LmMemoryMap::setAcquiredRawDataSize(int sizeW)
	{
		m_appWordAdressed.acquiredRawData.setSizeW(sizeW);

		return Address16(m_appWordAdressed.acquiredRawData.startAddress(), 0);
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
		return m_appWordAdressed.nonAcquiredAnalogInputSignals.startAddress() - m_appWordAdressed.acquiredRawData.startAddress();
	}

	bool LmMemoryMap::read16(int address)
	{
		if (address < 0 || address >= m_appMemorySize)
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
		if (address < 0 || address >= m_appMemorySize)
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
		if (address < 0 || address >= m_appMemorySize)
		{
			assert(false);
			return 0;
		}

		return m_memory[address].readCount;
	}


	int LmMemoryMap::getMemoryWriteCount(int address) const
	{
		if (address < 0 || address >= m_appMemorySize)
		{
			assert(false);
			return 0;
		}

		return m_memory[address].writeCount;
	}

	Address16 LmMemoryMap::constBit0Addr() const
	{
		return Address16(m_appBitAdressed.constBits.startAddress(), 0);
	}

	Address16 LmMemoryMap::constBit1Addr() const
	{
		return Address16(m_appBitAdressed.constBits.startAddress(), 1);
	}

	bool LmMemoryMap::addressInBitMemory(int address) const
	{
		if (address >= appBitMemoryStart() &&
			address < appBitMemoryStart() + appBitMemorySizeW())
		{
			return true;
		}

		return false;
	}

	bool LmMemoryMap::addressInWordMemory(int address) const
	{
		if (address >= appWordMemoryStart() &&
			address < appWordMemoryStart() + appWordMemorySizeW())
		{
			return true;
		}

		return false;
	}
}

