#include "SimDeviceEmulator.h"
#include <QtEndian>
#include "SimException.h"
#include "SimCommandProcessor.h"

namespace Sim
{
	DeviceCommand::DeviceCommand(const LmCommand& command) :
		m_command(command)
	{
	}

	void DeviceCommand::dump() const
	{
		QString str0 = QString("code = %1 (0x%2), codeMask = %3 (0x%4)")
						.arg(m_command.code)
						.arg(m_command.code, 4, 16, QChar('0'))
						.arg(m_command.codeMask)
						.arg(m_command.codeMask, 4, 16, QChar('0'));

		QString str1 = QString("caption = %1").arg(m_command.caption);
		QString str2 = QString("simFunct = %1, parseFunc = %2")
						.arg(m_command.simulationFunc)
						.arg(m_command.parseFunc);

		QString str3 = QString("offest = %1 (0x%2), size = %3")
						.arg(m_offset)
						.arg(m_offset, 4, 16, QChar('0'))
						.arg(m_size);

		QString str4 = QString("string = %1").arg(m_string);

		QString str5 = QString("afbOpCode = %1, instance = %2, pinOpCode = %3")
						.arg(m_afbOpCode)
						.arg(m_afbInstance)
						.arg(m_afbPinOpCode);

		QString str6 = QString("bitNo0 = %1, bitNo1 = %2")
						.arg(m_bitNo0)
						.arg(m_bitNo1);

		QString str7 = QString("word0 = %1 (0x%2), word1 = %3 (0x%4), word2 = %5 (0x%6)")
						.arg(m_word0)
						.arg(m_word0, 4, 16, QChar('0'))
						.arg(m_word1)
						.arg(m_word1, 4, 16, QChar('0'))
						.arg(m_word2)
						.arg(m_word2, 4, 16, QChar('0'));

		QString str8 = QString("dword0 = %1 (0x%2), dword1 = %3 (0x%4)")
						.arg(m_dword0)
						.arg(m_dword0, 8, 16, QChar('0'))
						.arg(m_dword1)
						.arg(m_dword1, 8, 16, QChar('0'));

		qDebug() << "DeviceCommand: " << m_string;
		qDebug() << "\t" << str0;
		qDebug() << "\t" << str1;
		qDebug() << "\t" << str2;
		qDebug() << "\t" << str3;
		qDebug() << "\t" << str4;
		qDebug() << "\t" << str5;
		qDebug() << "\t" << str6;
		qDebug() << "\t" << str7;
		qDebug() << "\t" << str8;

		return;
	}

	QString DeviceCommand::caption() const
	{
		return m_command.caption;
	}

	//
	// class DeviceEmulator script wrapper
	//
	ScriptDeviceEmulator::ScriptDeviceEmulator(DeviceEmulator* device) :
		m_device(device)
	{
		assert(m_device);
	}

	DeviceCommand* ScriptDeviceEmulator::command(int index)
	{
		if (index >= m_device->m_commands.size())
		{
			m_device->fault(QString("Index of m_commands is out of range"), "ScriptDeviceEmulator::command(int index)");
			return nullptr;
		}

		return &(m_device->m_commands[index]);
	}

	quint16 ScriptDeviceEmulator::appStartAddress() const
	{
		return m_device->m_logicUnit.appStartAddress;
	}

	void ScriptDeviceEmulator::setAppStartAddress(quint16 value)
	{
		m_device->m_logicUnit.appStartAddress = value;
	}

	Sim::CyclePhase ScriptDeviceEmulator::phase() const
	{
		return m_device->m_logicUnit.phase;
	}

	void ScriptDeviceEmulator::setPhase(Sim::CyclePhase value)
	{
		m_device->m_logicUnit.phase = value;
	}

	quint32 ScriptDeviceEmulator::programCounter() const
	{
		return m_device->m_logicUnit.programCounter;
	}

