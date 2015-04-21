#include "ApplicationLogicCode.h"

namespace Builder
{
	void CommandCode::setOpCode(CommandCodes code)
	{
		if (code >= CommandCodes::Count)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			opCode.code = static_cast<quint16>(code);
		}
	}


	void CommandCode::setFbType(quint16 fbType)
	{
		if (fbType > MAX_FB_TYPE)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			opCode.fbType = fbType;
		}
	}


	void CommandCode::setFbInstance(quint16 fbInstance)
	{
		if (fbInstance > MAX_FB_INSTANCE)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			param.fbInstance = fbInstance;
		}
	}


	void CommandCode::setFbParamNo(quint16 fbParamNo)
	{
		if (fbParamNo > MAX_FB_PARAM_NO)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			param.fbParamNo = fbParamNo;
		}
	}


	void CommandCode::setBitNo(quint16 bitNo)
	{
		if (bitNo > MAX_BIT_NO_16)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			word4 = bitNo;
		}
	}


	int CommandCode::getSizeW()
	{
		int cmdCode = static_cast<int>(opCode.code);

		if (cmdCode > COMMAND_LEN_COUNT)
		{
			assert(false);
			return 0;
		}

		return CommandLen[cmdCode];
	}


	void Command::nop()
	{
		m_code.setOpCode(CommandCodes::NOP);
	}


	void Command::start()
	{
		m_code.setOpCode(CommandCodes::START);

	}

	void Command::stop()
	{
		m_code.setOpCode(CommandCodes::STOP);
	}


	void Command::mov(quint16 addrFrom, quint16 addrTo)
	{
		m_code.setOpCode(CommandCodes::MOV);
		m_code.setWord2(addrFrom);
		m_code.setWord3(addrTo);
	}


	void Command::movMem(quint16 addrFrom, quint16 addrTo, quint16 sizeW)
	{
		m_code.setOpCode(CommandCodes::MOVMEM);
		m_code.setWord2(addrFrom);
		m_code.setWord3(addrTo);
		m_code.setWord4(sizeW);
	}


	void Command::movConst(quint16 constVal, quint16 addrTo)
	{
		m_code.setOpCode(CommandCodes::MOVC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constVal);
	}


	void Command::movBitConst(quint16 constBit, quint16 addrTo, quint16 bitNo)
	{
		m_code.setOpCode(CommandCodes::MOVBC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constBit);
		m_code.setBitNo(bitNo);
	}


	void Command::writeFuncBlock(quint16 addrFrom, quint16 fbType, quint16 fbInstance, quint16 fbParamNo)
	{
		m_code.setOpCode(CommandCodes::WRFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(addrFrom);
	}


	void Command::readFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrTo)
	{
		m_code.setOpCode(CommandCodes::RDFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(addrTo);
	}


	void Command::writeFuncBlockConst(quint16 constVal, quint16 fbType, quint16 fbInstance, quint16 fbParamNo)
	{
		m_code.setOpCode(CommandCodes::WRFBC);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(constVal);
	}


	void Command::writeFuncBlockBit(quint16 addrFrom, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo)
	{
		m_code.setOpCode(CommandCodes::WRFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(addrFrom);
		m_code.setBitNo(bitNo);
	}


	void Command::readFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrTo, quint16 bitNo)
	{
		m_code.setOpCode(CommandCodes::RDFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(addrTo);
		m_code.setBitNo(bitNo);
	}



	ApplicationLogicCode::ApplicationLogicCode()
	{
	}


	void ApplicationLogicCode::clear()
	{
		m_commands.clear();
		m_commandAddress = 0;
	}


	void ApplicationLogicCode::append(Command& cmd)
	{
		cmd.setAddress(m_commandAddress);

		m_commandAddress += cmd.getSizeW();

		m_commands.append(cmd);
	}
}
