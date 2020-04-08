#include <array>
#include <cfenv>
#include "SimCommandProcessor_LM5_LM6.h"
#include "SimException.h"
#include "SimAfb.h"

namespace Sim
{
	CommandProcessor_LM5_LM6::CommandProcessor_LM5_LM6(DeviceEmulator* device) :
		CommandProcessor(device)
	{
	}

	CommandProcessor_LM5_LM6::~CommandProcessor_LM5_LM6()
	{
	}

	void CommandProcessor_LM5_LM6::cacheCommands(std::vector<DeviceCommand>* commands)
	{
		static_assert(sizeof(SimCommandFuncCast::pmember) <= sizeof(SimCommandFuncCast::pvoid));

		for (DeviceCommand& command : *commands)
		{
			auto it = m_nameToFuncCommand.find(command.m_command.simulationFunc);
			if (it == m_nameToFuncCommand.end())
			{
				SimCommandFuncCast pcast;
				pcast.pmember = &CommandProcessor_LM5_LM6::command_not_implemented;

				command.m_commandFuncPtr = pcast.pvoid;
			}
			else
			{
				SimCommandFuncCast pcast;
				pcast.pmember = it->second;

				command.m_commandFuncPtr = pcast.pvoid;
			}
		}

		return;
	}

	bool CommandProcessor_LM5_LM6::updatePlatformInterfaceState()
	{
		// Blink signal, addr 57682[2] = 0xE152[2] -- read memry
		//
		if ((m_blinkCounter % (1000 / m_cycleDurationMs / 2)) == 0)
		{
			quint16 b = m_device->readRamBit(57682, 2, E::LogicModuleRamAccess::Read);
			m_device->writeRamBit(57682, 2, b == 0 ? 1 : 0, E::LogicModuleRamAccess::Read);
		}
		m_blinkCounter ++;

		// --
		//

		return true;
	}

	bool CommandProcessor_LM5_LM6::runCommand(const DeviceCommand& command)
	{
		SimCommandFuncCast pcast;
		pcast.pvoid = command.m_commandFuncPtr;

		auto& func = pcast.pmember;

		// Call the command
		//
		(this->*func)(command);

		return true;
	}

	void CommandProcessor_LM5_LM6::command_not_implemented(const DeviceCommand& command)
	{
		SimException::raise(QString("Command %1 is not implemented yet").arg(command.caption()), __FUNCTION__);
		return;
	}

	// Command: nop
	// Code: 1
	// Description: No operation
	//
	void CommandProcessor_LM5_LM6::parse_nop(DeviceCommand* command) const
	{
		command->m_size = 1;
		command->m_string = strCommand(command->caption());

		return;
	}

	void CommandProcessor_LM5_LM6::command_nop(const DeviceCommand& /*command*/)
	{
	}

	// Command: startafb
	// Code: 2
	// Description: Execute AFB
	//
	void CommandProcessor_LM5_LM6::parse_startafb(DeviceCommand* command) const
	{
		command->m_size = 2;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits

		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance);
		if (afb.simulationFunc().isEmpty() == true)
		{
			SimException::raise(QString("Simultaion function for AFB %1 is not found").arg(afb.caption()));
		}

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// startafb   LOGIC.0
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInst(command);

		//
		// Save pointer to function
		//
		auto it = m_nameToFuncAfb.find(afb.simulationFunc());
		if (it == m_nameToFuncAfb.end())
		{
			SimException::raise(QString("Cannot find AFB func %1").arg(afb.simulationFunc()), "CommandProcessor_LM5_LM6::command_startafb");
		}

		SimAfbFuncCast pcast;
		pcast.pmember = it->second;

		command->m_afbFuncPtr = pcast.pvoid;

