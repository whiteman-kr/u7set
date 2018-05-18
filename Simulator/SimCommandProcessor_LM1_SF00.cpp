#include <array>
#include "SimCommandProcessor_LM1_SF00.h"
#include "SimException.h"
#include "SimAfb.h"

namespace Sim
{
	CommandProcessor_LM1_SF00::CommandProcessor_LM1_SF00(DeviceEmulator* device) :
		CommandProcessor(device)
	{
	}

	CommandProcessor_LM1_SF00::~CommandProcessor_LM1_SF00()
	{
	}

	bool CommandProcessor_LM1_SF00::runCommand(const DeviceCommand& command)
	{
		auto it = m_nameToFuncCommand.find(command.m_command.simulationFunc);
		if (Q_UNLIKELY(it == m_nameToFuncCommand.end()))
		{
			SimException::raise(QString("Cannot find command %1").arg(command.m_command.simulationFunc), "CommandProcessor_LM1_SF00::runCommand");
		}

		auto& func = *it;

		// Call the command
		//
		(this->*func)(command);

		return true;
	}

	void CommandProcessor_LM1_SF00::command_not_implemented(const DeviceCommand& command)
	{
		SimException::raise(QString("Command %1 is not implemented yet").arg(command.caption()), __FUNCTION__);
		return;
	}

	// Command: startafb
	// Code: 2
	// Description: Execute AFB
	//
	void CommandProcessor_LM1_SF00::parse_startafb(DeviceCommand* command) const
	{
		command->m_size = 2;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits

		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance);

		if (afb.simulationFunc().isEmpty() == true)
		{
			SimException::raise(QString("Simultaion function for AFB %1 is not found").arg(afb.caption()));
		}

		// startafb   LOGIC.0
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInst(command);

