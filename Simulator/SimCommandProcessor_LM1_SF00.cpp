#include <array>
#include "SimCommandProcessor_LM1_SF00.h"
#include "SimException.h"
#include "SimAfb.h"

constexpr unsigned int funcNameToInt(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (funcNameToInt(str, h + 1) * 33) ^ str[h];
}

unsigned int funcNameToInt(const QString& str)
{
	return funcNameToInt(str.toStdString().data());
}

namespace Sim
{
	const QHash<QString, CommandProcessor_LM1_SF00::SimCommandFunc> CommandProcessor_LM1_SF00::m_nameToFuncCommand
	{
		{"command_startafb",	[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_startafb(cmd);	}},
		{"command_stop",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_stop(cmd);		}},
		{"command_movmem",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_movmem(cmd);	}},
		{"command_movc",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_movc(cmd);		}},
		{"command_movbc",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_movbc(cmd);		}},
		{"command_wrfbc",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_wrfbc(cmd);		}},
		{"command_wrfbb",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_wrfbb(cmd);		}},
		{"command_rdfbb",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_rdfbb(cmd);		}},
		{"command_appstart",	[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_appstart(cmd);	}},
		{"command_rdfb32",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_rdfb32(cmd);	}},
		{"command_wrfbc32",		[](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->command_wrfbc32(cmd);	}},
		//{"", [](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->(cmd);	}},
		//{"", [](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->(cmd);	}},
		//{"", [](CommandProcessor_LM1_SF00* obj, const DeviceCommand& cmd)	{	return obj->(cmd);	}},
	};


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
		if (it == m_nameToFuncCommand.end())
		{
			SimException::raise(QString("Cannot find command %1").arg(command.m_command.simulationFunc), "CommandProcessor_LM1_SF00::runCommand");
		}

		auto& func = *it;
		bool result = func(this, command);

		return result;
	}

	// Command: startafb
	// Code: 2
	// Description: Execute AFB
	//
	bool CommandProcessor_LM1_SF00::parse_startafb(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_startafb(const DeviceCommand& command)
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
		if (afbSimFunc.isEmpty() == true)
		{
			assert(afbSimFunc.isEmpty() == false);
			return false;
		}

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4307)		// warning: C4307: '*': integral constant overflow in funcNameToInt()
#endif
		switch (funcNameToInt(afbSimFunc))
		{
		case funcNameToInt("afb_logic"):
			afb_logic(afbInstance);
			break;

		case funcNameToInt("afb_not"):
			afb_not(afbInstance);
			break;

		case funcNameToInt("afb_math"):
			afb_math(afbInstance);
			break;

		default:
			SimException::raise(QString("Cannot call %1 function.").arg(afbSimFunc), "CommandProcessor_LM1_SF00::command_startafb");
			return false;
		}
#ifdef _MSC_VER
	#pragma warning(pop)
#endif