	void ScriptDeviceEmulator::setProgramCounter(quint32 value)
	{
		m_device->m_logicUnit.programCounter = value;
	}

	Sim::AfbComponent ScriptDeviceEmulator::afbComponent(int opCode) const
	{
		AfbComponent result(m_device->m_lmDescription.component(opCode));
		return result;
	}

	Sim::AfbComponentInstance* ScriptDeviceEmulator::afbComponentInstance(int opCode, int instanceNo)
	{
		Sim::AfbComponentInstance* result = m_device->m_afbComponents.componentInstance(opCode, instanceNo);

		if (result == nullptr)
		{
			m_device->fault(QString("Attempt to get not existing AFB (opcode %1) instance (%2)")
								.arg(opCode)
								.arg(instanceNo),
							"ScriptDeviceEmulator::afbComponentInstance");
		}

		return result;
	}

	bool ScriptDeviceEmulator::setAfbParam(int afbOpCode, int instanceNo, const AfbComponentParam& param)
	{
		QString errorMessage;
		bool ok = m_device->m_afbComponents.addInstantiatorParam(afbOpCode, instanceNo, param, &errorMessage);

		if (ok == false)
		{
			m_device->FAULT(QString("Add addInstantiatorParam error, %1").arg(errorMessage));
		}

		return ok;
	}

	// RAM access
	//
	bool ScriptDeviceEmulator::movRamMem(quint32 src, quint32 dst, quint32 size)
	{
		while (size-- > 0)
		{
			quint16 w = readRamWord(src++);
			writeRamWord(dst++, w);
		}

		// readRamWord, writeRamWord have checks and will set FAULT in case of error
		//
		return true;
	}

