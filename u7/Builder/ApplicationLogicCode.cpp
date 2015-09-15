#include "ApplicationLogicCode.h"
#include "../VFrame30/Afb.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// LmCommand structure static members implementation
	//
	// ---------------------------------------------------------------------------------------

	bool LmCommand::isValidCode(int commandCode)
	{
		return commandCode >= 0 && commandCode < LM_COMMAND_COUNT;
	}


	int LmCommand::getSizeW(int commandCode)
	{
		if (!isValidCode(commandCode))
		{
			assert(false);
			return 0;
		}

		return LmCommands[commandCode].sizeW;
	}


	const char* LmCommand::getStr(int commandCode)
	{
		if (!isValidCode(commandCode))
		{
			assert(false);
			return "";
		}

		return LmCommands[commandCode].str;
	}

	// ---------------------------------------------------------------------------------------
	//
	// CommandCode class implementation
	//
	// ---------------------------------------------------------------------------------------

	void CommandCode::setOpCode(LmCommandCode code)
	{
		if (!LmCommand::isValidCode(static_cast<int>(code)))
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


	void CommandCode::setBitNo1(quint16 bitNo)
	{
		if (bitNo > MAX_BIT_NO_16)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b1 = bitNo;
		}
	}


	void CommandCode::setBitNo2(quint16 bitNo)
	{
		if (bitNo > MAX_BIT_NO_16)
		{
			assert(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b2 = bitNo;
		}
	}


	quint16 CommandCode::getWord(int index) const
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


	int CommandCode::sizeW() const
	{
		int cmdCode = static_cast<int>(opCode.code);

		if (cmdCode > LM_COMMAND_COUNT)
		{
			assert(false);
			return 0;
		}

		return LmCommand::getSizeW(cmdCode);
	}


	QString CommandCode::getFbTypeStr() const
	{
		return Afb::AfbType::toText(getFbType());
	}


	void CommandCode::setConstFloat(float floatValue)
	{
		m_const.floatValue = floatValue;
		m_constIsFloat = true;
	}


	float CommandCode::getConstFloat() const
	{
		if (m_constIsFloat == true)
		{
			return m_const.floatValue;
		}

		assert(false);

		return 0;
	}


	void CommandCode::setConstInt32(qint32 int32Value)
	{
		m_const.int32Value = int32Value;
		m_constIsFloat = false;
	}


	qint32 CommandCode::getConstInt32() const
	{
		if (m_constIsFloat == false)
		{
			return m_const.int32Value;
		}

		assert(false);

		return 0;
	}



	// ---------------------------------------------------------------------------------------
	//
	// Comment class implementation
	//
	// ---------------------------------------------------------------------------------------


	QString Comment::toString()
	{
		QString comment = getComment();

		if (comment.isEmpty())
		{
			return "";
		}

		return QString("\t-- %1").arg(getComment());
	}



	// ---------------------------------------------------------------------------------------
	//
	// Command class implementation
	//
	// ---------------------------------------------------------------------------------------

	void Command::nop()
	{
		m_code.setOpCode(LmCommandCode::NOP);
	}


	void Command::start(quint16 fbType, quint16 fbInstance, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::START);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbCaption(fbCaption);
	}

	void Command::stop()
	{
		m_code.setOpCode(LmCommandCode::STOP);
	}


	void Command::mov(quint16 addrTo, quint16 addrFrom)
	{
		m_code.setOpCode(LmCommandCode::MOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}


	void Command::movMem(quint16 addrTo, quint16 addrFrom, quint16 sizeW)
	{
		m_code.setOpCode(LmCommandCode::MOVMEM);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(sizeW);
	}


	void Command::movConst(quint16 addrTo, quint16 constVal)
	{
		m_code.setOpCode(LmCommandCode::MOVC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constVal);
	}


	void Command::movBitConst(quint16 addrTo, quint16 bitNo, quint16 constBit)
	{
		m_code.setOpCode(LmCommandCode::MOVBC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constBit);
		m_code.setBitNo(bitNo);
	}


	void Command::writeFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::WRFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}


	void Command::readFuncBlock(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::RDFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}


	void Command::writeFuncBlockConst(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 constVal, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::WRFBC);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(constVal);
		m_code.setFbCaption(fbCaption);
	}


	void Command::writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, quint16 bitNo, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::WRFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setBitNo(bitNo);
		m_code.setFbCaption(fbCaption);
	}


	void Command::readFuncBlockBit(quint16 addrTo, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::RDFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setBitNo(bitNo);
		m_code.setFbCaption(fbCaption);
	}

	void Command::readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::RDFBTS);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(testValue);
		m_code.setFbCaption(fbCaption);
	}


	void Command::setMem(quint16 addr, quint16 sizeW, quint16 constValue)
	{
		m_code.setOpCode(LmCommandCode::SETMEM);
		m_code.setWord2(addr);
		m_code.setWord3(constValue);
		m_code.setWord4(sizeW);
	}


	void Command::moveBit(quint16 addrTo, quint16 addrToMask, quint16 addrFrom, quint16 addrFromMask)
	{
		m_code.setOpCode(LmCommandCode::MOVB);
		m_code.setWord2(addrTo);
		m_code.setBitNo2(addrToMask);
		m_code.setWord3(addrFrom);
		m_code.setBitNo1(addrFromMask);
	}


	void Command::nstart(quint16 fbType, quint16 fbInstance, quint16 startCount, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::NSTART);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setWord3(startCount);
		m_code.setFbCaption(fbCaption);
	}


	void Command::appStart(quint16 appStartAddr)
	{
		m_code.setOpCode(LmCommandCode::APPSTART);
		m_code.setWord2(appStartAddr);
	}


	void Command::mov32(quint16 addrTo, quint16 addrFrom)
	{
		m_code.setOpCode(LmCommandCode::MOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}


	void Command::movConstInt32(quint16 addrTo, qint32 constInt32)
	{
		m_code.setOpCode(LmCommandCode::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstInt32(constInt32);
	}


	void Command::movConstFloat(quint16 addrTo, double constFloat)
	{
		float floatValue = static_cast<float>(constFloat);

		qint32 constInt32 = *reinterpret_cast<qint32*>(&floatValue);		// map binary code of float to qint32

		m_code.setOpCode(LmCommandCode::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstFloat(constFloat);
	}


	void Command::writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::WRFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}


	void Command::readFuncBlock32(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::RDFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}


	void Command::writeFuncBlockConstInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 constInt32, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::WRFBC32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstInt32(constInt32);
	}


	void Command::writeFuncBlockConstFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, double constFloat, const QString& fbCaption)
	{
		float floatValue = static_cast<float>(constFloat);

		qint32 constInt32 = *reinterpret_cast<qint32*>(&floatValue);		// map binary code of float to qint32

		m_code.setOpCode(LmCommandCode::WRFBC32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(constFloat);
	}


	void Command::readFuncBlockTestInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 testInt32, const QString& fbCaption)
	{
		m_code.setOpCode(LmCommandCode::RDFBTS32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstInt32(testInt32);
	}


	void Command::readFuncBlockTestFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, double testFloat, const QString& fbCaption)
	{
		float floatValue = static_cast<float>(testFloat);

		qint32 testInt32 = *reinterpret_cast<qint32*>(&floatValue);		// map binary code of float to qint32

		m_code.setOpCode(LmCommandCode::RDFBTS32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(testFloat);
	}


	void Command::generateBinCode(ByteOrder byteOrder)
	{
		m_binCode.clear();

		int cmdSizeW = sizeW();

		m_binCode.resize(cmdSizeW * sizeof(quint16));

		for(int i = 0; i < cmdSizeW; i++)
		{
			quint16 cmdWord = m_code.getWord(i);

			if (byteOrder == ByteOrder::LittleEndian)
			{
				// Little Endian byte order
				//
				m_binCode[i * 2] = cmdWord & 0x00FF;
				m_binCode[i * 2 + 1] = (cmdWord & 0xFF00) >> 8;
			}
			else
			{
				// Big Endian byte order
				//
				m_binCode[i * 2] = (cmdWord & 0xFF00) >> 8;
				m_binCode[i * 2 + 1] = cmdWord & 0x00FF;
			}
		}
	}


	QString Command::getCodeWordStr(int wordNo)
	{
		QString str;

		if (m_binCode.count() < (wordNo + 1) * 2)
		{
			assert(false);
			return str;
		}

		unsigned int lowByte = m_binCode[wordNo * 2];
		unsigned int highByte = m_binCode[wordNo * 2 + 1];

		lowByte &= 0x00FF;
		highByte &= 0x00FF;

		str.sprintf("%02X%02X", lowByte, highByte);

		return str;
	}


	QString Command::getMnemoCode()
	{
		int opCodeInt = m_code.getOpCodeInt();

		QString mnemoCode = QString(LmCommand::getStr(opCodeInt)).leftJustified(10, ' ', false);

		QString params;

		switch(m_code.getOpCode())
		{
		case LmCommandCode::NoCommand:
		case LmCommandCode::NOP:
		case LmCommandCode::STOP:
			break;

		case LmCommandCode::START:
			params = QString("%1.%2").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt());
			break;

		case LmCommandCode::MOV:
		case LmCommandCode::MOV32:
			params = QString("%1, %2").
						arg(m_code.getWord2()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::MOVMEM:
			params = QString("%1, %2, %3").
						arg(m_code.getWord2()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommandCode::MOVC:
			params = QString("%1, #%2").
						arg(m_code.getWord2()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::MOVBC:
			params = QString("%1[%2], #%3").
						arg(m_code.getWord2()).
						arg(m_code.getWord4()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::WRFB:
		case LmCommandCode::WRFB32:
			params = QString("%1.%2[%3], %4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::RDFB:
		case LmCommandCode::RDFB32:
			params = QString("%1, %2.%3[%4]").
						arg(m_code.getWord3()).
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt());
			break;

		case LmCommandCode::WRFBC:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::WRFBB:
			params = QString("%1.%2[%3], %4[%5]").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommandCode::RDFBB:
			params = QString("%1[%2], %3.%4[%5]").
						arg(m_code.getWord3()).
						arg(m_code.getWord4()).
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt());
			break;

		case LmCommandCode::RDFBTS:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::SETMEM:
			params = QString("%1, #%2, %3").
						arg(m_code.getWord2()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommandCode::MOVB:
			params = QString("%1[%2], %3[%4]").
						arg(m_code.getWord2()).
						arg(m_code.getBitNo2()).
						arg(m_code.getWord3()).
						arg(m_code.getBitNo1());
			break;

		case LmCommandCode::NSTART:
			params = QString("%1.%2, %3").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getWord3());
			break;

		case LmCommandCode::APPSTART:
			params = QString("%1").
						arg(m_code.getWord2());
			break;

		case LmCommandCode::MOVC32:
			if (m_code.constIsFloat())
			{
				params = QString("%1, #%2").
							arg(m_code.getWord2()).
							arg(m_code.getConstFloat());
			}
			else
			{
				params = QString("%1, #%2").
							arg(m_code.getWord2()).
							arg(m_code.getConstInt32());
			}
			break;

		case LmCommandCode::WRFBC32:
			if (m_code.constIsFloat())
			{
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstFloat());
			}
			else
			{
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstInt32());
			}
			break;

		case LmCommandCode::RDFBTS32:
			if (m_code.constIsFloat())
			{
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstFloat());
			}
			else
			{
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstInt32());
			}
			break;

		default:
			assert(false);
		}

		return mnemoCode + params;
	}


	QString Command::toString()
	{
		QString cmdStr;

		cmdStr.sprintf("%04X\t", m_address);

		for(int w = 0; w < sizeW(); w++)
		{
			QString codeWordStr = getCodeWordStr(w);

			cmdStr += QString("%1 ").arg(codeWordStr);
		}

		int tabLen = 32 - (cmdStr.length() - 1 + 4);

		int tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

		for(int i = 0; i < tabCount; i++)
		{
			cmdStr += "\t";
		}

		QString mnemoCode = getMnemoCode();

		cmdStr += mnemoCode;

		if (!commentIsEmpty())
		{
			tabLen = 64 - 32 - mnemoCode.length();

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

		//newCommand->generateRawCode();

		m_commandAddress += newCommand->sizeW();

		m_codeItems.append(newCommand);
	}


	void ApplicationLogicCode::append(const Comment& cmt)
	{
		Comment* newComment = new Comment(cmt);

		m_codeItems.append(newComment);
	}


	void ApplicationLogicCode::replaceAt(int commandIndex, const Command &cmd)
	{
		if (commandIndex < 0 && commandIndex >= m_codeItems.count())
		{
			assert(false);
			return;
		}

		CodeItem* codeItem = m_codeItems[commandIndex];

		Command* oldCommand = dynamic_cast<Command*>(codeItem);

		if (oldCommand == nullptr)
		{
			assert(false);
			return;
		}

		Command* newCommand = new Command(cmd);

		m_codeItems[commandIndex] = newCommand;

		if (oldCommand->sizeW() == newCommand->sizeW())
		{
			// no need recalc commands addresses
			//
			newCommand->setAddress(oldCommand->address());
		}
		else
		{
			// recalc commands addresses
			//
			m_commandAddress = 0;

			for(CodeItem* codeItem : m_codeItems)
			{
				if (codeItem->isComment())
				{
					continue;
				}

				Command* cmd = dynamic_cast<Command*>(codeItem);

				if (cmd == nullptr)
				{
					assert(false);
					continue;
				}

				cmd->setAddress(m_commandAddress);

				m_commandAddress += cmd->sizeW();
			}
		}

		delete oldCommand;
	}


	void ApplicationLogicCode::comment(QString commentStr)
	{
		Comment* newComment = new Comment();

		newComment->setComment(commentStr);

		m_codeItems.append(newComment);
	}


	void ApplicationLogicCode::newLine()
	{
		Comment* newComment = new Comment();

		m_codeItems.append(newComment);
	}

	void ApplicationLogicCode::generateBinCode()
	{
		for(CodeItem* codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			codeItem->generateBinCode(m_byteOrder);
		}
	}


	void ApplicationLogicCode::getAsmCode(QStringList& asmCode)
	{
		asmCode.clear();

		for(auto codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			QString str = codeItem->toString();

			qDebug() << str;

			asmCode.append(str);
		}
	}


	void ApplicationLogicCode::getBinCode(QByteArray& byteArray)
	{
		byteArray.clear();

		int codeSizeW = 0;

		for(CodeItem* codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			codeSizeW += codeItem->sizeW();
		}

		byteArray.reserve(codeSizeW * sizeof(quint16));

		for(CodeItem* codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			byteArray.append(codeItem->getBinCode());
		}
	}


	void ApplicationLogicCode::getMifCode(QStringList& mifCode)
	{
		mifCode.clear();

		if (m_codeItems.count() < 1)
		{
			return;
		}

		int width = 16;
		int depth = 0;

		// find last command for compute address depth
		//
		for(int i = m_codeItems.count() - 1; i >= 0; i--)
		{
			CodeItem* codeItem = m_codeItems[i];

			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (codeItem->isComment())
			{
				continue;
			}

			Command* command = dynamic_cast<Command*>(codeItem);

			if (command == nullptr)
			{
				assert(false);
				continue;
			}

			depth = command->address() + command->sizeW() - 1;

			break;
		}

		mifCode.append(QString("WIDTH = %1;").arg(width));
		mifCode.append(QString("DEPTH = %1;").arg(depth + 1));

		mifCode.append("");

		mifCode.append("ADDRESS_RADIX = HEX;");
		mifCode.append("DATA_RADIX = HEX;");

		mifCode.append("");

		mifCode.append("CONTENT");
		mifCode.append("BEGIN");

		QString codeStr;
		QString str;

		for(CodeItem* codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (codeItem->isComment())
			{
				if (codeItem->getComment().isEmpty())
				{
					str.clear();
				}
				else
				{
					str = QString("\t-- %1").arg(codeItem->getComment());
				}

				mifCode.append(str);

				continue;
			}

			Command* command = dynamic_cast<Command*>(codeItem);

			const QByteArray& binCode = command->getBinCode();

			assert((binCode.count() % 2) == 0);

			int bytesCount = binCode.count();

			for(int i = 0; i < bytesCount; i++)
			{
				if (i == 0)
				{
					str.sprintf("\t%04X : ", command->address());
					codeStr = str;
				}

				unsigned int b = binCode[i];

				b &= 0xFF;

				if ((i % 2) == 1)
				{
					if (i == bytesCount-1)
					{
						str.sprintf("%02X;", b);
					}
					else
					{
						str.sprintf("%02X ", b);
					}
				}
				else
				{
					str.sprintf("%02X", b);
				}

				codeStr += str;
			}

			int tabLen = 40 - (codeStr.length() - 1 + 8);
			int tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

			for(int i = 0; i < tabCount; i++)
			{
				codeStr += "\t";
			}

			str = QString("-- %1").arg(command->getMnemoCode());

			codeStr += str;

			mifCode.append(codeStr);
		}

		mifCode.append("END;");
	}

}