//		bool ok = QMetaObject::invokeMethod(this,
//											afbSimFunc.toStdString().data(),
//											Qt::DirectConnection,
//											Q_ARG(AfbComponentInstance*, afbInstance));
//		if (ok == false)
//		{
//			SimException::raise(QString("Cannot call %1 function.").arg(afbSimFunc), "CommandProcessor_LM1_SF00::command_startafb");
//			return false;
//		}

		return true;
	}

	// Command: stop
	// Code: 3
	// Description: Stop IDR phase and start ALP, if ALP is current phase then stop work cycle
	//
	bool CommandProcessor_LM1_SF00::parse_stop(DeviceCommand* command) const
	{
		command->m_size = 1;
		command->m_string = command->m_command.caption;
		return true;
	}

	bool CommandProcessor_LM1_SF00::command_stop([[maybe_unused]] const DeviceCommand& command)
	{
		if (m_device.phase() == Sim::CyclePhase::IdrPhase)
		{
			m_device.setPhase(Sim::CyclePhase::AlpPhase);
			m_device.setProgramCounter(m_device.appStartAddress());
			return true;
		}

		if (m_device.phase() == Sim::CyclePhase::AlpPhase)
		{
			m_device.setPhase(Sim::CyclePhase::ODT);
			return true;
		}

		SimException::raise(QString("Command stop is cannot be run in current phase: %1")
								.arg(static_cast<int>(m_device.phase())));
		return false;

	}

	// Command: movmem
	// Code: 5
	// Description: Move N words from RAM to RAM
	//
	bool CommandProcessor_LM1_SF00::parse_movmem(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_movmem(const DeviceCommand& command)
	{
		const auto& size = command.m_word2;
		const auto& src = command.m_word1;
		const auto& dst = command.m_word0;

		m_device.movRamMem(src, dst, size);

		return true;
	}

	// Command: movc
	// Code: 6
	// Description: Write word const to RAM
	//
	bool CommandProcessor_LM1_SF00::parse_movc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 - address
		command->m_word1 = m_device.getWord(command->m_offset + 2);		// word1 - data

		// movc     0b402h, #0
		//
		command->m_string = strCommand(command->caption()) +
							strAddr(command->m_word0) + ", " +
							strWordConst(command->m_word1);

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_movc(const DeviceCommand& command)
	{
		m_device.writeRamWord(command.m_word0, command.m_word1);
		return true;
	}

	// Command: movbc
	// Code: 7
	// Description: Write constant bit to RAM
	//
	bool CommandProcessor_LM1_SF00::parse_movbc(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_movbc(const DeviceCommand& command)
	{
		m_device.writeRamBit(command.m_word0, command.m_bitNo0, command.m_word1);
		return true;
	}

	// Command: wrfbc
	// Code: 10
	// Description: Write constant word to AFB input
	//
	bool CommandProcessor_LM1_SF00::parse_wrfbc(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_wrfbc(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setWordValue(command.m_word0);

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);
		return true;
	}

	// Command: wrfbb
	// Code: 11
	// Description: Read bit from RAM and write it to AFB
	//
	bool CommandProcessor_LM1_SF00::parse_wrfbb(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_wrfbb(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setWordValue(m_device.readRamBit(command.m_word0, command.m_bitNo0));

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);

		return true;
	}

	// Command: rdfbb
	// Code: 12
	// Description: Read bit from AFB and write it to RAM
	//
	bool CommandProcessor_LM1_SF00::parse_rdfbb(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_rdfbb(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device.afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device.writeRamBit(command.m_word0, command.m_bitNo0, param->wordValue() & 0x01);

		return true;
	}

	// Command: appstart
	// Code: 17
	// Description: Save ALP phase start address
	//
	bool CommandProcessor_LM1_SF00::parse_appstart(DeviceCommand* command) const
	{
		command->m_size = 2;
		command->m_word0 = m_device.getWord(command->m_offset + 1);		// word0 keeps ALP phase start address

		// appstart  #000Ch
		//
		command->m_string = strCommand(command->m_command.caption) +
							strWordConst(command->m_word0);

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_appstart(const DeviceCommand& command)
	{
		m_device.setAppStartAddress(command.m_word0);
		return true;
	}

	// Command: rdfb32
	// Code: 21
	// Description: Read 32bit data from AFB output and write it to RAM
	//
	bool CommandProcessor_LM1_SF00::parse_rdfb32(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_rdfb32(const DeviceCommand& command)
	{
		AfbComponentInstance* afbInstance = m_device.afbComponentInstance(command.m_afbOpCode, command.m_afbInstance);
		AfbComponentParam* param = afbInstance->param(command.m_afbPinOpCode);

		m_device.writeRamDword(command.m_word0, param->dwordValue());

		return true;
	}

	// Command: wrfbc32
	// Code: 22
	// Description: Write 32bit constant to FunctionalBlock input
	//
	bool CommandProcessor_LM1_SF00::parse_wrfbc32(DeviceCommand* command) const
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

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_wrfbc32(const DeviceCommand& command)
	{
		AfbComponentParam param{command.m_afbPinOpCode};
		param.setDwordValue(command.m_dword0);

		m_device.setAfbParam(command.m_afbOpCode, command.m_afbInstance, param);

		return true;
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


}