		return;
	}

	void CommandProcessor_LM5_LM6::command_startafb(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = command.m_afbComponentInstance;

		if (afbInstance == nullptr)
		{
			SimException::raise(QString("Cannot find afbInstance with OpCode %1, InstanceNo %2")
									.arg(command.m_afbOpCode)
									.arg(command.m_afbInstance),
								"CommandProcessor_LM5_LM6::command_startafb");
		}

		// AFB
		//
		SimAfbFuncCast pcast;
		pcast.pvoid = command.m_afbFuncPtr;

		auto& func = pcast.pmember;

		// Call the command
		//
		(this->*func)(afbInstance);

		return;
	}

	// Command: stop
	// Code: 3
	// Description: Stop IDR phase and start ALP, if ALP is current phase then stop work cycle
	//
	void CommandProcessor_LM5_LM6::parse_stop(DeviceCommand* command) const
	{
		command->m_size = 1;
		command->m_string = command->m_command.caption;
		return;
	}

	void CommandProcessor_LM5_LM6::command_stop([[maybe_unused]] const DeviceCommand& command)
	{
		if (m_device->phase() == Sim::CyclePhase::IdrPhase)
		{
			m_device->setPhase(Sim::CyclePhase::AlpPhase);
			m_device->setProgramCounter(m_device->appStartAddress());
			return;
		}

		if (m_device->phase() == Sim::CyclePhase::AlpPhase)
		{
			m_device->setPhase(Sim::CyclePhase::ODT);
			return;
		}

		SimException::raise(QString("Command stop is cannot be run in current phase: %1")
								.arg(static_cast<int>(m_device->phase())));
		return;
	}

	// Command: mov
	// Code: 4
	// Description: Move word from RAM to RAM
	//
	void CommandProcessor_LM5_LM6::parse_mov(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - adderess2 - destionation
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - adderess1 - source

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_mov(const DeviceCommand& command)
	{
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		quint16 data = m_device->readRamWord(src);
		m_device->writeRamWord(dst, data);

		return;
	}

	// Command: movmem
	// Code: 5
	// Description: Move N words from RAM to RAM
	//
	void CommandProcessor_LM5_LM6::parse_movmem(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - adderess2
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - adderess1
		command->m_word2 = m_device->getWord(command->m_offset + 3);		// word2 - words to move

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1) + ", " +
							strAddr(command->m_word2);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movmem(const DeviceCommand& command)
	{
		const auto& size = command.m_word2;
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		m_device->movRamMem(src, dst, size);

		return;
	}

	// Command: movc
	// Code: 6
	// Description: Write word const to RAM
	//
	void CommandProcessor_LM5_LM6::parse_movc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - address
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - data

		// movc     0b402h, #0
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strWordConst(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movc(const DeviceCommand& command)
	{
		m_device->writeRamWord(command.m_word0, command.m_word1);
		return;
	}

	// Command: movbc
	// Code: 7
	// Description: Write constant bit to RAM
	//
	void CommandProcessor_LM5_LM6::parse_movbc(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - data address
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - data
		command->m_bitNo0 = m_device->getWord(command->m_offset + 3);	// bitNo0 - bitno

		checkParamRange(command->m_bitNo0, 0, 15, QStringLiteral("BitNo"));

		// MOVBC    0B402h[0], #0
		//
		command->m_string = strCommand(command->m_command.caption) +
							strBitAddr(command->m_word0, command->m_bitNo0) +
							", " +
							strBitConst(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movbc(const DeviceCommand& command)
	{
		m_device->writeRamBit(command.m_word0, command.m_bitNo0, command.m_word1);
		return;
	}

	// Command: wrfb
	// Code: 8
	// Description: Read 16bit data from RAM and write to AFB input
	//
	void CommandProcessor_LM5_LM6::parse_wrfb(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);					// Word0 - data address

		command->m_memoryAreaFrom = m_device->ram().memoryAreaHandle(E::LogicModuleRamAccess::Read, command->m_word0);

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) + ", " +
							strAddr(command->m_word0);
		return;
	}

	void CommandProcessor_LM5_LM6::command_wrfb(const DeviceCommand& command)
	{
		quint16 data = m_device->readRamWord(command.m_memoryAreaFrom, command.m_word0);

		bool ok = command.m_afbComponentInstance->addParam(AfbComponentParam{command.m_afbPinOpCode, data});
		if (ok == false)
		{
			m_device->SIM_FAULT(QString("Write param error"));
		}

//		m_device->setAfbParam(command.m_afbOpCode,
//							  command.m_afbInstance,
//							  AfbComponentParam{command.m_afbPinOpCode, data});
		return;
	}

	// Command: rdfb
	// Code: 9
	// Description: Read 16-bit word from AFB output and write to memory
	//
	void CommandProcessor_LM5_LM6::parse_rdfb(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);					// Word0 - data address

		command->m_memoryAreaTo = m_device->ram().memoryAreaHandle(E::LogicModuleRamAccess::Write, command->m_word0);
		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		// rdfb 0478h, LOGIC.0[i_2_oprd]
		//
		command->m_string = strCommand(command->caption()) +
								strAddr(command->m_word0) +
								", " +
								strAfbInstPin(command);

		return;
	}

	void CommandProcessor_LM5_LM6::command_rdfb(const DeviceCommand& command)
	{
		//AfbComponentInstance* afbInstance = m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentInstance* afbInstance = command.m_afbComponentInstance;
		const AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device->writeRamWord(command.m_memoryAreaTo, command.m_word0, param->wordValue());

		return;
	}


	// Command: wrfbc
	// Code: 10
	// Description: Write constant word to AFB input
	//
	void CommandProcessor_LM5_LM6::parse_wrfbc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);					// Data

		command->m_afbParam.setOpIndex(command->m_afbPinOpCode);
		command->m_afbParam.setWordValue(command->m_word0);

		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		// wrfbc LOGIC.0[i_oprd_15], #0003h
		//
		command->m_string = strCommand(command->m_command.caption) +
							strAfbInstPin(command) +
							", " +
							strWordConst(command->m_word0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_wrfbc(const DeviceCommand& command)
	{
		bool ok = command.m_afbComponentInstance->addParam(command.m_afbParam);
		if (ok == false)
		{
			m_device->SIM_FAULT(QString("Write param error"));
		}

//		m_device->setAfbParam(command.m_afbOpCode,
//							  command.m_afbInstance,
//							  command.m_afbParam);
		return;
	}

	// Command: wrfbb
	// Code: 11
	// Description: Read bit from RAM and write it to AFB
	//
	void CommandProcessor_LM5_LM6::parse_wrfbb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);					// Word0 - data address
		command->m_bitNo0 = m_device->getWord(command->m_offset + 3);					// BitNo

		command->m_memoryAreaFrom = m_device->ram().memoryAreaHandle(E::LogicModuleRamAccess::Read, command->m_word0);

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);
		checkParamRange(command->m_bitNo0, 0, 15, QStringLiteral("BitNo"));

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		// wrfbb LOGIC.0[i_input], 46083h[0]
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) +
							", " +
							strBitAddr(command->m_word0, command->m_bitNo0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_wrfbb(const DeviceCommand& command)
	{
		quint16 data = m_device->readRamBit(command.m_memoryAreaFrom, command.m_word0, command.m_bitNo0);

		bool ok = command.m_afbComponentInstance->addParam(AfbComponentParam{command.m_afbPinOpCode, data});
		if (ok == false)
		{
			m_device->SIM_FAULT(QString("Write param error"));
		}
//		m_device->setAfbParam(command.m_afbOpCode,
//							  command.m_afbInstance,
//							  AfbComponentParam{command.m_afbPinOpCode, data});

		return;
	}

	// Command: rdfbb
	// Code: 12
	// Description: Read bit from AFB and write it to RAM
	//
	void CommandProcessor_LM5_LM6::parse_rdfbb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);					// Word0 - data address
		command->m_bitNo0 = m_device->getWord(command->m_offset + 3);					// BitNo

		command->m_memoryAreaTo = m_device->ram().memoryAreaHandle(E::LogicModuleRamAccess::Write, command->m_word0);

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);
		checkParamRange(command->m_bitNo0, 0, 15, QStringLiteral("BitNo"));

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		// rdfbb 46083h[0], LOGIC.0[o_result]
		//
		command->m_string = strCommand(command->caption()) +
							strBitAddr(command->m_word0, command->m_bitNo0) +
							", " +
							strAfbInstPin(command);

		return;
	}

	void CommandProcessor_LM5_LM6::command_rdfbb(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = command.m_afbComponentInstance;
		const AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device->writeRamBit(command.m_memoryAreaTo, command.m_word0, command.m_bitNo0, param->wordValue() & 0x01);

		return;
	}

	// Command: rdfbcmp
	// Code: 13
	// Description: Read 16-bit data from AFB instance and compare it with constant, set compare bit if equal
	//
	void CommandProcessor_LM5_LM6::parse_rdfbcmp(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);						// Word0 - data to comapare with

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) + ", " +
							strWordConst(command->m_word0);
		return;
	}

	void CommandProcessor_LM5_LM6::command_rdfbcmp(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		const AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		bool result = param->wordValue() == command.m_word0;
		m_device->setFlagCmp(result ? 1 : 0);

		return;
	}

	// Command: setmem
	// Code: 14
	// Description: Set memory area to 16-bit word constant
	//
	void CommandProcessor_LM5_LM6::parse_setmem(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - adderess
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - data
		command->m_word2 = m_device->getWord(command->m_offset + 3);		// word2 - words to move

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strWordConst(command->m_word1) + ", " +
							strWordConst(command->m_word2);

		return;

	}

	void CommandProcessor_LM5_LM6::command_setmem(const DeviceCommand& command)
	{
		const quint16& size = command.m_word2;
		const quint16& data = command.m_word1;
		const quint16& address = command.m_word0;

		m_device->setRamMem(address, data, size);

		return;
	}

	// Command: movb
	// Code: 15
	// Description: Move bit from RAM to RAM
	//
	void CommandProcessor_LM5_LM6::parse_movb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 2);						// source address (ADR1)
		command->m_bitNo0 = m_device->getWord(command->m_offset + 3) & 0b1111;			//

		command->m_word1 = m_device->getWord(command->m_offset + 1);						// destionation address	(ADR2)
		command->m_bitNo1 = (m_device->getWord(command->m_offset + 3) >> 8) & 0b1111;	//

		// String representation
		//
		command->m_string =	strCommand(command->caption()) +
							strBitAddr(command->m_word0, command->m_bitNo0) + ", " +
							strBitAddr(command->m_word1, command->m_bitNo1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movb(const DeviceCommand& command)
	{
		quint16 data = m_device->readRamBit(command.m_word0, command.m_bitNo0);
		m_device->writeRamBit(command.m_word1, command.m_bitNo1, data);

		return;
	}

	// Command: appstart
	// Code: 17
	// Description: Save ALP phase start address
	//
	void CommandProcessor_LM5_LM6::parse_appstart(DeviceCommand* command) const
	{
		command->m_size = 2;
		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 keeps ALP phase start address

		// appstart  #000Ch
		//
		command->m_string = strCommand(command->m_command.caption) +
							strWordConst(command->m_word0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_appstart(const DeviceCommand& command)
	{
		m_device->setAppStartAddress(command.m_word0);
		return;
	}

	// Command: mov32
	// Code: 18
	// Description: Move 32-bit data from RAM to RAM
	//
	void CommandProcessor_LM5_LM6::parse_mov32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);		// word0 - address2 - destionation
		command->m_word1 = m_device->getWord(command->m_offset + 2);		// word1 - address1 - source

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_mov32(const DeviceCommand& command)
	{
		const quint16& src = command.m_word1;
		const quint16& dst = command.m_word0;

		quint32 data = m_device->readRamDword(src);
		m_device->writeRamDword(dst, data);

		return;
	}

	// Command: movc32
	// Code: 19
	// Description: Move 32bit constant to RAM
	//
	void CommandProcessor_LM5_LM6::parse_movc32(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 1);					// word0 - RAM address
		command->m_dword0 = m_device->getDword(command->m_offset + 2);				// Dword0 - data

		// movc32     0b402h, #0
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strDwordConst(command->m_dword0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movc32(const DeviceCommand& command)
	{
		m_device->writeRamDword(command.m_word0, command.m_dword0);
	}

	// Command: wrfb32
	// Code: 20
	// Description: Read 32bit data from RAM and write to AFB input
	//
	void CommandProcessor_LM5_LM6::parse_wrfb32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);						// Word0 - data address

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) + ", " +
							strAddr(command->m_word0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_wrfb32(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setDwordValue(m_device->readRamDword(command.m_word0));

		bool ok = command.m_afbComponentInstance->addParam(std::move(param));
		if (ok == false)
		{
			m_device->SIM_FAULT(QString("Write param error"));
		}

//		m_device->setAfbParam(command.m_afbOpCode,
//							  command.m_afbInstance,
//							  std::move(param));
		return;
	}

	// Command: rdfb32
	// Code: 21
	// Description: Read 32bit data from AFB output and write it to RAM
	//
	void CommandProcessor_LM5_LM6::parse_rdfb32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device->getWord(command->m_offset + 2);						// Word0 - data address

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		// rdfb32 0478h, LOGIC.0[i_2_oprd]
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) +
							", " +
							strAfbInstPin(command);

		return;
	}

	void CommandProcessor_LM5_LM6::command_rdfb32(const DeviceCommand& command)
	{
		//AfbComponentInstance* afbInstance = m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentInstance* afbInstance = command.m_afbComponentInstance;
		const AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device->writeRamDword(command.m_word0, param->dwordValue());

		return;
	}

	// Command: wrfbc32
	// Code: 22
	// Description: Write 32bit constant to FunctionalBlock input
	//
	void CommandProcessor_LM5_LM6::parse_wrfbc32(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_dword0 = m_device->getDword(command->m_offset + 2);					// Data

		command->m_afbParam.setOpIndex(command->m_afbPinOpCode);
		command->m_afbParam.setDwordValue(command->m_dword0);

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		command->m_afbComponentInstance = m_device->afbComponentInstance(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		// wrfbc32 MATH.0[i_oprd_1], #0423445h
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) +
							", " +
							strDwordConst(command->m_dword0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_wrfbc32(const DeviceCommand& command)
	{
		bool ok = command.m_afbComponentInstance->addParam(command.m_afbParam);
		if (ok == false)
		{
			m_device->SIM_FAULT(QString("Write param error"));
		}

		//m_device->setAfbParam(command.m_afbOpCode, command.m_afbInstance, command.m_afbParam);

		return;
	}

	// Command: rdfbcmp32
	// Code: 23
	// Description: Read 32-bit data from AFB instance and compare it with constant, set compare bit if equal
	//
	void CommandProcessor_LM5_LM6::parse_rdfbcmp32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device->getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device->getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device->getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_dword0 = m_device->getDword(command->m_offset + 2);					// Dword0 - data to comapare with

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) + ", " +
							strDwordConst(command->m_dword0);
		return;
	}

	void CommandProcessor_LM5_LM6::command_rdfbcmp32(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		const AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		bool result = param->dwordValue() == command.m_dword0;
		m_device->setFlagCmp(result ? 1 : 0);

		return;
	}


	// Command: movcmpf
	// Code: 24
	// Description: Write compare flag to memory [flag result from rdfbcmp(32)]
	//
	void CommandProcessor_LM5_LM6::parse_movcmpf(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);					// m_word0 - Destination address
		command->m_bitNo0 = m_device->getWord(command->m_offset + 2) & 0x0F;			// m_bitNo0 - Destination bit no

		command->m_string = strCommand(command->caption()) +
							strBitAddr(command->m_word0, command->m_bitNo0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_movcmpf(const DeviceCommand& command)
	{
		quint32 cmp = m_device->flagCmp();
		m_device->writeRamBit(command.m_word0, command.m_bitNo0, cmp);
		return;
	}

	// Command: pmov
	// Code: 25
	// Description: Copy 16-bit word from memory to memory written in prior cycle
	//
	void CommandProcessor_LM5_LM6::parse_pmov(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);			// destination address (ADR2)
		command->m_word1 = m_device->getWord(command->m_offset + 2);			// source address (ADR1)

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_pmov(const DeviceCommand& command)
	{
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		quint16 data = m_device->readRamWord(src);

		m_device->writeRamWord(dst, data);
		return;
	}

	// Command: pmov32
	// Code: 26
	// Description: Copy 32-bit word from memory to memory written in prior cycle
	//
	void CommandProcessor_LM5_LM6::parse_pmov32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device->getWord(command->m_offset + 1);			// destination address (ADR2)
		command->m_word1 = m_device->getWord(command->m_offset + 2);			// source address (ADR1)

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1);

		return;
	}

	void CommandProcessor_LM5_LM6::command_pmov32(const DeviceCommand& command)
	{
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		quint32 data = m_device->readRamDword(src);

		m_device->writeRamDword(dst, data);
		return;
	}

	// Command: fillb
	// Code: 27
	// Description: Fill 16-bit word with 1-bit constant and write it to memory
	//
	void CommandProcessor_LM5_LM6::parse_fillb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device->getWord(command->m_offset + 1);			// destination address (ADR2)
		command->m_word1 = m_device->getWord(command->m_offset + 2);			// source address (ADR1)
		command->m_bitNo0 = m_device->getWord(command->m_offset + 3);		// bit no to read

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strBitAddr(command->m_word1, command->m_bitNo0);

		return;
	}

	void CommandProcessor_LM5_LM6::command_fillb(const DeviceCommand& command)
	{
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;
		const auto& bitNo = command.m_bitNo0;

		quint16 bit = m_device->readRamBit(src, bitNo);

		m_device->writeRamWord(dst, bit ? 0xFFFF : 0x0000);
		return;
	}


	//
	// AFB's simultaion code
	//

	//	LOGIC, OpCode 1
	//
	void CommandProcessor_LM5_LM6::afb_logic_v207(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_oprd_quant = 0;
		const int i_bus_width = 1;
		const int i_conf = 2;
		const int i_input_0 = 3;
		const int o_result = 20;

		// Get params, throws exception in case of error
		//
		quint16 oprdQuant = instance->param(i_oprd_quant)->wordValue();
		quint16 busWidth = instance->param(i_bus_width)->wordValue();
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(oprdQuant, 1, 16, QStringLiteral("i_oprd_quant"));
		checkParamRange(busWidth, 1, 16, QStringLiteral("i_bus_width"));

		// Logic
		//
		std::array<quint16, 16> inputs;

		for (quint16 i = 0; i < oprdQuant; i++)
		{
			inputs[i] = instance->param(i_input_0 + i)->wordValue();
		}

		quint16 result = inputs[0];

		switch (conf)
		{
			case 1:	// AND
				for (size_t i = 1; i < oprdQuant; i++)
				{
					result &= inputs[i];
				}
				break;
			case 2:	// OR
				for (size_t i = 1; i < oprdQuant; i++)
				{
					result |= inputs[i];
				}
				break;
			case 3:	// XOR
				for (size_t i = 1; i < oprdQuant; i++)
				{
					result ^= inputs[i];
				}
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_logic");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);
		return;
	}

	//	NOT, OpCode 2
	//
	void CommandProcessor_LM5_LM6::afb_not_v103(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_oprd = 0;
		const int o_result = 2;

		// Logic
		//
		const AfbComponentParam* input = instance->param(i_oprd);
		quint16 result = ~input->wordValue();

		// Save result
		//
		instance->addParamWord(o_result, result);
		return;
	}


	//	TCT, OpCode 3
	//
	void CommandProcessor_LM5_LM6::afb_tct_v208(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;			// 1..5
		const int i_counter = 1;		// Time, SI
		const int i_prev_counter = 3;	// Previous counter value, SI
		const int i_saved_data = 5;		// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int i_input = 6;			// 1/0

		const int o_result = 8;			// 1/0
		const int o_counter = 9;		// Counter value -> i_prev_counter
		const int o_saved_data = 11;	// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int o_parem_err = 12;
		//const int o_tct_edi = 13;
		//const int o_version = 14;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		qint32 time = instance->param(i_counter)->signedIntValue();
		quint32 counter = instance->paramExists(i_prev_counter) ? instance->param(i_prev_counter)->dwordValue() : 0;

		quint16 prevInputValue = instance->paramExists(i_saved_data) ?
									 instance->param(i_saved_data)->wordValue() & 0x0001 : 0x0000;

		quint16 prevResultValue = instance->paramExists(i_saved_data) ?
									  (instance->param(i_saved_data)->wordValue() >> 1) & 0x0001 : 0x0000;

		quint16 currentInputValue = instance->param(i_input)->wordValue();

		checkParamRange(conf, 1, 5, QStringLiteral("i_conf"));

		// Logic
		//
		quint16 result = 0;
		quint16 paramError = 0;

		if (time < 0)
		{
			paramError = 1;
			time = 0;
		}

		switch (conf)
		{
		case 1:
			{
				// On
				//
				if (currentInputValue == 0)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 1
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 2:
			{
				// Off
				//
				if (currentInputValue == 1)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 0
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 3:
			{
				// Univibrator (TCTC_VIBR)
				//
				if (counter == 0 &&
					prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;
		case 4:
			{
				// In AFBL RM shown another diagram, what is wrong
				// De facto this RC filter is implemented

				// FILTER (tctc_filter) - defacto is RC FILTER, on next AFBL version 209 it is fixed, and this implementation move to conf 6
				//
				if (currentInputValue == 0)
				{
					if (counter == 0)
					{
						result = 0;
						counter = time / m_cycleDurationMs;		// counter cannot be more then (time / m_cycleDurationMs)
					}
					else
					{
						counter --;
						result = prevResultValue;
					}
				}
				else
				{
					if (counter >= static_cast<quint32>(time / m_cycleDurationMs))
					{
						result = 1;
						counter = time / m_cycleDurationMs;		// counter cannot be more then (time / m_cycleDurationMs)
					}
					else
					{
						counter ++;
						result = prevResultValue;
					}
				}
			}
			break;

		case 5:
			{
				// Univibrator R (TCTC_RSV)
				//
				if (prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;

		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
									.arg(conf), "afb_tct");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);

		instance->addParamDword(o_counter, counter);
		instance->addParamDword(i_prev_counter, counter);

		instance->addParamWord(o_saved_data, currentInputValue | (result << 1));
		instance->addParamWord(i_saved_data, currentInputValue | (result << 1));

		instance->addParamWord(o_parem_err, paramError);

		return;
	}

	void CommandProcessor_LM5_LM6::afb_tct_v209(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;			// 1..5
		const int i_counter = 1;		// Time, SI
		const int i_prev_counter = 3;	// Previous counter value, SI
		const int i_saved_data = 5;		// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int i_input = 6;			// 1/0

		const int o_result = 8;			// 1/0
		const int o_counter = 9;		// Counter value -> i_prev_counter
		const int o_saved_data = 11;	// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int o_parem_err = 12;
		//const int o_tct_edi = 13;
		//const int o_version = 14;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		qint32 time = instance->param(i_counter)->signedIntValue();
		quint32 counter = instance->paramExists(i_prev_counter) ? instance->param(i_prev_counter)->dwordValue() : 0;

		quint16 prevInputValue = instance->paramExists(i_saved_data) ?
									 instance->param(i_saved_data)->wordValue() & 0x0001 : 0x0000;

		quint16 prevResultValue = instance->paramExists(i_saved_data) ?
									  (instance->param(i_saved_data)->wordValue() >> 1) & 0x0001 : 0x0000;

		quint16 currentInputValue = instance->param(i_input)->wordValue();

		checkParamRange(conf, 1, 6, QStringLiteral("i_conf"));

		// Logic
		//
		quint16 result = 0;
		quint16 paramError = 0;

		if (time < 0)
		{
			paramError = 1;
			time = 0;
		}

		switch (conf)
		{
		case 1:
			{
				// On
				//
				if (currentInputValue == 0)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 1
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 2:
			{
				// Off
				//
				if (currentInputValue == 1)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 0
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 3:
			{
				// Univibrator (TCTC_VIBR)
				//
				if (counter == 0 &&
					prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;
		case 4:
			{
				// Filter (TCTC_FILTER)
				//
				if (prevInputValue != currentInputValue)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}

				if (counter != 0 )
				{
					counter --;

					if (counter == 0)
					{
						result = currentInputValue;
					}
					else
					{
						result = prevResultValue;
					}
				}
				else
				{
					result = prevResultValue;
				}
			}
			break;

		case 5:
			{
				// Univibrator R (TCTC_RSV)
				//
				if (prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;

		case 6:
			{
				// RC FILTER (tctc_rcfilter)
				//
				if (currentInputValue == 0)
				{
					if (counter == 0)
					{
						result = 0;
					}
					else
					{
						counter --;
						result = prevResultValue;
					}
				}
				else
				{
					if (counter >= static_cast<quint32>(time / m_cycleDurationMs))
					{
						result = 1;
						counter = time / m_cycleDurationMs;		// counter cannot be more then (time / m_cycleDurationMs)
					}
					else
					{
						counter ++;
						result = prevResultValue;
					}
				}
			}
			break;

		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
									.arg(conf), "afb_tct");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);

		instance->addParamDword(o_counter, counter);
		instance->addParamDword(i_prev_counter, counter);

		instance->addParamWord(o_saved_data, currentInputValue | (result << 1));
		instance->addParamWord(i_saved_data, currentInputValue | (result << 1));

		instance->addParamWord(o_parem_err, paramError);

		return;
	}

	void CommandProcessor_LM5_LM6::afb_tct_v210(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;			// 1..5
		const int i_counter = 1;		// Time, SI
		const int i_prev_counter = 3;	// Previous counter value, SI
		const int i_saved_data = 5;		// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int i_input = 6;			// 1/0

		const int o_result = 8;			// 1/0
		const int o_counter = 9;		// Counter value -> i_prev_counter
		const int o_saved_data = 11;	// keeps 2 signals, 0bit - prev_input, 1bit - prev_result
		const int o_parem_err = 12;
		//const int o_tct_edi = 13;
		//const int o_version = 14;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		qint32 time = instance->param(i_counter)->signedIntValue();
		quint32 counter = instance->paramExists(i_prev_counter) ? instance->param(i_prev_counter)->dwordValue() : 0;

		quint16 prevInputValue = instance->paramExists(i_saved_data) ?
									 instance->param(i_saved_data)->wordValue() & 0x0001 : 0x0000;

		quint16 prevResultValue = instance->paramExists(i_saved_data) ?
									  (instance->param(i_saved_data)->wordValue() >> 1) & 0x0001 : 0x0000;

		quint16 currentInputValue = instance->param(i_input)->wordValue();

		checkParamRange(conf, 1, 6, QStringLiteral("i_conf"));

		// Logic
		//
		quint16 result = 0;
		quint16 paramError = 0;

		if (time < m_cycleDurationMs)
		{
			paramError = 1;
			time = 0;
		}

		switch (conf)
		{
		case 1:
			{
				// On
				//
				if (currentInputValue == 0)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 1
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 2:
			{
				// Off
				//
				if (currentInputValue == 1)
				{
					result = 0;
					counter = 0;
				}
				else
				{
					// InputValue == 0
					//
					counter += m_cycleDurationMs;

					if (counter > static_cast<quint32>(time))
					{
						result = 1;
						counter = time;		// It keeps counter from overflow and getting to 0
					}
				}
			}
			break;
		case 3:
			{
				// Univibrator (TCTC_VIBR)
				//
				if (counter == 0 &&
					prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;
		case 4:
			{
				// Filter (TCTC_FILTER)
				//
				if (prevInputValue != currentInputValue)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}

				if (counter != 0 )
				{
					counter --;

					if (counter == 0)
					{
						result = currentInputValue;
					}
					else
					{
						result = prevResultValue;
					}
				}
				else
				{
					result = prevResultValue;
				}
			}
			break;

		case 5:
			{
				// Univibrator R (TCTC_RSV)
				//
				if (prevInputValue == 0 &&
					currentInputValue == 1)
				{
					// Start timer
					//
					counter = time / m_cycleDurationMs;
				}
				else
				{
					if (counter != 0)
					{
						counter --;
					}
				}

				result = (counter == 0) ? 0 : 1;
			}
			break;

		case 6:
			{
				// RC FILTER (tctc_rcfilter)
				//
				if (currentInputValue == 0)
				{
					if (counter == 0)
					{
						result = 0;
					}
					else
					{
						counter --;
						result = prevResultValue;
					}
				}
				else
				{
					if (counter >= static_cast<quint32>(time / m_cycleDurationMs))
					{
						result = 1;
						counter = time / m_cycleDurationMs;		// counter cannot be more then (time / m_cycleDurationMs)
					}
					else
					{
						counter ++;
						result = prevResultValue;
					}
				}
			}
			break;

		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
									.arg(conf), "afb_tct");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);

		instance->addParamDword(o_counter, counter);
		instance->addParamDword(i_prev_counter, counter);

		instance->addParamWord(o_saved_data, currentInputValue | (result << 1));
		instance->addParamWord(i_saved_data, currentInputValue | (result << 1));

		instance->addParamWord(o_parem_err, paramError);

		return;
	}

	//	FLIP FLOP, OpCode 4
	//
	void CommandProcessor_LM5_LM6::afb_flipflop_v106(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;					// 1 - 6
		const int i_prev_val = 1;				// Previous result value
		const int i_c_t_prev = 2;				// Previous state of input C-data/T-data signal
		const int i_r_c_t = 3;					// reset/C -data signal/T-data signal
		const int i_s_d = 4;					// Setting signal  S/D-data signal

		const int o_result = 6;					//
		const int o_c_t = 7;					// Input C-data/T-data signal
		//const int o_edi = 8;
		//const int o_version = 9;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		quint16 prevResult = instance->paramExists(i_prev_val) ? instance->param(i_prev_val)->wordValue() : 0;

		checkParamRange(conf, 1, 6, QStringLiteral("i_conf"));

		// Logic
		//
		quint16 result = 0;

		switch (conf)
		{
		case 1:
			{
				// Set/Reset trigger with set priority
				//
				quint16 set = instance->param(i_s_d)->wordValue();
				quint16 reset = instance->param(i_r_c_t)->wordValue();

				if (set == 0)
				{
					if (reset == 1)
					{
						result = 0;
					}
					else
					{
						result = prevResult;
					}
				}
				else
				{
					result = 1;
				}
			}
			break;
		case 2:
			{
				// Set/Reset trigger with reset priority
				//
				quint16 set = instance->param(i_s_d)->wordValue() & 0x0001;
				quint16 reset = instance->param(i_r_c_t)->wordValue() & 0x0001;

				quint16 t = (set << 1) | reset;

				switch (t)
				{
				case 0: // S:0 R:0
					result = prevResult;
					break;
				case 1:	// S:0 R:1
					result = 0;
					break;
				case 2: // S:1 R:0
					result = 1;
					break;
				case 3: // S:1 R:1
					result = 0;
					break;
				default:
					Q_ASSERT(false);
				}
			}
			break;
		case 3:
			{
				// D on front
				//
				quint16 data = instance->param(i_s_d)->wordValue();
				quint16 clock = instance->param(i_r_c_t)->wordValue();
				quint16 prevClock = instance->paramExists(i_c_t_prev) ? instance->param(i_c_t_prev)->wordValue() : 0;

				if (prevClock == 0 && clock == 1)
				{
					result = data;
				}
				else
				{
					result = prevResult;
				}

				instance->addParamWord(i_c_t_prev, clock);
				instance->addParamWord(o_c_t, clock);
			}
			break;
		case 4:
			{
				// T on front
				//
				quint16 t = instance->param(i_r_c_t)->wordValue();
				quint16 prevT = instance->paramExists(i_c_t_prev) ? instance->param(i_c_t_prev)->wordValue() : 0;

				if (prevT == 0 && t == 1)
				{
					result = prevResult ? 0x0000 : 0x0001;
				}
				else
				{
					result = prevResult;
				}

				instance->addParamWord(i_c_t_prev, t);
				instance->addParamWord(o_c_t, t);
			}
			break;
		case 5:
			{
				// D on decay
				//
				quint16 data = instance->param(i_s_d)->wordValue();
				quint16 clock = instance->param(i_r_c_t)->wordValue();
				quint16 prevClock = instance->paramExists(i_c_t_prev) ? instance->param(i_c_t_prev)->wordValue() : 0;

				if (prevClock == 1 && clock == 0)
				{
					result = data;
				}
				else
				{
					result = prevResult;
				}

				instance->addParamWord(i_c_t_prev, clock);
				instance->addParamWord(o_c_t, clock);
			}
			break;
		case 6:
			{
				// T on decay
				//
				quint16 t = instance->param(i_r_c_t)->wordValue();
				quint16 prevT = instance->paramExists(i_c_t_prev) ? instance->param(i_c_t_prev)->wordValue() : 0;

				if (prevT == 1 && t == 0)
				{
					result = prevResult ? 0x0000 : 0x0001;
				}
				else
				{
					result = prevResult;
				}

				instance->addParamWord(i_c_t_prev, t);
				instance->addParamWord(o_c_t, t);
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);
		instance->addParamWord(i_prev_val, result);

		return;
	}

	//	CTUD, OpCode 5
	//  Counter Up/Down
	//
	void CommandProcessor_LM5_LM6::afb_ctud_v106(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;			// 1, 2
		const int i_counter = 1;		// Time, SI, can be negative
		const int i_prev_input = 3;		// Previous counter value, 0/1
		const int i_input = 4;			// 0/1
		const int i_reset = 5;			// 0/1

		const int o_result = 7;			// Counter value -> i_counter
		const int o_prev_input = 9;		// o_prev_input -> i_prev_input
		//const int o_edi = 10;
		//const int o_version = 11;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		qint32 counter = instance->paramExists(i_counter) ? instance->param(i_counter)->signedIntValue() : 0;

		quint16 prevInputValue = instance->paramExists(i_prev_input) ? instance->param(i_prev_input)->wordValue() : 0;
		quint16 input = instance->param(i_input)->wordValue();
		quint16 reset = instance->param(i_reset)->wordValue();

		// Logic
		//
		if (reset == 1)
		{
			counter = 0;
		}
		else
		{
			switch (conf)
			{
			case 1:
				// Up - rising edges
				//
				if (prevInputValue == 0 &&
					input == 1)
				{
					counter ++;
				}
				break;
			case 2:
				// Up - falling edges
				//
				if (prevInputValue == 1 &&
					input == 0)
				{
					counter --;
				}
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_ctud");
			}
		}

		instance->addParamSignedInt(o_result, counter);
		instance->addParamSignedInt(i_counter, counter);

		instance->addParamWord(o_prev_input, input);
		instance->addParamWord(i_prev_input, input);

		return;
	}

	//	MAJ, OpCode 6
	//  Majority Block
	//
	void CommandProcessor_LM5_LM6::afb_maj_v107(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf_x = 0;
		const int i_conf_y = 1;
		const int i_input_0 = 2;	// 16 consecutive
		const int o_result = 19;
		const int o_err_ms = 22;
		const int o_err_st = 23;

		// Get params, throws exception in case of error
		//
		quint16 xCount = instance->param(i_conf_x)->wordValue();
		quint16 yCount = instance->param(i_conf_y)->wordValue();

		checkParamRange(xCount, 2, yCount - 1, QStringLiteral("i_conf_x"));
		checkParamRange(yCount, 3, 16, QStringLiteral("i_conf_y"));

		std::array<quint16, 16> inputs;
		for (quint16 i = 0; i < yCount; i++)
		{
			inputs[i] = instance->param(i_input_0 + i)->wordValue();
		}

		// Logic
		//
		std::array<quint16, 16> alertedCount = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		quint16 mask = 0x0001;
		for (size_t i = 0; i < 16; i++, mask <<= 1)
		{
			for (size_t inputIndex = 0; inputIndex < yCount; inputIndex++)
			{
				if ((inputs[inputIndex] & mask) == 1)
				{
					alertedCount[i] ++;
				}
			}
		}

		// Get result
		//
		quint16 result = 0;
		quint16 err_ms = 0;
		quint16 err_st = 0;

		for (size_t i = 0, mask = 0x0001; i < 16; i++, mask <<= 1)
		{
			int ac = alertedCount[i];

			if (ac >= xCount)
			{
				result |= mask;
			}

			if (ac != 0 && ac < xCount)
			{
				err_ms |= mask;
			}

			if (ac != 0 && ac != yCount)
			{
				err_st |= mask;
			}
		}

		// Save result
		//
		instance->addParamWord(o_result, result);
		instance->addParamWord(o_err_ms, err_ms);
		instance->addParamWord(o_err_st, err_st);
		return;
	}

	//	SRSST, OpCode 7
	//  SimLock
	//
	void CommandProcessor_LM5_LM6::afb_srsst_v104(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_data = 0;
		const int i_set = 1;
		const int i_reset = 2;

		const int o_result = 4;
		//const int o_version = 5;

		// Get params, throws exception in case of error
		//
		quint16 data = instance->param(i_data)->wordValue();
		quint16 set = instance->param(i_set)->wordValue();
		quint16 reset = instance->param(i_reset)->wordValue();

		// Logic
		//
		quint16 result = (data | set) & (~reset);

		// Save result
		//
		instance->addParamWord(o_result, result);
		return;
	}

	//	BCOD, OpCode 8
	//
	void CommandProcessor_LM5_LM6::afb_bcod_v103(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(conf, 1, 2, QStringLiteral("i_conf"));

		// Logic
		//
		afb_bcod_v104(instance);	// in 104 version added conf 3,

		return;
	}

	void CommandProcessor_LM5_LM6::afb_bcod_v104(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_oprd_quant = 1;		// Operand count
		const int i_1_oprd = 2;			// Operand 1, 2...

		const int o_result = 34;
		const int o_active = 36;

		// Get params, throws exception in case of error
		//
		quint16 oprdQuant = instance->param(i_oprd_quant)->wordValue();
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(oprdQuant, 1, 32, QStringLiteral("i_oprd_quant"));
		checkParamRange(conf, 1, 3, QStringLiteral("i_conf"));

		// Logic
		//
		qint32 result = 0;
		quint16 active = 0;

		switch (conf)
		{
		case 1:
			for (quint16 i = 0; i < oprdQuant; i++)
			{
				if (instance->param(i_1_oprd + i)->wordValue() == 1)
				{
					result = static_cast<qint32>(i);
					active = 1;
					break;	// Break, wee need just the first one
				}
			}
			break;
		case 2:
			for (quint16 i = 0; i < oprdQuant; i++)
			{
				qint16 inputValue = static_cast<qint32>(instance->param(i_1_oprd + i)->wordValue() & 0x0001);
				result |= inputValue << i;
			}
			break;
		case 3:
			for (quint16 i = 0; i < oprdQuant; i++)
			{
				if (instance->param(i_1_oprd + i)->wordValue() == 1)
				{
					result = static_cast<qint32>(i);
					active = 1;
					// no break, we need the last active input
				}
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcod_v103/104");
		}

		// Save result
		//
		instance->addParamSignedInt(o_result, result);
		instance->addParamWord(o_active, active);
		return;
	}

	//	BDEC, OpCode 9
	//
	void CommandProcessor_LM5_LM6::afb_bdec_v103(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_oprd_quant = 1;		// Operand count
		const int i_number = 2;			// SignedInt
		const int o_1_result = 4;		// Operand 1, 2...

		// Get params, throws exception in case of error
		//
		qint32 oprdQuant = instance->param(i_oprd_quant)->wordValue();
		quint16 conf = instance->param(i_conf)->wordValue();
		qint32 inputValue = instance->param(i_number)->signedIntValue();

		checkParamRange(oprdQuant, 1, 32, QStringLiteral("i_oprd_quant"));
		checkParamRange(conf, 1, 2, QStringLiteral("i_conf"));

		// AFB Logic
		//
		switch (conf)
		{
		case 1:
			for (quint16 i = 0; i < oprdQuant; i++)
			{
				int value = i == inputValue ? 0x0001 : 0x0000;
				instance->addParamWord(o_1_result + i, value);
			}
			break;
		case 2:
			for (qint16 i = 0; i < oprdQuant; i++)
			{
				int value = inputValue & (0x01 << i) ?  1 : 0;
				instance->addParamWord(o_1_result + i, value);
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcod");
		}

		return;
	}

	//	BCOMP, OpCode 10
	//
	void CommandProcessor_LM5_LM6::afb_bcomp_v111(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_sp_s = 1;		// Setting value
		const int i_sp_r = 3;		// Reset value
		const int i_prev_result = 5;// Prev result
		const int i_data = 6;		// Input data
		const int o_result = 9;		// Result
		const int o_nan = 10;		// Any input FP param NaN

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		// AFB Logic
		//
		if (conf >=1 && conf <= 4)
		{
			qint32 settingValue = instance->param(i_sp_s)->signedIntValue();
			qint32 resetValue = instance->param(i_sp_r)->signedIntValue();
			qint32 inputValue = instance->param(i_data)->signedIntValue();
			quint16 prevResult = 0;

			switch (conf)
			{
			case 1:		// SignedInt32, ==
				if (inputValue >= resetValue && inputValue <= settingValue)
				{
					instance->addParamWord(o_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
				}
				break;

			case 2:		// SignedInt32, >
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if (inputValue <= settingValue)
				{
					if (prevResult == 0)
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);	 // can be commneted as it's already 0
						break;
					}
					else
					{
						// Prev result is 1, so setting is alerted
						//
						if (inputValue > resetValue)
						{
							instance->addParamWord(o_result, 1);		// can be commneted as it's already 1
							instance->addParamWord(i_prev_result, 1);	// can be commneted as it's already 1
						}
						else
						{
							instance->addParamWord(o_result, 0);
							instance->addParamWord(i_prev_result, 0);
						}
						break;
					}
				}
				else
				{
					// if (inputValue > settingValue)
					//
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
					break;
				}

				Q_ASSERT(false);
				break;

			case 3:		// SignedInt32, <
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if (inputValue >= settingValue)
				{
					if (prevResult == 0)
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);	 // can be commneted as it's already 0
						break;
					}
					else
					{
						// Prev result is 1, so setting is alerted
						//
						if (inputValue < resetValue)
						{
							instance->addParamWord(o_result, 1);		// can be commneted as it's already 1
							instance->addParamWord(i_prev_result, 1);	// can be commneted as it's already 1
						}
						else
						{
							instance->addParamWord(o_result, 0);
							instance->addParamWord(i_prev_result, 0);
						}
						break;
					}
				}
				else
				{
					// if (inputValue < settingValue)
					//
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
					break;
				}

				Q_ASSERT(false);
				break;

			case 4:		// SignedInt32, <>
				if (inputValue >= resetValue && inputValue <= settingValue)
				{
					instance->addParamWord(o_result, 0);
				}
				else
				{
					instance->addParamWord(o_result, 1);
				}
				break;

			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		if (conf >=5 && conf <= 8)
		{
			float settingValue = instance->param(i_sp_s)->floatValue();
			float resetValue = instance->param(i_sp_r)->floatValue();
			float inputValue = instance->param(i_data)->floatValue();
			quint16 prevResult = 0;
			quint16 nan = (std::isnan(settingValue) ||
						  std::isnan(resetValue) ||
						  std::isnan(inputValue)) ? 0x0001 : 0x0000;

			switch (conf)
			{
			case 5:		// FloatingPoint32, ==
				instance->addParamWord(o_nan, nan);
				if (nan != 0)
				{
					break;
				}

				if (inputValue >= resetValue && inputValue <= settingValue)
				{
					instance->addParamWord(o_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
				}
				break;

			case 6:		// FloatingPoint32, >
				{
					instance->addParamWord(o_nan, nan);
					if (nan != 0)
					{
						break;
					}

					if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
					{
						prevResult = instance->param(i_prev_result)->wordValue();
					}

					if (inputValue <= settingValue)
					{
						if (prevResult == 0)
						{
							instance->addParamWord(o_result, 0);
							instance->addParamWord(i_prev_result, 0);	 // can be commneted as it's already 0
							break;
						}
						else
						{
							// Prev result is 1, so setting is alerted
							//
							if (inputValue > resetValue)
							{
								instance->addParamWord(o_result, 1);		// can be commneted as it's already 1
								instance->addParamWord(i_prev_result, 1);	// can be commneted as it's already 1
							}
							else
							{
								instance->addParamWord(o_result, 0);
								instance->addParamWord(i_prev_result, 0);
							}
							break;
						}
					}
					else
					{
						// if (inputValue > settingValue)
						//
						instance->addParamWord(o_result, 1);
						instance->addParamWord(i_prev_result, 1);
						break;
					}

					Q_ASSERT(false);
				}
				break;
			case 7:		// FloatingPoint32, <
				instance->addParamWord(o_nan, nan);
				if (nan != 0)
				{
					break;
				}

				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if (inputValue >= settingValue)
				{
					if (prevResult == 0)
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);	 // can be commneted as it's already 0
						break;
					}
					else
					{
						// Prev result is 1, so setting is alerted
						//
						if (inputValue < resetValue)
						{
							instance->addParamWord(o_result, 1);		// can be commneted as it's already 1
							instance->addParamWord(i_prev_result, 1);	// can be commneted as it's already 1
						}
						else
						{
							instance->addParamWord(o_result, 0);
							instance->addParamWord(i_prev_result, 0);
						}
						break;
					}
				}
				else
				{
					// if (inputValue < settingValue)
					//
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
					break;
				}

				Q_ASSERT(false);
				break;
			case 8:		// FloatingPoint32, <>
				instance->addParamWord(o_nan, nan);
				if (nan != 0)
				{
					break;
				}

				if (inputValue >= resetValue && inputValue <= settingValue)
				{
					instance->addParamWord(o_result, 0);
				}
				else
				{
					instance->addParamWord(o_result, 1);
				}
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
		return;
	}

	//	DAMPER, OpCode 11
	//
	void CommandProcessor_LM5_LM6::afb_damper_v112(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_time = 1;		// 32 bit SI
		const int i_prev = 3;		// 48-bit prev data after filer (inputs 3, 4, 5)
		const int i_data = 6;		// Input data
		const int i_track = 8;		//

		const int o_current = 10;	// 48-bit current data after filer (inputs 10, 11, 12)
		const int o_result = 13;	// 32-bit current result value (SI/FP)
		const int o_overflow = 15;
		const int o_underflow = 16;
		const int o_zero = 17;
		const int o_nan = 18;		// Any input FP param NaN
		const int o_param_err = 19;
		//const int o_version = 21;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();
		qint64 time = instance->param(i_time)->dwordValue();

		const AfbComponentParam* prevValueParam = instance->paramExists(i_prev) ? instance->param(i_prev) : nullptr;
		const AfbComponentParam* dataParam = instance->param(i_data);

		quint16 track = instance->param(i_track)->wordValue();

		if (time == 0 || (conf != 1 && conf != 2))
		{
			// ?????
			//
			instance->addParamSignedInt(o_result, 0);
			instance->addParamSignedInt64(o_current, 0);
			instance->addParamSignedInt64(i_prev, 0);

			instance->addParamWord(o_overflow, 0);
			instance->addParamWord(o_underflow, 0);
			instance->addParamWord(o_zero, 0);
			instance->addParamWord(o_nan, 0);

			instance->addParamWord(o_param_err, 1);
			return;
		}
		else
		{
			instance->addParamWord(o_param_err, 0);
		}

		// --
		//
		quint16 isOverflow = false;
		quint16 isUnderflow = false;
		quint16 isZero = false;
		quint16 isNan = false;

		if (track == 0)
		{
			// Damping
			//
			if (conf == 1)
			{
				// SignedInt
				//
				qint64 inputValue = dataParam->signedIntValue();
				inputValue <<= 16;

				qint64 prevValue = prevValueParam ? prevValueParam->signedInt64Value() : 0;	// First cycle prevValue is 0
				qint64 n = time / m_cycleDurationMs;
				qint64 resultExt = prevValue + inputValue / n - prevValue / n;

				instance->addParamSignedInt(o_result, static_cast<qint32>(resultExt >> 16));
				instance->addParamSignedInt64(o_current, resultExt);						// Save exdended value
				instance->addParamSignedInt64(i_prev, resultExt);							// Save exdended value

				isOverflow = (resultExt >> 16) > std::numeric_limits<qint32>::max() ||
							 (resultExt >> 16) < std::numeric_limits<qint32>::min();
				isUnderflow = false;
				isZero = (resultExt >> 16) == 0;
				isNan = false;
			}
			else
			{
				// Float
				//
				float inputValue = dataParam->floatValue();

				float prevValue = prevValueParam ? prevValueParam->floatValue() : 0.0f;	// First cycle prevValue is 0
				float n = time / m_cycleDurationMs;

				std::feclearexcept(FE_ALL_EXCEPT);
				float result = prevValue + inputValue / n - prevValue / n;

				isOverflow = std::fetestexcept(FE_OVERFLOW);
				isUnderflow = std::fetestexcept(FE_UNDERFLOW);
				isZero = (result == .0f) || isUnderflow;
				isNan = std::isnan(result);

				instance->addParamFloat(o_result, result);
				instance->addParamFloat(o_current, result);
				instance->addParamFloat(i_prev, result);
			}
		}
		else
		{
			// track == 1, Output is input
			//
			if (conf == 1)
			{
				// SignedInt
				//
				qint32 inputValue = dataParam->signedIntValue();

				instance->addParamSignedInt(o_result, inputValue);
				instance->addParamSignedInt64(o_current, inputValue << 16);		// This input is extended for SI
				instance->addParamSignedInt64(i_prev, inputValue << 16);		// This output is extended for SI

				isOverflow = false;
				isUnderflow = false;
				isZero = (inputValue == 0);
				isNan = false;
			}
			else
			{
				// Float
				//
				float inputValue = dataParam->floatValue();

				instance->addParamFloat(o_result, inputValue);
				instance->addParamFloat(o_current, inputValue);
				instance->addParamFloat(i_prev, inputValue);

				isOverflow = false;
				isUnderflow = false;
				isZero = (inputValue == .0f);
				isNan = std::isnan(inputValue);
			}
		}

		instance->addParamWord(o_overflow, isOverflow);
		instance->addParamWord(o_underflow, isUnderflow);
		instance->addParamWord(o_zero, isZero);
		instance->addParamWord(o_nan, isNan);

		return;
	}

	//	MEM - Median and Extremum, OpCode 12
	//
	void CommandProcessor_LM5_LM6::afb_mem_v7(AfbComponentInstance* instance)
	{
		const int i_count = 0;			// Input count
		const int i_conf = 1;			// SI/FP
		const int i_enable_1 = 2;		// up to 8 - opcodes: 2, 3, 4, 5, 6, 7, 8, 9
		// ...
		const int i_in_1 = 10;			// up to 8 - opcodes: 10, 12, 14, 16, 18, 20, 22, 24
		// ...

		const int o_med_val = 28;
		const int o_max_val = 30;
		const int o_min_val = 32;
		const int o_med_index = 34;
		const int o_max_index = 35;
		const int o_min_index = 36;
		const int o_overflow = 37;
		const int o_underflow = 38;
		const int o_zero = 39;
		const int o_nan = 40;
		const int o_dev_by_zero = 41;
		const int o_mem_edi = 43;
		//const int o_version = 44;

		const int maxInputCount = 8;

		quint16 count = instance->param(i_count)->wordValue();
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(count, 3, maxInputCount, QStringLiteral("i_count"));
		checkParamRange(conf, 1, 2, QStringLiteral("i_conf"));

		// These outputs do not work really good in real AFB, so we taking them off the AFB items,
		// they do not usable now
		//
		instance->addParamWord(o_med_index, 0);
		instance->addParamWord(o_max_index, 0);
		instance->addParamWord(o_min_index, 0);
		instance->addParamWord(o_dev_by_zero, 0);	// There is impossible duv by zero

		// --
		//
		if (conf == 1)
		{
			// SI - SignedInteger
			//
			struct Operand
			{
				qint64 value;
				quint16 operandIndex;	// 1-based
			};

			std::array<Operand, maxInputCount> operands;
			Operand maxOperand{0, 0};
			Operand minOperand{0, 0};

			size_t operandCount = 0;

			for (quint16 i = 0; i < count; i++)
			{
				const AfbComponentParam* enable = instance->param(i_enable_1 + i);
				if (enable->wordValue() == 0)
				{
					continue;
				}

				qint32 value = instance->param(i_in_1 + i * 2)->signedIntValue();

				if (operandCount == 0 || value < minOperand.value)
				{
					minOperand = {value, i};
				}

				if (operandCount == 0 || value > maxOperand.value)
				{
					maxOperand = {value, i};
				}

				operands[operandCount++] = {value, i};
			}

			// --
			//

			Operand median = {0, 0};
			quint16 overflow = 0;
			quint16 underflow = 0;
			quint16 zero = 0;
			quint16 nan = 0;
			quint16 mem_edi = 0;

			switch (operandCount)
			{
			case 0:
				median = {0, 0};
				maxOperand = {0, 0};
				minOperand = {0, 0};
				mem_edi = 1;			// <<<< Error indication
				break;

			case 1:
				median = operands[0];
				maxOperand = {0, 0};	// Such wierd behavior is now, subject to chanhe in future version
				minOperand = {0, 0};	// Such wierd behavior is now, subject to chanhe in future version
				zero = median.value == 0 ? 0x0001 : 0x0000;
				break;

			case 2:
				// Specific case, return medium
				//
				median.value = (operands[0].value + operands[1].value) / 2;
				median.operandIndex = 0;
				maxOperand = {0, 0};	// Such wierd behavior is now, subject to chanhe in future version
				minOperand = {0, 0};	// Such wierd behavior is now, subject to chanhe in future version
				zero = median.value == 0 ? 0x0001 : 0x0000;
				break;

			default:
				std::sort(std::begin(operands), std::begin(operands) + operandCount,
							[](const Operand& a, const Operand& b)
							{
								return a.value < b.value;
							});

				median = operands[operandCount / 2];
				zero = median.value == 0 ? 0x0001 : 0x0000;
				break;
			}

			instance->addParamWord(o_med_val, median.value);
			instance->addParamWord(o_max_val, maxOperand.value);
			instance->addParamWord(o_min_val, minOperand.value);
//			instance->addParamWord(o_med_index, median.operandIndex);			// This output is not used in AFB
//			instance->addParamWord(o_max_index, maxOperand.operandIndex);		// This output is not used in AFB
//			instance->addParamWord(o_min_index, minOperand.operandIndex);		// This output is not used in AFB
			instance->addParamWord(o_overflow, overflow);
			instance->addParamWord(o_underflow, underflow);
			instance->addParamWord(o_zero, zero);
			instance->addParamWord(o_nan, nan);
			instance->addParamWord(o_mem_edi, mem_edi);

			return;
		}

		if (conf == 2)
		{
			// Floting Point
			//
			struct Operand
			{
				AfbComponentParam value;
				quint16 operandIndex;	// 1-based
			};

			std::array<Operand, maxInputCount> operands;
			Operand maxOperand{AfbComponentParam{static_cast<quint16>(o_max_val)}, 0};
			Operand minOperand{AfbComponentParam{static_cast<quint16>(o_min_val)}, 0};

			size_t operandCount = 0;

			for (quint16 i = 0; i < count; i++)
			{
				const AfbComponentParam* enable = instance->param(i_enable_1 + i);
				if (enable->wordValue() == 0)
				{
					continue;
				}

				const AfbComponentParam* value = instance->param(i_in_1 + i * 2);

				if (operandCount == 0 || value->floatValue() < minOperand.value.floatValue())
				{
					minOperand = {*value, i};
				}

				if (operandCount == 0 || value->floatValue() > maxOperand.value.floatValue())
				{
					maxOperand = {*value, i};
				}

				operands[operandCount++] = {*value, i};
			}

			// --
			//
			Operand median = {AfbComponentParam{static_cast<quint16>(o_med_val)}, 0};

			quint16 overflow = 0;
			quint16 underflow = 0;
			quint16 zero = 0;
			quint16 nan = 0;
			quint16 mem_edi = 0;

			switch (operandCount)
			{
			case 0:
				median = {AfbComponentParam{0}, 0};
				maxOperand = {AfbComponentParam{0}, 0};
				minOperand = {AfbComponentParam{0}, 0};
				mem_edi = 1;			// <<<< Error indication
				break;

			case 1:
				median = operands[0];
				maxOperand = {AfbComponentParam{0}, 0};	// Such wierd behavior is now, subject to chanhe in future version
				minOperand = {AfbComponentParam{0}, 0};	// Such wierd behavior is now, subject to chanhe in future version
				zero = median.value.floatValue() == 0 ? 0x0001 : 0x0000;
				break;

			case 2:
				// Specific case, return medium
				//
				maxOperand = {AfbComponentParam{0}, 0};	// Such wierd behavior is now, subject to chanhe in future version
				minOperand = {AfbComponentParam{0}, 0};	// Such wierd behavior is now, subject to chanhe in future version

				median = operands[0];

				median.value.addFloatingPoint(operands[1].value);
				overflow |= median.value.mathOverflow();
				underflow |= median.value.mathUnderflow();
				nan |= median.value.mathNan();

				median.value.divFloatingPoint(2.0f);
				overflow |= median.value.mathOverflow();
				underflow |= median.value.mathUnderflow();
				nan |= median.value.mathNan();

				median.operandIndex = 0;		// Operand index set 0, subject to change in the next MEDIAN version

				zero = median.value.floatValue() == 0 ? 0x0001 : 0x0000;
				break;

			default:
				std::sort(std::begin(operands), std::begin(operands) + operandCount,
							[](const Operand& a, const Operand& b)
							{
								return a.value.floatValue() < b.value.floatValue();
							});

				median = operands[operandCount / 2];

				zero = median.value.floatValue() == 0 ? 0x0001 : 0x0000;
				break;
			}

			instance->addParamFloat(o_med_val, median.value.floatValue());
			instance->addParamFloat(o_max_val, maxOperand.value.floatValue());
			instance->addParamFloat(o_min_val, minOperand.value.floatValue());
			//instance->addParamWord(o_med_index, median.operandIndex);
			//instance->addParamWord(o_max_index, maxOperand.operandIndex);
			//instance->addParamWord(o_min_index, minOperand.operandIndex);
			instance->addParamWord(o_overflow, overflow);
			instance->addParamWord(o_underflow, underflow);
			instance->addParamWord(o_zero, zero);
			instance->addParamWord(o_nan, nan);
			instance->addParamWord(o_mem_edi, mem_edi);

			return;
		}

		return;
	}


	//	MATH, OpCode 13
	//
	void CommandProcessor_LM5_LM6::afb_math_v104(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_1_oprd = 1;
		const int i_2_oprd = 3;
		const int o_result = 6;
		//const int o_mat_edi = 8;
		const int o_overflow = 9;
		const int o_underflow = 10;
		const int o_zero = 11;
		const int o_nan = 12;
		const int o_div_by_zero = 13;

		// Get params,  check_param throws exception in case of error
		//
		const AfbComponentParam* conf = instance->param(i_conf);
		AfbComponentParam operand1 = *instance->param(i_1_oprd);
		AfbComponentParam operand2 = *instance->param(i_2_oprd);

		// Logic	conf: 1'-'+' (SI),  '2'-'-' (SI),  '3'-'*' (SI),  '4'-'/' (SI), '5'-'+' (FP),  '6'-'-' (FP),  '7'-'*' (FP),  '8'-'/' (FP)
		//
		switch (conf->wordValue())
		{
			case 1:
				operand1.addSignedInteger(operand2);
				break;
			case 2:
				operand1.subSignedInteger(operand2);
				break;
			case 3:
				operand1.mulSignedInteger(operand2);
				break;
			case 4:
				operand1.divSignedInteger(operand2);
				break;
			case 5:
				operand1.addFloatingPoint(operand2);
				break;
			case 6:
				operand1.subFloatingPoint(operand2);
				break;
			case 7:
				operand1.mulFloatingPoint(operand2);
				break;
			case 8:
				operand1.divFloatingPoint(operand2);
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
										.arg(conf->wordValue()),
									"CommandProcessor_LM5_LM6::afb_math");
		}

		// Save result
		//
		AfbComponentParam result = operand1;
		result.setOpIndex(o_result);

		instance->addParam(result);
		instance->addParamWord(o_overflow, operand1.mathOverflow());
		instance->addParamWord(o_underflow, operand1.mathUnderflow());
		instance->addParamWord(o_zero, operand1.mathZero());
		instance->addParamWord(o_nan, operand1.mathNan());
		instance->addParamWord(o_div_by_zero, operand1.mathDivByZero());

		return;
	}

	//	SCALE, OpCode 14
	//
	void CommandProcessor_LM5_LM6::afb_scale_v108(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_scal_k1_coef = 1;
		const int i_scal_k2_coef = 3;
		const int i_ui_data = 5;		// 16 bit data, unsigned integer input
		const int i_si_fp_data = 6;		// 32 bit data, signed integer or float input

		const int o_ui_result = 8;		// 16 bit data, unsigned integer output
		const int o_si_fp_result = 9;	// 32 bit data, signed integer or float output
		const int o_scal_edi = 11;		// error
		const int o_overflow = 12;
		const int o_underflow = 13;
		const int o_zero = 14;
		const int o_nan = 15;

		// Get params,  check_param throws exception in case of error
		//
		const AfbComponentParam* conf = instance->param(i_conf);
		const AfbComponentParam* k1 = instance->param(i_scal_k1_coef);	// for  1, 2, 3, 4 -- k1/k2 SignedInteger
		const AfbComponentParam* k2 = instance->param(i_scal_k2_coef);	//      5, 6, 7, 8, 9 -- k1/k2 float
		AfbComponentParam result;

		// Scale, conf:  1-16(UI)/16(UI); 2-16(UI)/32(SI); 3-32(SI)/16(UI); 4-32(SI)/32(SI); 5-32(SI)/32(FP); 6-32(FP)/32(FP); 7-32(FP)/16(UI); 8-32(FP)/32(SI); 9-16(UI)/32(FP);
		//
		switch (conf->wordValue())
		{
		case 1: // 16(UI)/16(UI)
			{
				result = *instance->param(i_ui_data);
				result.setOpIndex(o_ui_result);

				result.convertWordToSignedInt();

				result.mulSignedInteger(*k1);
				result.divSignedIntegerNumber(32768);
				result.addSignedInteger(*k2);

				if (result.signedIntValue() > std::numeric_limits<quint16>().max())
				{
					// Overflow
					//
					result.setMathOverflow(1);
				}

				// Save result
				//
				instance->addParamWord(o_ui_result, result.wordValue());
			}
			break;
		case 2: // 16(UI)/32(SI)
			{
				result = *instance->param(i_ui_data);
				result.setOpIndex(o_si_fp_result);

				result.convertWordToSignedInt();

				result.mulSignedInteger(*k1);
				result.divSignedIntegerNumber(32768);
				result.addSignedInteger(*k2);

				// Save result
				//
				instance->addParam(result);
			}
			break;
		case 3: // 32(SI)/16(UI)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_ui_result);

				result.mulSignedInteger(*k1);
				result.divSignedIntegerNumber(32768);
				result.addSignedInteger(*k2);

				if (result.signedIntValue() > std::numeric_limits<quint16>().max())
				{
					// Overflow
					//
					result.setMathOverflow(1);
				}

				// Save result
				//
				instance->addParamWord(o_ui_result, result.wordValue());
			}
			break;
		case 4: // 32(SI)/32(SI)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_si_fp_result);

				result.mulSignedInteger(*k1);
				result.divSignedIntegerNumber(32768);
				result.addSignedInteger(*k2);

				// Save result
				//
				instance->addParamWord(o_si_fp_result, result.signedIntValue());
			}
			break;
		case 5: // 32(SI)/32(FP)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_si_fp_result);

				result.convertSignedIntToFloat();

				result.mulFloatingPoint(*k1);
				result.addFloatingPoint(*k2);

				// Save result
				//
				instance->addParam(result);
			}
			break;
		case 6: // 32(FP)/32(FP)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_si_fp_result);

				result.mulFloatingPoint(*k1);
				result.addFloatingPoint(*k2);

				// Save result
				//
				instance->addParam(result);
			}
			break;
		case 7: // 32(FP)/16(UI)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_si_fp_result);

				result.mulFloatingPoint(*k1);
				result.addFloatingPoint(*k2);

				if (result.floatValue() > std::numeric_limits<quint16>().max())
				{
					// Overflow
					//
					result.setMathOverflow(1);
				}

				// Save result
				//
				instance->addParamWord(o_ui_result, static_cast<quint16>(result.floatValue()));
			}
			break;
		case 8: // 32(FP)/32(SI)
			{
				result = *instance->param(i_si_fp_data);
				result.setOpIndex(o_si_fp_result);

				result.mulFloatingPoint(*k1);
				result.addFloatingPoint(*k2);

				if (result.floatValue() > std::numeric_limits<qint32>().max())
				{
					// Overflow
					//
					result.setMathOverflow(1);
				}

				// Save result
				//
				instance->addParamSignedInt(o_si_fp_result, static_cast<qint32>(result.floatValue()));
			}
			break;
		case 9: // 16(UI)/32(FP)
			{
				result = *instance->param(i_ui_data);
				result.setOpIndex(o_si_fp_result);

				result.convertWordToFloat();

				result.mulFloatingPoint(*k1);
				result.addFloatingPoint(*k2);

				// Save result
				//
				instance->addParam(result);
			}
			break;
		default:
			instance->addParamWord(o_scal_edi, 0x0001);
			SimException::raise("Unknown AFB configuration: " + QString::number(conf->wordValue()) + " , or this configuration is not implemented yet.");
			break;
		}

		// Save result
		//
		instance->addParamWord(o_scal_edi, 0x0000);
		instance->addParamWord(o_overflow, result.mathOverflow());
		instance->addParamWord(o_underflow, result.mathUnderflow());
		instance->addParamWord(o_zero, result.mathZero());
		instance->addParamWord(o_nan, result.mathNan());

		return;
	}

	//	FUNC, OpCode 16
	//
	void CommandProcessor_LM5_LM6::afb_func_v3(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_data = 1;

		const int o_result = 5;
		const int o_overflow = 7;
		const int o_underflow = 8;
		const int o_zero = 9;
		const int o_nan = 10;
		const int o_div_by_zero = 11;
		//const int o_func_edi = 12;
		//const int o_version = 13;

		// Get params,  check_param throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		AfbComponentParam result{static_cast<quint16>(o_result)};
		quint16 overflow = 0;
		quint16 underflow = 0;
		quint16 zero = 0;
		quint16 nan = 0;
		quint16 div_by_zero = 0;

		switch (conf)
		{
		case 1:		// FP SQRT
			{
				float floatData = instance->param(i_data)->floatValue();

				if (std::isnormal(floatData) == true)
				{
					if (std::signbit(floatData) == true)
					{
						result.setFloatValue(std::numeric_limits<float>::quiet_NaN());
						nan = 1;
						overflow  = 0;
						zero = 0;
					}
					else
					{
						result.setFloatValue(std::sqrtf(floatData));
						nan = 0;
						overflow  = 0;
						zero = 0;
					}

					break;
				}

				if (std::isinf(floatData) == true)
				{
					if (std::signbit(floatData) == false)
					{
						result.setFloatValue(std::numeric_limits<float>::infinity());
						nan = 0;
						overflow  = 1;
						zero = 0;
					}
					else
					{
						result.setFloatValue(std::numeric_limits<float>::quiet_NaN());
						nan = 1;
						overflow  = 0;
						zero = 0;
					}

					break;
				}

				if (std::isnan(floatData) == true)
				{
					result.setFloatValue(std::numeric_limits<float>::quiet_NaN());
					nan = 1;
					overflow  = 0;
					zero = 0;
					break;
				}

				if (std::fpclassify(floatData) == FP_SUBNORMAL)
				{
					result.setFloatValue(.0f);
					nan = 0;
					overflow  = 0;
					zero = 1;
					break;
				}

				if (floatData == .0f || floatData == -.0f)
				{
					result.setFloatValue(.0f);
					nan = 0;
					overflow  = 0;
					zero = 1;
					break;
				}

				// what type og float is it ?
				SimException::raise(QString("Specific type of float: %1, %2.")
									.arg(floatData),
									"CommandProcessor_LM5_LM6::afb_func_v3");
			}
			break;
		case 2:		// FP ABS
			{
				float floatData = instance->param(i_data)->floatValue();

				result.setFloatValue(std::fabsf(floatData));
			}
			break;
		case 7:		// FP INV is  = 1.0/data;
			{
				float floatData = instance->param(i_data)->floatValue();

				result.setFloatValue(1.0f);
				result.divFloatingPoint(floatData);

				underflow = result.mathUnderflow();
				zero = result.mathZero();
				nan = result.mathNan();
				div_by_zero = result.mathDivByZero();
			}
			break;
		case 8:		// SI ABS
			{
				qint32 intData = instance->param(i_data)->signedIntValue();

				result.setSignedIntValue(std::abs(intData));
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
								.arg(conf),
								"CommandProcessor_LM5_LM6::afb_func_v3");
		}

		// Save result
		//
		result.setOpIndex(o_result);
		instance->addParam(result);

		instance->addParamWord(o_overflow, overflow);
		instance->addParamWord(o_underflow, underflow);
		instance->addParamWord(o_zero, zero);
		instance->addParamWord(o_nan, nan);
		instance->addParamWord(o_div_by_zero, div_by_zero);

		return;
	}

	//	INTEGRATOR, OpCode 17
	//
	void CommandProcessor_LM5_LM6::afb_int_v6_tiunlim(AfbComponentInstance* instance)
	{
		return afb_int_v6(instance, std::numeric_limits<qint32>::max());
	}

	void CommandProcessor_LM5_LM6::afb_int_v6_ti350000(AfbComponentInstance* instance)
	{
		return afb_int_v6(instance, 350000);
	}

	void CommandProcessor_LM5_LM6::afb_int_v6(AfbComponentInstance* instance, qint32 maxTiValue)
	{
		// Define inputs/outputs opIndexes
		//
		const int i_ki = 0;				// [0, 1]	FP (32bit)	Int. Coef. of Gain
		const int i_ti = 2;				// [2, 3]	SI (32bit)	The integration time Ti(ms) (0..((2^31) - 1)
		const int i_max = 4;			// [4, 5]	FP (32bit)	Max output value
		const int i_min = 6;			// [6, 7]	FP (32bit)	Min output value
		const int i_ri_const = 8;		// [8, 9]	FP (32bit)	Integral constant for reset state
		const int i_yi_prev = 10;		// [10, 11]	FP (32bit)	Previous integral value
		const int i_x_tr = 12;			// [12, 13]	FP (32bit)	Tracking data
		const int i_x = 14;				// [14, 15]	FP (32bit)	Input X
		const int i_reset = 16;			// Discrete		Reset
		const int i_pause = 17;			// Discrete		Pause
		const int i_track = 18;			// Discrete		Tracking mode

		const int o_result = 20;		// [20, 21]	FP (32bit)	Result - Output Y
		const int o_max = 22;			// Discrete		Maximum value signal
		const int o_min = 23;			// Discrete		Minimum value signal

		const int o_param_err = 24;		// Discrete
		const int o_overflow = 25;		// Discrete
		const int o_underflow = 26;		// Discrete
		const int o_zero = 27;			// Discrete
		const int o_nan = 28;			// Discrete
		//const int o_version = 29;		// Discrete

		// Get input data
		//
		float ki = instance->param(i_ki)->floatValue();
		qint32 ti = instance->param(i_ti)->signedIntValue();
		float maxValue = instance->param(i_max)->floatValue();
		float minValue = instance->param(i_min)->floatValue();
		float ri_const = instance->param(i_ri_const)->floatValue();
		float yi_prev = instance->paramExists(i_yi_prev) == true ?
							instance->param(i_yi_prev)->floatValue() :
							0.0f;
		float x_tr = instance->param(i_x_tr)->floatValue();
		float x = instance->param(i_x)->floatValue();
		quint16 reset = instance->param(i_reset)->wordValue();
		quint16 pause = instance->param(i_pause)->wordValue();
		quint16 track = instance->param(i_track)->wordValue();

		// Logic
		// Y = F(,Ki,Ti,)
		// Yi = (X * Ki) / TI + (Yi - 1);
		// *TI = i(ms) / 5(ms)
		// i_ti = Ti(ms)
		//
		AfbComponentParam result{static_cast<quint16>(o_result)};
		result.setFloatValue(0);

		quint16 maxOut = 0;
		quint16 minOut = 0;

		quint16 param_err = 0;
		quint16 overflow = 0;
		quint16 underflow = 0;
		quint16 zero = 0;
		quint16 nan = 0;

		do
		{
			if (maxValue <= minValue)
			{
				result.setFloatValue(yi_prev);
				param_err = 0x0001;
				break;
			}

			if (ti < m_cycleDurationMs ||
				ti > maxTiValue)
			{
				param_err = 1;
				ti = std::clamp(ti, m_cycleDurationMs, maxTiValue);

				// Continue to work with correctd value
				//
			}

			// Reset, Pause and Track are set in accordance to priority
			//
			if (reset == 1)
			{
				result.setFloatValue(ri_const);
				break;
			}

			if (pause == 1)
			{
				result.setFloatValue(yi_prev);
				break;
			}

			if (track == 1)
			{
				result.setFloatValue(x_tr);
				break;
			}

			// Normal operating
			// Yi = (X * Ki) / TI + (Yi - 1);
			//
			float ti_value = ti / static_cast<float>(m_cycleDurationMs);

			// X * Ki
			//
			result.setFloatValue(x);
			result.mulFloatingPoint(ki);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// div TI
			//
			result.divFloatingPoint(ti_value);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// + Y(i-1)
			//
			result.addFloatingPoint(yi_prev);

			// Check max/min
			//
			if (result.floatValue() >= maxValue)
			{
				result.setFloatValue(maxValue);
				maxOut = 0x0001;
			}

			if (result.floatValue() <= minValue)
			{
				result.setFloatValue(minValue);
				minOut = 0x0001;
			}

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();
			zero = result.mathZero();

		} while (false);


		// Set outputs
		//
		instance->addParamFloat(o_result, result.floatValue());
		instance->addParamFloat(i_yi_prev, result.floatValue());

		instance->addParamWord(o_max, maxOut);
		instance->addParamWord(o_min, minOut);

		instance->addParamWord(o_overflow, overflow);
		instance->addParamWord(o_underflow, underflow);
		instance->addParamWord(o_zero, zero);
		instance->addParamWord(o_nan, nan);
		instance->addParamWord(o_param_err, param_err);

		return;
	}

	//	DPCOMP, OpCode 20
	//
	void CommandProcessor_LM5_LM6::afb_dpcomp_v3(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_hyst = 1;			// Hysteresis value
		const int i_prev_result = 3;	// Prev result

		const int i_data = 4;			// Input data
		const int i_setting = 6;		// Setting value

		const int o_result = 9;			// Result
		const int o_overflow = 10;		// Result
		const int o_underflow = 11;		// Result
		const int o_nan = 13;			// Any input FP param NaN

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		// AFB Logic
		//
		if (conf >=1 && conf <= 4)
		{
			qint32 hystValue = instance->param(i_hyst)->signedIntValue();
			qint32 settingValue = instance->param(i_setting)->signedIntValue();
			qint32 inputValue = instance->param(i_data)->signedIntValue();
			quint16 prevResult = 0;

			switch (conf)
			{
			case 1:		// SignedInt32, ==
				if (inputValue >= (settingValue - hystValue) &&
					inputValue <= (settingValue + hystValue))
				{
					instance->addParamWord(o_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
				}
				break;

			case 2:		// SignedInt32, >
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if (inputValue > settingValue ||
					(prevResult == 1 && inputValue > settingValue - hystValue))
				{
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
					instance->addParamWord(i_prev_result, 0);
				}
				break;

			case 3:		// SignedInt32, <
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if ((inputValue < settingValue) ||
					(prevResult == 1 && inputValue < settingValue + hystValue))
				{
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
					instance->addParamWord(i_prev_result, 0);
				}
				break;

			case 4:		// SignedInt32, <>
				if (inputValue >= (settingValue - hystValue) &&
					inputValue <= (settingValue + hystValue))
				{
					instance->addParamWord(o_result, 0);
				}
				else
				{
					instance->addParamWord(o_result, 1);
				}
				break;

			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		if (conf >=5 && conf <= 8)
		{
			float hystValue = instance->param(i_hyst)->floatValue();
			float settingValue = instance->param(i_setting)->floatValue();
			float inputValue = instance->param(i_data)->floatValue();
			quint16 prevResult = 0;

			// NaN
			//
			quint16 nan = (std::isnan(settingValue) ||
						  std::isnan(hystValue) ||
						  std::isnan(inputValue)) ? 0x0001 : 0x0000;

			instance->addParamWord(o_nan, nan);

			if (nan != 0)
			{
				instance->addParamWord(o_result, 0);
				instance->addParamWord(i_prev_result, 0);
				instance->addParamWord(o_overflow, 0);
				instance->addParamWord(o_underflow, 0);
				return;
			}

			// --
			//
			switch (conf)
			{
			case 5:		// FloatingPoint32, ==
				{
					AfbComponentParam setValLow;
					AfbComponentParam setValHigh;

					setValLow.setFloatValue(settingValue - hystValue);
					setValHigh.setFloatValue(settingValue + hystValue);

					if (inputValue >= setValLow.floatValue() &&
						inputValue <= setValHigh.floatValue())
					{
						instance->addParamWord(o_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
					}

					instance->addParamWord(o_overflow, setValLow.mathOverflow() || setValHigh.mathOverflow());
					instance->addParamWord(o_underflow, setValHigh.mathUnderflow() || setValHigh.mathUnderflow());
				}
				break;

			case 6:		// FloatingPoint32, >
				{
					if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
					{
						prevResult = instance->param(i_prev_result)->wordValue();
					}

					AfbComponentParam t;

					t.setFloatValue(settingValue);
					t.subFloatingPoint(hystValue);

					if ((inputValue > settingValue) ||
						(prevResult == 1 && inputValue > t.floatValue()))
					{
						instance->addParamWord(o_result, 1);
						instance->addParamWord(i_prev_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);
					}

					instance->addParamWord(o_overflow, t.mathOverflow());
					instance->addParamWord(o_underflow, t.mathUnderflow());
				}
				break;

			case 7:		// FloatingPoint32, <
				{
					if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
					{
						prevResult = instance->param(i_prev_result)->wordValue();
					}

					AfbComponentParam t;

					t.setFloatValue(settingValue);
					t.addFloatingPoint(hystValue);

					if ((inputValue < settingValue) ||
						(prevResult == 1 && inputValue < t.floatValue()))
					{
						instance->addParamWord(o_result, 1);
						instance->addParamWord(i_prev_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);
					}

					instance->addParamWord(o_overflow, t.mathOverflow());
					instance->addParamWord(o_underflow, t.mathUnderflow());
				}
				break;

			case 8:		// FloatingPoint32, <>
				{
					AfbComponentParam setValLow;
					AfbComponentParam setValHigh;

					setValLow.setFloatValue(settingValue - hystValue);
					setValHigh.setFloatValue(settingValue + hystValue);

					if (inputValue >= setValLow.floatValue() &&
						inputValue <= setValHigh.floatValue())
					{
						instance->addParamWord(o_result, 0);
					}
					else
					{
						instance->addParamWord(o_result, 1);
					}

					instance->addParamWord(o_overflow, setValLow.mathOverflow() || setValHigh.mathOverflow());
					instance->addParamWord(o_underflow, setValHigh.mathUnderflow() || setValHigh.mathUnderflow());
				}
				break;

			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
		return;
	}

	void CommandProcessor_LM5_LM6::afb_dpcomp_v4(AfbComponentInstance* instance)
	{
		return afb_dpcomp_v5(instance);		// it has the same implementation, in LM's version 5 fixed error with forming o_nan
	}

	void CommandProcessor_LM5_LM6::afb_dpcomp_v5(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_hyst = 1;			// Hysteresis value
		const int i_prev_result = 3;	// Prev result

		const int i_data = 4;			// Input data
		const int i_setting = 6;		// Setting value

		const int o_result = 9;			// Result
		const int o_overflow = 10;		// Result
		const int o_underflow = 11;		// Result
		const int o_nan = 13;			// Any input FP param NaN

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		// AFB Logic
		//
		if (conf >=1 && conf <= 4)
		{
			qint32 hystValue = instance->param(i_hyst)->signedIntValue();
			qint32 settingValue = instance->param(i_setting)->signedIntValue();
			qint32 inputValue = instance->param(i_data)->signedIntValue();
			quint16 prevResult = 0;

			switch (conf)
			{
			case 1:		// SignedInt32, ==
				if (inputValue >= (settingValue - hystValue / 2) &&
					inputValue <= (settingValue + hystValue / 2))
				{
					instance->addParamWord(o_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
				}
				break;

			case 2:		// SignedInt32, >
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if (inputValue > settingValue ||
					(prevResult == 1 && inputValue > settingValue - hystValue))
				{
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
					instance->addParamWord(i_prev_result, 0);
				}
				break;

			case 3:		// SignedInt32, <
				if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
				{
					prevResult = instance->param(i_prev_result)->wordValue();
				}

				if ((inputValue < settingValue) ||
					(prevResult == 1 && inputValue < settingValue + hystValue))
				{
					instance->addParamWord(o_result, 1);
					instance->addParamWord(i_prev_result, 1);
				}
				else
				{
					instance->addParamWord(o_result, 0);
					instance->addParamWord(i_prev_result, 0);
				}
				break;

			case 4:		// SignedInt32, <>
				if (inputValue >= (settingValue - hystValue / 2) &&
					inputValue <= (settingValue + hystValue / 2))
				{
					instance->addParamWord(o_result, 0);
				}
				else
				{
					instance->addParamWord(o_result, 1);
				}
				break;

			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		if (conf >=5 && conf <= 8)
		{
			float hystValue = instance->param(i_hyst)->floatValue();
			float settingValue = instance->param(i_setting)->floatValue();
			float inputValue = instance->param(i_data)->floatValue();
			quint16 prevResult = 0;

			// NaN
			//
			quint16 nan = (std::isnan(settingValue) ||
						   std::isnan(hystValue) ||
						   std::isnan(inputValue)) ? 0x0001 : 0x0000;

			instance->addParamWord(o_nan, nan);

			if (nan != 0)
			{
				instance->addParamWord(o_result, 0);
				instance->addParamWord(i_prev_result, 0);
				instance->addParamWord(o_overflow, 0);
				instance->addParamWord(o_underflow, 0);
				return;
			}

			// --
			//
			switch (conf)
			{
			case 5:		// FloatingPoint32, ==
				{
					AfbComponentParam setValLow;
					AfbComponentParam setValHigh;

					setValLow.setFloatValue(settingValue - hystValue / 2.0f);
					setValHigh.setFloatValue(settingValue + hystValue / 2.0f);

					if (inputValue >= setValLow.floatValue() &&
						inputValue <= setValHigh.floatValue())
					{
						instance->addParamWord(o_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
					}

					instance->addParamWord(o_overflow, setValLow.mathOverflow() || setValHigh.mathOverflow());
					instance->addParamWord(o_underflow, setValHigh.mathUnderflow() || setValHigh.mathUnderflow());
				}
				break;

			case 6:		// FloatingPoint32, >
				{
					if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
					{
						prevResult = instance->param(i_prev_result)->wordValue();
					}

					AfbComponentParam t;

					t.setFloatValue(settingValue);
					t.subFloatingPoint(hystValue);

					if ((inputValue > settingValue) ||
						(prevResult == 1 && inputValue > t.floatValue()))
					{
						instance->addParamWord(o_result, 1);
						instance->addParamWord(i_prev_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);
					}

					instance->addParamWord(o_overflow, t.mathOverflow());
					instance->addParamWord(o_underflow, t.mathUnderflow());
				}
				break;

			case 7:		// FloatingPoint32, <
				{
					if (instance->paramExists(i_prev_result) == true)	// There is not prev result for first cycle;
					{
						prevResult = instance->param(i_prev_result)->wordValue();
					}

					AfbComponentParam t;

					t.setFloatValue(settingValue);
					t.addFloatingPoint(hystValue);

					if ((inputValue < settingValue) ||
						(prevResult == 1 && inputValue < t.floatValue()))
					{
						instance->addParamWord(o_result, 1);
						instance->addParamWord(i_prev_result, 1);
					}
					else
					{
						instance->addParamWord(o_result, 0);
						instance->addParamWord(i_prev_result, 0);
					}

					instance->addParamWord(o_overflow, t.mathOverflow());
					instance->addParamWord(o_underflow, t.mathUnderflow());
				}
				break;

			case 8:		// FloatingPoint32, <>
				{
					AfbComponentParam setValLow;
					AfbComponentParam setValHigh;

					setValLow.setFloatValue(settingValue - hystValue / 2.0f);
					setValHigh.setFloatValue(settingValue + hystValue / 2.0f);

					if (inputValue >= setValLow.floatValue() &&
						inputValue <= setValHigh.floatValue())
					{
						instance->addParamWord(o_result, 0);
					}
					else
					{
						instance->addParamWord(o_result, 1);
					}

					instance->addParamWord(o_overflow, setValLow.mathOverflow() || setValHigh.mathOverflow());
					instance->addParamWord(o_underflow, setValHigh.mathUnderflow() || setValHigh.mathUnderflow());
				}
				break;

			default:
				SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
			}

			return;
		}

		SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
		return;
	}

	//	MUX, OpCode 21
	//
	void CommandProcessor_LM5_LM6::afb_mux_v1(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_sel = 0;
		const int i_input_1 = 1;
		const int i_input_2 = 3;

		const int o_result = 6;
		//const int o_version = 8;

		// Get params,  check_param throws exception in case of error
		//
		quint16 selector = instance->param(i_sel)->wordValue();
		const AfbComponentParam* input1 = instance->param(i_input_1);
		const AfbComponentParam* input2 = instance->param(i_input_2);

		AfbComponentParam result = {selector == 0 ? *input1 : *input2};
		result.setOpIndex(o_result);

		instance->addParam(result);

		return;
	}

	//	LATCH, OpCode 22
	//
	void CommandProcessor_LM5_LM6::afb_latch_v4(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;					// 1 - 6
		const int i_set_prev = 1;				// Previous set input state
		const int i_y_prev = 2;					// Previous in state
		const int i_data = 4;					// input
		const int i_set = 6;					// Set input
		const int i_reset = 7;					// Reset input


		//const int o_set_prev = 9;				// Previous set -> i_set_prev
		//const int o_y_prev = 10;				// output Y/input 
		const int o_result = 12;				// output Y
		//const int o_edi = 14;
		//const int o_version = 15;

		// Get params, throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(conf, 1, 4, QStringLiteral("i_conf"));

		// Logic
		//
		switch (conf)
		{
		case 1:
			{
				// Latch on Front
				//
				quint32 input = instance->param(i_data)->dwordValue();
				quint32 prevResult = instance->paramExists(i_y_prev) ? instance->param(i_y_prev)->dwordValue() : 0;

				quint16 set = instance->param(i_set)->wordValue() & 0x0001;
				quint16 prevSet = instance->paramExists(i_set_prev) ? instance->param(i_set_prev)->wordValue() : 0;
				quint16 reset = instance->param(i_reset)->wordValue() & 0x0001;

				// Logic
				//
				quint16 state = (set << 1) | reset;
				quint32 result = 0;	// for SI and FP it will 0

				switch (state)
				{
				case 0:	// set 0, reset 0
					result = prevResult;
					break;
				case 1:	// set 0, reset 1
					result = 0;
					break;
				case 2:	// set 1, reset 0
					if (prevSet == 0)
					{
						// Front catch
						//
						result = input;
					}
					else
					{
						result = prevResult;
					}
					break;
				case 3:	// set 1, reset 1
					result = 0;
					break;
				default:
					Q_ASSERT(false);
				}

				// Result
				//
				instance->addParamDword(o_result, result);

				instance->addParamDword(i_y_prev, result);
				//instance->addParamDword(o_y_prev, result);		// Commented for optimization

				instance->addParamWord(i_set_prev, set);
				//instance->addParamWord(o_set_prev, set);			// Commented for optimization
			}
			break;
		case 2:
			{
				// Latch on Decay
				//
				quint32 input = instance->param(i_data)->dwordValue();
				quint32 prevResult = instance->paramExists(i_y_prev) ? instance->param(i_y_prev)->dwordValue() : 0;

				quint16 set = instance->param(i_set)->wordValue() & 0x0001;
				quint16 prevSet = instance->paramExists(i_set_prev) ? instance->param(i_set_prev)->wordValue() : 0;
				quint16 reset = instance->param(i_reset)->wordValue() & 0x0001;

				// Logic
				//
				quint16 state = (set << 1) | reset;
				quint32 result = 0;	// for SI and FP it will 0

				switch (state)
				{
				case 0:	// set 0, reset 0
					if (prevSet == 1)
					{
						// Decay catch
						//
						result = input;
					}
					else
					{
						result = prevResult;
					}
					break;
				case 1:	// set 0, reset 1
					result = 0;
					break;
				case 2:	// set 1, reset 0
					result = prevResult;
					break;
				case 3:	// set 1, reset 1
					result = 0;
					break;
				default:
					Q_ASSERT(false);
				}

				// Result
				//
				instance->addParamDword(o_result, result);

				instance->addParamDword(i_y_prev, result);
				//instance->addParamDword(o_y_prev, result);		// Commented for optimization

				instance->addParamWord(i_set_prev, set);
				//instance->addParamWord(o_set_prev, set);			// Commented for optimization
			}
			break;
		case 3:
			{
				// Latch on State
				//
				quint32 input = instance->param(i_data)->dwordValue();
				quint32 prevResult = instance->paramExists(i_y_prev) ? instance->param(i_y_prev)->dwordValue() : 0;

				quint16 set = instance->param(i_set)->wordValue() & 0x0001;
				quint16 reset = instance->param(i_reset)->wordValue() & 0x0001;

				// Logic
				//
				quint16 state = (set << 1) | reset;
				quint32 result = 0;	// for SI and FP it will 0

				switch (state)
				{
				case 0:	// set 0, reset 0
					result = prevResult;
					break;
				case 1:	// set 0, reset 1
					result = 0;
					break;
				case 2:	// set 1, reset 0
					result = input;
					break;
				case 3:	// set 1, reset 1
					result = 0;
					break;
				default:
					Q_ASSERT(false);
				}

				// Result
				//
				instance->addParamDword(o_result, result);
				//instance->addParamDword(o_y_prev, result);		// Commented for optimization
				instance->addParamDword(i_y_prev, result);
			}
			break;
		case 4:
			{
				// Latch on tm1 - delay for 1 cycle
				// On version 4 of the element for one cycle after reset  output will be always 0 (even if prev cycle input was 1)
				// It is an error, hopefully it will be fixed in the next version
				//
				quint32 input = instance->param(i_data)->dwordValue();
				quint32 prevInput = instance->paramExists(i_y_prev) ? instance->param(i_y_prev)->dwordValue() : 0;

				quint16 reset = instance->param(i_reset)->wordValue() & 0x0001;

				// Logic
				//
				quint32 result = 0;	// for SI and FP it will 0

				if (reset == 0)
				{
					result = prevInput;
				}
				else
				{
					result = 0;
				}

				// Result
				//
				instance->addParamDword(o_result, result);

				if (reset == 0)
				{
					instance->addParamDword(i_y_prev, input);
				}
				else
				{
					// That is how implemented in VHDL code for v4 of AFB LATCH
					// if reset has gone then output will be in 0 in any case for one cycle.
					// It is an error, hopefully it will be fixed in the next version.
					//
					instance->addParamDword(i_y_prev, 0);
				}
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_bcomp");
		}

		return;
	}


	//	LIM, OpCode 23
	//
	void CommandProcessor_LM5_LM6::afb_lim_v7(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_conf = 0;
		const int i_lim_max = 1;
		const int i_lim_min = 3;
		const int i_data = 5;

		const int o_result = 8;
		//const int o_lim_edi = 10;
		const int o_nan = 11;
		const int o_max = 12;
		const int o_min = 13;
		//const int o_version = 14;
		const int o_param_err = 15;

		// Get params,  check_param throws exception in case of error
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		const AfbComponentParam* limMax = instance->param(i_lim_max);
		const AfbComponentParam* limMin = instance->param(i_lim_min);
		const AfbComponentParam* data = instance->param(i_data);

		AfbComponentParam result = *data;

		// 1: SignedInt32, 2: Float32
		//
		bool setToMin = false;
		bool setToMax = false;
		bool setParamError = false;
		bool setNan = false;

		switch (conf)
		{
		case 1:
			{
				qint32 dataInt = data->signedIntValue();
				qint32 minInt = limMin->signedIntValue();
				qint32 maxInt = limMax->signedIntValue();

				if (minInt > maxInt)
				{
					result.setSignedIntValue(0);
					setParamError = true;
					break;
				}

				if (dataInt <= minInt)
				{
					result.setSignedIntValue(minInt);
					setToMin = true;
				}

				if (dataInt >= maxInt)
				{
					result.setSignedIntValue(maxInt);
					setToMax = true;
				}
			}
			break;
		case 2:
			{
				float dataFloat = data->floatValue();
				float minFloat = limMin->floatValue();
				float maxFloat = limMax->floatValue();

				if (std::isnan(dataFloat) ||
					std::isnan(minFloat) ||
					std::isnan(maxFloat))
				{
					result.setFloatValue(0);
					setNan = true;
					break;
				}

				if (minFloat > maxFloat)
				{
					result.setFloatValue(0);
					setParamError = true;
					break;
				}

				if (dataFloat <= minFloat)
				{
					result = *limMin;
					setToMin = true;
				}

				if (dataFloat >= maxFloat)
				{
					result = *limMax;
					setToMax = true;
				}
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
								.arg(conf),
								"CommandProcessor_LM5_LM6::afb_lim");
		}

		// Save result
		//
		result.setOpIndex(o_result);
		instance->addParam(result);

		instance->addParamWord(o_max, setToMax);
		instance->addParamWord(o_min, setToMin);
		instance->addParamWord(o_param_err, setParamError);
		instance->addParamWord(o_nan, setNan);

		return;
	}

	//	DEADZONE, OpCode 24
	//
	void CommandProcessor_LM5_LM6::fb_deadzone_v5(AfbComponentInstance* /*instance*/)
	{
		SimException::raise(QString("fb_deadzone_v5: Is not implemented as hardware vesrion of this AFB has a number of error. Wait for version 7."), "CommandProcessor_LM5_LM6::fb_deadzone_v5");
	}

	void CommandProcessor_LM5_LM6::fb_deadzone_v6(AfbComponentInstance* /*instance*/)
	{
		SimException::raise(QString("fb_deadzone_v6: Is not implemented as hardware vesrion of this AFB has a number of error. Wait for version 7."), "CommandProcessor_LM5_LM6::fb_deadzone_v6");
	}

	//	POL, OpCode 25
	//
	void CommandProcessor_LM5_LM6::afb_pol_v3(AfbComponentInstance* instance)
	{
		// Y=C0+C1*X**1+C2*X**2+..+Cm*X**m
		// m=9

		// Define opIndexes
		//
		const int i_conf = 0;
		const int i_1_oprd = 1;
//		const int i_2_oprd = 3;
//		const int i_3_oprd = 5;
//		const int i_4_oprd = 7;
//		const int i_5_oprd = 9;
//		const int i_6_oprd = 11;
//		const int i_7_oprd = 13;
//		const int i_8_oprd = 15;
//		const int i_9_oprd = 17;
//		const int i_10_oprd = 19;
		const int i_data = 21;

		const int o_result = 24;
		//const int o_pol_edi = 26;
		const int o_overflow = 27;
		const int o_underflow = 28;
		const int o_zero = 29;
		const int o_nan = 30;
		//const int o_version = 31;

		// Get params, check_param throws exception in case of error
		//
		const size_t maxOperandCount = 10;

		quint16 settings = instance->param(i_conf)->wordValue();
		checkParamRange(settings, 2, maxOperandCount, QStringLiteral("i_conf"));

		std::array<AfbComponentParam, maxOperandCount> operands;

		quint16 overflow = 0;
		quint16 underflow = 0;

		float x = instance->param(i_data)->floatValue();

		for (int i = 0; i < settings; i++)
		{
			operands[i] = *(instance->param(i_1_oprd + i * 2));

			float c = std::powf(x, static_cast<float>(i));
			operands[i].mulFloatingPoint(c);

			overflow |= operands[i].mathOverflow();
			underflow |= operands[i].mathUnderflow();
		}

		AfbComponentParam result = operands[0];
		for (int i = 1; i < settings; i++)
		{
			result.addFloatingPoint(operands[i]);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
		}

		// Save result
		//
		result.setOpIndex(o_result);
		instance->addParam(result);

		instance->addParamWord(o_overflow, overflow);
		instance->addParamWord(o_underflow, underflow);
		instance->addParamWord(o_zero, result.mathZero());
		instance->addParamWord(o_nan, result.mathNan());

		return;
	}

	//	DER, OpCode 26
	//
	void CommandProcessor_LM5_LM6::afb_der_v5_tdunlim(AfbComponentInstance* instance)
	{
		return afb_der_v5(instance, std::numeric_limits<qint32>::max());
	}

	void CommandProcessor_LM5_LM6::afb_der_v5_td350000(AfbComponentInstance* instance)
	{
		return afb_der_v5(instance, 350000);
	}

	void CommandProcessor_LM5_LM6::afb_der_v5(AfbComponentInstance* instance, qint32 maxTdValue)
	{
		// Define inputs/outputs opIndexes
		//
		const int i_kd = 0;				// [0, 1]	FP (32bit)	Dif. Coef. of Gain
		const int i_td = 2;				// [2, 3]	SI (32bit)	The Dif. time td(ms) (0..((2^31) - 1)
		const int i_max = 4;			// [4, 5]	FP (32bit)	Max output value
		const int i_min = 6;			// [6, 7]	FP (32bit)	Min output value
		const int i_x_prev = 8;			// [8, 9]	FP (32bit)	Previouse input value X
		const int i_yd_prev = 10;		// [10, 11]	FP (32bit)	Previous result value
		const int i_x = 12;				// [12, 13]	FP (32bit)	Input X
		const int i_reset = 14;			// Discrete		Reset
		const int i_pause = 15;			// Discrete		Pause mode

		//const int o_x_prev = 17;		// [17, 18]	FP (32bit)	Previous (now is current) input value X
		const int o_result = 19;		// [19, 20]	FP (32bit)	Result - Output Y
		const int o_max = 21;			// Discrete		Maximum value signal
		const int o_min = 22;			// Discrete		Minimum value signal

		const int o_param_err = 23;		// Discrete
		const int o_overflow = 24;		// Discrete
		const int o_underflow = 25;		// Discrete
		const int o_zero = 26;			// Discrete
		const int o_nan = 27;			// Discrete
		//const int o_version = 28;		// Discrete

		// Get input data
		//
		float kd = instance->param(i_kd)->floatValue();
		qint32 td = instance->param(i_td)->signedIntValue();
		float maxValue = instance->param(i_max)->floatValue();
		float minValue = instance->param(i_min)->floatValue();

		float x_prev = instance->paramExists(i_x_prev) == true ?
							instance->param(i_x_prev)->floatValue() :
							0.0f;

		float yd_prev = instance->paramExists(i_yd_prev) == true ?
						   instance->param(i_yd_prev)->floatValue() :
						   0.0f;

		float x = instance->param(i_x)->floatValue();

		quint16 reset = instance->param(i_reset)->wordValue();
		quint16 pause = instance->param(i_pause)->wordValue();

		if (std::isnan(yd_prev) == true)	// NaN
		{
			yd_prev = .0f;
		}

		// Logic
		// Y= F(,Kd,Td)
		// Yd = (X - (Xi-1)) * Kd - (Yi-1) / TD + (Yi-1);
		// TD=d(ms) / 5(ms)
		// i_td = Ti(ms)
		//
		AfbComponentParam result{static_cast<quint16>(o_result)};
		result.setFloatValue(0);

		float newXPrev = x;

		quint16 maxOut = 0;
		quint16 minOut = 0;

		quint16 param_err = 0;
		quint16 overflow = 0;
		quint16 underflow = 0;
		quint16 zero = 0;
		quint16 nan = 0;

		do
		{
			if (maxValue <= minValue)
			{
				result.setFloatValue(yd_prev);
				param_err = 0x0001;
				newXPrev = x_prev;
				break;
			}

			if (td < m_cycleDurationMs ||
				td > maxTdValue)
			{
				param_err = 1;
				td = std::clamp(td, m_cycleDurationMs, maxTdValue);

				// Continue to work with corrected value
				//
			}

			// Reset, Pause and Track are set in accordance to priority
			//
			if (reset == 1)
			{
				result.setFloatValue(0);
				newXPrev = .0f;
				break;
			}

			if (pause == 1)
			{
				result.setFloatValue(yd_prev);
				newXPrev = x_prev;
				break;
			}

			// Normal operating
			// Yd = (X - (Xi-1)) * Kd - (Yi-1) / TD + (Yi-1);
			//
			float td_value = td / static_cast<float>(m_cycleDurationMs);

			// (X - (Xi-1))
			//
			result.setFloatValue(x);
			result.subFloatingPoint(x_prev);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// * Kd
			//
			result.mulFloatingPoint(kd);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// (Yi-1) / TD
			//
			AfbComponentParam yprev_div_td{0};

			yprev_div_td.setFloatValue(yd_prev);
			yprev_div_td.divFloatingPoint(td_value);

			overflow |= yprev_div_td.mathOverflow();
			underflow |= yprev_div_td.mathUnderflow();
			nan |= yprev_div_td.mathNan();

			// - (Yi-1) / TD
			//
			result.subFloatingPoint(yprev_div_td.floatValue());

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// + (Yi-1)
			//
			result.addFloatingPoint(yd_prev);

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();

			// Check max/min
			//
			if (result.floatValue() >= maxValue)
			{
				result.setFloatValue(maxValue);
				maxOut = 0x0001;
			}

			if (result.floatValue() <= minValue)
			{
				result.setFloatValue(minValue);
				minOut = 0x0001;
			}

			overflow |= result.mathOverflow();
			underflow |= result.mathUnderflow();
			nan |= result.mathNan();
			zero = result.mathZero();

		} while (false);


		// Set outputs
		//
		instance->addParamFloat(o_result, result.floatValue());
		instance->addParamFloat(i_yd_prev, result.floatValue());
		instance->addParamFloat(i_x_prev, newXPrev);

		instance->addParamWord(o_max, maxOut);
		instance->addParamWord(o_min, minOut);

		instance->addParamWord(o_overflow, overflow);
		instance->addParamWord(o_underflow, underflow);
		instance->addParamWord(o_zero, zero);
		instance->addParamWord(o_nan, nan);
		instance->addParamWord(o_param_err, param_err);

		return;

	}

	// MISMATCH, OpCode 27
	// Analog Mismatch
	// v2 - base version
	// v3 - removed o_zero
	// v4 - no functional difference to v3, just different VHDL implementation
	//
	void CommandProcessor_LM5_LM6::afb_mismatch_v2(AfbComponentInstance* instance)
	{
		return afb_mismatch_impl(instance, 2);
	}

	void CommandProcessor_LM5_LM6::afb_mismatch_v3(AfbComponentInstance* instance)
	{
		return afb_mismatch_impl(instance, 3);
	}

	void CommandProcessor_LM5_LM6::afb_mismatch_v4(AfbComponentInstance* instance)
	{
		return afb_mismatch_impl(instance, 4);
	}

	void CommandProcessor_LM5_LM6::afb_mismatch_impl(AfbComponentInstance* instance, int version)
	{
		const int i_conf = 0;			// SI/FP
		quint16 conf = instance->param(i_conf)->wordValue();

		checkParamRange(conf, 1, 2, QStringLiteral("i_conf"));

		switch (conf)
		{
		case 1:
			afb_mismatch_impl_si(instance, version);
			break;
		case 2:
			afb_mismatch_impl_fp(instance, version);
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
								.arg(conf),
								"CommandProcessor_LM5_LM6::afb_mismatch");
		}

		return;
	}

	void CommandProcessor_LM5_LM6::afb_mismatch_impl_si(AfbComponentInstance* instance, int version)
	{
		// Define input opIndexes
		//
		const int i_conf_n = 1;			// Param count 2 - 4
		const int i_ust = 2;			// Setpoint - diff val
		const int i_data_1 = 4;
		const int i_data_2 = 6;
		const int i_data_3 = 8;
		const int i_data_4 = 10;

		const int o_mismatch_1 = 12;
		const int o_mismatch_2 = 13;
		const int o_mismatch_3 = 14;
		const int o_mismatch_4 = 15;
		const int o_param_err = 16;
		const int o_zero = 19;			// for v2 only

		// Get params,  check_param throws exception in case of error
		//
		quint16 paramCount = instance->param(i_conf_n)->wordValue();
		checkParamRange(paramCount, 1, 4, QStringLiteral("i_conf_n"));

		// Get setpoint
		//
		const AfbComponentParam* setpoint = instance->param(i_ust);
		qint64 sp = setpoint->signedIntValue();

		if (sp < 0)
		{
			instance->addParamWord(o_param_err, 1);
			sp = 0;
		}
		else
		{
			instance->addParamWord(o_param_err, 0);
		}

		// Logic
		//
		bool zero = false;

		switch (paramCount)
		{
		case 2:
			{
				qint64 data1 = instance->param(i_data_1)->signedIntValue();
				qint64 data2 = instance->param(i_data_2)->signedIntValue();

				qint64 x1_x2 = std::abs(data1 - data2);
				bool x1_x2_b = x1_x2 > sp;

				bool out1 = x1_x2_b;
				bool out2 = x1_x2_b;
				zero = (x1_x2 == 0);

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
			}
			break;

		case 3:
			{
				qint64 data1 = instance->param(i_data_1)->signedIntValue();
				qint64 data2 = instance->param(i_data_2)->signedIntValue();
				qint64 data3 = instance->param(i_data_3)->signedIntValue();

				qint64 x1_x2 = std::abs(data1 - data2);
				qint64 x1_x3 = std::abs(data1 - data3);
				qint64 x2_x3 = std::abs(data2 - data3);

				bool x1_x2_b = x1_x2 > sp;
				bool x1_x3_b = x1_x3 > sp;
				bool x2_x3_b = x2_x3 > sp;

				bool out1 = x1_x2_b && x1_x3_b;
				bool out2 = x1_x2_b && x2_x3_b;
				bool out3 = x1_x3_b && x2_x3_b;

				zero = (x1_x2 == 0) || (x1_x3 == 0) || (x2_x3 == 0);

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
				instance->addParamWord(o_mismatch_3, out3 ? 1 : 0);
			}
			break;

		case 4:
			{
				qint64 data1 = instance->param(i_data_1)->signedIntValue();
				qint64 data2 = instance->param(i_data_2)->signedIntValue();
				qint64 data3 = instance->param(i_data_3)->signedIntValue();
				qint64 data4 = instance->param(i_data_4)->signedIntValue();

				qint64 x1_x2 = std::abs(data1 - data2);
				qint64 x1_x3 = std::abs(data1 - data3);
				qint64 x1_x4 = std::abs(data1 - data4);
				qint64 x2_x3 = std::abs(data2 - data3);
				qint64 x2_x4 = std::abs(data2 - data4);
				qint64 x3_x4 = std::abs(data3 - data4);

				bool x1_x2_b = x1_x2 > sp;
				bool x1_x3_b = x1_x3 > sp;
				bool x1_x4_b = x1_x4 > sp;
				bool x2_x3_b = x2_x3 > sp;
				bool x2_x4_b = x2_x4 > sp;
				bool x3_x4_b = x3_x4 > sp;

				bool out1 = (x1_x2_b && x1_x3_b) || (x1_x2_b && x1_x4_b) || (x1_x3_b && x1_x4_b);
				bool out2 = (x1_x2_b && x2_x3_b) || (x1_x2_b && x2_x4_b) || (x2_x3_b && x2_x4_b);
				bool out3 = (x1_x3_b && x2_x3_b) || (x1_x3_b && x3_x4_b) || (x2_x3_b && x3_x4_b);
				bool out4 = (x1_x4_b && x2_x4_b) || (x1_x4_b && x3_x4_b) || (x2_x4_b && x3_x4_b);

				zero = (x1_x2 == 0) || (x1_x3 == 0) || (x1_x4 == 0) ||
					   (x2_x3 == 0) || (x2_x4 == 0) || (x3_x4 == 0);

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
				instance->addParamWord(o_mismatch_3, out3 ? 1 : 0);
				instance->addParamWord(o_mismatch_4, out4 ? 1 : 0);
			}
			break;

		default:
			Q_ASSERT(false);
		}

		if (version == 2)
		{
			// o_zero present only in AFB MISMATCH version 2
			//
			instance->addParamWord(o_zero, zero ? 1 : 0);
		}

		return;
	}

	void CommandProcessor_LM5_LM6::afb_mismatch_impl_fp(AfbComponentInstance* instance, int version)
	{
		// Define input opIndexes
		//
		const int i_conf_n = 1;			// Param count 2 - 4
		const int i_ust = 2;			// Setpoint - diff val
		const int i_data_1 = 4;
		const int i_data_2 = 6;
		const int i_data_3 = 8;
		const int i_data_4 = 10;

		const int o_mismatch_1 = 12;
		const int o_mismatch_2 = 13;
		const int o_mismatch_3 = 14;
		const int o_mismatch_4 = 15;
		const int o_param_err = 16;
		const int o_overflow = 17;
		const int o_underflow = 18;
		const int o_zero = 19;			// for v2 only
		const int o_nan = 20;

		// Get params,  check_param throws exception in case of error
		//
		quint16 paramCount = instance->param(i_conf_n)->wordValue();
		checkParamRange(paramCount, 1, 4, QStringLiteral("i_conf_n"));

		// Get setpoint
		//
		const AfbComponentParam* setpoint = instance->param(i_ust);
		float sp = setpoint->floatValue();

		if (sp < 0.0f)
		{
			instance->addParamWord(o_param_err, 1);
			sp = 0;
		}
		else
		{
			instance->addParamWord(o_param_err, 0);
		}

		// Logic
		//
		quint16 zero = 0;
		quint16 overflow = 0;
		quint16 underflow = 0;
		quint16 nan = 0;

		switch (paramCount)
		{
		case 2:
			{
				AfbComponentParam x1_x2 = *instance->param(i_data_1);

				x1_x2.subFloatingPoint(*instance->param(i_data_2));

				overflow = x1_x2.mathOverflow();
				underflow = x1_x2.mathUnderflow();
				zero = x1_x2.mathZero();
				nan = x1_x2.mathNan();

				x1_x2.absFloatingPoint();

				bool x1_x2_b = x1_x2.floatValue() > sp;

				bool out1 = x1_x2_b;
				bool out2 = x1_x2_b;

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
			}
			break;

		case 3:
			{
				AfbComponentParam x1_x2 = *instance->param(i_data_1);	// si_a_g_b1
				AfbComponentParam x1_x3 = *instance->param(i_data_1);	// si_a_g_b2
				AfbComponentParam x2_x3 = *instance->param(i_data_2);	// si_a_g_b3

				x1_x2.subFloatingPoint(*instance->param(i_data_2));
				x1_x3.subFloatingPoint(*instance->param(i_data_3));
				x2_x3.subFloatingPoint(*instance->param(i_data_3));

				overflow = x1_x2.mathOverflow() | x1_x3.mathOverflow() | x2_x3.mathOverflow();
				underflow = x1_x2.mathUnderflow() | x1_x3.mathUnderflow() | x2_x3.mathUnderflow();
				zero = x1_x2.mathZero() | x1_x3.mathZero() | x2_x3.mathZero();
				nan = x1_x2.mathNan() | x1_x3.mathNan() | x2_x3.mathNan();

				x1_x2.absFloatingPoint();
				x1_x3.absFloatingPoint();
				x2_x3.absFloatingPoint();

				bool x1_x2_b = x1_x2.floatValue() > sp;
				bool x1_x3_b = x1_x3.floatValue() > sp;
				bool x2_x3_b = x2_x3.floatValue() > sp;

				bool out1 = x1_x2_b && x1_x3_b;
				bool out2 = x1_x2_b && x2_x3_b;
				bool out3 = x1_x3_b && x2_x3_b;

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
				instance->addParamWord(o_mismatch_3, out3 ? 1 : 0);
			}
			break;

		case 4:
			{
				AfbComponentParam x1_x2 = *instance->param(i_data_1);	// 1
				AfbComponentParam x1_x3 = *instance->param(i_data_1);	// 2
				AfbComponentParam x1_x4 = *instance->param(i_data_1);	// 3
				AfbComponentParam x2_x3 = *instance->param(i_data_2);	// 4
				AfbComponentParam x2_x4 = *instance->param(i_data_2);	// 5
				AfbComponentParam x3_x4 = *instance->param(i_data_3);	// 6

				x1_x2.subFloatingPoint(*instance->param(i_data_2));
				x1_x3.subFloatingPoint(*instance->param(i_data_3));
				x1_x4.subFloatingPoint(*instance->param(i_data_4));
				x2_x3.subFloatingPoint(*instance->param(i_data_3));
				x2_x4.subFloatingPoint(*instance->param(i_data_4));
				x3_x4.subFloatingPoint(*instance->param(i_data_4));

				overflow = x1_x2.mathOverflow() | x1_x3.mathOverflow() | x1_x4.mathOverflow() |
						   x2_x3.mathOverflow() | x2_x4.mathOverflow() | x3_x4.mathOverflow();

				underflow = x1_x2.mathUnderflow() | x1_x3.mathUnderflow() | x1_x4.mathUnderflow() |
							x2_x3.mathUnderflow() | x2_x4.mathUnderflow() | x3_x4.mathUnderflow();

				zero = x1_x2.mathZero() | x1_x3.mathZero() | x1_x4.mathZero() |
					   x2_x3.mathZero() | x2_x4.mathZero() | x3_x4.mathZero();

				nan = x1_x2.mathNan() | x1_x3.mathNan() | x1_x4.mathNan() |
					  x2_x3.mathNan() | x2_x4.mathNan() | x3_x4.mathNan();

				x1_x2.absFloatingPoint();
				x1_x3.absFloatingPoint();
				x1_x4.absFloatingPoint();
				x2_x3.absFloatingPoint();
				x2_x4.absFloatingPoint();
				x3_x4.absFloatingPoint();

				bool x1_x2_b = x1_x2.floatValue() > sp;
				bool x1_x3_b = x1_x3.floatValue() > sp;
				bool x1_x4_b = x1_x4.floatValue() > sp;
				bool x2_x3_b = x2_x3.floatValue() > sp;
				bool x2_x4_b = x2_x4.floatValue() > sp;
				bool x3_x4_b = x3_x4.floatValue() > sp;

				bool out1 = (x1_x2_b && x1_x3_b) || (x1_x2_b && x1_x4_b) || (x1_x3_b && x1_x4_b);
				bool out2 = (x1_x2_b && x2_x3_b) || (x1_x2_b && x2_x4_b) || (x2_x3_b && x2_x4_b);
				bool out3 = (x1_x3_b && x2_x3_b) || (x1_x3_b && x3_x4_b) || (x2_x3_b && x3_x4_b);
				bool out4 = (x1_x4_b && x2_x4_b) || (x1_x4_b && x3_x4_b) || (x2_x4_b && x3_x4_b);

				instance->addParamWord(o_mismatch_1, out1 ? 1 : 0);
				instance->addParamWord(o_mismatch_2, out2 ? 1 : 0);
				instance->addParamWord(o_mismatch_3, out3 ? 1 : 0);
				instance->addParamWord(o_mismatch_4, out4 ? 1 : 0);
			}
			break;

		default:
			Q_ASSERT(false);
		}

		instance->addParamWord(o_overflow, overflow);
		instance->addParamWord(o_underflow, underflow);
		instance->addParamWord(o_nan, nan);

		if (version == 2)
		{
			// o_zero present only in AFB MISMATCH version 2
			//
			instance->addParamWord(o_zero, zero);
		}

		return;
	}

	// TCONV, OpCode 28
	// Analog Conversion
	//
	void CommandProcessor_LM5_LM6::afb_tconv_v0(AfbComponentInstance* instance)
	{
		// Define inputs/outputs opIndexes
		//
		const int i_conf = 0;
		const int i_data_16 = 1;
		const int i_data_32 = 2;

		const int o_data_16 = 4;
		const int o_data_32 = 5;
		const int o_overflow = 7;
		const int o_underflow = 8;
		const int o_nan = 9;
		//const int o_tconv_edi = 10;
		//const int o_version = 11;

		// Get AFB configuration
		//
		quint16 conf = instance->param(i_conf)->wordValue();

		// Logic with outputs
		//
		switch (conf)
		{
		case 1: // Swap Endians for 16 -> 16
			{
				qint16 input = instance->param(i_data_16)->wordValue();
				qint16 result = (input >> 8) | (input << 8);
				instance->addParamWord(o_data_16, result);
			}
			break;

		case 2: // Swap Endians for 32 -> 32
			{
				quint32 input = instance->param(i_data_16)->dwordValue();

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
				quint32 result = qToLittleEndian<quint32>(input);
#else
				quint32 result = qToBigEndian<quint32>(input);
#endif

				instance->addParamDword(o_data_32, result);
			}
			break;

		case 3: // SI32 -> FP32
			{
				qint32 input = instance->param(i_data_32)->signedIntValue();
				float result = static_cast<float>(input);
				instance->addParamFloat(o_data_32, result);
			}
			break;

		case 4: // FP32 -> SI32
			{
				float input = instance->param(i_data_32)->floatValue();

				qint32 result = static_cast<qint32>(input);

				// Flags
				//
				quint16 overflow = 0x0000;
				if (static_cast<qint64>(input) > std::numeric_limits<qint32>::max())
				{
					overflow = 0x0001;
					result = std::numeric_limits<qint32>::max();
				}

				if (static_cast<qint64>(input) < std::numeric_limits<qint32>::lowest())
				{
					overflow = 0x0001;
					result = std::numeric_limits<qint32>::lowest();
				}

				if (std::isinf(input) == true)
				{
					if (std::signbit(input) == true)
					{
						result = std::numeric_limits<qint32>::lowest();
					}
					else
					{
						result = std::numeric_limits<qint32>::max();
					}
				}

				quint16 underflow = (std::fpclassify(input) == FP_SUBNORMAL) ? 0x0001 : 0x0000;
				if (underflow == 0x0001)
				{
					result = 0;
				}

				quint16 nan = std::isnan(input);
				if (nan == 0x0001)
				{
					result = 0;
					overflow = 0;
				}

				// Set result
				//
				instance->addParamSignedInt(o_data_32, result);
				instance->addParamWord(o_overflow, overflow);
				instance->addParamWord(o_underflow, underflow);
				instance->addParamWord(o_nan, nan);
			}
			break;

		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_tconv_v0");
		}

		return;
	}

	// INDICATION, OpCode 29
	//
	void CommandProcessor_LM5_LM6::afb_indication_v1(AfbComponentInstance* instance)
	{
		// Define inputs/outputs opIndexes
		//
		const int i_conf = 0;
		const int i_x_prev = 1;
		const int i_1_trigger = 2;
		const int i_2_trigger = 3;
		const int i_x = 4;
		const int i_blink_off = 5;
		const int i_test = 6;
		const int i_blink = 7;

		const int o_result = 8;
		const int o_x_prev = i_x_prev;		// const int o_x_prev = 9
		const int o_1_trigger = i_1_trigger;	// const int o_1_triger = 10
		const int o_2_trigger = i_2_trigger;	// const int o_1_triger = 11
		//const int o_indication_edi = 12;
		//const int o_version = 13;
		const int o_univibrator = 14;

		// Get AFB configuration
		//
		const quint16 conf = instance->param(i_conf)->wordValue();

		const quint16 x_prev_bus = instance->paramExists(i_x_prev)						// 16-bit bus
								   ? instance->param(i_x_prev)->wordValue()
								   : 0;

		const quint16 trigger_1_bus = instance->paramExists(i_1_trigger)				// 16-bit bus
									  ? instance->param(i_1_trigger)->wordValue()
									  : 0;

		const quint16 trigger_2_bus = instance->paramExists(i_2_trigger)				// 16-bit bus
									  ? instance->param(i_2_trigger)->wordValue()
									  : 0;

		const quint16 x_in_bus = instance->param(i_x)->wordValue();						// 16-bit bus
		const quint16 blink_off_bus = instance->param(i_blink_off)->wordValue();		// 16-bit bus
		const quint16 test_bus = instance->param(i_test)->wordValue();					// 16-bit bus
		const quint16 blink_bus = instance->param(i_blink)->wordValue();				// 16-bit bus

		// Logic with outputs
		//
		switch (conf)
		{
		case 1:
			{
				quint16 result_bus = 0;
				quint16 trigger_1_result_bus = 0;
				//quint16 trigger_2_result_bus = 0;
				quint16 x_prev_result_bus = 0;
				quint16 univibrator_bus = 0;

				for (int bitNo = 0; bitNo < 16; bitNo++)
				{
					quint16 blink_off = (blink_off_bus >> bitNo) & 0x0001;
					quint16 x_in = (x_in_bus >> bitNo) & 0x0001;
					quint16 x_prev = (x_prev_bus >> bitNo) & 0x0001;
					quint16 blink = (blink_bus >> bitNo) & 0x0001;
					quint16 trigger_1 = (trigger_1_bus >> bitNo) & 0x0001;

					quint16 result = 0;
					quint16 trigger_1_result = 0;
					//quint16 trigger_2_result = 0;
					//quint16 x_prev_result = 0;
					quint16 univibrator_result = 0;

					if (blink_off == 1 && x_in == 1)
					{
						result = 1;
						trigger_1_result = 0;
					}
					else
					{
						if (x_in == 1)
						{
							if (x_prev == 0)
							{
								result = blink;
								trigger_1_result = 1;
							}
							else
							{
								if (trigger_1 == 1)
								{
									result = blink;
									trigger_1_result = 1;
								}
								else
								{
									result = 1;
									trigger_1_result = 0;
								}
							}
						}
						else
						{
							result = 0;
							trigger_1_result = 0;
						}
					}

					if (x_prev == 0 && x_in == 1)
					{
						univibrator_result = 1;
					}
					else
					{
						univibrator_result = 0;
					}

					// --
					//
					result_bus |= result << bitNo;
					x_prev_result_bus |= x_in << bitNo;
					trigger_1_result_bus |= trigger_1_result << bitNo;
					univibrator_bus |= univibrator_result << bitNo;
				}

				instance->addParamWord(o_result, result_bus | test_bus);
				instance->addParamWord(o_x_prev, x_prev_result_bus);
				instance->addParamWord(o_1_trigger, trigger_1_result_bus);
				instance->addParamWord(o_univibrator, univibrator_bus);
			}
			break;
		case 2:
			{
				quint16 result_bus = 0;
				quint16 trigger_1_result_bus = 0;
				quint16 trigger_2_result_bus = 0;
				quint16 x_prev_result_bus = 0;
				quint16 univibrator_bus = 0;

				for (int bitNo = 0; bitNo < 16; bitNo++)
				{
					quint16 blink_off = (blink_off_bus >> bitNo) & 0x0001;
					quint16 x_in = (x_in_bus >> bitNo) & 0x0001;
					quint16 x_prev = (x_prev_bus >> bitNo) & 0x0001;
					quint16 blink = (blink_bus >> bitNo) & 0x0001;
					quint16 trigger_1 = (trigger_1_bus >> bitNo) & 0x0001;
					quint16 trigger_2 = (trigger_2_bus >> bitNo) & 0x0001;

					quint16 result = 0;
					quint16 trigger_1_result = 0;
					quint16 trigger_2_result = 0;
					quint16 univibrator_result = 0;

					// ---
					//
					if (blink_off == 1 && x_in == 1)
					{
						result = 1;
						trigger_1_result = 0;
						trigger_2_result = 0;
					}
					else
					{
						if (blink_off == 0 && x_in == 0)
						{
							if (trigger_2 == 0)
							{
								result = 0;
								trigger_1_result = 0;
								trigger_2_result = 0;
							}
							else
							{
								result = blink;
								trigger_1_result = 1;
								trigger_2_result = 1;
							}
						}
						else
						{
							if (x_in == 1)
							{
								if (x_prev == 0)
								{
									result = blink;
									trigger_1_result = 1;
									trigger_2_result = 1;
								}
								else
								{
									if (trigger_1 == 1)
									{
										result = blink;
										trigger_1_result = 1;
										trigger_2_result = 1;
									}
									else
									{
										result	= 1;
										trigger_1_result = 0;
										trigger_2_result = 0;
									}
								}
							}
							else
							{
								result	= 0;
								trigger_1_result = 0;
								trigger_2_result = 0;
							}
						}
					}

					if (x_prev == 0 && x_in == 1)
					{
						univibrator_result = 1;
					}
					else
					{
						univibrator_result = 0;
					}

					// --
					//
					result_bus |= result << bitNo;
					x_prev_result_bus |= x_in << bitNo;
					trigger_1_result_bus |= trigger_1_result << bitNo;
					trigger_2_result_bus |= trigger_2_result << bitNo;
					univibrator_bus |= univibrator_result << bitNo;
				}

				instance->addParamWord(o_result, result_bus | test_bus);
				instance->addParamWord(o_x_prev, x_prev_result_bus);
				instance->addParamWord(o_1_trigger, trigger_1_result_bus);
				instance->addParamWord(o_2_trigger, trigger_2_result_bus);
				instance->addParamWord(o_univibrator, univibrator_bus);
			}
			break;
		default:
			SimException::raise(QString("Unknown AFB configuration: %1").arg(conf), "CommandProcessor_LM5_LM6::afb_indication_v1");
		}

		return;
	}

	// PULSE_GEN, OpCode 30
	//
	void CommandProcessor_LM5_LM6::afb_pulse_gen_v0(AfbComponentInstance* instance)
	{
		// Define inputs/outputs opIndexes
		//
		const int i_conf = 0;
		const int i_t_high = 1;
		const int i_t_low = 3;
		const int i_t_prev = 5;
		const int i_result_i_en_prev = 7;
		const int i_start_from = 8;
		const int i_enable = 9;

		const int o_t_prev = i_t_prev;
		const int o_result_en_prev = 13;
		const int o_param_err = 16;

		// Get AFB configuration
		//
		const quint16 conf = instance->param(i_conf)->wordValue();
		const qint32 t_high = instance->param(i_t_high)->signedIntValue();
		const qint32 t_low = instance->param(i_t_low)->signedIntValue();

		qint32 t_prev = instance->paramExists(i_t_prev)
						? instance->param(i_t_prev)->signedIntValue()
						: 0;

		const qint16 result_i_en_prev = instance->paramExists(i_result_i_en_prev)
							  ? instance->param(i_result_i_en_prev)->wordValue()
							  : 0;

		const qint16 prev_result = result_i_en_prev & 0x0001;
		const qint16 prev_enable = (result_i_en_prev >> 1) & 0x0001;

		const qint16 start_from = instance->param(i_start_from)->wordValue();
		const qint16 enable = instance->param(i_enable)->wordValue();

		checkParamRange(conf, 1, 2, QStringLiteral("i_conf"));

		// Logic -- translated VHDL code
		//
		quint16 result_param_err = 0;
		quint16 result_result = 0;

		do
		{
			if (t_high < m_cycleDurationMs || t_low < m_cycleDurationMs)
			{
				result_param_err = 1;
				result_result = 0;
				t_prev = 0;
				break;
			}

			if (enable == 0)
			{
				result_result = 0;
				t_prev = 0;
				break;
			}

			// Detect front for enable 0 -> 1 and restart pulse gen is this case
			//
			if (prev_enable == 0 && enable == 1)
			{
				if (start_from == 1)
				{
					result_result = 1;
					t_prev = t_high / m_cycleDurationMs;
				}
				else
				{
					result_result = 0;
					t_prev = t_low / m_cycleDurationMs;
				}

				break;
			}

			// Decreasing counter
			//
			t_prev --;
			result_result = prev_result;

			// Check if it's time to swap out and restart timer
			//
			if (t_prev <= 0)
			{
				if (prev_result == 0)	// swap out
				{
					result_result = 1;
					t_prev = t_high / m_cycleDurationMs;
				}
				else
				{
					result_result = 0;
					t_prev = t_low / m_cycleDurationMs;
				}

				break;
			}

		}
		while (false);

		instance->addParamWord(o_param_err, result_param_err);
		instance->addParamSignedInt(o_t_prev, t_prev);
		instance->addParamWord(o_result_en_prev, result_result);
		instance->addParamWord(i_result_i_en_prev, ((enable & 1) << 1) | (result_result & 1 ));

		return;
	}

}
