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


	quint16 CommandCode::getWord(int index)
	{
		switch(index)
		{
		case 0:
			return word1;

		case 1:
			return word2;

		case 2:
			return word3;

		case 3:
			return word4;

		default:
			assert(false);
		}

		return 0;
	}


	int CommandCode::getSizeW()
	{
		int cmdCode = static_cast<int>(opCode.code);

		if (cmdCode > COMMAND_COUNT)
		{
			assert(false);
			return 0;
		}

		return CommandLen[cmdCode];
	}


	QString CommandCode::getFbTypeStr()
	{
		int fbType = getFbType();

		if (fbType < FB_TYPE_STR_COUNT)
		{
			return FbTypeStr[fbType];
		}

		assert(false);			// need add string in FbTypeStr array

		return QString("FB%1").arg(fbType);
	}


	CodeItem::~CodeItem()
	{
	}


	QString Comment::toString()
	{
		QString comment = getComment();

		if (comment.isEmpty())
		{
			return "";
		}

		return QString("\t-- %1").arg(getComment());
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

	void Command::readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue)
	{
		m_code.setOpCode(CommandCodes::RDFBTS);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord2(testValue);
	}


	void Command::generateRawCode()
	{
		m_rawCode.clear();

		int cmdSizeW = getSizeW();

		m_rawCode.resize(cmdSizeW * 2);

		for(int i = 0; i < cmdSizeW; i++)
		{
			quint16 cmdWord = m_code.getWord(i);

			m_rawCode[i * 2] = cmdWord & 0x00FF;
			m_rawCode[i * 2 + 1] = (cmdWord & 0xFF00) >> 8;

/*			if (byteOrder == ByteOrder::LittleEdndian)
			{
				m_rawCode[i * 2] = cmdWord & 0x00FF;
				m_rawCode[i * 2 + 1] = (cmdWord & 0xFF00) >> 8;
			}
			else
			{
				m_rawCode[i * 2] = (cmdWord & 0xFF00) >> 8;
				m_rawCode[i * 2 + 1] = cmdWord & 0x00FF;;
			}*/
		}
	}


	QString Command::getCodeWordStr(int wordNo)
	{
		QString str;

		if (m_rawCode.count() < (wordNo + 1) * 2)
		{
			assert(false);
			return str;
		}

		unsigned int word = m_rawCode[wordNo * 2 + 1] & 0x00FF;

		word <<= 8;

		word |= m_rawCode[wordNo * 2] & 0x00FF;

		word &= 0xFFFF;

		str.sprintf("%04X", word);

		return str;
	}


	QString Command::getMnemoCode()
	{
		QString mnemoCode;

		CommandCodes opCode = m_code.getOpCode();

		switch(opCode)
		{
		case NoCommand:
		case NOP:
		case START:
		case STOP:
			mnemoCode = CommandStr[opCode];
			break;

		case MOV:
			mnemoCode.sprintf("%s\t0x%04X, 0x%04X", CommandStr[opCode], m_code.getWord2(), m_code.getWord3());
			break;

		case MOVMEM:
			mnemoCode.sprintf("%s\t0x%04X, 0x%04X, %d", CommandStr[opCode], m_code.getWord2(), m_code.getWord3(), m_code.getWord4());
			break;

		case MOVC:
			mnemoCode.sprintf("%s\t#0x%04X, 0x%04X", CommandStr[opCode], m_code.getWord3(), m_code.getWord2());
			break;

		case MOVBC:
			mnemoCode.sprintf("%s\t#%d, 0x%04X[%d]", CommandStr[opCode], m_code.getWord3(), m_code.getWord2(), m_code.getWord4());
			break;

		case WRFB:
			mnemoCode.sprintf("%s\t0x%04X, %s.%d[%d]", CommandStr[opCode], m_code.getWord2(),
							  m_code.getFbTypeStr().toUtf8().data(), m_code.getFbInstanceInt(), m_code.getFbParamNoInt());
			break;

		case RDFB:
			mnemoCode.sprintf("%s\t%s.%d[%d], 0x%04X", CommandStr[opCode], m_code.getFbTypeStr().toUtf8().data(),
							  m_code.getFbInstanceInt(), m_code.getFbParamNoInt(), m_code.getWord2());
			break;

		case WRFBC:
			mnemoCode.sprintf("%s\t#0x%04X, %s.%d[%d]", CommandStr[opCode], m_code.getWord2(),
							  m_code.getFbTypeStr().toUtf8().data(), m_code.getFbInstanceInt(), m_code.getFbParamNoInt());
			break;

		case WRFBB:
			mnemoCode.sprintf("%s\t0x%04X[%d], %s.%d[%d]", CommandStr[opCode], m_code.getWord2(), m_code.getWord4(),
							  m_code.getFbTypeStr().toUtf8().data(), m_code.getFbInstanceInt(), m_code.getFbParamNoInt());
			break;

		case RDFBB:
			mnemoCode.sprintf("%s\t%s.%d[%d], 0x%04X[%d]", CommandStr[opCode], m_code.getFbTypeStr().toUtf8().data(),
							  m_code.getFbInstanceInt(), m_code.getFbParamNoInt(), m_code.getWord2(), m_code.getWord4());
			break;

		case RDFBTS:
			mnemoCode.sprintf("%s\t%s.%d[%d], 0x%04X", CommandStr[opCode], m_code.getFbTypeStr().toUtf8().data(),
							  m_code.getFbInstanceInt(), m_code.getFbParamNoInt(), m_code.getWord2());
		default:
			assert(false);
		}

		return mnemoCode;
	}

	QString Command::toString()
	{
		QString cmdStr;

		cmdStr.sprintf("%04X\t", m_address);

		for(int w = 0; w < getSizeW(); w++)
		{
			QString codeWordStr = getCodeWordStr(w);

			cmdStr += QString("%1 ").arg(codeWordStr);
		}

		int tabLen = 32 - cmdStr.length();

		int tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

		for(int i = 0; i < tabCount; i++)
		{
			cmdStr += "\t";
		}

		QString mnemoCode = getMnemoCode();

		cmdStr += mnemoCode;

		if (!commentIsEmpty())
		{
			tabLen = 56 - cmdStr.length();

			tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

			for(int i = 0; i < tabCount; i++)
			{
				cmdStr += "\t";
			}

			cmdStr += QString("-- %1").arg(getComment());
		}

		return cmdStr;
	}



	ApplicationLogicCode::ApplicationLogicCode()
	{
	}


	ApplicationLogicCode::~ApplicationLogicCode()
	{
		for(auto codeItem : m_codeItems)
		{
			delete codeItem;
		}

		m_codeItems.clear();
	}


	void ApplicationLogicCode::clear()
	{
		m_codeItems.clear();
		m_commandAddress = 0;
	}


	void ApplicationLogicCode::append(const Command& cmd)
	{
		Command* newCommand = new Command(cmd);

		newCommand->setAddress(m_commandAddress);

		newCommand->generateRawCode();

		m_commandAddress += newCommand->getSizeW();

		m_codeItems.append(newCommand);
	}


	void ApplicationLogicCode::append(const Comment& cmt)
	{
		Comment* newComment = new Comment(cmt);

		m_codeItems.append(newComment);
	}


	void ApplicationLogicCode::toString()
	{
		int count = m_codeItems.count();

		for(int i = 0; i < count; i++)
		{
			if (m_codeItems[i] == nullptr)
			{
				assert(false);
				continue;
			}

			QString str = m_codeItems[i]->toString();

			qDebug() << str;
		}
	}
}