		return;
	}

	void CommandProcessor_LM1_SF00::command_startafb(const DeviceCommand& command)
	{
		AfbComponent afb = m_device.afbComponent(command.m_afbOpCode);
		if (afb.isNull() ==  true)
		{
			SimException::raise(QString("Cannot find AfbComponent with OpCode ")
									.arg(command.m_afbOpCode),
								"CommandProcessor_LM1_SF00::command_startafb");
		}

		AfbComponentInstance* afbInstance = m_device.afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		if (afbInstance == nullptr)
		{
			SimException::raise(QString("Cannot find afbInstance with OpCode %1, InstanceNo %2")
									.arg(command.m_afbOpCode)
									.arg(command.m_afbInstance),
								"CommandProcessor_LM1_SF00::command_startafb");
		}

		QString afbSimFunc = afb.simulationFunc();
		assert(afbSimFunc.isEmpty() == false);

		// AFB
		//
		auto it = m_nameToFuncAfb.find(afbSimFunc);
		if (Q_UNLIKELY(it == m_nameToFuncAfb.end()))
		{
			SimException::raise(QString("Cannot find AFB func %1").arg(afbSimFunc), "CommandProcessor_LM1_SF00::command_startafb");
		}

		auto& func = *it;

		// Call the command
		//
		(this->*func)(afbInstance);

		return;
	}

	// Command: stop
	// Code: 3
	// Description: Stop IDR phase and start ALP, if ALP is current phase then stop work cycle
	//
	void CommandProcessor_LM1_SF00::parse_stop(DeviceCommand* command) const
	{
		command->m_size = 1;
		command->m_string = command->m_command.caption;
		return;
	}

	void CommandProcessor_LM1_SF00::command_stop([[maybe_unused]] const DeviceCommand& command)
	{
		if (m_device.phase() == Sim::CyclePhase::IdrPhase)
		{
			m_device.setPhase(Sim::CyclePhase::AlpPhase);
			m_device.setProgramCounter(m_device.appStartAddress());
			return;
		}

		if (m_device.phase() == Sim::CyclePhase::AlpPhase)
		{
			m_device.setPhase(Sim::CyclePhase::ODT);
			return;
		}

		SimException::raise(QString("Command stop is cannot be run in current phase: %1")
								.arg(static_cast<int>(m_device.phase())));
		return;

	}

	// Command: mov
	// Code: 4
	// Description: Move word from RAM to RAM
	//
	void CommandProcessor_LM1_SF00::parse_mov(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 - adderess2 - destionation
		command->m_word1 = m_device.getWord(command->m_offset + 2);		// word1 - adderess1 - source

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1);

		return;
	}

	void CommandProcessor_LM1_SF00::command_mov(const DeviceCommand& command)
	{
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		quint16 data = m_device.readRamWord(src);
		m_device.writeRamWord(dst, data);

		return;
	}

	// Command: movmem
	// Code: 5
	// Description: Move N words from RAM to RAM
	//
	void CommandProcessor_LM1_SF00::parse_movmem(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 - adderess2
		command->m_word1 = m_device.getWord(command->m_offset + 2);		// word1 - adderess1
		command->m_word2 = m_device.getWord(command->m_offset + 3);		// word2 - words to move

		// --
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strAddr(command->m_word1) + ", " +
							strAddr(command->m_word2);

		return;
	}

	void CommandProcessor_LM1_SF00::command_movmem(const DeviceCommand& command)
	{
		const auto& size = command.m_word2;
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		m_device.movRamMem(src, dst, size);

		return;
	}

	// Command: movc
	// Code: 6
	// Description: Write word const to RAM
	//
	void CommandProcessor_LM1_SF00::parse_movc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 - address
		command->m_word1 = m_device.getWord(command->m_offset + 2);		// word1 - data

		// movc     0b402h, #0
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strWordConst(command->m_word1);

		return;
	}

	void CommandProcessor_LM1_SF00::command_movc(const DeviceCommand& command)
	{
		m_device.writeRamWord(command.m_word0, command.m_word1);
		return;
	}

	// Command: movbc
	// Code: 7
	// Description: Write constant bit to RAM
	//
	void CommandProcessor_LM1_SF00::parse_movbc(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 - data address
		command->m_word1 = m_device.getWord(command->m_offset + 2);		// word1 - data
		command->m_bitNo0 = m_device.getWord(command->m_offset + 3);	// bitNo0 - bitno

		checkParamRange(command->m_bitNo0, 0, 15, "BitNo");

		// MOVBC    0B402h[0], #0
		//
		command->m_string = strCommand(command->m_command.caption) +
							strBitAddr(command->m_word0, command->m_bitNo0) +
							", " +
							strBitConst(command->m_word1);

		return;
	}

	void CommandProcessor_LM1_SF00::command_movbc(const DeviceCommand& command)
	{
		m_device.writeRamBit(command.m_word0, command.m_bitNo0, command.m_word1);
		return;
	}

	// Command: wrfb
	// Code: 8
	// Description: Read 16bit data from RAM and write to AFB input
	//
	void CommandProcessor_LM1_SF00::parse_wrfb(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device.getWord(command->m_offset + 2);						// Word0 - data address

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance);

		// String representation
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) + ", " +
							strAddr(command->m_word0);
		return;
	}

	void CommandProcessor_LM1_SF00::command_wrfb(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setWordValue(m_device.readRamWord(command.m_word0));

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);
		return;
	}


	// Command: wrfbc
	// Code: 10
	// Description: Write constant word to AFB input
	//
	void CommandProcessor_LM1_SF00::parse_wrfbc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;	// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;		// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = m_device.getWord(command->m_offset + 2);					// word0 - data address

		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		// wrfbc LOGIC.0[i_oprd_15], #0003h
		//
		command->m_string = strCommand(command->m_command.caption) +
							strAfbInstPin(command) +
							", " +
							strWordConst(command->m_word0);

		return;
	}

	void CommandProcessor_LM1_SF00::command_wrfbc(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setWordValue(command.m_word0);

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);
		return;
	}

	// Command: wrfbb
	// Code: 11
	// Description: Read bit from RAM and write it to AFB
	//
	void CommandProcessor_LM1_SF00::parse_wrfbb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device.getWord(command->m_offset + 2);						// Word0 - data address
		command->m_bitNo0 = m_device.getWord(command->m_offset + 3);					// BitNo

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);
		checkParamRange(command->m_bitNo0, 0, 15, "BitNo");

		// String representation
		// wrfbb LOGIC.0[i_input], 46083h[0]
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) +
							", " +
							strBitAddr(command->m_word0, command->m_bitNo0);

		return;
	}

	void CommandProcessor_LM1_SF00::command_wrfbb(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setWordValue(m_device.readRamBit(command.m_word0, command.m_bitNo0));

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);

		return;
	}

	// Command: rdfbb
	// Code: 12
	// Description: Read bit from AFB and write it to RAM
	//
	void CommandProcessor_LM1_SF00::parse_rdfbb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device.getWord(command->m_offset + 2);						// Word0 - data address
		command->m_bitNo0 = m_device.getWord(command->m_offset + 3);					// BitNo

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);
		checkParamRange(command->m_bitNo0, 0, 15, "BitNo");

		// String representation
		// rdfbb 46083h[0], LOGIC.0[o_result]
		//
		command->m_string = strCommand(command->caption()) +
							strBitAddr(command->m_word0, command->m_bitNo0) +
							", " +
							strAfbInstPin(command);

		return;
	}

	void CommandProcessor_LM1_SF00::command_rdfbb(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device.afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device.writeRamBit(command.m_word0, command.m_bitNo0, param->wordValue() & 0x01);

		return;
	}

	// Command: movb
	// Code: 15
	// Description: Move bit from RAM to RAM
	//
	void CommandProcessor_LM1_SF00::parse_movb(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_word0 = m_device.getWord(command->m_offset + 2);						// source address (ADR1)
		command->m_bitNo0 = m_device.getWord(command->m_offset + 3) & 0b1111;			//

		command->m_word1 = m_device.getWord(command->m_offset + 1);						// destionation address	(ADR2)
		command->m_bitNo1 = (m_device.getWord(command->m_offset + 3) >> 8) & 0b1111;	//

		// String representation
		//
		command->m_string =	strCommand(command->caption()) +
							strBitAddr(command->m_word0, command->m_bitNo0) + ", " +
							strBitAddr(command->m_word1, command->m_bitNo1);

		return;
	}

	void CommandProcessor_LM1_SF00::command_movb(const DeviceCommand& command)
	{
		quint16 data = m_device.readRamBit(command.m_word0, command.m_bitNo0);
		m_device.writeRamBit(command.m_word1, command.m_bitNo1, data);

		return;
	}

	// Command: appstart
	// Code: 17
	// Description: Save ALP phase start address
	//
	void CommandProcessor_LM1_SF00::parse_appstart(DeviceCommand* command) const
	{
		command->m_size = 2;
		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 keeps ALP phase start address

		// appstart  #000Ch
		//
		command->m_string = strCommand(command->m_command.caption) +
							strWordConst(command->m_word0);

		return;
	}

	void CommandProcessor_LM1_SF00::command_appstart(const DeviceCommand& command)
	{
		m_device.setAppStartAddress(command.m_word0);
		return;
	}

	// Command: rdfb32
	// Code: 21
	// Description: Read 32bit data from AFB output and write it to RAM
	//
	void CommandProcessor_LM1_SF00::parse_rdfb32(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_word0 = m_device.getWord(command->m_offset + 2);						// Word0 - data address

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		// rdfb32 0478h, LOGIC.0[i_2_oprd]
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) +
							", " +
							strAfbInstPin(command);

		return;
	}

	void CommandProcessor_LM1_SF00::command_rdfb32(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device.afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device.writeRamDword(command.m_word0, param->dwordValue());

		return;
	}

	// Command: wrfbc32
	// Code: 22
	// Description: Write 32bit constant to FunctionalBlock input
	//
	void CommandProcessor_LM1_SF00::parse_wrfbc32(DeviceCommand* command) const
	{
		command->m_size = 4;

		command->m_afbOpCode = m_device.getWord(command->m_offset + 0) & 0x003F;		// Lowest 6 bit
		command->m_afbInstance = m_device.getWord(command->m_offset + 1) >> 6;			// Highest 10 bits
		command->m_afbPinOpCode = m_device.getWord(command->m_offset + 1) & 0x003F;		// Lowest 6 bit

		command->m_dword0 = m_device.getDword(command->m_offset + 2);					// Dword0 - data

		// Checks
		//
		AfbComponent afb = checkAfb(command->m_afbOpCode, command->m_afbInstance, command->m_afbPinOpCode);

		// String representation
		// wrfbc32 MATH.0[i_oprd_1], #0423445h
		//
		command->m_string = strCommand(command->caption()) +
							strAfbInstPin(command) +
							", " +
							strDwordConst(command->m_dword0);

		return;
	}

	void CommandProcessor_LM1_SF00::command_wrfbc32(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setDwordValue(command.m_dword0);

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);

		return;
	}

	//
	// AFB's simultaion code
	//

	//	LOGIC, OpCode 1
	//
	void CommandProcessor_LM1_SF00::afb_logic(AfbComponentInstance* instance)
	{
		if (instance == nullptr)
		{
			assert(instance);
			return;
		}

		// Define input opIndexes
		//
		const int i_oprd_quant = 0;
		const int i_bus_width = 1;
		const int i_conf = 2;
		const int i_input_0 = 3;
		const int o_result = 20;

		// Get params, throws exception in case of error
		//
		AfbComponentParam* oprdQuant = instance->param(i_oprd_quant);
		AfbComponentParam* busWidth = instance->param(i_bus_width);
		AfbComponentParam* conf = instance->param(i_conf);

		checkParamRange(oprdQuant->wordValue(), 1, 16, "i_oprd_quant");
		checkParamRange(busWidth->wordValue(), 1, 16, "i_bus_width");

		// Logic
		//
		std::array<AfbComponentParam, 16> inputs;

		for (quint16 i = 0; i < oprdQuant->wordValue(); i++)
		{
			inputs[i] = *instance->param(i_input_0 + i);
		}

		quint16 result = inputs[0].wordValue();

		switch (conf->wordValue())
		{
			case 1:	// AND
				for (size_t i = 1; i < oprdQuant->wordValue(); i++)
				{
					result &= inputs[i].wordValue();
				}
				break;
			case 2:	// OR
				for (size_t i = 1; i < oprdQuant->wordValue(); i++)
				{
					result |= inputs[i].wordValue();
				}
				break;
			case 3:	// XOR
				for (size_t i = 1; i < oprdQuant->wordValue(); i++)
				{
					result ^= inputs[i].wordValue();
				}
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: 51").arg(conf->wordValue()), "CommandProcessor_LM1_SF00::afb_logic");
		}

		// Save result
		//
		instance->addParamWord(o_result, result);
		return;
	}

	//	NOT, OpCode 2
	//
	void CommandProcessor_LM1_SF00::afb_not(AfbComponentInstance* instance)
	{
		// Define input opIndexes
		//
		const int i_oprd = 0;
		const int o_result = 2;

		// Logic
		//
		AfbComponentParam* input = instance->param(i_oprd);
		quint16 result = ~input->wordValue();

		// Save result
		//
		instance->addParamWord(o_result, result);
		return;
	}

	//	MATH, OpCode 13
	//
	void CommandProcessor_LM1_SF00::afb_math(AfbComponentInstance* instance)
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
		AfbComponentParam* conf = instance->param(i_conf);
		AfbComponentParam* operand1 = instance->param(i_1_oprd);
		AfbComponentParam* operand2 = instance->param(i_2_oprd);

		// Logic	conf: 1'-'+' (SI),  '2'-'-' (SI),  '3'-'*' (SI),  '4'-'/' (SI), '5'-'+' (FP),  '6'-'-' (FP),  '7'-'*' (FP),  '8'-'/' (FP)
		//
		switch (conf->wordValue())
		{
			case 1:
				operand1->addSignedInteger(operand2);
				break;
			case 2:
				operand1->subSignedInteger(operand2);
				break;
			case 3:
				operand1->mulSignedInteger(operand2);
				break;
			case 4:
				operand1->divSignedInteger(operand2);
				break;
			case 5:
				operand1->addFloatingPoint(operand2);
				break;
			case 6:
				operand1->subFloatingPoint(operand2);
				break;
			case 7:
				operand1->mulFloatingPoint(operand2);
				break;
			case 8:
				operand1->divFloatingPoint(operand2);
				break;
			default:
				SimException::raise(QString("Unknown AFB configuration: %1, or this configuration is not implemented yet.")
										.arg(conf->wordValue()),
									"CommandProcessor_LM1_SF00::afb_math");
		}

		// Save result
		//
		AfbComponentParam result = *operand1;
		result.setOpIndex(o_result);

		instance->addParam(result);
		instance->addParamWord(o_overflow, operand1->mathOverflow());
		instance->addParamWord(o_underflow, operand1->mathUnderflow());
		instance->addParamWord(o_zero, operand1->mathZero());
		instance->addParamWord(o_nan, operand1->mathNan());
		instance->addParamWord(o_div_by_zero, operand1->mathDivByZero());

		return;
	}

	void CommandProcessor_LM1_SF00::afb_scale(AfbComponentInstance* instance)
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
		AfbComponentParam* conf = instance->param(i_conf);
		AfbComponentParam* k1 = instance->param(i_scal_k1_coef);	// for  1, 2, 3, 4 -- k1/k2 SignedInteger
		AfbComponentParam* k2 = instance->param(i_scal_k2_coef);	//      5, 6, 7, 8, 9 -- k1/k2 float
		AfbComponentParam result;

		// Scale, conf:  1-16(UI)/16(UI); 2-16(UI)/32(SI); 3-32(SI)/16(UI); 4-32(SI)/32(SI); 5-32(SI)/32(FP); 6-32(FP)/32(FP); 7-32(FP)/16(UI); 8-32(FP)/32(SI); 9-16(UI)/32(FP);
		//
		switch (conf->wordValue())
		{
			case 1: // 16(UI)/16(UI)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			 case 2: // 16(UI)/32(SI)
				{
					result = *instance->param(i_ui_data);
					result.setOpIndex(o_si_fp_result);

					result.convertWordToSignedInt();

					result.mulSignedInteger(k1);
					result.divSignedIntegerNumber(32768);
					result.addSignedInteger(k2);

					// Save result
					//
					instance->addParam(result);
				}
				break;
			case 3: // 32(SI)/16(UI)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			case 4: // 32(SI)/32(SI)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			case 5: // 32(SI)/32(FP)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			case 6: // 32(FP)/32(FP)
				{
					result = *instance->param(i_si_fp_data);
					result.setOpIndex(o_si_fp_result);

					result.mulFloatingPoint(k1);
					result.addFloatingPoint(k2);

					// Save result
					//
					instance->addParam(result);
				}
				break;
			case 7: // 32(FP)/16(UI)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			case 8: // 32(FP)/32(SI)
				SimException::raise("Scale configuration: " + QString::number(conf->wordValue()) + " is not implemented yet.");
				break;
			case 9: // 16(UI)/32(FP)
				{
					result = *instance->param(i_ui_data);
					result.setOpIndex(o_si_fp_result);

					result.convertWordToFloat();

					result.mulFloatingPoint(k1);
					result.addFloatingPoint(k2);

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


}
