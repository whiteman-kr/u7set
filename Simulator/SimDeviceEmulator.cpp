#include "SimDeviceEmulator.h"
#include <QtEndian>
#include "SimException.h"
#include "SimCommandProcessor.h"
#include "Simulator.h"

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
		writeDebug("~DeviceEmulator");
		return;
	}

	bool DeviceEmulator::clear()
	{
		setLogicModuleInfo(Hardware::LogicModuleInfo());

		m_commandProcessor.reset();

		m_lmDescription.clear();
		m_logicModuleExtraInfo = {};

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

	DeviceError DeviceEmulator::init(const Hardware::LogicModuleInfo& logicModuleInfo,
									 const LmDescription& lmDescription,
									 const Eeprom& tuningEeprom,
									 const Eeprom& confEeprom,
									 const Eeprom& appLogicEeprom,
									 const Connections& connections)
	{
		clear();

		setOutputScope(QString("DeviceEmulator %1").arg(logicModuleInfo.equipmentId));
		writeDebug(tr("Init device."));

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
			return DeviceError::NoCommandProcessor;
		}

		//--
		//
		if (bool ok = m_afbComponents.init(m_lmDescription);
			ok == false)
		{
			return DeviceError::AfbComponentNotFound;
		}

		// --
		//
		if (bool ok = initEeprom();
			ok == false)
		{
			return DeviceError::InitEepromError;
		}

		// Init RAM
		// Command processor needs initialized memory areas, so it can cache them to DeviceCommand
		//
		initMemory();

		// Get LMs connections
		//
		m_connections = connections.lmConnections(equipmentId());

		// --
		//
		if (bool ok = parseAppLogicCode();
			ok == false)
		{
			return DeviceError::ParsingAppLogicError;
		}

		return DeviceError::Ok;
	}

	bool DeviceEmulator::powerOff()
	{
		writeDebug(tr("Off"));
		setCurrentMode(DeviceMode::Off);
		return true;
	}

	bool DeviceEmulator::reset()
	{
		writeDebug(tr("Reset"));
		setCurrentMode(DeviceMode::Start);
		return true;
	}

	bool DeviceEmulator::runWorkcycle(std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle)
	{
		bool ok = false;

		do
		{
			if (m_currentMode == DeviceMode::Start)
			{
				if (bool ok = processStartMode();
					ok == false)
				{
					return false;
				}
			}

			if (m_currentMode == DeviceMode::Fault)
			{
				ok = processFaultMode();
				break;
			}
			else
			{
				if (m_currentMode == DeviceMode::Off)
				{
					ok = processOffMode();
				}
				else
				{
					Q_ASSERT(m_currentMode == DeviceMode::Operate);
					ok = processOperate(currentTime, currentDateTime, workcycle);
				}
				break;
			}
		}
		while (false);

		// Perform post run cycle actions
		//
		if (m_appSignalManager == nullptr ||
			m_appDataTransmitter == nullptr)
		{
			Q_ASSERT(m_appSignalManager);
			Q_ASSERT(m_appDataTransmitter);

			return false;
		}

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime);

		TimeStamp plantTime{ms.count() + QDateTime::currentDateTime().offsetFromUtc() * 1000};
		TimeStamp localTime{plantTime};
		TimeStamp systemTime{ms.count()};

		// Set LogicModule's RAM to Sim::AppSignalManager
		//
		m_appSignalManager->setData(equipmentId(), ram(), plantTime, localTime, systemTime);

		// Send reg data to AppDataSrv
		//
		if (m_appDataTransmitter->enabled() == true && logicModuleExtraInfo().appDataEnable == true)
		{
			QByteArray regData;

			quint32 regDataOffsetW = m_lmDescription.memory().m_appLogicWordDataOffset;
			quint32 regDataSizeW = m_logicModuleExtraInfo.appDataSizeBytes / 2;

			if (regDataSizeW > m_lmDescription.memory().m_appLogicWordDataSize)
			{
				writeError(tr("Send reg data error, buffer size is too big. LM %1, OffsetW %2, SizeW %3, AppLogicWordDataSize %4.")
						   .arg(equipmentId())
						   .arg(regDataOffsetW)
						   .arg(regDataSizeW)
						   .arg(m_lmDescription.memory().m_appLogicWordDataSize));

				setCurrentMode(DeviceMode::Fault);
				return false;
			}

			bool readRegBufferOk = m_ram.readToBuffer(regDataOffsetW, E::LogicModuleRamAccess::Write, regDataSizeW, &regData, true);
			if (readRegBufferOk == false)
			{
				writeError(tr("Error reading regBuffer for LM %1, OffsetW %2, SizeW %3.")
						   .arg(equipmentId())
						   .arg(regDataOffsetW)
						   .arg(regDataSizeW));

				setCurrentMode(DeviceMode::Fault);
				return false;
			}

			m_appDataTransmitter->sendData(equipmentId(), std::move(regData), plantTime);
		}

		return ok;
	}

	DeviceCommand* DeviceEmulator::command(int index)
	{
		if (index >= static_cast<int>(m_commands.size()))
		{
			fault(QString("Index of m_commands is out of range"), "DeviceEmulator::command(int index)");
			return nullptr;
		}

		return &(m_commands[index]);
	}

	quint16 DeviceEmulator::appStartAddress() const
	{
		return m_logicUnit.appStartAddress;
	}

	void DeviceEmulator::setAppStartAddress(quint16 value)
	{
		m_logicUnit.appStartAddress = value;
	}

	Sim::CyclePhase DeviceEmulator::phase() const
	{
		return m_logicUnit.phase;
	}

	void DeviceEmulator::setPhase(Sim::CyclePhase value)
	{
		m_logicUnit.phase = value;
	}

	quint32 DeviceEmulator::programCounter() const
	{
		return m_logicUnit.programCounter;
	}

	void DeviceEmulator::setProgramCounter(quint32 value)
	{
		m_logicUnit.programCounter = value;
	}

	quint32 DeviceEmulator::flagCmp() const
	{
		return m_logicUnit.flags.cmp;
	}

	void DeviceEmulator::setFlagCmp(quint32 value)
	{
		m_logicUnit.flags.cmp = value;
	}

	Sim::AfbComponent DeviceEmulator::afbComponent(int opCode) const
	{
		return AfbComponent{m_lmDescription.component(opCode)};
	}

	Sim::AfbComponentInstance* DeviceEmulator::afbComponentInstance(int opCode, int instanceNo)
	{
		Sim::AfbComponentInstance* result = m_afbComponents.componentInstance(opCode, instanceNo);

		if (result == nullptr)
		{
			fault(QString("Attempt to get not existing AFB (opcode %1) instance (%2)")
							.arg(opCode)
							.arg(instanceNo),
							"ScriptDeviceEmulator::afbComponentInstance");
		}

		return result;
	}

	bool DeviceEmulator::setAfbParam(int afbOpCode, int instanceNo, const AfbComponentParam& param)
	{
		QString errorMessage;
		bool ok = m_afbComponents.addInstantiatorParam(afbOpCode, instanceNo, param, &errorMessage);

		if (ok == false)
		{
			SIM_FAULT(QString("Add addInstantiatorParam error, %1").arg(errorMessage));
		}

		return ok;
	}

	bool DeviceEmulator::setAfbParam(int afbOpCode, int instanceNo, AfbComponentParam&& param)
	{
		QString errorMessage;
		bool ok = m_afbComponents.addInstantiatorParam(afbOpCode, instanceNo, std::move(param), &errorMessage);

		if (ok == false)
		{
			SIM_FAULT(QString("Add addInstantiatorParam error, %1").arg(errorMessage));
		}

		return ok;
	}

	// RAM access
	//
	bool DeviceEmulator::movRamMem(quint32 src, quint32 dst, quint32 sizeW)
	{
//		while (size-- > 0)
//		{
//			quint16 w = readRamWord(src++);
//			writeRamWord(dst++, w);
//		}

		switch (sizeW)
		{
		case 1:
			{
				quint16 w = readRamWord(src);
				writeRamWord(dst, w);
			}
			return true;
		case 2:
			{
				quint32 dw = readRamDword(src);
				writeRamDword(dst, dw);
			}
			return true;
		default:
			{
				bool result = m_ram.movMem(src, dst, sizeW);
				if (result == false)
				{
					SIM_FAULT(QString("move ram mem error, srcW %1, dstW %2, sizeW %3")
								.arg(src)
								.arg(dst)
								.arg(sizeW));
				}

				return result;
			}
		}
	}

	bool DeviceEmulator::movRamMem(Ram::Handle memoryAreaHandleSrc, quint32 src, Ram::Handle memoryAreaHandleDst, quint32 dst, quint32 sizeW)
	{
		RamArea* ramAreaSrc = m_ram.memoryArea(memoryAreaHandleSrc);
		if (ramAreaSrc == nullptr)
		{
			SIM_FAULT(QString("Command movmem error, memory area src handle %1 not found").arg(memoryAreaHandleSrc));
		}

		RamArea* ramAreaDst = m_ram.memoryArea(memoryAreaHandleDst);
		if (ramAreaDst == nullptr)
		{
			SIM_FAULT(QString("Command movmem error, memory area dst handle %1 not found").arg(memoryAreaHandleDst));
		}

		QByteArray buffer;

		bool ok = ramAreaSrc->readToBuffer(src, sizeW, &buffer, true);
		if (ok == false)
		{
			SIM_FAULT(QString("Command movmem error, read buffer error, handle %1, offset %2, sizeW %3")
						.arg(memoryAreaHandleSrc)
						.arg(src)
						.arg(sizeW));
		}

		ok = ramAreaDst->writeBuffer(dst, buffer);
		if (ok == false)
		{
			SIM_FAULT(QString("Command movmem error, write data error, handle %1, offset %2, sizeW %3")
					  .arg(memoryAreaHandleDst)
					  .arg(dst)
					  .arg(sizeW));
		}

		return ok;
	}

	bool DeviceEmulator::setRamMem(quint32 address, quint16 data, quint16 size)
	{
		switch (size)
		{
		case 1:
			{
				writeRamWord(address, data);
				return true;
			}
		case 2:
			{
				writeRamWord(address++, data);
				writeRamWord(address, data);
				return true;
			}
		default:
			{
				bool ok = m_ram.setMem(address, size, data);
				if (ok == false)
				{
					SIM_FAULT(QString("setmem error, address %1, sizeW %2, data %3")
								.arg(address)
								.arg(size)
								.arg(data));
				}

				return ok;
			}
		}
	}

	bool DeviceEmulator::setRamMem(Ram::Handle memoryAreaHandle, quint32 address, quint16 data, quint16 size)
	{
		switch (size)
		{
		case 1:
			{
				writeRamWord(memoryAreaHandle, address, data);
				return true;
			}
		case 2:
			{
				writeRamWord(memoryAreaHandle, address++, data);
				writeRamWord(memoryAreaHandle, address, data);
				return true;
			}
		default:
			{
				RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
				if (ramArea == nullptr)
				{
					SIM_FAULT(QString("setmem error, can't get memory area by handle %1").arg(memoryAreaHandle));
				}

				bool ok = ramArea->setMem(address, size, data);
				if (ok == false)
				{
					SIM_FAULT(QString("setmem error, address %1, sizeW %2, data %3")
								.arg(address)
								.arg(size)
								.arg(data));
				}

				return ok;
			}
		}
	}

	bool DeviceEmulator::writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data)
	{
		bool ok = m_ram.writeBit(offsetW, bitNo, data, E::ByteOrder::BigEndian);
		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return ok;
	}

	bool DeviceEmulator::writeRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo, quint16 data)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Write RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		bool ok = ramArea->writeBit(offsetW, bitNo, data, E::ByteOrder::BigEndian);
		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return ok;
	}

	quint16 DeviceEmulator::readRamBit(quint32 offsetW, quint16 bitNo)
	{
		quint16 data = 0;
		bool ok = m_ram.readBit(offsetW, bitNo, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return data;
	}

	quint16 DeviceEmulator::readRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Read RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		quint16 data = 0;
		bool ok = ramArea->readBit(offsetW, bitNo, &data, E::ByteOrder::BigEndian, true);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1, bitNo %2").arg(offsetW).arg(bitNo));
		}

		return data;
	}

	bool DeviceEmulator::writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data, E::LogicModuleRamAccess access)
	{
		bool ok = m_ram.writeBit(offsetW, bitNo, data, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1, bitNo %2, acess %3")
					  .arg(offsetW)
					  .arg(bitNo)
					  .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return ok;
	}

	quint16 DeviceEmulator::readRamBit(quint32 offsetW, quint16 bitNo, E::LogicModuleRamAccess access)
	{
		quint16 data = 0;
		bool ok = m_ram.readBit(offsetW, bitNo, &data, E::ByteOrder::BigEndian, access);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1, bitNo %2, access %3")
					  .arg(offsetW)
					  .arg(bitNo)
					  .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return data;
	}

	bool DeviceEmulator::writeRamWord(quint32 offsetW, quint16 data)
	{
		bool ok = m_ram.writeWord(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	bool DeviceEmulator::writeRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 data)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Write RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		bool ok = ramArea->writeWord(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	quint16 DeviceEmulator::readRamWord(quint32 offsetW)
	{
		quint16 data = 0;
		bool ok = m_ram.readWord(offsetW, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	quint16 DeviceEmulator::readRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Read RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		quint16 data = 0;
		bool ok = ramArea->readWord(offsetW, &data, E::ByteOrder::BigEndian, true);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	bool DeviceEmulator::writeRamWord(quint32 offsetW, quint16 data, E::LogicModuleRamAccess access)
	{
		bool ok = m_ram.writeWord(offsetW, data, E::ByteOrder::BigEndian, access);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1, access %2")
			          .arg(offsetW)
			          .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return ok;
	}
	quint16 DeviceEmulator::readRamWord(quint32 offsetW, E::LogicModuleRamAccess access)
	{
		quint16 data = 0;
		bool ok = m_ram.readWord(offsetW, &data, E::ByteOrder::BigEndian, access);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1, access %2")
			          .arg(offsetW)
			          .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return data;
	}

	bool DeviceEmulator::writeRamDword(quint32 offsetW, quint32 data)
	{
		bool ok = m_ram.writeDword(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	bool DeviceEmulator::writeRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW, quint32 data)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Write RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		bool ok = ramArea->writeDword(offsetW, data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1").arg(offsetW));
		}

		return ok;
	}

	quint32 DeviceEmulator::readRamDword(quint32 offsetW)
	{
		quint32 data = 0;
		bool ok = m_ram.readDword(offsetW, &data, E::ByteOrder::BigEndian);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	quint32 DeviceEmulator::readRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW)
	{
		RamArea* ramArea = m_ram.memoryArea(memoryAreaHandle);
		if (ramArea == nullptr)
		{
			SIM_FAULT(QString("Read RAM error, can't get memory area by handle %1").arg(memoryAreaHandle));
		}

		quint32 data = 0;
		bool ok = ramArea->readDword(offsetW, &data, E::ByteOrder::BigEndian, true);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1").arg(offsetW));
		}

		return data;
	}

	bool DeviceEmulator::writeRamDword(quint32 offsetW, quint32 data, E::LogicModuleRamAccess access)
	{
		bool ok = m_ram.writeDword(offsetW, data, E::ByteOrder::BigEndian, access);

		if (ok == false)
		{
			SIM_FAULT(QString("Write RAM error, offsetW %1, access %2")
			          .arg(offsetW)
			          .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return ok;
	}

	quint32 DeviceEmulator::readRamDword(quint32 offsetW, E::LogicModuleRamAccess access)
	{
		quint32 data = 0;
		bool ok = m_ram.readDword(offsetW, &data, E::ByteOrder::BigEndian, access);

		if (ok == false)
		{
			SIM_FAULT(QString("Read RAM error, offsetW %1, access %2")
			          .arg(offsetW)
			          .arg(E::valueToString<E::LogicModuleRamAccess>(access)));
		}

		return data;
	}

	// Getting data from m_plainAppLogic
	//
	quint16 DeviceEmulator::getWord(int wordOffset) const
	{
		return getData<quint16>(wordOffset * 2);
	}

	quint32 DeviceEmulator::getDword(int wordOffset) const
	{
		return getData<quint32>(wordOffset * 2);
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
									  false,
									  QString("Input I/O Module %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
									  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
									  memory.m_moduleDataSize,
									  true,
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
									  false,
									  QString("Rx Opto Port  %1").arg(i + 1));

			ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
									  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
									  optoInterface.m_optoPortDataSize,
									  false,
									  QString("Tx Opto Port  %1").arg(i + 1));
		}

		// RAM - ApplicationLogic Block, bit/word access
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
								  memory.m_appLogicBitDataOffset,
								  memory.m_appLogicBitDataSize,
								  false,
								  QLatin1String("Application Logic Block (bit access)"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
								  memory.m_appLogicWordDataOffset,
								  memory.m_appLogicWordDataSize,
								  false,
								  QLatin1String("Application Logic Block (word access)"));

		// RAM - Tuninng Block
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_tuningDataOffset,
								  memory.m_tuningDataSize,
								  false,
								  QLatin1String("Tuning Block"));

		// Copy EEPROM tuning data to memory
		//
		{
			Q_ASSERT(m_plainTuningData.size() % 2 == 0);

			const quint16* dataPtr = reinterpret_cast<const quint16*>(m_plainTuningData.constData());
			quint32 tuningDataOffset = lmDescription().memory().m_tuningDataOffset;

			for (int offsetW = 0; offsetW < m_plainTuningData.size() / 2; offsetW++)
			{
				quint16 data =  dataPtr[offsetW];
				m_ram.writeWord(tuningDataOffset + offsetW, data, E::ByteOrder::NoEndian, E::LogicModuleRamAccess::Read);
			}
		}

		// RAM - Diag Data
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  false,
								  QLatin1String("Input Diag Data"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
								  memory.m_txDiagDataOffset,
								  memory.m_txDiagDataSize,
								  false,
								  QLatin1String("Output Diag Data"));


		// RAM - App Data
		//
		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  false,
								  QLatin1String("Input App Data"));

		ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
								  memory.m_appDataOffset,
								  memory.m_appDataSize,
								  false,
								  QLatin1String("Output App Data"));

		return ok;
	}

	bool DeviceEmulator::initEeprom()
	{
		writeDebug(tr("Init EEPROM"));

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
		{
			m_plainAppLogic.clear();
			m_plainAppLogic.reserve(m_appLogicEeprom.size());

			int startFrame = m_appLogicEeprom.configFrameIndex(m_logicModuleInfo.lmNumber);
			if (startFrame == 0)
			{
				writeError(QString("Can't get start frame for logic number %1 in m_appLogicEeprom").arg(m_logicModuleInfo.lmNumber));
				return false;
			}

			int frameCount = m_appLogicEeprom.configFramesCount(m_logicModuleInfo.lmNumber);
			Q_ASSERT(startFrame + frameCount < m_appLogicEeprom.frameCount());

			for (int frameIndex = startFrame;
				 frameIndex < startFrame + frameCount && frameIndex < m_appLogicEeprom.frameCount();
				 frameIndex++)
			{
				for (int offset = 0; offset < m_appLogicEeprom.framePayloadSize(); offset++)
				{
					m_plainAppLogic.push_back(m_appLogicEeprom.getByte(frameIndex, offset));
				}
			}
		}

		// Get plain tuning data for specific LmNumber
		//
		{
			m_plainTuningData.clear();
			m_plainTuningData.reserve(m_tuningEeprom.size());

			int tuningStartFrame = m_tuningEeprom.configFrameIndex(m_logicModuleInfo.lmNumber);
			if (tuningStartFrame == 0)
			{
				writeError(QString("Can't get start frame for logic number %1 in m_tuningEeprom").arg(m_logicModuleInfo.lmNumber));
				return false;
			}

			int tuningFrameCount = m_tuningEeprom.configFramesCount(m_logicModuleInfo.lmNumber);
			Q_ASSERT(tuningStartFrame + tuningFrameCount < m_tuningEeprom.frameCount());

			for (int frameIndex = tuningStartFrame;
				 frameIndex < tuningStartFrame + tuningFrameCount && frameIndex < m_tuningEeprom.frameCount();
				 frameIndex++)
			{
				for (int byteNo = 0; byteNo < m_tuningEeprom.framePayloadSize(); byteNo++)
				{
					// 1st frame is service information  [D8.21.10, 3.1.1.3]
					//
					m_plainTuningData.push_back(m_tuningEeprom.getByte(frameIndex, byteNo));
				}
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

		m_commandProcessor->beforeAppLogicParse();

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
					Q_ASSERT(m_commands.empty() == false);

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

					//qDebug() << m_commands.back().m_string;
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

		// Let command processor to make its' cache optimizations
		//
		m_commandProcessor->afterAppLogicParse(&m_commands);

		// This block is related to Mutex, it's not about pre do-while loop
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

		return true;
	}

	bool DeviceEmulator::parseCommand(const LmCommand& command, int programCounter)
	{
		if (m_commandProcessor == nullptr)
		{
			Q_ASSERT(m_commandProcessor);
			return  false;
		}

		quint16 commandWord = getWord(programCounter);
		quint16 commandCode = (commandWord & command.codeMask);
		if (commandCode != command.code)
		{
			Q_ASSERT(commandCode == command.code);
			return false;
		}

		// --
		//
		DeviceCommand& deviceCommand = m_commands.emplace_back(command);
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
			writeError(QString("Command parsing error: %1, %2. ProgrammCounter = %3 (0x%4), ParseFunction = %5")
						.arg(e.message())
						.arg(e.where())
						.arg(programCounter)
						.arg(programCounter, 0, 16, QChar('0'))
						.arg(deviceCommand.m_command.parseFunc));
			return false;
		}

		// Debug
		//
		//deviceCommand.dump();

		// Add command to offsetToCommand map
		//
		while (static_cast<int>(m_offsetToCommand.size()) < deviceCommand.m_offset)
		{
			m_offsetToCommand.push_back(-1);
		}

		m_offsetToCommand.push_back(static_cast<int>(m_commands.size() - 1));

		return true;
	}

	bool DeviceEmulator::processOperate(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 /*workcycle*/)
	{
		//qDebug() << "DeviceEmulator::processOperate " << equipmentId();

		// One LogicModule Cycle
		//
		bool result = true;

		// Initialization before work cycle
		//
		m_logicUnit = LogicUnitData();
		m_commandProcessor->updatePlatformInterfaceState(currentDateTime);

		if (m_overrideSignals != nullptr)
		{
			m_ram.updateOverrideData(equipmentId(), m_overrideSignals);
		}
		else
		{
			Q_ASSERT(m_overrideSignals);
		}

		// COMMENTED as for now there is no need to zero IO modules memory
		// as there is no control of reading uninitialized memory.
		//
		//m_ram.clearMemoryAreasOnStartCycle();				// Reset to 0 some meory areas before start work cylce (like memory area for write i/o modules)

		// Get data from fiber optic channels (LM, OCM)
		// !!! receiveConnectionsData !!! was moved to Sim::Control,
		// as it is must be called before ALL LMs started to avoid gaps in communication
		//
		//		result = receiveConnectionsData(currentTime);
		//		if (result == false)
		//		{
		//			return false;
		//		}

		// Run work cylce
		//
		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
			  (m_logicUnit.phase == CyclePhase::IdrPhase || m_logicUnit.phase == CyclePhase::AlpPhase))
		{
			if (m_logicUnit.programCounter >= static_cast<int>(m_offsetToCommand.size()))
			{
				Q_ASSERT(false);
				SIM_FAULT("Command not found in current ProgramCounter.");
				break;
			}

			int commandIndex = m_offsetToCommand[m_logicUnit.programCounter];

			if (commandIndex == -1 || commandIndex > static_cast<int>(m_commands.size()))
			{
				SIM_FAULT(QString("Command not found for ProgramCounter %1").arg(m_logicUnit.programCounter));
				break;
			}

			DeviceCommand& command = m_commands[commandIndex];
			Q_ASSERT(m_logicUnit.programCounter == command.m_offset);

			if (bool ok = runCommand(command);
				ok == false && m_currentMode != DeviceMode::Fault)
			{
				SIM_FAULT(QString("Run command %1 unknown error.").arg(command.m_string));
				result = false;
				break;
			}

			if (m_currentMode == DeviceMode::Fault)
			{
				result = false;
				break;
			}

			if (m_currentMode == DeviceMode::Fault)
			{
				result = true;
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

		// Send data to fiber optic channels (LM, OCM)
		//
		result &= sendConnectionsData(currentTime);

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
			Q_ASSERT(false);
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

		setCurrentMode(DeviceMode::Fault);

		return;
	}

	bool DeviceEmulator::processOffMode()
	{
		Q_ASSERT(m_currentMode == DeviceMode::Off);
		return true;
	}

	bool DeviceEmulator::processStartMode()
	{
		Q_ASSERT(m_currentMode == DeviceMode::Start);
		writeDebug(tr("Start mode"));

		bool ok = initMemory();
		if (ok == false)
		{
			writeError(tr("Init memory error."));
			setCurrentMode(DeviceMode::Fault);
			return false;
		}

		m_afbComponents.resetState();	// It will clear all AFBs' params

		setCurrentMode(DeviceMode::Operate);

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
			writeError(QString("Command run error: %1, %2. Offset = %3 (%4), SimFunction = %5")
						.arg(e.message())
						.arg(e.where())
						.arg(deviceCommand.m_offset)
						.arg(deviceCommand.m_offset, 0, 16)
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

	bool DeviceEmulator::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		if (m_currentMode == DeviceMode::Off)
		{
			return true;
		}

		for (ConnectionPtr& c : m_connections)
		{
			if (c->enabled() == false)
			{
				// Even though the connection is disabled we should procced it to zero
				// memory and to set validity flag. If connection is disabled nothing will be send and
				// then nothing can be received, it will cause timeout
				//
			}

			// Get port for this connection for this LM
			//
			ConnectionPortPtr port = c->portForLm(equipmentId());
			if (port == nullptr)
			{
				Q_ASSERT(port);
				SIM_FAULT(QString("Communication port not found for connection %1 in LM %2.")
						  .arg(c->connectionId())
						  .arg(equipmentId()));
				return false;
			}

			// Get receive buffer for port
			//
			const ::ConnectionPortInfo& portInfo = port->portInfo();

			Q_ASSERT(portInfo.lmID == equipmentId());

			//std::vector<char> rb;
			//std::vector<char>* receiveBuffer = &rb;
			std::vector<char>* receiveBuffer = c->getPortReceiveBuffer(portInfo.portNo);
			if (receiveBuffer == nullptr)
			{
				SIM_FAULT(QString("Get port receive buffer error, connection %1, port %2 (%3).")
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

			// Get data (actually swap) to receive buffer
			//
			bool timeout = false;
			bool ok = c->receiveData(portInfo.portNo,
									 receiveBuffer,
									 currentTime,
									 std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration * 2},	// timeout
									 &timeout);

			if (ok == false)
			{
				SIM_FAULT(QString("Receive data error, connection %1, port %2 (%3).")
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

			if (timeout == true)
			{
				// If receive buffer is empty then it is timeout
				// Clear memory in dedicated memory area
				//
				m_ram.clearMemoryArea(portInfo.rxBufferAbsAddr, E::LogicModuleRamAccess::Read);

				//qDebug() << "DeviceEmulator::receiveConnectionsData: Connection timeout " << c->connectionId();
			}
			else
			{
				if (receiveBuffer->empty() == true)
				{
					// If timeout not happened yet, but receiveBuffer is empty, wait mor time
					// do not exit from function here, later validity bit will be written
					//
				}
				else
				{
					// Payload is received
					// Write data to memory
					//
					Q_ASSERT(receiveBuffer->size() % 2 == 0);

					if (receiveBuffer->size() / 2 != portInfo.rxDataSizeW)
					{
						SIM_FAULT(QString("Receive data error, expected %1 words but received %2 words, connection %3, port %4 (%5).")
								  .arg(portInfo.rxDataSizeW)
								  .arg(receiveBuffer->size() / 2)
								  .arg(c->connectionId())
								  .arg(portInfo.portNo)
								  .arg(portInfo.equipmentID));
						return false;
					}

					ok = m_ram.writeBuffer(portInfo.rxBufferAbsAddr, E::LogicModuleRamAccess::Read, *receiveBuffer);
					if (ok == false)
					{
						SIM_FAULT(QString("Received buffer write memory error, %1 words, connection %2, port %3 (%4).")
								  .arg(portInfo.rxDataSizeW)
								  .arg(c->connectionId())
								  .arg(portInfo.portNo)
								  .arg(portInfo.equipmentID));
						return false;
					}

					// Check the DataID which is the part of the payload (first 4 bytes)
					//
					if (c->connectionInfo().disableDataIDControl == false)
					{
						quint32 receivedDataId = readRamDword(portInfo.rxBufferAbsAddr);

						if (port->portInfo().rxDataID != receivedDataId)
						{
							SIM_FAULT(QString("Received DataID mismatch, received 0x%1, expected 0x%2, connection %3, port %4 (%5).")
									  .arg(receivedDataId, 8, 16, QChar('0'))
									  .arg(port->portInfo().rxDataID, 8, 16, QChar('0'))
									  .arg(c->connectionId())
									  .arg(portInfo.portNo)
									  .arg(portInfo.equipmentID));
							return false;
						}
					}
				}
			}

			// Set port receive validity flag to 0 or 1
			//
			ok = m_ram.writeBit(portInfo.rxValiditySignalAbsAddr.offset(),
								portInfo.rxValiditySignalAbsAddr.bit(),
								timeout ? 0x0000 : 0x0001,
								E::ByteOrder::BigEndian,
								E::LogicModuleRamAccess::Read);
			if (ok == false)
			{
				SIM_FAULT(QString("Write receive validity signal error, signal %1 (%2), connection %3, port %4 (%5).")
						  .arg(portInfo.rxValiditySignalEquipmentID)
						  .arg(portInfo.rxValiditySignalAbsAddr.toString())
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

		}

		return true;
	}

	bool DeviceEmulator::sendConnectionsData(std::chrono::microseconds currentTime)
	{
		if (m_currentMode == DeviceMode::Off)
		{
			return true;
		}

		for (ConnectionPtr& c : m_connections)
		{
			if (c->enabled() == false)
			{
				// Connection is disabled, just skip it
				//
				continue;
			}

			// Get port for this connection for this LM
			//
			ConnectionPortPtr port = c->portForLm(equipmentId());
			if (port == nullptr)
			{
				Q_ASSERT(port);
				SIM_FAULT(QString("Communication port not found for connection %1 in LM %2.")
						  .arg(c->connectionId())
						  .arg(equipmentId()));
				return false;
			}

			// Get send buffer for port
			//
			const ::ConnectionPortInfo& portInfo = port->portInfo();

			Q_ASSERT(portInfo.lmID == equipmentId());

			//std::vector<char> sb;
			//std::vector<char>* sendBuffer = &sb;
			std::vector<char>* sendBuffer = c->getPortSendBuffer(portInfo.portNo);
			if (sendBuffer == nullptr)
			{
				SIM_FAULT(QString("Get port send buffer error, connection %1, port %2 (%3).")
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

			// Write data from RAM to send buffer
			//
			bool ok = m_ram.readToBuffer(portInfo.txBufferAbsAddr, E::LogicModuleRamAccess::Write, portInfo.txDataSizeW, sendBuffer);
			if (ok == false)
			{
				SIM_FAULT(QString("Send data error, read data from memory (address %1, count %2) returned error, connection %3, port %4 (%5).")
						  .arg(portInfo.txBufferAbsAddr)
						  .arg(portInfo.txDataSizeW)
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

			// Send data (actually swap) to send buffer
			//
			ok = c->sendData(portInfo.portNo, sendBuffer, currentTime);

			if (ok == false)
			{
				SIM_FAULT(QString("Send data error, connection %1, port %2 (%3).")
						  .arg(c->connectionId())
						  .arg(portInfo.portNo)
						  .arg(portInfo.equipmentID));
				return false;
			}

		}

		return true;
	}

	// Getting data from m_plainAppLogic
	//
	template <typename TYPE>
	TYPE DeviceEmulator::getData(int eepromOffset) const
	{
		// eepromOffset - in bytes
		//
		if (eepromOffset < 0 || eepromOffset > static_cast<int>(m_plainAppLogic.size() - sizeof(TYPE)))
		{
			Q_ASSERT(eepromOffset >= 0 &&
					 static_cast<int>(eepromOffset - sizeof(TYPE)) <= m_plainAppLogic.size());
			return 0;
		}

		TYPE result = qFromBigEndian<TYPE>(m_plainAppLogic.constData() + eepromOffset);
		return result;
	}

	const QString& DeviceEmulator::equipmentId() const
	{
		return m_logicModuleInfo.equipmentId;
	}

	int DeviceEmulator::buildNo() const
	{
		// On loading eeprom buildNo was checked, so it is guarantee to be the same across all the eeproms
		//
		return m_appLogicEeprom.buildNo();
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

	const ::LogicModuleInfo& DeviceEmulator::logicModuleExtraInfo() const
	{
		return m_logicModuleExtraInfo;
	}

	void DeviceEmulator::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_logicModuleExtraInfo = value;
	}

	const LmDescription& DeviceEmulator::lmDescription() const
	{
		return m_lmDescription;
	}

	void DeviceEmulator::setOverrideSignals(OverrideSignals* overrideSignals)
	{
		m_overrideSignals = overrideSignals;
	}

	void DeviceEmulator::setAppSignalManager(AppSignalManager* appSignalManager)
	{
		m_appSignalManager = appSignalManager;
	}

	void DeviceEmulator::setAppDataTransmitter(AppDataTransmitter* appDataTransmitter)
	{
		m_appDataTransmitter = appDataTransmitter;
	}

	std::vector<DeviceCommand> DeviceEmulator::commands() const
	{
		QMutexLocker ml(&m_cacheMutex);
		Q_ASSERT(m_commands.size() == m_cachedCommands.size());
		return m_cachedCommands;
	}

	std::unordered_map<int, size_t> DeviceEmulator::offsetToCommands() const
	{
		QMutexLocker ml(&m_cacheMutex);
		Q_ASSERT(m_cachedOffsetToCommand.size() == m_cachedOffsetToCommand.size());
		return m_cachedOffsetToCommand;
	}

	const Ram& DeviceEmulator::ram() const
	{
		return m_ram;
	}

	Ram& DeviceEmulator::mutableRam()
	{
		return m_ram;
	}

	DeviceMode DeviceEmulator::currentMode() const
	{
		return m_currentMode;
	}

	void DeviceEmulator::setCurrentMode(DeviceMode value)
	{
		m_currentMode = value;
	}
}
