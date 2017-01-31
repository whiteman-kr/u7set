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

	CommandCode::CommandCode()
	{
		setNoCommand();
	}


	CommandCode::CommandCode(const CommandCode& cCode)
	{
		*this = cCode;
	}


	CommandCode& CommandCode::operator = (const CommandCode& cCode)
	{
		word1 = cCode.word1;
		word2 = cCode.word2;
		word3 = cCode.word3;
		word4 = cCode.word4;

		m_fbCaption = cCode.m_fbCaption;

		m_const = cCode.m_const;
		m_constDataFormat = cCode.m_constDataFormat;

		return *this;
	}

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
		m_constDataFormat = E::DataFormat::Float;
    }


    float CommandCode::getConstFloat() const
    {
		if (m_constDataFormat == E::DataFormat::Float)
        {
            return m_const.floatValue;
        }

        assert(false);

        return 0;
    }


    void CommandCode::setConstInt32(qint32 int32Value)
    {
        m_const.int32Value = int32Value;
		m_constDataFormat = E::DataFormat::SignedInt;
    }


    qint32 CommandCode::getConstInt32() const
    {
		if (m_constDataFormat == E::DataFormat::SignedInt)
        {
            return m_const.int32Value;
        }

        assert(false);

        return 0;
    }


	void CommandCode::setConstUInt32(quint32 uint32Value)
	{
		m_const.uint32Value = uint32Value;
		m_constDataFormat = E::DataFormat::UnsignedInt;
	}


	quint32 CommandCode::getConstUInt32() const
	{
		if (m_constDataFormat == E::DataFormat::UnsignedInt)
		{
			return m_const.uint32Value;
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

    QHash<int, const LmCommand*> Command::m_lmCommands;

	LmMemoryMap* Command::m_memoryMap = nullptr;
	IssueLogger* Command::m_log = nullptr;

    Command::Command()
    {
		initStaticMembers();
    }


	Command::Command(const Command& cmd)
	{
		initStaticMembers();

		m_address = cmd.m_address;
		m_fbRunTime = cmd.m_fbRunTime;
		m_result = cmd.m_result;
		m_code = cmd.m_code;
	}


	void Command::initStaticMembers()
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);			// call ApplicationLogicCode::initMemoryMap() first
			return;
		}

		if (m_lmCommands.isEmpty())
		{
			for(const LmCommand& lmCommand : LmCommands)
			{
				m_lmCommands.insert(static_cast<int>(lmCommand.code), &lmCommand);
			}
		}
	}


	void Command::setMemoryMap(LmMemoryMap* memoryMap, IssueLogger* log)
    {
		if (memoryMap == nullptr ||
			log == nullptr)
		{
			assert(false);
			return;
		}

		m_memoryMap = memoryMap;
		m_log = log;
    }


	void Command::resetMemoryMap()
    {
		m_memoryMap = nullptr;
		m_log = nullptr;
    }


    void Command::nop()
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::NOP);
    }


    void Command::start(quint16 fbType, quint16 fbInstance, const QString& fbCaption, int fbRunTime)
    {
		m_result = true;

		m_fbRunTime = fbRunTime;

        m_code.setOpCode(LmCommandCode::START);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbCaption(fbCaption);

		//

		// check: fbType, fbInstance

		if (fbRunTime == 0)
		{
			assert(false);		// fbRunTime can't be 0
		}
    }


    void Command::stop()
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::STOP);
    }


    void Command::mov(quint16 addrTo, quint16 addrFrom)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOV);
        m_code.setWord2(addrTo);
        m_code.setWord3(addrFrom);

		//

		read16(addrFrom);
		write16(addrTo);
    }


    void Command::movMem(quint16 addrTo, quint16 addrFrom, quint16 sizeW)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVMEM);
        m_code.setWord2(addrTo);
        m_code.setWord3(addrFrom);
        m_code.setWord4(sizeW);

		//

		if (addressInBitMemory(addrTo) ||
			addressInBitMemory(addrTo + sizeW - 1))
		{
			// Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.
			//
			m_log->errALC5066(addrTo, addrFrom, sizeW);
			m_result = false;
		}

		readArea(addrFrom, sizeW);
		writeArea(addrTo, sizeW);
    }


    void Command::movConst(quint16 addrTo, quint16 constVal)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVC);
        m_code.setWord2(addrTo);
        m_code.setWord3(constVal);

		//

		write16(addrTo);
    }


    void Command::movBitConst(quint16 addrTo, quint16 bitNo, quint16 constBit)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVBC);
        m_code.setWord2(addrTo);
        m_code.setWord3(constBit);
        m_code.setBitNo(bitNo);

		//

		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{

			//	Command 'MOVBC %1, %2, #%3' can't write out of application bit- or word-addressed memory.
			//
			m_log->errALC5067(addrTo, bitNo, constBit);

			m_result = false;
		}

		write16(addrTo);
    }


    void Command::writeFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::WRFB);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrFrom);
        m_code.setFbCaption(fbCaption);

		//

		read16(addrFrom);
    }


    void Command::readFuncBlock(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::RDFB);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrTo);
        m_code.setFbCaption(fbCaption);

		//

		write16(addrTo);
    }


    void Command::writeFuncBlockConst(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 constVal, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::WRFBC);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(constVal);
        m_code.setFbCaption(fbCaption);
	}


    void Command::writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, quint16 bitNo, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::WRFBB);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrFrom);
        m_code.setBitNo(bitNo);
        m_code.setFbCaption(fbCaption);

		//

		read16(addrFrom);
    }


    void Command::readFuncBlockBit(quint16 addrTo, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::RDFBB);
		m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrTo);
        m_code.setBitNo(bitNo);
        m_code.setFbCaption(fbCaption);

		//

		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{
			assert(false);			// RDFBB command can write only in bit- or word-addressed memory
			m_result = false;
			return;
		}

		m_memoryMap->write16(addrTo);
    }

    void Command::readFuncBlockTest(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::RDFBTS);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(testValue);
        m_code.setFbCaption(fbCaption);
    }


	void Command::setMem(quint16 addr, quint16 constValue, quint16 sizeW)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::SETMEM);
        m_code.setWord2(addr);
        m_code.setWord3(constValue);
        m_code.setWord4(sizeW);

		//

		if (addressInBitMemory(addr) ||
			addressInBitMemory(addr + sizeW - 1))
		{
			assert(false);			// SETMEM command can't write to bit-addressed memory
			m_result = false;
		}

		writeArea(addr, sizeW);
    }


    void Command::movBit(quint16 addrTo, quint16 addrToMask, quint16 addrFrom, quint16 addrFromMask)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVB);
        m_code.setWord2(addrTo);
        m_code.setBitNo2(addrToMask);
        m_code.setWord3(addrFrom);
        m_code.setBitNo1(addrFromMask);

		//

		read16(addrFrom);
    }


    void Command::nstart(quint16 fbType, quint16 fbInstance, quint16 startCount, const QString& fbCaption, int fbRunTime)
    {
		m_result = true;

		m_fbRunTime = fbRunTime;

        m_code.setOpCode(LmCommandCode::NSTART);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setWord3(startCount);
        m_code.setFbCaption(fbCaption);

		//

		if (fbRunTime == 0)
		{
			assert(false);		// fbRunTime can't be 0
		}
    }


    void Command::appStart(quint16 appStartAddr)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::APPSTART);
        m_code.setWord2(appStartAddr);
    }


    void Command::mov32(quint16 addrTo, quint16 addrFrom)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOV32);
        m_code.setWord2(addrTo);
        m_code.setWord3(addrFrom);

		//

		read32(addrFrom);
		write32(addrTo);
    }


    void Command::movConstInt32(quint16 addrTo, qint32 constInt32)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVC32);
        m_code.setWord2(addrTo);
        m_code.setWord3((constInt32 >> 16) & 0xFFFF);
        m_code.setWord4(constInt32 & 0xFFFF);
        m_code.setConstInt32(constInt32);

		//

		write32(addrTo);
    }


	void Command::movConstUInt32(quint16 addrTo, quint32 constUInt32)
	{
		m_result = true;

		m_code.setOpCode(LmCommandCode::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constUInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constUInt32 & 0xFFFF);
		m_code.setConstUInt32(constUInt32);

		//

		write32(addrTo);
	}


    void Command::movConstFloat(quint16 addrTo, float constFloat)
    {
		m_result = true;

		qint32 constInt32 = *reinterpret_cast<qint32*>(&constFloat);		// map binary code of float to qint32

        m_code.setOpCode(LmCommandCode::MOVC32);
        m_code.setWord2(addrTo);
        m_code.setWord3((constInt32 >> 16) & 0xFFFF);
        m_code.setWord4(constInt32 & 0xFFFF);
        m_code.setConstFloat(constFloat);

		//

		write32(addrTo);
    }


    void Command::writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::WRFB32);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrFrom);
        m_code.setFbCaption(fbCaption);

		//

		read32(addrFrom);
    }


    void Command::readFuncBlock32(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
    {
		m_result = true;

		m_code.setOpCode(LmCommandCode::RDFB32);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3(addrTo);
        m_code.setFbCaption(fbCaption);

		//

		write32(addrTo);
    }


    void Command::writeFuncBlockConstInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 constInt32, const QString& fbCaption)
    {
		m_result = true;

        m_code.setOpCode(LmCommandCode::WRFBC32);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3((constInt32 >> 16) & 0xFFFF);
        m_code.setWord4(constInt32 & 0xFFFF);
        m_code.setFbCaption(fbCaption);
        m_code.setConstInt32(constInt32);
    }


    void Command::writeFuncBlockConstFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float constFloat, const QString& fbCaption)
    {
		m_result = true;

		qint32 constInt32 = *reinterpret_cast<qint32*>(&constFloat);		// map binary code of float to qint32

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
		m_result = true;

		m_code.setOpCode(LmCommandCode::RDFBTS32);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3((testInt32 >> 16) & 0xFFFF);
        m_code.setWord4(testInt32 & 0xFFFF);
        m_code.setFbCaption(fbCaption);
        m_code.setConstInt32(testInt32);
    }


    void Command::readFuncBlockTestFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float testFloat, const QString& fbCaption)
    {
		m_result = true;

		qint32 testInt32 = *reinterpret_cast<qint32*>(&testFloat);		// map binary code of float to qint32

        m_code.setOpCode(LmCommandCode::RDFBTS32);
        m_code.setFbType(fbType);
        m_code.setFbInstance(fbInstance);
        m_code.setFbParamNo(fbParamNo);
        m_code.setWord3((testInt32 >> 16) & 0xFFFF);
        m_code.setWord4(testInt32 & 0xFFFF);
        m_code.setFbCaption(fbCaption);
        m_code.setConstFloat(testFloat);
    }


    void Command::generateBinCode(E::ByteOrder byteOrder)
    {
        m_binCode.clear();

        int cmdSizeW = sizeW();

        m_binCode.resize(cmdSizeW * sizeof(quint16));

        for(int i = 0; i < cmdSizeW; i++)
        {
            quint16 cmdWord = m_code.getWord(i);

            if (byteOrder == E::ByteOrder::LittleEndian)
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
			switch(m_code.constDataFormat())
            {
			case E::DataFormat::Float:
                params = QString("%1, #%2").
                            arg(m_code.getWord2()).
                            arg(m_code.getConstFloat());
				break;

			case E::DataFormat::SignedInt:
                params = QString("%1, #%2").
                            arg(m_code.getWord2()).
                            arg(m_code.getConstInt32());
				break;

			case E::DataFormat::UnsignedInt:
				params = QString("%1, #%2").
							arg(m_code.getWord2()).
							arg(m_code.getConstUInt32());
				break;

			default:
				assert(false);
			}

            break;

        case LmCommandCode::WRFBC32:
			switch(m_code.constDataFormat())
			{
			case E::DataFormat::Float:
				params = QString("%1.%2[%3], #%4").
                            arg(m_code.getFbCaption()).
                            arg(m_code.getFbInstanceInt()).
                            arg(m_code.getFbParamNoInt()).
                            arg(m_code.getConstFloat());
				break;

			case E::DataFormat::SignedInt:
				params = QString("%1.%2[%3], #%4").
                            arg(m_code.getFbCaption()).
                            arg(m_code.getFbInstanceInt()).
                            arg(m_code.getFbParamNoInt()).
                            arg(m_code.getConstInt32());
				break;

			case E::DataFormat::UnsignedInt:
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstUInt32());
				break;

			default:
				assert(false);
            }

            break;

        case LmCommandCode::RDFBTS32:
			switch(m_code.constDataFormat())
			{
			case E::DataFormat::Float:
				params = QString("%1.%2[%3], #%4").
                            arg(m_code.getFbCaption()).
                            arg(m_code.getFbInstanceInt()).
                            arg(m_code.getFbParamNoInt()).
                            arg(m_code.getConstFloat());
				break;

			case E::DataFormat::SignedInt:
                params = QString("%1.%2[%3], #%4").
                            arg(m_code.getFbCaption()).
                            arg(m_code.getFbInstanceInt()).
                            arg(m_code.getFbParamNoInt()).
                            arg(m_code.getConstInt32());
				break;

			case E::DataFormat::UnsignedInt:
				params = QString("%1.%2[%3], #%4").
							arg(m_code.getFbCaption()).
							arg(m_code.getFbInstanceInt()).
							arg(m_code.getFbParamNoInt()).
							arg(m_code.getConstUInt32());
				break;

			default:
				assert(false);
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

        int readTime = 0;
        int runTime = 0;

        getReadAndRunTimes(&readTime, &runTime);

        QString str;

        str.sprintf("[%02d:%02d]", readTime, runTime);
        str = str.leftJustified(12, ' ');

        cmdStr += str;

        QString mnemoCode = getMnemoCode();

        cmdStr += mnemoCode;

        if (!commentIsEmpty())
        {
            tabLen = 80 - 32 - mnemoCode.length();

            if (tabLen <= 0)
            {
                tabLen += 16;
            }

            tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

            for(int i = 0; i < tabCount; i++)
            {
                cmdStr += "\t";
            }

            cmdStr += QString("-- %1").arg(getComment());
        }

        return cmdStr;
    }


    bool Command::getReadAndRunTimes(int* readTime, int* runTime)
    {
        if (readTime == nullptr || runTime == nullptr)
        {
            assert(false);
            return false;
        }

        if (m_lmCommands.contains(m_code.getOpCodeInt()) == false)
        {
            assert(false);			// unknown command code!
            return false;
        }

        const LmCommand* lmCommand = m_lmCommands[m_code.getOpCodeInt()];

        if (lmCommand == nullptr)
        {
            assert(false);
            return false;
        }

        *readTime = lmCommand->readTime;

        switch(lmCommand->runTime)
        {
        case RUNTIME_START:
            *runTime = 5 + m_fbRunTime;
            break;

        case RUNTIME_MOVE:
            *runTime = addressInBitMemory(m_code.getWord2()) == true ? 54 : 8;
            break;

        case RUNTIME_MOVEMEM:
            *runTime = 2 + m_code.getWord4() * 6 + 3;
            break;

        case RUNTIME_MOVC:
            *runTime = addressInBitMemory(m_code.getWord2()) == true ? 52 : 5;
            break;

        case RUNTIME_MOVBC:
            *runTime = addressInBitMemory(m_code.getWord2()) == true ? 6 : 11;
            break;

        case RUNTIME_RDFBB:
            *runTime = addressInBitMemory(m_code.getWord3()) == true ? 8 : 9;
            break;

        case RUNTIME_SETMEM:
            *runTime = 2 + m_code.getWord4() * 3;
            break;

        case RUNTIME_MOVB:
            *runTime = addressInBitMemory(m_code.getWord2()) == true ? 7 : 11;
            break;

        case RUNTIME_NSTART:
            *runTime = 3 + m_code.getWord3() * (2 + m_fbRunTime);
            break;

        default:
            assert(lmCommand->runTime < RUNTIME_START);

            *runTime = lmCommand->runTime;
        }

        return true;
    }


    bool Command::addressInBitMemory(int address)
    {
		if (m_memoryMap == nullptr)
		{
			assert(false);		// call setMemoryMap first
			return false;
		}

		if (address >= m_memoryMap->appBitMemoryStart() &&
			address < m_memoryMap->appBitMemoryStart() + m_memoryMap->appBitMemorySizeW())
        {
            return true;
        }

        return false;
    }


    bool Command::addressInWordMemory(int address)
    {
		if (m_memoryMap == nullptr)
		{
			assert(false);		// call setMemoryMap first
			return false;
		}

		if (address >= m_memoryMap->appWordMemoryStart() &&
			address < m_memoryMap->appWordMemoryStart() + m_memoryMap->appWordMemorySizeW())
        {
            return true;
        }

        return false;
    }


	bool Command::read16(int addrFrom)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->read16(addrFrom);
		}

		return m_result;
	}


	bool Command::read32(int addrFrom)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->read32(addrFrom);
		}

		return m_result;
	}


	bool Command::readArea(int addrFrom, int sizeW)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->readArea(addrFrom, sizeW);
		}

		return m_result;
	}


	bool Command::write16(int addrTo)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->write16(addrTo);
		}

		return m_result;
	}


	bool Command::write32(int addrTo)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->write32(addrTo);
		}

		return m_result;
	}


	bool Command::writeArea(int addrTo, int sizeW)
	{
		if (m_memoryMap == nullptr)
		{
			assert(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->writeArea(addrTo, sizeW);
		}

		return m_result;
	}


    // ---------------------------------------------------------------------------------------
    //
    // ApplicationLogicCode structure static members implementation
    //
    // ---------------------------------------------------------------------------------------

	ApplicationLogicCode::ApplicationLogicCode()
    {
    }


    ApplicationLogicCode::~ApplicationLogicCode()
    {
		Command::resetMemoryMap();

        for(auto codeItem : m_codeItems)
        {
            delete codeItem;
        }

        m_codeItems.clear();
    }


	void ApplicationLogicCode::setMemoryMap(LmMemoryMap* lmMemory, IssueLogger* log)
    {
		if (lmMemory == nullptr ||
			log == nullptr)
		{
			assert(false);
			return;
		}

		Command::setMemoryMap(lmMemory, log);
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


	void ApplicationLogicCode::getAsmMetadataFields(QStringList& metadataFields)
	{
		metadataFields.clear();

		metadataFields.append("IsCommand");
		metadataFields.append("Address");
		metadataFields.append("BinCode");
		metadataFields.append("MnemoCode");
		metadataFields.append("Comment");
	}

	void ApplicationLogicCode::getAsmMetadata(std::vector<QVariantList>& metadata)
	{
		metadata.clear();

		for(CodeItem* codeItem : m_codeItems)
		{
			if (codeItem == nullptr)
			{
				assert(false);
				continue;
			}

			QVariantList data;

			bool isCommand = false;
			QString address;
			QString binCode;
			QString mnemoCode;
			QString comment;

			if (codeItem->isCommand())
			{
				isCommand = true;

				Command* cmd = dynamic_cast<Command*>(codeItem);

				if (cmd == nullptr)
				{
					assert(false);
					continue;
				}

				address = QString().sprintf("%04X", cmd->address());
				binCode = QString(cmd->getBinCode().toHex()).toUpper();
				mnemoCode = cmd->getMnemoCode();
				comment = cmd->getComment();
			}
			else
			{
				isCommand = false;

				comment = codeItem->getComment();

				if (comment.isEmpty())
				{
					continue;			// skip empty strings
				}
			}

			data.append(QVariant(isCommand));
			data.append(QVariant(address));
			data.append(QVariant(binCode));
			data.append(QVariant(mnemoCode));
			data.append(QVariant(comment));

			metadata.push_back(data);
		}
	}



    bool ApplicationLogicCode::getRunTimes(int* idrPhaseClockCount, int* alpPhaseClockCount)
    {
        if (idrPhaseClockCount == nullptr || alpPhaseClockCount == nullptr)
        {
            assert(false);
            return false;
        }

        *idrPhaseClockCount = 0;
        *alpPhaseClockCount = 0;

        if (m_codeItems.isEmpty())
        {
            return true;
        }

        // find appStart command and read application logic processing code start address
        //

        int appLogicProcessingCodeStartAddress = -1;

        for(CodeItem* codeItem : m_codeItems)
        {
            if (codeItem == nullptr)
            {
                assert(false);
                return false;
            }

            if (codeItem->isCommand() == false)
            {
                continue;
            }

            Command* command = dynamic_cast<Command*>(codeItem);

            if (command == nullptr)
            {
                assert(false);
                return false;
            }

            if (command->isOpCode(LmCommandCode::APPSTART))
            {
                appLogicProcessingCodeStartAddress = command->getWord2();
                break;
            }
        }

        if (appLogicProcessingCodeStartAddress == -1)
        {
            assert(false);
            return false;
        }

        // read commands and calculate code runtime
        //
        bool idrPhaseCode = true;

        int prevRunTime = 0;

		int count = 0;

        for(CodeItem* codeItem : m_codeItems)
        {
			count++;

            if (codeItem == nullptr)
            {
                assert(false);
                return false;
            }

			if (codeItem->isCommand() == false)
            {
                continue;
            }

			Command* command = dynamic_cast<Command*>(codeItem);

            if (command == nullptr)
            {
                assert(false);
                return false;
            }

            if (command->address() == appLogicProcessingCodeStartAddress)
            {
                *idrPhaseClockCount += prevRunTime;

                prevRunTime = 0;

                idrPhaseCode = false;
            }

            int readTime = 0;
            int runTime = 0;

            command->getReadAndRunTimes(&readTime, &runTime);

            if (idrPhaseCode == true)
            {
                // alpPhaseCode
                //
                if (prevRunTime >= readTime)
                {
                    *idrPhaseClockCount += prevRunTime;
                }
                else
                {
                    *idrPhaseClockCount += readTime;
                }

                prevRunTime = runTime;
            }
            else
            {
                // alpPhaseCode
                //
                if (prevRunTime >= readTime)
                {
                    *alpPhaseClockCount += prevRunTime;
                }
                else
                {
                    *alpPhaseClockCount += readTime;
                }

                prevRunTime = runTime;
            }
        }

        *alpPhaseClockCount += prevRunTime;

        return true;
    }


}