	bool ScriptDeviceEmulator::writeRamBit(quint32 offsetW, quint32 bitNo, quint32 data)
	{
		bool ok = m_device->m_ram.writeBit(offsetW, bitNo, data, E::ByteOrder::BigEndian);
		if (ok == false)
		{
			m_device->FAULT(QString("Write access RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return ok;
	}

	quint16 ScriptDeviceEmulator::readRamBit(quint32 offsetW, quint32 bitNo)
	{
		quint16 data = 0;
		bool ok = m_device->m_ram.readBit(offsetW, bitNo, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			m_device->FAULT(QString("Read access RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return data;
	}

	bool ScriptDeviceEmulator::writeRamWord(quint32 offsetW, quint16 data)
	{
		bool ok = m_device->m_ram.writeWord(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			m_device->FAULT(QString("Write access RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	quint16 ScriptDeviceEmulator::readRamWord(quint32 offsetW)
	{
		quint16 data = 0;
		bool ok = m_device->m_ram.readWord(offsetW, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			m_device->FAULT(QString("Read access RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	bool ScriptDeviceEmulator::writeRamDword(quint32 offsetW, quint32 data)
	{
		bool ok = m_device->m_ram.writeDword(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			m_device->FAULT(QString("Write access RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	quint32 ScriptDeviceEmulator::readRamDword(quint32 offsetW)
	{
		quint32 data = 0;
		bool ok = m_device->m_ram.readDword(offsetW, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			m_device->FAULT(QString("Read access RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	// Getting data from m_plainAppLogic
	//
	quint16 ScriptDeviceEmulator::getWord(int wordOffset) const
	{
		return m_device->getWord(wordOffset);
	}

	quint32 ScriptDeviceEmulator::getDword(int wordOffset) const
	{
		return m_device->getDword(wordOffset);
	}

	//
	// DeviceEmulator
	//
	DeviceEmulator::DeviceEmulator() :
		Output("DeviceEmulator")
	{
		m_offsetToCommand.reserve(32000);
		return;
	}

	DeviceEmulator::~DeviceEmulator()
	{
		Output("~DeviceEmulator");
		return;
	}

	bool DeviceEmulator::clear()
	{
		setLogicModuleInfo(Hardware::LogicModuleInfo());

		m_commandProcessor.reset();

		m_lmDescription.clear();
		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		m_afbComponents.clear();

		m_commands.clear();
		m_offsetToCommand.clear();

		{
			m_cacheMutex.lock();
			m_cachedLogicModuleInfo = Hardware::LogicModuleInfo();
			m_cachedCommands.clear();
			m_cachedOffsetToCommand.clear();
			m_cacheMutex.unlock();
		}

		return true;
	}

	bool DeviceEmulator::init(const Hardware::LogicModuleInfo& logicModuleInfo,
							  const LmDescription& lmDescription,
							  const Eeprom& tuningEeprom,
							  const Eeprom& confEeprom,
							  const Eeprom& appLogicEeprom)
	{
		clear();

		setOutputScope(QString("DeviceEmulator %1").arg(logicModuleInfo.equipmentId));
		writeMessage(tr("Init device."));

		// --
		//
		setLogicModuleInfo(logicModuleInfo);

		m_lmDescription = lmDescription;
		m_tuningEeprom = tuningEeprom;
		m_confEeprom = confEeprom;
		m_appLogicEeprom = appLogicEeprom;

		// Create specific CommnadProcessor
		//
		m_commandProcessor = std::unique_ptr<CommandProcessor>(CommandProcessor::createInstance(this));

		if (m_commandProcessor == nullptr)
		{
			writeWaning(QString("There is no simulation for %1, LmDescription.name = %2")
							.arg(logicModuleInfo.equipmentId)
							.arg(lmDescription.name()));
			return false;
		}

		//--
		//
		if (bool ok = m_afbComponents.init(m_lmDescription);
			ok == false)
		{
			return false;
		}

		// --
		//
		if (bool ok = initEeprom();
			ok == false)
		{
			return false;
		}

		// --
		//
		if (bool ok = parseAppLogicCode();
			ok == false)
		{
			return false;
		}

		return true;
	}

	bool DeviceEmulator::reset()
	{
		writeMessage(tr("Reset"));

		m_currentMode = DeviceMode::Start;

		bool ok = initMemory();
		if (ok == false)
		{
			writeError(tr("Init memory error."));
			return false;
		}

		return true;
	}

	bool DeviceEmulator::run(int cycles)
	{
		if (m_currentMode == DeviceMode::Start)
		{
			if (bool ok = processStartMode();
				ok == false)
			{
				return false;
			}
		}

		bool ok = true;

		for (int i = 0; i < cycles; i++)
		{
			if (m_currentMode == DeviceMode::Fault)
			{
				ok &= processFaultMode();
				break;
			}

			ok &= processOperate();
		}

		return ok;
	}

	bool DeviceEmulator::initMemory()
	{
		bool ok = true;

		m_ram.reset();

		// RAM - I/O modules
		//
		const LmDescription::Memory& memory = m_lmDescription.memory();

		for (quint32 i = 0; i < memory.m_moduleCount; i++)
		{
			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Input I/O Module %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  QString("Output I/O Module %1").arg(i + 1));
		}

		// RAM - TX/RX Opto Interfaces
		//
		const LmDescription::OptoInterface& optoInterface = m_lmDescription.optoInterface();

		for (quint32 i = 0; i < optoInterface.m_optoPortCount; i++)
		{
			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  QString("Rx Opto Port  %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  QString("Tx Opto Port  %1").arg(i + 1));
		}

		// RAM - ApplicationLogic Block, bit/word access
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
								  memory.m_appLogicBitDataOffset,
								  memory.m_appLogicBitDataSize,
								  QLatin1String("Application Logic Block (bit access)"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
								  memory.m_appLogicWordDataOffset,
								  memory.m_appLogicWordDataSize,
								  QLatin1String("Application Logic Block (word access)"));

		// RAM - Tuninng Block
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_tuningDataOffset,
								  memory.m_tuningDataSize,
								  QLatin1String("Tuning Block"));

		// RAM - Diag Data
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Input Diag Data"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  QLatin1String("Output Diag Data"));


		// RAM - App Data
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Input App Data"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  QLatin1String("Output App Data"));

		return ok;
	}

//	void DeviceEmulator::pause()
//	{
//		writeMessage(tr("Pause"));

//		if (m_timerId != -1)
//		{
//			killTimer(m_timerId);
//			m_timerId = -1;
//		}

//		return;
//	}

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
		m_commands.clear();
		m_offsetToCommand.clear();

		{
			m_cacheMutex.lock();
			m_cachedCommands.clear();
			m_cachedOffsetToCommand.clear();
			m_cacheMutex.unlock();
		}

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
						emit appCodeParsed(false);
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
						emit appCodeParsed(false);
						return false;
					}

					programCounter += commandSize;

					//qDebug() << m_commands.back().m_string;
					break;
				}
			}

			if (commandFound == false)
			{
				QString str = tr("Parse command error, command cannot be found for word 0x%1\n")
									.arg(commandWord, 4, 16, QChar('0'));
				writeError(str);

				emit appCodeParsed(false);
				return false;
			}

		}
		while (programCounter < m_plainAppLogic.size());

		// This block is related to Mutex, it;s nit about pre do-while loop
		//
		{
			m_cacheMutex.lock();
			m_cachedCommands = m_commands;

			m_cachedOffsetToCommand.clear();
			for (size_t i = 0; i < m_offsetToCommand.size(); i++)
			{
				int value = m_offsetToCommand[i];
				if (value != -1)
				{
					m_cachedOffsetToCommand[static_cast<int>(i)] = static_cast<size_t>(value);
				}
			}

			m_cacheMutex.unlock();
		}

		emit appCodeParsed(true);

		return true;
	}

	bool DeviceEmulator::parseCommand(const LmCommand& command, int programCounter)
	{
		if (m_commandProcessor == nullptr)
		{
			assert(m_commandProcessor);
			return  false;
		}

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

		try
		{
			if (bool ok = m_commandProcessor->parseFunc(deviceCommand.m_command.parseFunc, &deviceCommand);
				ok == false)
			{
				 SimException::raise("m_commandProcessor->parseFunc error.", "DeviceEmulator::parseCommand");
			}
		}
		catch (SimException& e)
		{
			writeError(QString("Command parsing error: %1, %2. ProgrammCounter = %3, ParseFunction = %4")
						.arg(e.message())
						.arg(e.where())
						.arg(programCounter)
						.arg(deviceCommand.m_command.parseFunc));
			return false;
		}

		// Debug
		//
		//deviceCommand.dump();

		// Add command to offsetToCommand map
		//
		while (m_offsetToCommand.size() < deviceCommand.m_offset)
		{
			m_offsetToCommand.push_back(-1);
		}

		m_offsetToCommand.push_back(static_cast<int>(m_commands.size() - 1));

		return true;
	}

	bool DeviceEmulator::processOperate()
	{
		// One LogicModule Cycle
		//
		bool result = true;

		// Initialization before work cycle
		//
		m_logicUnit = LogicUnitData();

		if (m_overrideSignals != nullptr)
		{
			m_ram.updateOverrideData(equpimnetId(), m_overrideSignals);
		}

		// Run work cylce
		//
		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
			  (m_logicUnit.phase == CyclePhase::IdrPhase || m_logicUnit.phase == CyclePhase::AlpPhase))
		{
			if (m_logicUnit.programCounter >= m_offsetToCommand.size())
			{
				assert(false);
				FAULT("Command not found in current ProgramCounter.");
				break;
			}

			int commandIndex = m_offsetToCommand[m_logicUnit.programCounter];

			if (commandIndex == -1 || commandIndex > m_commands.size())
			{
				FAULT(QString("Command not found for ProgramCounter %1").arg(m_logicUnit.programCounter));
				break;
			}

			DeviceCommand& command = m_commands[commandIndex];
			assert(m_logicUnit.programCounter == command.m_offset);

			if (bool ok = runCommand(command);
				ok == false && m_currentMode != DeviceMode::Fault)
			{
				FAULT(QString("Run command %1 unknown error.").arg(command.m_string));
				result = false;
				break;
			}

			if (m_currentMode == DeviceMode::Fault)
			{
				result = false;
				break;
			}

			// If ProgramCounter was not changed in runCommand (can be changed by APPSTART command), then
			// incerement ProgramCounter to coommand size
			//
			if (m_logicUnit.programCounter == command.m_offset)
			{
				m_logicUnit.programCounter += command.m_size;
			}
		}

		return result;
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

		emit faulted(str1 + "\n" + str2 + "\n" + str3);

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

	bool DeviceEmulator::runCommand(DeviceCommand& deviceCommand)
	{
		//qDebug() << "DeviceEmulator::runCommand" << "| " << deviceCommand.m_string;

		// Run command
		//
		try
		{
			if (bool ok = m_commandProcessor->runCommand(deviceCommand);
				ok == false)
			{
				SimException::raise(QString("Cannot call %1 function").arg(deviceCommand.m_command.simulationFunc), "DeviceEmulator::runCommand");
			}

		}
		catch (SimException& e)
		{
			writeError(QString("Command run error: %1, %2. Offset = %3, SimFunction = %4")
						.arg(e.message())
						.arg(e.where())
						.arg(deviceCommand.m_offset)
						.arg(deviceCommand.m_command.simulationFunc));
			return false;
		}
		catch (...)
		{
			writeError(QString("Call function %1 unknown exception")
					   .arg(deviceCommand.m_command.simulationFunc));
			return false;
		}

		return true;
	}

	// Getting data from m_plainAppLogic
	//
	quint16 DeviceEmulator::getWord(int wordOffset)
	{
		return getData<quint16>(wordOffset * 2);
	}

	quint32 DeviceEmulator::getDword(int wordOffset)
	{
		return getData<quint32>(wordOffset * 2);
	}

	template <typename TYPE>
	TYPE DeviceEmulator::getData(int eepromOffset)
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

	QString DeviceEmulator::equpimnetId() const
	{
		return logicModuleInfo().equipmentId;
	}

	Hardware::LogicModuleInfo DeviceEmulator::logicModuleInfo() const
	{
		m_cacheMutex.lock();
		Hardware::LogicModuleInfo result = m_cachedLogicModuleInfo;
		m_cacheMutex.unlock();

		return result;
	}

	void DeviceEmulator::setLogicModuleInfo(const Hardware::LogicModuleInfo& lmInfo)
	{
		m_logicModuleInfo = lmInfo;

		m_cacheMutex.lock();
		m_cachedLogicModuleInfo = lmInfo;
		m_cacheMutex.unlock();

		return;
	}

	const LmDescription& DeviceEmulator::lmDescription() const
	{
		return m_lmDescription;
	}

	void DeviceEmulator::setOverrideSignals(OverrideSignals* overrideSignals)
	{
		m_overrideSignals = overrideSignals;
	}

	std::vector<DeviceCommand> DeviceEmulator::commands() const
	{
		QMutexLocker ml(&m_cacheMutex);
		assert(m_commands.size() == m_cachedCommands.size());
		return m_cachedCommands;
	}

	std::unordered_map<int, size_t> DeviceEmulator::offsetToCommands() const
	{
		QMutexLocker ml(&m_cacheMutex);
		assert(m_cachedOffsetToCommand.size() == m_cachedOffsetToCommand.size());
		return m_cachedOffsetToCommand;
	}

	const Ram& DeviceEmulator::ram() const
	{
		return m_ram;
	}


}
