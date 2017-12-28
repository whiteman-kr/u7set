#include "DeviceEmulator.h"
#include <QQmlEngine>

namespace Sim
{
	DeviceCommand::DeviceCommand(const LmCommand& command) :
		m_command(command)
	{

	}

	DeviceCommand::DeviceCommand(const DeviceCommand& that)
	{
		*this = that;
	}

	DeviceCommand& DeviceCommand::operator=(const DeviceCommand& that)
	{
		if (Q_UNLIKELY(this == &that))
		{
			return *this;
		}

		this->setParent(that.parent());

		m_command = that.m_command;

		m_offset = that.m_offset;

		m_size = that.m_size;
		m_string = that.m_string;

		m_afbOpCode = that.m_afbOpCode;
		m_afbInstance = that.m_afbInstance;
		m_afbPinOpCode = that.m_afbPinOpCode;

		m_bitNo0 = that.m_bitNo0;
		m_bitNo1 = that.m_bitNo1;

		m_word0 = that.m_word0;
		m_word1 = that.m_word1;

		m_dword0 = that.m_dword0;
		m_dword1 = that.m_dword1;

		return *this;
	}

	DeviceEmulator::DeviceEmulator(const Output& output) :
		Output(output, "DeviceEmulator")
	{
		return;
	}

	DeviceEmulator::~DeviceEmulator()
	{
	}

