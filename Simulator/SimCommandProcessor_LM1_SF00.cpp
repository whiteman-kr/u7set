#include "SimCommandProcessor_LM1_SF00.h"
#include "SimException.h"

namespace Sim
{

	CommandProcessor_LM1_SF00::CommandProcessor_LM1_SF00(DeviceEmulator* device) :
		CommandProcessor(device)
	{
	}

	CommandProcessor_LM1_SF00::~CommandProcessor_LM1_SF00()
	{
	}

	QString CommandProcessor_LM1_SF00::logicModuleName() const
	{
		return m_logicModuleName;
	}

	// Command: wrfbc
	// Code: 10
	// Description: Write constant word to AFB input
	//
	bool CommandProcessor_LM1_SF00::parse_wrfbc(DeviceCommand* command) const
	{
		command->m_size = 3;

		command->m_afbOpCode = device().getWord(command->m_offset + 0) & 0x003F;	// Lowest 6 bit
		command->m_afbInstance = device().getWord(command->m_offset + 1) >> 6;		// Highest 10 bits
		command->m_afbPinOpCode = device().getWord(command->m_offset + 1) & 0x003F;	// Lowest 6 bit

		command->m_word0 = device().getWord(command->m_offset + 2);					// word0 - data address

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
		SimException::raise("Not implemented", "CommandProcessor_LM_SF00::command_appstart");
		return false;
//		local param = AfbComponentParam(command.afbPinOpCode);
//		param.asWord = command.word0;

//		device:setAfbParam(command.afbOpCode, command.afbInstance, param);
	}

	// Command: appstart
	// Code: 17
	// Description: Save ALP phase start address
	//
	bool CommandProcessor_LM1_SF00::parse_appstart(DeviceCommand* command) const
	{
		command->m_size = 2;
		command->m_word0 = device().getWord(command->m_offset + 1);		// word0 keeps ALP phase start address

		// appstart  #000Ch
		//
		command->m_string = strCommand(command->m_command.caption) +
							strWordConst(command->m_word0);

		return true;
	}

	bool CommandProcessor_LM1_SF00::command_appstart(const DeviceCommand& command)
	{
		SimException::raise("Not implemented", "CommandProcessor_LM_SF00::command_appstart");
		return false;
	}


}
