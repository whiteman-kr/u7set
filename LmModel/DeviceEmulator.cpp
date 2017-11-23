#include "DeviceEmulator.h"

namespace LmModel
{

	DeviceEmulator::DeviceEmulator(QTextStream* outputStream) :
		m_output(outputStream)
	{
		assert(outputStream);
		output() << "DeviceEmulator: Instance created" << endl;

		return;
	}

	DeviceEmulator::~DeviceEmulator()
	{
		output() << "DeviceEmulator: Instance destroyed" << endl;
	}

	bool DeviceEmulator::init(int logicModuleNumber,
							  const LmDescription& lmDescription,
							  const Eeprom& tuningEeprom,
							  const Eeprom& confEeprom,
							  const Eeprom& appLogicEeprom)
	{
		output() << "DeviceEmulator: Init device" << endl;

		m_logicModuleNumber = logicModuleNumber;
		m_lmDescription = lmDescription;
		m_tuningEeprom = tuningEeprom;
		m_confEeprom = confEeprom;
		m_appLogicEeprom = appLogicEeprom;

		bool ok = initMemory();
		if (ok == false)
		{
			output() << "DeviceEmulator: Init memory error" << endl;
		}

		return ok;
	}

	bool DeviceEmulator::initMemory()
	{
		bool ok = true;

		// RAM - I/O modules
		//
		const LmDescription::Memory& memory = m_lmDescription.memory();

		for (quint32 i = 0; i < memory.m_moduleCount; i++)
		{
			ok &= m_ram.addMemoryArea(RamAccess::ReadOnly,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Input I/O Module %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(RamAccess::WriteOnly,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Output I/O Module %1").arg(i + 1));
		}

		// RAM - TX/RX Opto Interfaces
		//
		const LmDescription::OptoInterface& optoInterface = m_lmDescription.optoInterface();

		for (quint32 i = 0; i < optoInterface.m_optoPortCount; i++)
		{
			ok &= m_ram.addMemoryArea(RamAccess::ReadOnly,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  QString("Rx Opto Port  %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(RamAccess::WriteOnly,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  QString("Tx Opto Port  %1").arg(i + 1));
		}

		// RAM - ApplicationLogic Block, bit/word access
		//
		ok &= m_ram.addMemoryArea(RamAccess::ReadWrite,
								  memory.m_appLogicBitDataOffset,
								  memory.m_appLogicBitDataSize,
								  QLatin1String("Appliaction Logic Block (bit access)"));

		ok &= m_ram.addMemoryArea(RamAccess::ReadWrite,
								  memory.m_appLogicWordDataOffset,
								  memory.m_appLogicWordDataSize,
								  QLatin1String("Appliaction Logic Block (word access)"));

		// RAM - Tuninng Block
		//
		ok &= m_ram.addMemoryArea(RamAccess::ReadOnly,
								  memory.m_tuningDataOffset,
								  memory.m_tuningDataSize,
								  QLatin1String("Tuning Block"));

		// RAM - Diag Data
		//
		ok &= m_ram.addMemoryArea(RamAccess::ReadOnly,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Input Diag Data"));

		ok &= m_ram.addMemoryArea(RamAccess::WriteOnly,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Output Diag Data"));


		// RAM - App Data
		//
		ok &= m_ram.addMemoryArea(RamAccess::ReadOnly,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Input App Data"));

		ok &= m_ram.addMemoryArea(RamAccess::WriteOnly,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Output App Data"));

		return ok;
	}

	void DeviceEmulator::pause()
	{
		output() << "DeviceEmulator: Pause" << endl;

		if (m_timerId != -1)
		{
			killTimer(m_timerId);
			m_timerId = -1;
		}

		return;
	}

	void DeviceEmulator::start(int cycles)
	{
		output() << "DeviceEmulator: Start, cyles = " << cycles << endl;

		if (m_timerId == -1)
		{
			m_timerId = startTimer(5, Qt::PreciseTimer);
		}
	}

	void DeviceEmulator::fault(QString reasone)
	{
		output() << "DeviceEmulator: Fault, Reasone: " << reasone << endl;
		m_currentMode = DeviceMode::Fault;
	}

	void DeviceEmulator::timerEvent(QTimerEvent* event)
	{
		if (event->timerId() == m_timerId)
		{
			switch (m_currentMode)
			{
			case DeviceMode::Start:
				processStartMode();
				break;
			case DeviceMode::Fault:
				processFaultMode();
				break;
			case DeviceMode::LoadEeprom:
				processLoadEeprom();
				break;
			case DeviceMode::Operate:
				processOperate();
				break;
			default:
				assert(false);
				output() << "DeviceEmulator: Unknown device mode: " << static_cast<int>(m_currentMode) << endl;
			}
		}

		return;
	}

	bool DeviceEmulator::processStartMode()
	{
		assert(m_currentMode == DeviceMode::Start);
		output() << "DeviceEmulator: Start mode" << endl;

		m_currentMode = DeviceMode::LoadEeprom;

		return true;
	}

	bool DeviceEmulator::processFaultMode()
	{
		// Relax, you can do nothing
		//
		return true;
	}

	bool DeviceEmulator::processLoadEeprom()
	{
		assert(m_currentMode == DeviceMode::LoadEeprom);
		output() << "DeviceEmulator: LoadEeprom mode" << endl;

		bool result = true;
		bool ok = true;

		ok = m_tuningEeprom.parseAllocationFrame();
		if (ok == false)
		{
			output() << "DeviceEmulator: Parse tuning EEPROM allocation frame error" << endl;
			result = false;
		}

		ok = m_confEeprom.parseAllocationFrame();
		if (ok == false)
		{
			output() << "DeviceEmulator: Parse configuration EEPROM allocation frame error" << endl;
			result = false;
		}

		ok = m_appLogicEeprom.parseAllocationFrame();
		if (ok == false)
		{
			output() << "DeviceEmulator: Parse application logic EEPROM allocation frame error" << endl;
			result = false;
		}

		if (m_tuningEeprom.subsystemKey() != m_confEeprom.subsystemKey() ||
			m_tuningEeprom.subsystemKey() != m_appLogicEeprom.subsystemKey())
		{
			output() << "DeviceEmulator: EEPROMs have different subsystemKeys" <<
						", tuningEeprom.subsystemKey = " << m_tuningEeprom.subsystemKey() <<
						", confEeprom.subsystemKey = " << m_confEeprom.subsystemKey() <<
						", appLogicEeprom.subsystemKey = " << m_appLogicEeprom.subsystemKey() << endl;
			result = false;
		}

		if (m_tuningEeprom.buildNo() != m_confEeprom.buildNo() ||
			m_tuningEeprom.buildNo() != m_appLogicEeprom.buildNo())
		{
			output() << "DeviceEmulator: EEPROMs have different buildNo" <<
						", tuningEeprom.buildNo = " << m_tuningEeprom.buildNo() <<
						", confEeprom.buildNo = " << m_confEeprom.buildNo() <<
						", appLogicEeprom.buildNo = " << m_appLogicEeprom.buildNo() << endl;
			result = false;
		}

		if (m_tuningEeprom.configrationsCount() != m_confEeprom.configrationsCount() ||
			m_tuningEeprom.configrationsCount() != m_appLogicEeprom.configrationsCount())
		{
			output() << "DeviceEmulator: EEPROMs have different configrationsCount" <<
						", tuningEeprom.configrationsCount = " << m_tuningEeprom.configrationsCount() <<
						", confEeprom.configrationsCount = " << m_confEeprom.configrationsCount() <<
						", appLogicEeprom.configrationsCount = " << m_appLogicEeprom.configrationsCount() << endl;
			result = false;
		}

		if (result == false)
		{
			fault("Loading configuration error");
			return false;
		}

		// Get plain application logic data for specific m_logicModuleNumber
		//
		m_plainAppLogic.clear();
		m_plainAppLogic.reserve(1024 * 1024);

		int startFrame = m_appLogicEeprom.configFrameIndex(m_logicModuleNumber);
		if (startFrame == -1)
		{
			fault(QString("Can't get start frame for logic number %1").arg(m_logicModuleNumber));
			return false;
		}

		for (int i = startFrame + 1; i < m_appLogicEeprom.frameCount(); i++)	// 1st frame is service information  [D8.21.19, 3.1.1.2.2.1]
		{
			for (int f = 0; f < m_appLogicEeprom.framePayloadSize(); f++)
			{
				m_plainAppLogic.push_back(m_appLogicEeprom.getByte(i, f));
			}
		}

		// --
		//
		m_currentMode = DeviceMode::Operate;

		return result;
	}

	bool DeviceEmulator::processOperate()
	{
		// One LogicModule Cycle
		//
		bool result = true;

		// Initialization before work cycle
		//
		m_logicUnit = LogicUnitData();
		m_afbComponents.clear();

		// Run work cylce
		//
		do
		{
			quint16 commandWord = getWord(m_logicUnit.programCounter);

			quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
			quint16 command = (commandWord & 0x7C0) >> 6;
			quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);

			qDebug() << "DeviceEmulator::processOperate Command " << command << ", N " << funcBlock;

			// Control command processing
			//
			bool ok = runCommand(static_cast<LmCommandCode>(command));

			if (ok == false && m_currentMode != DeviceMode::Fault)
			{
				fault("Run command %1 unknown error.");
				result = false;
				break;
			}

			if (m_currentMode == DeviceMode::Fault)
			{
				result = false;
				break;
			}
		}
		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
			   (m_logicUnit.phase == CyclePhase::IdrPhase ||  m_logicUnit.phase == CyclePhase::AlpPhase));

		return result;
	}

	bool DeviceEmulator::runCommand(LmCommandCode commandCode)
	{
		switch (commandCode)
		{
		case LmCommandCode::NOP:
			return command_nop();

		case LmCommandCode::START:
			return command_startafb();

		case LmCommandCode::STOP:
			return command_stop();

		case LmCommandCode::MOV:
			return command_mov();

		case LmCommandCode::MOVMEM:
			return command_movmem();

		case LmCommandCode::MOVC:
			return command_movc();

		case LmCommandCode::MOVBC:
			return command_movbc();

		case LmCommandCode::WRFB:
			return command_wrbf();

		case LmCommandCode::RDFB:
			return command_rdbf();

		case LmCommandCode::WRFBC:
			return command_wrfbc();

		case LmCommandCode::WRFBB:
			return command_wrfbb();

		case LmCommandCode::RDFBB:
			return command_rdfbb();

		case LmCommandCode::RDFBTS:
			return command_rdfbts();

		case LmCommandCode::SETMEM:
			return command_setmem();

		case LmCommandCode::MOVB:
			return command_movb();

		case LmCommandCode::NSTART:
			return command_nstart();

		case LmCommandCode::APPSTART:
			return command_appstart();

		case LmCommandCode::MOV32:
			return command_mov32();

		case LmCommandCode::MOVC32:
			return command_movc32();

		case LmCommandCode::WRFB32:
			return command_wrfb32();

		case LmCommandCode::RDFB32:
			return command_rdfb32();

		case LmCommandCode::WRFBC32:
			return command_wrfbc32();

		case LmCommandCode::RDFBTS32:
			return command_rdfbts32();

		case LmCommandCode::MOVCF:
			return command_movcf();

		case LmCommandCode::PMOV:
			return command_pmov();

		case LmCommandCode::PMOV32:
			return command_pmov32();

		case LmCommandCode::FILLB:
			return command_fillb();

		default:
			fault(QString("Unknown command code %1").arg(static_cast<int>(commandCode)));
			return false;
		}
	}

	bool DeviceEmulator::command_nop()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_startafb()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	// OpCode 3
	// Command stop, output signal Stop
	//
	bool DeviceEmulator::command_stop()
	{
		quint16 commandWord = getWord(m_logicUnit.programCounter);
		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
		assert(command == 3);
		m_logicUnit.programCounter++;

		// Command logic
		//
		if (m_logicUnit.phase == CyclePhase::IdrPhase)
		{
			m_logicUnit.programCounter = m_logicUnit.appStartAddress;
			m_logicUnit.phase = CyclePhase::AlpPhase;
			return true;
		}

		if (m_logicUnit.phase == CyclePhase::AlpPhase)
		{
			m_logicUnit.programCounter = m_logicUnit.appStartAddress;
			m_logicUnit.phase = CyclePhase::ODT;
			return true;
		}

		fault(QString("Command STOP in wrong phase %1").arg(static_cast<int>(m_logicUnit.phase)));

		return false;
	}

	bool DeviceEmulator::command_mov()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_movmem()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_movc()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	// OpCode 7
	//
	bool DeviceEmulator::command_movbc()
	{
		quint16 commandWord = getWord(m_logicUnit.programCounter);
		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
		assert(command == 7);
		m_logicUnit.programCounter++;

		quint16 addr = getWord(m_logicUnit.programCounter++);
		quint16 data = getWord(m_logicUnit.programCounter++);
		quint16 bitNo = getWord(m_logicUnit.programCounter++);

		// Command Logic
		//
		bool ok = m_ram.writeBit(addr, data & 0x01, bitNo);

		return ok;
	}

	bool DeviceEmulator::command_wrbf()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_rdbf()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	// OpCode 10
	// Entry constant DDD to the functional block N, impelementation of block R, option W
	//
	bool DeviceEmulator::command_wrfbc()
	{
		quint16 commandWord = getWord(m_logicUnit.programCounter);
		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
		m_logicUnit.programCounter++;

		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
		m_logicUnit.programCounter++;

		quint16 data = getWord(m_logicUnit.programCounter);
		m_logicUnit.programCounter++;

		// --
		//
		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

		if (afbComp == nullptr)
		{
			fault(QString("Run command_wrfbc32 error, AfbComponent with OpCode %1 not found").arg(funcBlock));
			return false;
		}

		// --
		//
		InstantiatorParam ip(implNo, implParamOpIndex, data);

		QString errorMessage;
		bool ok = m_afbComponents.addInstantiatorParam(afbComp, ip, &errorMessage);

		if (ok == false)
		{
			fault(QString("Run command_wrfbc error, %1").arg(errorMessage));
		}

		return ok;
	}

	bool DeviceEmulator::command_wrfbb()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_rdfbb()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_rdfbts()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_setmem()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_movb()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_nstart()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_appstart()
	{
		m_logicUnit.programCounter ++;
		m_logicUnit.appStartAddress = getWord(m_logicUnit.programCounter++);
		return true;
	}

	bool DeviceEmulator::command_mov32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_movc32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_wrfb32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_rdfb32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	// OpCode 22
	// Entry constant DDD to the functional block N, impelementation of block R, option W(DDD(31..16))  and W+1(DDD(15..0))
	//
	bool DeviceEmulator::command_wrfbc32()
	{
		quint16 commandWord = getWord(m_logicUnit.programCounter);
		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
		m_logicUnit.programCounter++;

		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
		m_logicUnit.programCounter++;

		quint32 data = getDword(m_logicUnit.programCounter);
		m_logicUnit.programCounter += 2;

		// --
		//
		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

		if (afbComp == nullptr)
		{
			fault(QString("Run command_wrfbc32 error, AfbComponent with OpCode %1 not found").arg(funcBlock));
			return false;
		}

		// --
		//
		InstantiatorParam ip(implNo, implParamOpIndex, data);
		QString errorMessage;
		bool ok = m_afbComponents.addInstantiatorParam(afbComp, ip, &errorMessage);

		if (ok == false)
		{
			fault(QString("Run command_wrfbc32 error, %1").arg(errorMessage));
		}

		return ok;
	}

	bool DeviceEmulator::command_rdfbts32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_movcf()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_pmov()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_pmov32()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	bool DeviceEmulator::command_fillb()
	{
		fault("Command not implemented " __FUNCTION__);
		return false;
	}

	quint16 DeviceEmulator::getWord(int wordOffset) const
	{
		return getData<quint16>(wordOffset * 2);
	}

	quint16 DeviceEmulator::getDword(int wordOffset) const
	{
		return getData<quint32>(wordOffset * 2);
	}

	template <typename TYPE>
	TYPE DeviceEmulator::getData(int eepromOffset) const
	{
		// eepromOffset - in bytes
		//
		if (eepromOffset < 0 || eepromOffset > m_plainAppLogic.size() - sizeof(TYPE))
		{
			assert(eepromOffset >= 0 &&
				   eepromOffset - sizeof(TYPE) <= m_plainAppLogic.size());
			return 0;
		}

		TYPE result = qFromBigEndian<TYPE>(m_plainAppLogic.constData() + eepromOffset);
		return result;
	}

	QTextStream& DeviceEmulator::output()
	{
		return *m_output;
	}

	QTextStream& DeviceEmulator::output() const
	{
		return *m_output;
	}

}