	bool DeviceEmulator::init(const Hardware::LogicModuleInfo& logicModuleInfo,
							  const LmDescription& lmDescription,
							  const Eeprom& tuningEeprom,
							  const Eeprom& confEeprom,
							  const Eeprom& appLogicEeprom,
							  const QString& simulationScript)
	{
		setOutputScope(QString("DeviceEmulator %1").arg(logicModuleInfo.equipmentId));
		writeMessage(tr("Init device."));

		m_logicModuleInfo = logicModuleInfo;
		m_lmDescription = lmDescription;
		m_simulationScript = simulationScript;
		m_tuningEeprom = tuningEeprom;
		m_confEeprom = confEeprom;
		m_appLogicEeprom = appLogicEeprom;

		bool ok = initMemory();
		if (ok == false)
		{
			writeError(tr("Init memory error."));
			return false;
		}

		// Evaluate simulation script
		//
		m_evaluatedJs = m_jsEngine.evaluate(m_simulationScript);

		if (m_evaluatedJs.isError() == true)
		{
			QString str = QString("Evaluate simulation script error:\n"
								  "\tLine %1\n"
								  "\tStack: %2\n"
								  "\tMessage: %3")
						  .arg(m_evaluatedJs.property("lineNumber").toInt())
						  .arg(m_evaluatedJs.property("stack").toString())
						  .arg(m_evaluatedJs.toString());

			writeError(str);
			return false;
		}

		// --
		//
		ok = initEeprom();
		if (ok == false)
		{
			return false;
		}

		// --
		//
		ok = parseAppLogicCode();
		if (ok == false)
		{
			return false;
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
			ok &= m_ram.addMemoryArea(RamAccess::Read,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Input I/O Module %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(RamAccess::Write,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Output I/O Module %1").arg(i + 1));
		}

		// RAM - TX/RX Opto Interfaces
		//
		const LmDescription::OptoInterface& optoInterface = m_lmDescription.optoInterface();

		for (quint32 i = 0; i < optoInterface.m_optoPortCount; i++)
		{
			ok &= m_ram.addMemoryArea(RamAccess::Read,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  QString("Rx Opto Port  %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(RamAccess::Write,
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
		ok &= m_ram.addMemoryArea(RamAccess::Read,
								  memory.m_tuningDataOffset,
								  memory.m_tuningDataSize,
								  QLatin1String("Tuning Block"));

		// RAM - Diag Data
		//
		ok &= m_ram.addMemoryArea(RamAccess::Read,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Input Diag Data"));

		ok &= m_ram.addMemoryArea(RamAccess::Write,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Output Diag Data"));


		// RAM - App Data
		//
		ok &= m_ram.addMemoryArea(RamAccess::Read,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Input App Data"));

		ok &= m_ram.addMemoryArea(RamAccess::Write,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Output App Data"));

		return ok;
	}

	void DeviceEmulator::pause()
	{
		writeMessage(tr("Pause"));

		if (m_timerId != -1)
		{
			killTimer(m_timerId);
			m_timerId = -1;
		}

		return;
	}

	bool DeviceEmulator::initEeprom()
	{
		writeMessage(tr("Init EEPROM"));

		bool result = true;
		bool ok = true;

		ok = m_tuningEeprom.parseAllocationFrame(m_lmDescription.flashMemory().m_maxConfigurationCount);
		if (ok == false)
		{
			writeError(tr("Parse tuning EEPROM allocation frame error."));
			result = false;
		}

		ok = m_confEeprom.parseAllocationFrame(m_lmDescription.flashMemory().m_maxConfigurationCount);
		if (ok == false)
		{
			writeError(tr("Parse configuration EEPROM allocation frame error."));
			result = false;
		}

		ok = m_appLogicEeprom.parseAllocationFrame(m_lmDescription.flashMemory().m_maxConfigurationCount);
		if (ok == false)
		{
			writeError(tr("Parse application logic EEPROM allocation frame error."));
			result = false;
		}

		if (m_tuningEeprom.subsystemKey() != m_confEeprom.subsystemKey() ||
			m_tuningEeprom.subsystemKey() != m_appLogicEeprom.subsystemKey())
		{
			QString str = tr("EEPROMs have different subsystemKeys: \n"
						  "\ttuningEeprom.subsystemKey: %1\n"
						  "\tconfEeprom.subsystemKey: %2\n"
						  "\tappLogicEeprom.subsystemKey: %3")
							.arg(m_tuningEeprom.subsystemKey())
							.arg(m_confEeprom.subsystemKey())
							.arg(m_appLogicEeprom.subsystemKey());
			writeError(str);
			result = false;
		}

		if (m_tuningEeprom.buildNo() != m_confEeprom.buildNo() ||
			m_tuningEeprom.buildNo() != m_appLogicEeprom.buildNo())
		{
			QString str = tr("EEPROMs have different buildNo: \n"
						  "\ttuningEeprom.buildNo: %1\n"
						  "\tconfEeprom.buildNo: %2\n"
						  "\tappLogicEeprom.buildNo: %3")
							.arg(m_tuningEeprom.buildNo())
							.arg(m_confEeprom.buildNo())
							.arg(m_appLogicEeprom.buildNo());
			writeError(str);
			result = false;
		}

		if (m_tuningEeprom.configrationsCount() != m_confEeprom.configrationsCount() ||
			m_tuningEeprom.configrationsCount() != m_appLogicEeprom.configrationsCount())
		{
			QString str = tr("EEPROMs EEPROMs have different configrationsCount: \n"
						  "\ttuningEeprom.configrationsCount: %1\n"
						  "\tconfEeprom.configrationsCount: %2\n"
						  "\tappLogicEeprom.configrationsCount: %3")
							.arg(m_tuningEeprom.configrationsCount())
							.arg(m_confEeprom.configrationsCount())
							.arg(m_appLogicEeprom.configrationsCount());
			writeError(str);
			result = false;
		}

		if (result == false)
		{
			return false;
		}

		// Get plain application logic data for specific LmNumber
		//
		m_plainAppLogic.clear();
		m_plainAppLogic.reserve(m_appLogicEeprom.size());

		int startFrame = m_appLogicEeprom.configFrameIndex(m_logicModuleInfo.lmNumber);
		if (startFrame == 0)
		{
			writeError(QString("Can't get start frame for logic number %1").arg(m_logicModuleInfo.lmNumber));
			return false;
		}

		for (int i = startFrame + 1; i < m_appLogicEeprom.frameCount(); i++)	// 1st frame is service information  [D8.21.19, 3.1.1.2.2.1]
		{
			for (int f = 0; f < m_appLogicEeprom.framePayloadSize(); f++)
			{
				m_plainAppLogic.push_back(m_appLogicEeprom.getByte(i, f));
			}
		}

		return result;
	}

	bool DeviceEmulator::parseAppLogicCode()
	{
		// Parse AppLogic code:
		// Parse till commandWord is 0x0000
		//
		bool ok = true;
		int programCounter = 0;
		const std::vector<LmCommand> commands = m_lmDescription.commandsAsVector();

		do
		{
			quint16 commandWord = getWord(programCounter);
			if (commandWord == 0x0000)
			{
				break;
			}

			bool commandFound = false;
			for (const LmCommand& c : commands)
			{
				quint16 commandCode = (commandWord & c.codeMask);
				if (commandCode == c.code)
				{
					commandFound = true;

					// Parse this command
					//
					ok = parseCommand(c, programCounter);
					if (ok == false)
					{
						return false;
					}

					// Move ProgramCounter
					//
					assert(m_commands.empty() == false);

					int commandSize = m_commands.back().m_size;
					if (commandSize == 0)
					{
						QString str = tr("Parse command %1 error, ProgramCounter %2, returned command size is 0.\n")
											.arg(c.caption)
											.arg(programCounter);
						writeError(str);
						return false;
					}

					programCounter += commandSize;

					qDebug() << m_commands.back().m_string;

					break;
				}
			}

			if (commandFound == false)
			{
				QString str = tr("Parse command error, command cannot be found for word 0x%1\n")
									.arg(commandWord, 4, 16, QChar('0'));
				writeError(str);
				return false;
			}

		}
		while (programCounter < m_plainAppLogic.size());

		return true;
	}

	bool DeviceEmulator::parseCommand(const LmCommand& command, int programCounter)
	{
		quint16 commandWord = getWord(programCounter);
		quint16 commandCode = (commandWord & command.codeMask);
		if (commandCode != command.code)
		{
			assert(commandCode == command.code);
			return false;
		}

		// --
		//
		m_commands.emplace_back(command);
		DeviceCommand& deviceCommand = m_commands.back();

		deviceCommand.m_offset = programCounter;

		// Set argument list
		//
		QJSValue jsDeviceEmulator = m_jsEngine.newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsDeviceCommand = m_jsEngine.newQObject(&deviceCommand);
		QQmlEngine::setObjectOwnership(&deviceCommand, QQmlEngine::CppOwnership);

		QJSValueList args;
		args << jsDeviceEmulator;
		args << jsDeviceCommand;

		//--
		//
		const QString parseFunc = command.parseFunc;

		if (m_jsEngine.globalObject().hasProperty(parseFunc) == false ||
			m_jsEngine.globalObject().property(parseFunc).isCallable() == false)
		{
			writeError(tr("Parse Application Logic Code error, script function %1 not found or is not callable. "
						  "HasProperty %1: %2, "
						  "Collable: %3")
							.arg(parseFunc)
							.arg(m_jsEngine.globalObject().hasProperty(parseFunc))
							.arg(m_jsEngine.globalObject().property(parseFunc).isCallable())
						);
			return false;
		}

		QJSValue jsResult = m_jsEngine.globalObject().property(parseFunc).call(args);
		if (jsResult.isError() == true)
		{
			dumpJsError(jsResult);
			return false;
		}

		// --
		//

		return true;
	}

	void DeviceEmulator::dumpJsError(const QJSValue& value)
	{
		if (value.isError() == true)
		{
			QString str = QString("Script running uncaught exception at line %1\n"
								  "\tStack: %2\n"
								  "\tMessage: %3\n")
							.arg(value.property("lineNumber").toInt())
							.arg(value.property("stack").toString())
							.arg(value.toString());
			writeError(str);
		}

		return;
	}

	bool DeviceEmulator::processOperate()
	{
		// One LogicModule Cycle
		//
		//bool result = true;

		// Initialization before work cycle
		//
		m_logicUnit = LogicUnitData();
		m_afbComponents.clear();

		return false;

		// Run work cylce
		//
//		do
//		{
//			quint16 commandWord = getWord(m_logicUnit.programCounter);

//			//quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//			quint16 command = (commandWord & 0x7C0) >> 6;
//			//quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);

//			//qDebug() << "DeviceEmulator::processOperate Command " << command << ", N " << funcBlock;

//			// Control command processing
//			//
//			bool ok = runCommand(static_cast<LmCommandCode>(command));

//			if (ok == false && m_currentMode != DeviceMode::Fault)
//			{
//				FAULT("Run command %1 unknown error.");
//				result = false;
//				break;
//			}

//			if (m_currentMode == DeviceMode::Fault)
//			{
//				result = false;
//				break;
//			}
//		}
//		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
//			   (m_logicUnit.phase == CyclePhase::IdrPhase ||  m_logicUnit.phase == CyclePhase::AlpPhase));

//		return result;
	}

	void DeviceEmulator::start(int cycles)
	{
		writeMessage(tr("Start, cycles = %1").arg(cycles));

		if (m_timerId == -1)
		{
			m_timerId = startTimer(5, Qt::PreciseTimer);
		}
	}

	void DeviceEmulator::fault(QString reasone, QString func)
	{
		QString phase;
		switch (m_logicUnit.phase)
		{
		case CyclePhase::IdrPhase:	phase = "IDR";	break;
		case CyclePhase::AlpPhase:	phase = "ALP";	break;
		case CyclePhase::ODT:		phase = "ODT";	break;
		case CyclePhase::ST:		phase = "ST";	break;
		default:
			assert(false);
		}

		QString str1 = QString("Fault");
		QString str2 = QString("\tPhase: %1, ProgramCounter: %2 (0x%3), function: %4")
						.arg(phase)
						.arg(m_logicUnit.programCounter)
						.arg(m_logicUnit.programCounter, 4, 16, QChar('0'))
						.arg(func);
		QString str3 = QString("\tReasone: %1")
						.arg(reasone);

		writeError(str1);
		writeError(str2);
		writeError(str3);

		m_currentMode = DeviceMode::Fault;

		return;
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
			case DeviceMode::Operate:
				processOperate();
				break;
			default:
				assert(false);
				writeError(tr("Unknown device mode: %1").arg(static_cast<int>(m_currentMode)));
			}
		}

		return;
	}

	bool DeviceEmulator::processStartMode()
	{
		assert(m_currentMode == DeviceMode::Start);
		writeMessage(tr("Start mode"));

		m_currentMode = DeviceMode::Operate;

		return true;
	}

	bool DeviceEmulator::processFaultMode()
	{
		// Relax, you can do nothing
		//
		return true;
	}

//	bool DeviceEmulator::processLoadEeprom()
//	{
//		assert(m_currentMode == DeviceMode::LoadEeprom);
//		writeMessage(tr("LoadEeprom mode"));

//		bool result = true;
//		bool ok = true;

//		ok = m_tuningEeprom.parseAllocationFrame();
//		if (ok == false)
//		{
//			writeError(tr("Parse tuning EEPROM allocation frame error."));
//			result = false;
//		}

//		ok = m_confEeprom.parseAllocationFrame();
//		if (ok == false)
//		{
//			writeError(tr("Parse configuration EEPROM allocation frame error."));
//			result = false;
//		}

//		ok = m_appLogicEeprom.parseAllocationFrame();
//		if (ok == false)
//		{
//			writeError(tr("Parse application logic EEPROM allocation frame error."));
//			result = false;
//		}

//		if (m_tuningEeprom.subsystemKey() != m_confEeprom.subsystemKey() ||
//			m_tuningEeprom.subsystemKey() != m_appLogicEeprom.subsystemKey())
//		{
//			QString str = tr("EEPROMs have different subsystemKeys: \n"
//						  "\ttuningEeprom.subsystemKey: %1\n"
//						  "\tconfEeprom.subsystemKey: %2\n"
//						  "\tappLogicEeprom.subsystemKey: %3")
//							.arg(m_tuningEeprom.subsystemKey())
//							.arg(m_confEeprom.subsystemKey())
//							.arg(m_appLogicEeprom.subsystemKey());
//			writeError(str);

//			result = false;
//		}

//		if (m_tuningEeprom.buildNo() != m_confEeprom.buildNo() ||
//			m_tuningEeprom.buildNo() != m_appLogicEeprom.buildNo())
//		{
//			QString str = tr("EEPROMs have different buildNo: \n"
//						  "\ttuningEeprom.buildNo: %1\n"
//						  "\tconfEeprom.buildNo: %2\n"
//						  "\tappLogicEeprom.buildNo: %3")
//							.arg(m_tuningEeprom.buildNo())
//							.arg(m_confEeprom.buildNo())
//							.arg(m_appLogicEeprom.buildNo());
//			writeError(str);

//			result = false;
//		}

//		if (m_tuningEeprom.configrationsCount() != m_confEeprom.configrationsCount() ||
//			m_tuningEeprom.configrationsCount() != m_appLogicEeprom.configrationsCount())
//		{
//			QString str = tr("EEPROMs EEPROMs have different configrationsCount: \n"
//						  "\ttuningEeprom.configrationsCount: %1\n"
//						  "\tconfEeprom.configrationsCount: %2\n"
//						  "\tappLogicEeprom.configrationsCount: %3")
//							.arg(m_tuningEeprom.configrationsCount())
//							.arg(m_confEeprom.configrationsCount())
//							.arg(m_appLogicEeprom.configrationsCount());
//			writeError(str);
//			result = false;
//		}

//		if (result == false)
//		{
//			fault("Loading configuration error", "processLoadEeprom()");
//			return false;
//		}

//		// Get plain application logic data for specific m_logicModuleNumber
//		//
//		m_plainAppLogic.clear();
//		m_plainAppLogic.reserve(1024 * 1024);

//		int startFrame = m_appLogicEeprom.configFrameIndex(m_logicModuleInfo.lmNumber);
//		if (startFrame == -1)
//		{
//			FAULT(QString("Can't get start frame for logic number %1").arg(m_logicModuleInfo.lmNumber));
//			return false;
//		}

//		for (int i = startFrame + 1; i < m_appLogicEeprom.frameCount(); i++)	// 1st frame is service information  [D8.21.19, 3.1.1.2.2.1]
//		{
//			for (int f = 0; f < m_appLogicEeprom.framePayloadSize(); f++)
//			{
//				m_plainAppLogic.push_back(m_appLogicEeprom.getByte(i, f));
//			}
//		}

//		// --
//		//
//		m_currentMode = DeviceMode::Operate;

//		return result;
//	}

//	bool DeviceEmulator::processOperate()
//	{
//		// One LogicModule Cycle
//		//
//		bool result = true;

//		// Initialization before work cycle
//		//
//		m_logicUnit = LogicUnitData();
//		m_afbComponents.clear();

//		// Run work cylce
//		//
//		do
//		{
//			quint16 commandWord = getWord(m_logicUnit.programCounter);

//			//quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//			quint16 command = (commandWord & 0x7C0) >> 6;
//			//quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);

//			//qDebug() << "DeviceEmulator::processOperate Command " << command << ", N " << funcBlock;

//			// Control command processing
//			//
//			bool ok = runCommand(static_cast<LmCommandCode>(command));

//			if (ok == false && m_currentMode != DeviceMode::Fault)
//			{
//				FAULT("Run command %1 unknown error.");
//				result = false;
//				break;
//			}

//			if (m_currentMode == DeviceMode::Fault)
//			{
//				result = false;
//				break;
//			}
//		}
//		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
//			   (m_logicUnit.phase == CyclePhase::IdrPhase ||  m_logicUnit.phase == CyclePhase::AlpPhase));

//		return result;
//	}

//	bool DeviceEmulator::runCommand(LmCommandCode commandCode)
//	{
//		switch (commandCode)
//		{
//		case LmCommandCode::NOP:
//			return command_nop();

//		case LmCommandCode::START:
//			return command_startafb();

//		case LmCommandCode::STOP:
//			return command_stop();

//		case LmCommandCode::MOV:
//			return command_mov();

//		case LmCommandCode::MOVMEM:
//			return command_movmem();

//		case LmCommandCode::MOVC:
//			return command_movc();

//		case LmCommandCode::MOVBC:
//			return command_movbc();

//		case LmCommandCode::WRFB:
//			return command_wrbf();

//		case LmCommandCode::RDFB:
//			return command_rdbf();

//		case LmCommandCode::WRFBC:
//			return command_wrfbc();

//		case LmCommandCode::WRFBB:
//			return command_wrfbb();

//		case LmCommandCode::RDFBB:
//			return command_rdfbb();

//		case LmCommandCode::RDFBTS:
//			return command_rdfbts();

//		case LmCommandCode::SETMEM:
//			return command_setmem();

//		case LmCommandCode::MOVB:
//			return command_movb();

//		case LmCommandCode::NSTART:
//			return command_nstart();

//		case LmCommandCode::APPSTART:
//			return command_appstart();

//		case LmCommandCode::MOV32:
//			return command_mov32();

//		case LmCommandCode::MOVC32:
//			return command_movc32();

//		case LmCommandCode::WRFB32:
//			return command_wrfb32();

//		case LmCommandCode::RDFB32:
//			return command_rdfb32();

//		case LmCommandCode::WRFBC32:
//			return command_wrfbc32();

//		case LmCommandCode::RDFBTS32:
//			return command_rdfbts32();

//		case LmCommandCode::MOVCF:
//			return command_movcf();

//		case LmCommandCode::PMOV:
//			return command_pmov();

//		case LmCommandCode::PMOV32:
//			return command_pmov32();

//		case LmCommandCode::FILLB:
//			return command_fillb();

//		default:
//			FAULT(QString("Unknown command code %1").arg(static_cast<int>(commandCode)));
//			return false;
//		}
//	}

//	// OpCode 1
//	//
//	bool DeviceEmulator::command_nop()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);

//		assert(command == 1);

//		m_logicUnit.programCounter++;
//		return true;
//	}

//	// OpCode 2
//	//
//	bool DeviceEmulator::command_startafb()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		m_logicUnit.programCounter++;

//		// Get afb component
//		//
//		std::shared_ptr<Afb::AfbComponent> component = m_lmDescription.component(funcBlock);
//		if (component == nullptr)
//		{
//			QString str = QString("STARTAFB error, component not found. ComponentOpCode %1")
//							.arg(funcBlock);
//			FAULT(str);
//			return false;
//		}

//		ComponentInstance* instance = m_afbComponents.componentInstance(funcBlock, implNo);
//		if (instance == false)
//		{
//			QString str = QString("STARTAFB error, component instance not found. ComponentOpCode %1, Instance %2")
//							.arg(funcBlock)
//							.arg(implNo);
//			FAULT(str);
//			return false;
//		}

//		// Run script
//		//
//		if (m_evaluatedJs.isError() == true)
//		{
//			FAULT("Simulation script is not evaluated.");
//			return false;
//		}

//		// Set argument list
//		//
//		QJSValue jsInstance = m_jsEngine.newQObject(instance);
//		QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);

//		QJSValueList args;
//		args << jsInstance;

//		// Run script
//		//
//		QString simulationFunc = component->simulationFunc();

//		if (m_jsEngine.globalObject().hasProperty(simulationFunc) == false)
//		{
//			FAULT(QString("Script function %1 not found.").arg(simulationFunc));
//			return false;
//		}

//		if (m_jsEngine.globalObject().property(simulationFunc).isCallable() == false)
//		{
//			FAULT(QString("Script function %1 is not collable. ").arg(simulationFunc));
//			return false;
//		}

//		QJSValue jsResult = m_jsEngine.globalObject().property(simulationFunc).call(args);

//		if (jsResult.isError() == true)
//		{
//			QString str = QString("Script running uncaught exception at line %1\n"
//								  "\tStack: %2\n"
//								  "\tMessage: %3\n")
//						  .arg(jsResult.property("lineNumber").toInt())
//						  .arg(jsResult.property("stack").toString())
//						  .arg(jsResult.toString());

//			FAULT(str);
//			return false;
//		}

//		QString resultMessage = jsResult.toString();

//		if (resultMessage.isEmpty() == false)
//		{
//			QString formatted = QString("Command_startafb, AFB %1 (%2), instance %3, message: %4")
//									.arg(component->caption())
//									.arg(component->opCode())
//									.arg(implNo)
//									.arg(resultMessage);
//			writeMessage(formatted);
//		}

//		return true;
//	}

//	// OpCode 3
//	// Command stop, output signal Stop
//	//
//	bool DeviceEmulator::command_stop()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 3);
//		m_logicUnit.programCounter++;

//		// Command logic
//		//
//		if (m_logicUnit.phase == CyclePhase::IdrPhase)
//		{
//			m_logicUnit.programCounter = m_logicUnit.appStartAddress;
//			m_logicUnit.phase = CyclePhase::AlpPhase;
//			return true;
//		}

//		if (m_logicUnit.phase == CyclePhase::AlpPhase)
//		{
//			m_logicUnit.programCounter = m_logicUnit.appStartAddress;
//			m_logicUnit.phase = CyclePhase::ODT;
//			return true;
//		}

//		FAULT("Command STOP in wrong phase.");

//		return false;
//	}

//	bool DeviceEmulator::command_mov()
//	{
//		FAULT("Command not implemented ");
//		return false;
//	}

//	// OpCode 5
//	// Copy data from memory address1 to memory address2, N words
//	//
//	bool DeviceEmulator::command_movmem()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 5);
//		m_logicUnit.programCounter++;

//		quint16 addr2 = getWord(m_logicUnit.programCounter++);
//		quint16 addr1 = getWord(m_logicUnit.programCounter++);
//		quint16 n = getWord(m_logicUnit.programCounter++);

//		// Command Logic
//		//
//		bool ok = true;
//		for (quint16 offset = 0; offset < n; offset++)
//		{
//			quint16 data;

//			ok &= m_ram.readWord(addr1 + offset, &data);
//			ok &= m_ram.writeWord(addr2 + offset, data);
//		}

//		if (ok == false)
//		{
//			QString formattedMessage = QString("Move memory error, addr1 %1, addr2, number of words %2")
//										.arg(addr1)
//										.arg(addr2)
//										.arg(n);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 6
//	// Set constant to memory by address
//	//
//	bool DeviceEmulator::command_movc()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 6);
//		m_logicUnit.programCounter++;

//		quint16 addr = getWord(m_logicUnit.programCounter++);
//		quint16 data = getWord(m_logicUnit.programCounter++);

//		// Command Logic
//		//
//		bool ok = m_ram.writeWord(addr, data);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("Write memory error, addr %1, data %2")
//										.arg(addr)
//										.arg(data);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 7
//	//
//	bool DeviceEmulator::command_movbc()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 7);
//		m_logicUnit.programCounter++;

//		quint16 addr = getWord(m_logicUnit.programCounter++);
//		quint16 data = getWord(m_logicUnit.programCounter++);
//		quint16 bitNo = getWord(m_logicUnit.programCounter++);

//		// Command Logic
//		//
//		bool ok = m_ram.writeBit(addr, data & 0x01, bitNo);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("Write bit in memory error, addrw %1, data %2, bitno %3")
//										.arg(addr)
//										.arg(data & 0x01)
//										.arg(bitNo);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 8
//	//
//	bool DeviceEmulator::command_wrbf()
//	{
//		FAULT("Command not implemented");
//		return false;
//	}

//	// OpCode 9
//	//
//	bool DeviceEmulator::command_rdbf()
//	{
//		FAULT("Command not implemented");
//		return false;
//	}

//	// OpCode 10
//	// Entry constant DDD to the functional block N, impelementation of block R, option W
//	//
//	bool DeviceEmulator::command_wrfbc()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
//		m_logicUnit.programCounter++;

//		quint16 data = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		// --
//		//
//		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

//		if (afbComp == nullptr)
//		{
//			FAULT(QString("Run command_wrfbc32 error, AfbComponent with OpCode %1 not found").arg(funcBlock));
//			return false;
//		}

//		// --
//		//
//		ComponentParam param(implParamOpIndex, data);

//		QString errorMessage;
//		bool ok = m_afbComponents.addInstantiatorParam(afbComp, implNo, param, &errorMessage);

//		if (ok == false)
//		{
//			FAULT(QString("Run command_wrfbc error, %1").arg(errorMessage));
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 11
//	// Read bit from memory, write it to AFB
//	//
//	bool DeviceEmulator::command_wrfbb()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 11);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
//		m_logicUnit.programCounter++;

//		quint16 address = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		quint16 bitNo = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		// --
//		//
//		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

//		if (afbComp == nullptr)
//		{
//			FAULT(QString("command_wrfbb, AfbComponent with OpCode %1 not found").arg(funcBlock));
//			return false;
//		}

//		// Read bit from memory
//		//
//		if (bitNo > 15)
//		{
//			QString formattedError = QString("command_wrfbb, bitNo is out of range (>15). BitNo %1, AFB %2 (%3), instance %4, operand OpIndex %5.")
//										.arg(bitNo)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode())
//										.arg(implNo)
//										.arg(implParamOpIndex);
//			FAULT(formattedError);
//			return false;
//		}

//		quint16 data = 0;
//		bool ok = m_ram.readBit(address, bitNo, &data);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("command_wrfbb, Read bit from memory error, addrw %1, bitno %3")
//										.arg(address)
//										.arg(bitNo);
//			FAULT(formattedMessage);
//			return false;
//		}

//		// --
//		//
//		ComponentParam param(implParamOpIndex, data);
//		QString errorMessage;

//		ok = m_afbComponents.addInstantiatorParam(afbComp, implNo, param, &errorMessage);

//		if (ok == false)
//		{
//			FAULT(QString("command_wrfbb error, %1").arg(errorMessage));
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 12
//	// Read the first bit from result of AFB and write it to memory
//	//
//	bool DeviceEmulator::command_rdfbb()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
//		m_logicUnit.programCounter++;

//		quint16 address = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		quint16 bitNo = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		// --
//		//
//		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

//		if (afbComp == nullptr)
//		{
//			FAULT(QString("Run command_wrfbc32 error, AfbComponent with OpCode %1 not found").arg(funcBlock));
//			return false;
//		}

//		if (bitNo > 15)
//		{
//			QString formattedError = QString("command_rdfbb, bitNo is out of range (>15). BitNo %1, AFB %2 (%3), instance %4, output OpIndex %5.")
//										.arg(bitNo)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode())
//										.arg(implNo)
//										.arg(implParamOpIndex);
//			FAULT(formattedError);
//			return false;
//		}

//		// --
//		//
//		const ComponentInstance* instance = m_afbComponents.componentInstance(funcBlock, implNo);
//		if (instance == nullptr)
//		{
//			QString formattedError = QString("command_rdfbb, instance %1 does not exists, AFB %2 (%3).")
//										.arg(implNo)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode());
//			FAULT(formattedError);
//			return false;
//		}

//		const ComponentParam* param = instance->param(implParamOpIndex);
//		if (param == nullptr)
//		{
//			QString formattedError = QString("command_rdfbb, param %1 does not exists, AFB %2 (%3), instance %4.")
//										.arg(implParamOpIndex)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode())
//										.arg(implNo);
//			FAULT(formattedError);
//			return false;
//		}

//		quint16 value = param->wordValue() & 0x01;

//		bool ok = m_ram.writeBit(address, value, bitNo);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("Write bit in memory error, addrw %1, data %2, bitno %3")
//										.arg(address)
//										.arg(value)
//										.arg(bitNo);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return true;
//	}

//	// OpCode 13
//	//
//	bool DeviceEmulator::command_rdfbts()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 14
//	//
//	bool DeviceEmulator::command_setmem()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 15
//	//
//	bool DeviceEmulator::command_movb()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 16
//	//
//	bool DeviceEmulator::command_nstart()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 17
//	//
//	bool DeviceEmulator::command_appstart()
//	{
//		m_logicUnit.programCounter ++;
//		m_logicUnit.appStartAddress = getWord(m_logicUnit.programCounter++);
//		return true;
//	}

//	// OpCode 18
//	//
//	bool DeviceEmulator::command_mov32()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 19
//	// mov 32bit constant to memory
//	//
//	bool DeviceEmulator::command_movc32()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		assert(command == 19);
//		m_logicUnit.programCounter++;

//		quint16 address = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		quint32 data = getDword(m_logicUnit.programCounter);
//		m_logicUnit.programCounter += 2;

//		// Command Logic
//		//
//		bool ok = m_ram.writeDword(address, data);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("Write memory error, addr %1, data %2")
//										.arg(address)
//										.arg(data);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return ok;
//	}

//	// OpCode 20
//	//
//	bool DeviceEmulator::command_wrfb32()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 21
//	// Read data from functional block, write it to memory
//	//
//	bool DeviceEmulator::command_rdfb32()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
//		m_logicUnit.programCounter++;

//		quint16 address = getWord(m_logicUnit.programCounter);
//		m_logicUnit.programCounter++;

//		// Get data from functional block
//		//
//		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);
//		if (afbComp == nullptr)
//		{
//			FAULT(QString("AfbComponent with OpCode %1 not found").arg(funcBlock));
//			return false;
//		}

//		const ComponentInstance* instance = m_afbComponents.componentInstance(funcBlock, implNo);
//		if (instance == nullptr)
//		{
//			QString formattedError = QString("Instance %1 does not exists, AFB %2 (%3).")
//										.arg(implNo)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode());
//			FAULT(formattedError);
//			return false;
//		}

//		const ComponentParam* param = instance->param(implParamOpIndex);
//		if (param == nullptr)
//		{
//			QString formattedError = QString("command_rdfbb, param %1 does not exists, AFB %2 (%3), instance %4.")
//										.arg(implParamOpIndex)
//										.arg(afbComp->caption())
//										.arg(afbComp->opCode())
//										.arg(implNo);
//			FAULT(formattedError);
//			return false;
//		}

//		qint32 data = param->signedIntValue();

//		bool ok = m_ram.writeSignedInt(address, data);
//		if (ok == false)
//		{
//			QString formattedMessage = QString("Write memory error, addrw %1, data %2")
//										.arg(address)
//										.arg(data);
//			FAULT(formattedMessage);
//			return false;
//		}

//		return true;
//	}

//	// OpCode 22
//	// Entry constant DDD to the functional block N, impelementation of block R, option W(DDD(31..16))  and W+1(DDD(15..0))
//	//
//	bool DeviceEmulator::command_wrfbc32()
//	{
//		quint16 commandWord = getWord(m_logicUnit.programCounter);
//		quint16 crc5 = (commandWord & 0xF800) >> 11;		Q_UNUSED(crc5);
//		quint16 command = (commandWord & 0x7C0) >> 6;		Q_UNUSED(command);
//		quint16 funcBlock = commandWord & 0x01F;			Q_UNUSED(funcBlock);
//		m_logicUnit.programCounter++;

//		quint16 implNo = getWord(m_logicUnit.programCounter) >> 6;
//		quint16 implParamOpIndex = getWord(m_logicUnit.programCounter) & 0b0000000000111111;
//		m_logicUnit.programCounter++;

//		quint32 data = getDword(m_logicUnit.programCounter);
//		m_logicUnit.programCounter += 2;

//		// --
//		//
//		std::shared_ptr<Afb::AfbComponent> afbComp = m_lmDescription.component(funcBlock);

//		if (afbComp == nullptr)
//		{
//			FAULT(QString("Run command_wrfbc32 error, AfbComponent with OpCode %1 not found").arg(funcBlock));
//			return false;
//		}

//		// --
//		//
//		ComponentParam param(implParamOpIndex, data);
//		QString errorMessage;

//		bool ok = m_afbComponents.addInstantiatorParam(afbComp, implNo, param, &errorMessage);

//		if (ok == false)
//		{
//			FAULT(QString("Run command_wrfbc32 error, %1").arg(errorMessage));
//		}

//		return ok;
//	}

//	// OpCode 23
//	//
//	bool DeviceEmulator::command_rdfbts32()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 24
//	//
//	bool DeviceEmulator::command_movcf()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 25
//	//
//	bool DeviceEmulator::command_pmov()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 26
//	//
//	bool DeviceEmulator::command_pmov32()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}

//	// OpCode 27
//	//
//	bool DeviceEmulator::command_fillb()
//	{
//		FAULT("Command not implemented " __FUNCTION__);
//		return false;
//	}


	quint16 DeviceEmulator::getWord(int wordOffset) const
	{
		return getData<quint16>(wordOffset * 2);
	}

	quint32 DeviceEmulator::getDword(int wordOffset) const
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

	const Hardware::LogicModuleInfo& DeviceEmulator::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}

}
