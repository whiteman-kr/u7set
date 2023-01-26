#include "ApplicationLogicCode.h"
#include "../VFrame30/Afb.h"
#include "../lib/ConstStrings.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// LmCommand structure static members implementation
	//
	// ---------------------------------------------------------------------------------------

	LmCommands lmCommands;		// static map of LM's commands

	LmCommands::LmCommands()
	{
		for(const LmCommand& lmCommand : lmCommandSet)
		{
			if (contains(TO_INT(lmCommand.code)) == true)
			{
				Q_ASSERT(false);			// duplicate command code
				continue;
			}

			insert({TO_INT(lmCommand.code), lmCommand});
		}
	}

	bool LmCommands::isValidCode(LmCommand::Code commandCode) const
	{
		return isValidCode(TO_INT(commandCode));
	}

	bool LmCommands::isValidCode(int commandCode) const
	{
		bool codeExists = contains(commandCode);

		if (codeExists == false)
		{
			Q_ASSERT(false);
		}

		return codeExists;
	}

	int LmCommands::getSizeW(LmCommand::Code commandCode) const
	{
		return getSizeW(TO_INT(commandCode));
	}

	int LmCommands::getSizeW(int commandCode) const
	{
		auto it = find(commandCode);

		if (it == end())
		{
			Q_ASSERT(false);
			return 0;
		}

		return it->second.sizeW;
	}

	QString LmCommands::getMnemo(LmCommand::Code commandCode) const
	{
		return getMnemo(TO_INT(commandCode));
	}

	QString LmCommands::getMnemo(int commandCode) const
	{
		auto it = find(commandCode);

		if (it == end())
		{
			Q_ASSERT(false);
			return QString();
		}

		return it->second.mnemo;
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

	void CommandCode::setOpCode(LmCommand::Code code)
	{
		if (lmCommands.isValidCode(code) == false)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			opCode.code = static_cast<quint16>(code);
		}
	}

	void CommandCode::setFbType(int fbType)
	{
		if (fbType > LmCommand::MAX_FB_TYPE)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			opCode.fbType = CHECK_AND_CAST_TO_QUINT16(fbType);
		}
	}

	void CommandCode::setFbInstance(int fbInstance)
	{
		param.fbInstance = CHECK_AND_CAST_TO_QUINT16(fbInstance);
	}

	void CommandCode::setFbParamNo(int fbParamNo)
	{
		if (fbParamNo > LmCommand::MAX_FB_PARAM_NO)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			param.fbParamNo = CHECK_AND_CAST_TO_QUINT16(fbParamNo);
		}
	}

	void CommandCode::setBitNo(int bitNo)
	{
		if (bitNo < 0 || bitNo > LmCommand::MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			word4 = static_cast<quint16>(bitNo);
		}
	}

	void CommandCode::setBitNo1(int bitNo)
	{
		if (bitNo < 0 || bitNo > LmCommand::MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b1 = static_cast<quint8>(bitNo);
		}
	}

	void CommandCode::setBitNo2(int bitNo)
	{
		if (bitNo < 0 || bitNo > LmCommand::MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b2 = static_cast<quint8>(bitNo);
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
			Q_ASSERT(false);
		}

		return 0;
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

		Q_ASSERT(false);

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

		Q_ASSERT(false);

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

		Q_ASSERT(false);

		return 0;
	}

	int CommandCode::sizeW() const
	{
		return lmCommands.getSizeW(opCode.code);
	}

	void CommandCode::calcCrc5()
	{
		opCode.CRC5 = 0;

		const int DATA_SIZE = 64;
		const int CRC_SIZE = 5;
		const int BIT_COUNT = DATA_SIZE - CRC_SIZE;

		const quint64 UPPER_BIT = 0x8000000000000000ull;

		const quint64 POLYNOM = 0x05ll << BIT_COUNT;

		quint64 crc5  =  0x1Full << BIT_COUNT;

		quint64 data = 0;

		data |= word1;
		data <<= SIZE_16BIT;

		data |= word2;
		data <<= SIZE_16BIT;

		data |= word3;
		data <<= SIZE_16BIT;

		data |= word4;

		for(int i = 0; i < DATA_SIZE; i++)
		{
			if (((data ^ crc5) & UPPER_BIT) != 0)
			{
				crc5 <<= 1;
				crc5 ^= POLYNOM;
			}
			else
			{
				crc5 <<= 1;
			}

			data <<= 1;
		}

		// Shift back into position
		crc5 >>= BIT_COUNT;

		opCode.CRC5 = crc5;
	}

	void CommandCode::clear()
	{
		word1 = 0;
		word2 = 0;
		word3 = 0;
		word4 = 0;
	}

	// ---------------------------------------------------------------------------------------
	//
	// CodeItem class implementation
	//
	// ---------------------------------------------------------------------------------------

	CodeItem::CodeItem()
	{
	}

	void CodeItem::nop()
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::NOP);
	}

	void CodeItem::start(int fbType, int fbInstance, const QString& fbCaption, int fbRunTime)
	{
		initCommand();

		m_result = true;

		m_fbExecTime = fbRunTime;

		m_code.setOpCode(LmCommand::Code::START);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbCaption(fbCaption);

		if (fbRunTime == 0)
		{
			Q_ASSERT(false);		// fbRunTime can't be 0
		}
	}

	void CodeItem::stop()
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::STOP);
	}


	void CodeItem::mov(int addrTo, int addrFrom)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::mov(Address16 addrTo, Address16 addrFrom)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		mov(addrTo.offset(), addrFrom.offset());
	}

	void CodeItem::movMem(int addrTo, int addrFrom, int sizeW)
	{
		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::MOVMEM);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(sizeW);
	}

	void CodeItem::movMem(Address16 addrTo, Address16 addrFrom, int sizeW)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		movMem(addrTo.offset(), addrFrom.offset(), sizeW);
	}

	void CodeItem::movConst(int addrTo, int constVal)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constVal);
	}

	void CodeItem::movConst(Address16 addrTo, int constVal)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		movConst(addrTo.offset(), constVal);
	}

	void CodeItem::movBitConst(int addrTo, int bitNo, int constBit)
	{
		Q_ASSERT(addrTo >=0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVBC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constBit);
		m_code.setBitNo(bitNo);
	}

	void CodeItem::movBitConst(Address16 addr16, int constBit)
	{
		Q_ASSERT(addr16.isValid() == true);

		movBitConst(addr16.offset(), addr16.bit(), constBit);
	}

	void CodeItem::writeFuncBlock(int fbType, int fbInstance, int fbParamNo, const Address16& addrFrom, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom.bit() == 0);

		writeFuncBlock(fbType, fbInstance, fbParamNo, addrFrom.offset(), fbCaption);
	}

	void CodeItem::writeFuncBlock(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::readFuncBlock(const Address16& addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo.bit() == 0);

		readFuncBlock(addrTo.offset(), fbType, fbInstance, fbParamNo, fbCaption);
	}

	void CodeItem::readFuncBlock(int addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFBC);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(constVal);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setBitNo(bitNo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom.isValid() == true);

		writeFuncBlockBit(fbType, fbInstance, fbParamNo, addrFrom.offset(), addrFrom.bit(), fbCaption);
	}

	void CodeItem::readFuncBlockBit(int addrTo, int bitNo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFBB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setBitNo(bitNo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::readFuncBlockBit(Address16 addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo.isValid() == true);

		readFuncBlockBit(addrTo.offset(), addrTo.bit(), fbType, fbInstance, fbParamNo, fbCaption);
	}

	void CodeItem::readFuncBlockCompare(int fbType, int fbInstance, int fbParamNo, int testValue, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFBCMP);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(testValue);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::setMem(int addr, int constValue, int sizeW)
	{
		Q_ASSERT(addr >=0);

		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::SETMEM);
		m_code.setWord2(addr);
		m_code.setWord3(constValue);
		m_code.setWord4(sizeW);
	}

	void CodeItem::setMem(Address16 addr, int constValue, int sizeW)
	{
		Q_ASSERT(addr.isValid() == true);
		Q_ASSERT(addr.bit() == 0);

		setMem(addr.offset(), constValue, sizeW);
	}

	void CodeItem::movBit(int addrTo, int bitTo, int addrFrom, int bitFrom)
	{
		Q_ASSERT(addrTo >=0);
		Q_ASSERT(addrFrom >=0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVB);
		m_code.setWord2(addrTo);
		m_code.setBitNo2(bitTo);
		m_code.setWord3(addrFrom);
		m_code.setBitNo1(bitFrom);
	}

	void CodeItem::movBit(Address16 addrTo, Address16 addrFrom)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrFrom.isValid() == true);

		movBit(addrTo.offset(), addrTo.bit(), addrFrom.offset(), addrFrom.bit());
	}

	void CodeItem::nstart(int fbType, int fbInstance, int startCount, const QString& fbCaption, int fbRunTime)
	{
		initCommand();

		m_result = true;

		m_fbExecTime = fbRunTime;

		m_code.setOpCode(LmCommand::Code::NSTART);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setWord3(startCount);
		m_code.setFbCaption(fbCaption);

		//

		if (fbRunTime == 0)
		{
			Q_ASSERT(false);		// fbRunTime can't be 0
		}
	}

	void CodeItem::appStart(int appStartAddr)
	{
		Q_ASSERT(appStartAddr >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::APPSTART);
		m_code.setWord2(CHECK_AND_CAST_TO_QUINT16(appStartAddr));
	}

	void CodeItem::mov32(int addrTo, int addrFrom)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::mov32(Address16 addrTo, Address16 addrFrom)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		mov32(addrTo.offset(), addrFrom.offset());
	}

	void CodeItem::movConstInt32(int addrTo, qint32 constInt32)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstInt32(constInt32);
	}

	void CodeItem::movConstUInt32(int addrTo, quint32 constUInt32)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3(static_cast<quint16>((constUInt32 >> 16) & 0xFFFF));
		m_code.setWord4(static_cast<quint16>(constUInt32 & 0xFFFF));
		m_code.setConstUInt32(constUInt32);
	}

	void CodeItem::movConstUInt32(Address16 addrTo, quint32 constUInt32)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		movConstUInt32(addrTo.offset(), constUInt32);
	}

	void CodeItem::movConstFloat(int addrTo, float constFloat)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		qint32 constInt32 = std::bit_cast<qint32>(constFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstFloat(constFloat);
	}

	void CodeItem::writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, int addrFrom, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom, const QString& fbCaption)
	{
		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		writeFuncBlock32(fbType, fbInstance, fbParamNo, addrFrom.offset(), fbCaption);
	}

	void CodeItem::readFuncBlock32(int addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo >=0 );

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::readFuncBlock32(Address16 addrTo, int fbType, int fbInstance, int fbParamNo, const QString& fbCaption)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		readFuncBlock32(addrTo.offset(), fbType, fbInstance, fbParamNo, fbCaption);
	}

	void CodeItem::writeFuncBlockConstInt32(int fbType, int fbInstance, int fbParamNo, qint32 constInt32, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFBC32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstInt32(constInt32);
	}

	void CodeItem::writeFuncBlockConstFloat(int fbType, int fbInstance, int fbParamNo, float constFloat, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		qint32 constInt32 = std::bit_cast<qint32>(constFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::WRFBC32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(constFloat);
	}

	void CodeItem::readFuncBlockCompareInt32(int fbType, int fbInstance, int fbParamNo, qint32 testInt32, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFBCMP32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstInt32(testInt32);
	}

	void CodeItem::readFuncBlockCompareFloat(int fbType, int fbInstance, int fbParamNo, float testFloat, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		qint32 testInt32 = std::bit_cast<qint32>(testFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::RDFBCMP32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(testFloat);
	}

	void CodeItem::movCompareFlag(int addrTo, int bitNo)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(bitNo >= 0 && bitNo <= LmCommand::MAX_BIT_NO_16);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVCMPF);
		m_code.setWord2(addrTo);
		m_code.setWord3(bitNo);
	}

	void CodeItem::prevMov(int addrTo, int addrFrom)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::prevMov32(int addrTo, int addrFrom)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::fill(int addrTo, int addrFrom, int addrFromBit)
	{
		Q_ASSERT(addrTo >= 0);

		Q_ASSERT(addrFrom >= 0);
		Q_ASSERT(addrFromBit >= 0 && addrFromBit <= LmCommand::MAX_BIT_NO_16);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::FILL);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(addrFromBit);
	}

	void CodeItem::fill(Address16 addrTo, Address16 addrFrom)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);

		fill(addrTo.offset(), addrFrom.offset(), addrFrom.bit());
	}

	bool CodeItem::checkNop()
	{
		return true;
	}

	bool CodeItem::checkStart()
	{
		// need check fbType and fbInstance
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkStop()
	{
		// need check fbType and fbInstance
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMov()
	{
		// chek addresses

		//read16(addrFrom);
		//write16(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovMem()
	{
		/*if (addressInBitMemory(addrTo) ||
			addressInBitMemory(addrTo + sizeW - 1))
		{
			// Command 'MOVEMEM %1, %2, %3' can't write to bit-addressed memory.
			//
			m_log->errALC5066(addrTo, addrFrom, sizeW);
			m_result = false;
		}

		readArea(addrFrom, sizeW);
		writeArea(addrTo, sizeW);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovConst()
	{
		//write16(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovBitConst()
	{
		/*
		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{

			//	Command 'MOVBC %1, %2, #%3' can't write out of application bit- or word-addressed memory.
			//
			m_log->errALC5067(addrTo, bitNo, constBit);

			m_result = false;
		}

		write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlock()
	{
		//	read16(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlock()
	{
		// write16(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockConst()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockBit()
	{
		// read16(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockBit()
	{
		/*
		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{
			Q_ASSERT(false);			// RDFBB command can write only in bit- or word-addressed memory
			m_result = false;
			return;
		}

		m_memoryMap->write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockTest()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkSetMem()
	{
		/*
		if (addressInBitMemory(addr) ||
			addressInBitMemory(addr + sizeW - 1))
		{
			Q_ASSERT(false);			// SETMEM command can't write to bit-addressed memory
			m_result = false;
		}

		writeArea(addr, sizeW);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovBit()
	{
		/*if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{

			// Command 'MOVB %1[%2], %3[%4]' can't write out of application bit- or word-addressed memory.
			//
			m_log->errALC5089(addrTo, bitTo, addrFrom, bitFrom);

			m_result = false;
		}

		//

		read16(addrFrom);
		write16(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkNstart()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkAppStart()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMov32()
	{
		/*read32(addrFrom);
		write32(addrTo);*/
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovConst32()
	{
		// write32(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlock32()
	{
		// read32(addrFrom);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlock32()
	{
		// write32(addrTo);
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockConst32()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockTest32()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkMovConstIfFlag()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkPrevMov()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkPrevMov32()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::checkFill()
	{
		// ??
		Q_ASSERT(false);
		return false;
	}

	bool CodeItem::isWaitingForFbExecution() const
	{
/*		return lmCommands[m_code.getOpCodeInt()].;

		if (lmCommand.waitFbExecution == true)
		{
			int fbType = m_code.getFbType();*/

		return lmCommands[m_code.getOpCodeInt()].waitFbExecution;
	}

	bool CodeItem::generateBinCode(QByteArray* binCode) const
	{
		TEST_PTR_RETURN_FALSE(binCode);

		binCode->clear();

		if (isComment() == true)
		{
			return true;
		}

		E::ByteOrder byteOrder = E::ByteOrder::BigEndian;

		int cmdSizeW = sizeW();

		binCode->resize(cmdSizeW * WORD_SIZE_IN_BYTES);

		CommandCode cmdCode = m_code;

		cmdCode.calcCrc5();

		for(int i = 0; i < cmdSizeW; i++)
		{
			quint16 cmdWord = cmdCode.getWord(i);

			if (byteOrder == E::ByteOrder::LittleEndian)
			{
				// Little Endian byte order
				//
				(*binCode)[i * WORD_SIZE_IN_BYTES] = cmdWord & 0x00FF;
				(*binCode)[i * WORD_SIZE_IN_BYTES + 1] = (cmdWord & 0xFF00) >> 8;
			}
			else
			{
				// Big Endian byte order
				//
				(*binCode)[i * WORD_SIZE_IN_BYTES] = (cmdWord & 0xFF00) >> 8;
				(*binCode)[i * WORD_SIZE_IN_BYTES + 1] = cmdWord & 0x00FF;
			}
		}

		return true;
	}

	QString CodeItem::getAsmCode(bool printCmdCode, int* clockCount) const
	{
		TEST_PTR_RETURN_VALUE(clockCount, QString());

		if (m_isCommand == false)
		{
			// this is a comment
			//
			if (m_comment.isEmpty() == true)
			{
				return QString();
			}
			else
			{
				return QString("\t-- %1").arg(m_comment);
			}
		}

		QString cmdStr;

		// print address of command
		//
		cmdStr = QString("%1\t").arg(m_address, 5, 16, Latin1Char::ZERO).toUpper();

		if (printCmdCode == true)
		{
			// print command code

			for(int w = 0; w < sizeW(); w++)
			{
				QString codeWordStr = getCodeWordStr(w);

				cmdStr += QString("%1 ").arg(codeWordStr).toUpper();
			}

			qsizetype tabLen = 32 - (cmdStr.length() - 1 + 3);

			qsizetype tabCount = tabLen / 8 + ((tabLen % 8) ? 1 : 0);

			for(qsizetype i = 0; i < tabCount; i++)
			{
				cmdStr += "\t";
			}
		}

		Q_ASSERT(m_execTime >= 0);			// check that times already calculated

		*clockCount += m_waitTime + m_execTime;

		char cstr[32];
		snprintf(cstr, 32, "[%02d:%02d %6d]", m_waitTime, m_execTime, *clockCount);
		cmdStr += QString(cstr).leftJustified(16, ' ');

		QString mnemo = mnemoCode();

		cmdStr += mnemo;

		if (m_comment.isEmpty() == false)
		{
			qsizetype tabLen = 72 - 32 - mnemo.length();

			if (tabLen <= 0)
			{
				tabLen += 16;
			}

			qsizetype tabCount = tabLen / 8 + ((tabLen % 8) ? 1 : 0);

			for(int i = 0; i < tabCount; i++)
			{
				cmdStr += "\t";
			}

			cmdStr += QString("-- %1").arg(m_comment);
		}

		return cmdStr;
	}

	QString CodeItem::mnemoCode() const
	{
		int opCodeInt = m_code.getOpCodeInt();

		QString mnemoCode = lmCommands.getMnemo(opCodeInt).leftJustified(10, ' ', false);

		QString params;

		switch(m_code.getOpCode())
		{
		case LmCommand::Code::NoCommand:
		case LmCommand::Code::NOP:
		case LmCommand::Code::STOP:
			break;

		case LmCommand::Code::START:
			params = QString("%1.%2").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt());
			break;

		case LmCommand::Code::MOV:
		case LmCommand::Code::PMOV:
		case LmCommand::Code::MOV32:
		case LmCommand::Code::PMOV32:
			params = QString("%1, %2").
						arg(m_code.getWord2()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::MOVMEM:
			params = QString("%1, %2, %3").
						arg(m_code.getWord2()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommand::Code::MOVC:
			params = QString("%1, #%2").
						arg(m_code.getWord2()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::MOVBC:
			params = QString("%1[%2], #%3").
						arg(m_code.getWord2()).
						arg(m_code.getWord4()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::WRFB:
		case LmCommand::Code::WRFB32:
			params = QString("%1.%2[%3], %4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::RDFB:
		case LmCommand::Code::RDFB32:
			params = QString("%1, %2.%3[%4]").
						arg(m_code.getWord3()).
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt());
			break;

		case LmCommand::Code::WRFBC:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::WRFBB:
			params = QString("%1.%2[%3], %4[%5]").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommand::Code::RDFBB:
			params = QString("%1[%2], %3.%4[%5]").
						arg(m_code.getWord3()).
						arg(m_code.getWord4()).
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt());
			break;

		case LmCommand::Code::RDFBCMP:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::SETMEM:
			params = QString("%1, #%2, %3").
						arg(m_code.getWord2()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;

		case LmCommand::Code::MOVB:
			params = QString("%1[%2], %3[%4]").
						arg(m_code.getWord2()).
						arg(m_code.getBitNo2()).
						arg(m_code.getWord3()).
						arg(m_code.getBitNo1());
			break;

		case LmCommand::Code::NSTART:
			params = QString("%1.%2, %3").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::APPSTART:
			params = QString("%1").
						arg(m_code.getWord2());
			break;

		case LmCommand::Code::MOVC32:
			params = QString("%1, #%2").
						arg(m_code.getWord2()).
						arg(getConstValueString());
			break;

		case LmCommand::Code::WRFBC32:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(getConstValueString());
			break;

		case LmCommand::Code::RDFBCMP32:
			params = QString("%1.%2[%3], #%4").
						arg(m_code.getFbCaption()).
						arg(m_code.getFbInstanceInt()).
						arg(m_code.getFbParamNoInt()).
						arg(getConstValueString());
			break;

		case LmCommand::Code::MOVCMPF:
			params = QString("%1[%2]").
						arg(m_code.getWord2()).
						arg(m_code.getWord3());
			break;

		case LmCommand::Code::FILL:
			params = QString("%1, %2[%3]").
						arg(m_code.getWord2()).
						arg(m_code.getWord3()).
						arg(m_code.getWord4());
			break;


		default:
			Q_ASSERT(false);
		}

		return mnemoCode + params;
	}

	QString CodeItem::getConstValueString() const
	{
		switch(m_code.constDataFormat())
		{
		case E::DataFormat::Float:
			return QString("%1").arg(m_code.getConstFloat());

		case E::DataFormat::SignedInt:
			return QString("%1").arg(m_code.getConstInt32());

		case E::DataFormat::UnsignedInt:
			return QString("%1").arg(m_code.getConstUInt32());

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	bool CodeItem::calcRunTime(const LmMemoryMap* lmMemMap,
							   int prevCmdExecTime,
							   int* waitTime,
							   int* execTime,
							   int* fbExecTime)
	{
		TEST_PTR_RETURN_FALSE(lmMemMap);

		*waitTime = 0;
		*execTime = 0;
		*fbExecTime = 0;

		m_waitTime = 0;
		m_execTime = 0;

		if (lmCommands.contains(m_code.getOpCodeInt()) == false)
		{
			Q_ASSERT(false);			// unknown command code!
			return false;
		}

		const LmCommand& lmCommand = lmCommands[m_code.getOpCodeInt()];

		int cmdReadTime = lmCommand.readTime;

		if (prevCmdExecTime >= cmdReadTime)
		{
			m_waitTime = 0;
		}
		else
		{
			m_waitTime = cmdReadTime - prevCmdExecTime;
		}

		Q_ASSERT(m_waitTime >= 0);

		int cmdExecTime = 0;

		switch(m_code.getOpCode())
		{
		case LmCommand::Code::NoCommand:
			Q_ASSERT(false);
			break;

			// commands with const runtime
			//
		case LmCommand::Code::NOP:
		case LmCommand::Code::STOP:
		case LmCommand::Code::WRFB:
		case LmCommand::Code::RDFB:
		case LmCommand::Code::WRFBC:
		case LmCommand::Code::WRFBB:
		case LmCommand::Code::RDFBCMP:
		case LmCommand::Code::APPSTART:
		case LmCommand::Code::MOV32:
		case LmCommand::Code::MOVC32:
		case LmCommand::Code::WRFB32:
		case LmCommand::Code::RDFB32:
		case LmCommand::Code::WRFBC32:
		case LmCommand::Code::RDFBCMP32:
		case LmCommand::Code::MOVCMPF:
		case LmCommand::Code::PMOV32:
			Q_ASSERT(lmCommand.runTime != LmCommand::CALC_RUNTIME);
			cmdExecTime = lmCommand.runTime;
			break;

			// specific commands START, NSTART
			//
		case LmCommand::Code::START:
			{
				Q_ASSERT(lmCommand.runTime != LmCommand::CALC_RUNTIME);

				cmdExecTime = lmCommand.runTime;

				*fbExecTime = m_fbExecTime;
			}
			break;

		case LmCommand::Code::NSTART:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord3();

				cmdExecTime = 3 + n * 2;

				*fbExecTime = m_fbExecTime * n;
			}
			break;

			// commands with calculated runtime
			//
		case LmCommand::Code::MOV:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 69 : 10;
			break;

		case LmCommand::Code::MOVMEM:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 7 + (n - 1) * 7 + 1;
			}
			break;

		case LmCommand::Code::MOVC:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 66 : 6;
			break;

		case LmCommand::Code::MOVBC:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 7 : 14;
			break;

		case LmCommand::Code::RDFBB:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord3()) == true ? 7 : 12;
			break;

		case LmCommand::Code::SETMEM:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 4 + (n - 1) * 4 + 1;
			}
			break;

		case LmCommand::Code::MOVB:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 10 : 17;
			break;

		case LmCommand::Code::PMOV:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 70 : 10;
			break;

		case LmCommand::Code::FILL:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 69 : 10;
			break;

		default:
			Q_ASSERT(false);								// unknown command code
		}

		m_execTime = cmdExecTime;

		*waitTime = m_waitTime;
		*execTime = m_execTime;

		return true;
	}

	bool CodeItem::getTimes(int* waitTime, int* execTime) const
	{
		TEST_PTR_RETURN_FALSE(waitTime);
		TEST_PTR_RETURN_FALSE(execTime);

		if (m_waitTime == -1 ||
			m_execTime == -1)
		{
			Q_ASSERT(false);			// calcRunTime() must be called before
			return false;
		}

		*waitTime = m_waitTime;
		*execTime = m_execTime;

		return true;
	}

	void CodeItem::initCommand()
	{
		m_isCommand = true;
		m_code.clear();
	}

	QString CodeItem::getCodeWordStr(int wordNo) const
	{
		QString str;

		QByteArray binCode;

		generateBinCode(&binCode);

		if (binCode.count() < (wordNo + 1) * WORD_SIZE_IN_BYTES)
		{
			Q_ASSERT(false);
			return str;
		}

		unsigned int lowByte = binCode[wordNo * 2];
		unsigned int highByte = binCode[wordNo * 2 + 1];

		lowByte &= 0x00FF;
		highByte &= 0x00FF;

		str = QString("%1%2").arg(lowByte, 2, 16, Latin1Char::ZERO).arg(highByte, 2, 16, Latin1Char::ZERO);

		return str;
	}

	bool CodeItem::read16(int /*addrFrom*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->read16(addrFrom);
		}

		return m_result;*/

		return false;
	}

	bool CodeItem::read32(int /*addrFrom*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->read32(addrFrom);
		}

		return m_result;*/

		return false;
	}

	bool CodeItem::readArea(int /*addrFrom*/, int /*sizeW*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->readArea(addrFrom, sizeW);
		}

		return m_result;*/

		return false;
	}

	bool CodeItem::write16(int /*addrTo*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->write16(addrTo);
		}

		return m_result;*/

		return false;
	}

	bool CodeItem::write32(int /*addrTo*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->write32(addrTo);
		}

		return m_result;*/

		return false;
	}

	bool CodeItem::writeArea(int /*addrTo*/, int /*sizeW*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			Q_ASSERT(false);
			m_result = false;
		}
		else
		{
			m_result &= m_memoryMap->writeArea(addrTo, sizeW);
		}

		return m_result;*/

		return false;
	}

	// -----------------------------------------------------------------------------------------------
	//
	// CodeSnippet class implementation
	//
	// -----------------------------------------------------------------------------------------------

	CodeSnippet::CodeSnippet()
	{
	}

	void CodeSnippet::append(const CodeItem& codeItem)
	{
		m_code.emplace_back(codeItem);
	}

	void CodeSnippet::append(const CodeSnippet& codeSnippet)
	{
		m_code.insert(m_code.end(), codeSnippet.m_code.begin(), codeSnippet.m_code.end());
	}

	CodeSnippet& CodeSnippet::operator << (const CodeItem& ci)
	{
		append(ci);
		return *this;
	}

	CodeSnippet& CodeSnippet::operator << (const CodeSnippet& codeShippet)
	{
		append(codeShippet);
		return *this;
	}

	void CodeSnippet::comment(const QString& cmt)
	{
		CodeItem commentItem;

		commentItem.setComment(cmt);

		append(commentItem);
	}

	void CodeSnippet::newLine()
	{
		comment(QString());
	}

	void CodeSnippet::comment_nl(const QString& cmt)
	{
		comment(cmt);
		newLine();
	}

	void CodeSnippet::finalizeByNewLine()
	{
		if (m_code.back().isNewLine() == false)
		{
			newLine();
		}
	}

	void CodeSnippet::clear()
	{
		m_code.clear();
	}

	void CodeSnippet::reserve(int size)
	{
		m_code.reserve(size);
	}

	bool CodeSnippet::isEmpty() const
	{
		return m_code.empty();
	}

	int CodeSnippet::itemsCount() const
	{
		return static_cast<int>(m_code.size());
	}

	void CodeSnippet::getAsmCode(QStringList* asmCode) const
	{
		TEST_PTR_RETURN(asmCode);

		asmCode->clear();

		int clockCount = 0;

		for(const CodeItem& codeItem : m_code)
		{
			QString str = codeItem.getAsmCode(true, &clockCount);

			asmCode->append(str);

			if (codeItem.getOpcode() == LmCommand::Code::STOP)
			{
				clockCount = 0;
			}
		}
	}

	void CodeSnippet::getBinCode(QByteArray* binCode) const
	{
		TEST_PTR_RETURN(binCode);

		binCode->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QByteArray cmdBinCode;

			codeItem.generateBinCode(&cmdBinCode);

			binCode->append(cmdBinCode);
		}
	}

	void CodeSnippet::getMifCode(QStringList* mifCode) const
	{
		TEST_PTR_RETURN(mifCode);

		mifCode->clear();

		if (m_code.size() < 1)
		{
			return;
		}

		int width = 16;
		int depth = 0;

		// find last command for compute address depth
		//
		qsizetype codeItemsCount = m_code.size();

		for(qsizetype i = codeItemsCount - 1; i >= 0; i--)
		{
			if (m_code[i].isComment() == true)
			{
				continue;
			}

			depth = m_code[i].address() + m_code[i].sizeW() - 1;
			break;
		}

		mifCode->append(QString("WIDTH = %1;").arg(width));
		mifCode->append(QString("DEPTH = %1;").arg(depth + 1));

		mifCode->append("");

		mifCode->append("ADDRESS_RADIX = HEX;");
		mifCode->append("DATA_RADIX = HEX;");

		mifCode->append("");

		mifCode->append("CONTENT");
		mifCode->append("BEGIN");

		QString codeStr;
		QString str;

		for(const CodeItem& codeItem : m_code)
		{
			if (codeItem.isComment() == true)
			{
				if (codeItem.comment().isEmpty() == true)
				{
					str.clear();
				}
				else
				{
					str = QString("\t-- %1").arg(codeItem.comment());
				}

				mifCode->append(str);

				continue;
			}

			QByteArray binCode;

			codeItem.generateBinCode(&binCode);

			Q_ASSERT((binCode.count() % 2) == 0);

			qsizetype bytesCount = binCode.count();

			for(qsizetype i = 0; i < bytesCount; i++)
			{
				if (i == 0)
				{
					str = QString("\t%1 : ").arg(codeItem.address(), 4, 16, Latin1Char::ZERO);
					codeStr = str;
				}

				unsigned int b = binCode[i];

				b &= 0xFF;

				if ((i % 2) == 1)
				{
					if (i == bytesCount-1)
					{
						str = QString("%1;").arg(b, 2, 16, Latin1Char::ZERO);
					}
					else
					{
						str = QString("%1 ").arg(b, 2, 16, Latin1Char::ZERO);
					}
				}
				else
				{
					str = QString("%1").arg(b, 2, 16, Latin1Char::ZERO);
				}

				codeStr += str;
			}

			qsizetype tabLen = 40 - (codeStr.length() - 1 + 8);
			qsizetype tabCount = tabLen / 8 + ((tabLen % 8) ? 1 : 0);

			for(qsizetype i = 0; i < tabCount; i++)
			{
				codeStr += "\t";
			}

			str = QString("-- %1").arg(codeItem.mnemoCode());

			codeStr += str;

			mifCode->append(codeStr);
		}

		mifCode->append("END;");
	}

	void CodeSnippet::getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const
	{
		TEST_PTR_RETURN(metadataFields);
		TEST_PTR_RETURN(metadataVersion);

		const int ASM_METADATA_VERSION = 1;
		*metadataVersion = ASM_METADATA_VERSION;

		metadataFields->clear();

		metadataFields->append("IsCommand");
		metadataFields->append("Address");
		metadataFields->append("BinCode");
		metadataFields->append("MnemoCode");
		metadataFields->append("Comment");
	}

	void CodeSnippet::getAsmMetadata(std::vector<QVariantList>* metadata) const
	{
		TEST_PTR_RETURN(metadata);

		metadata->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QVariantList data;

			bool isCommand = false;
			QString address;
			QString binCode;
			QString mnemoCode;
			QString comment;

			if (codeItem.isCommand() == true)
			{
				isCommand = true;

				address = QString("%1").arg(codeItem.address(), 4, 16, Latin1Char::ZERO);

				QByteArray cmdBinCode;

				codeItem.generateBinCode(&cmdBinCode);

				binCode = QString(cmdBinCode.toHex()).toUpper();
				mnemoCode = codeItem.mnemoCode();
				comment = codeItem.comment();
			}
			else
			{
				isCommand = false;

				comment = codeItem.comment();

				if (comment.isEmpty() == true)
				{
					continue;			// skip empty strings
				}
			}

			data.append(QVariant(isCommand));
			data.append(QVariant(address));
			data.append(QVariant(binCode));
			data.append(QVariant(mnemoCode));
			data.append(QVariant(comment));

			metadata->push_back(data);
		}
	}

	AppLogicCode::AppLogicCode(Type type) :
		m_codeType(type)
	{
	}

	void AppLogicCode::setMemoryMapAndLogger(const LmMemoryMap* lmMemory, IssueLogger* log)
	{
		TEST_PTR_RETURN(lmMemory);
		TEST_PTR_RETURN(log);

		m_lmMemoryMap = lmMemory;
		m_log = log;
	}

	void AppLogicCode::setAppStartAddr(int addr)
	{
		Q_ASSERT(m_codeType == Type::IDR_Code);

		for(CodeItem& codeItem : m_code)
		{
			if (codeItem.getOpcode() == LmCommand::Code::APPSTART)
			{
				codeItem.appStart(addr);
				return;
			}
		}

		Q_ASSERT(false);
	}

	void AppLogicCode::finalize(const LmDescription& lmDesc)
	{
		m_codeSizeW = 0;
		m_clockCount = 0;
		m_commandsCount = 0;

		m_lmCodeMemUsage = 0;
		m_execTimeMcs = 0;
		m_lmCycleTimeUsage = 0;

		if (m_code.empty() == true)
		{
			return;
		}

		// read commands and calculate code runtime
		//
		int prevCmdExecTime = 0;
		int waitTime = 0;
		int execTime = 0;
		int fbExecTime = 0;
		int waitFbTime = 0;

		for(CodeItem& codeItem : m_code)
		{
			if (codeItem.isCommand() == false)
			{
				continue;
			}

			m_commandsCount++;

			codeItem.setAddress(m_codeSizeW);

			m_codeSizeW += codeItem.sizeW();

			//

			if (codeItem.isWaitingForFbExecution() == true)
			{
				waitFbTime = getFbRemainingExecTime(codeItem.getFbType());
			}

			prevCmdExecTime = std::max(prevCmdExecTime, waitFbTime);

			codeItem.calcRunTime(m_lmMemoryMap, prevCmdExecTime,
								 &waitTime, &execTime, &fbExecTime);

			m_clockCount += (waitTime + execTime);		// !!!

			decFbExecTime(waitTime + execTime);

			prevCmdExecTime = execTime;

			if (fbExecTime != 0)
			{
				Q_ASSERT(codeItem.getOpcode() == LmCommand::Code::START ||
						 codeItem.getOpcode() == LmCommand::Code::NSTART);

				startFbExec(codeItem.getFbType(), fbExecTime);
			}

			//

			if (codeItem.getOpcode() == LmCommand::Code::STOP)
			{
				int maxTime = getMaxFbRemainingExecTimeAndClear();

				prevCmdExecTime = std::max(prevCmdExecTime, maxTime);

				codeItem.addExecTime(prevCmdExecTime);
				m_clockCount += prevCmdExecTime;
				prevCmdExecTime = 0;
			}
		}

		if (lmDesc.memory().m_codeMemorySize != 0)
		{
			m_lmCodeMemUsage = static_cast<double>(m_codeSizeW * 100) /
								static_cast<double>(lmDesc.memory().m_codeMemorySize);
		}

		m_execTimeMcs = m_clockCount * lmDesc.logicUnit().clockTimeSecs() * 1000000.0;

		double totalTimeMcs = 0;

		switch(m_codeType)
		{
		case Type::IDR_Code:
			totalTimeMcs = lmDesc.logicUnit().m_idrPhaseTime;
			break;

		case Type::ALP_Code:
			totalTimeMcs = lmDesc.logicUnit().m_alpPhaseTime;
			break;

		case Type::AllCode:
			totalTimeMcs =	lmDesc.logicUnit().m_idrPhaseTime +
							lmDesc.logicUnit().m_alpPhaseTime;
			break;

		default:
			Q_ASSERT(false);
			return;
		}

		if (totalTimeMcs !=  0)
		{
			m_lmCycleTimeUsage = static_cast<double>(m_execTimeMcs * 100) / totalTimeMcs;
		}
	}

	void AppLogicCode::clear()
	{
		CodeSnippet::clear();

		m_lmMemoryMap = nullptr;
		m_log = nullptr;

		m_runningAfbs.clear();

		m_codeSizeW = -1;
		m_clockCount = -1;
		m_commandsCount = -1;

		m_lmCodeMemUsage = 0;
		m_execTimeMcs = 0;
		m_lmCycleTimeUsage = 0;
	}

	AppLogicCode::Type AppLogicCode::codeType() const
	{
		return m_codeType;
	}

	int AppLogicCode::codeSizeW() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_codeSizeW;
	}

	int AppLogicCode::clockCount() const
	{
		Q_ASSERT(m_clockCount != -1);		// finalize() should be called first
		return m_clockCount;
	}

	int AppLogicCode::commandsCount() const
	{
		Q_ASSERT(m_commandsCount != -1);	// finalize() should be called first
		return m_commandsCount;
	}

	double AppLogicCode::lmCodeMemoryUsage() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_lmCodeMemUsage;
	}

	double AppLogicCode::execTimeMcs() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_execTimeMcs;
	}

	double AppLogicCode::lmCycleTimeUsage() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_lmCycleTimeUsage;
	}

	bool AppLogicCode::getCommandsStatistics(std::vector<CommandStatistics>* stat) const
	{
		TEST_PTR_RETURN_FALSE(stat);

		stat->clear();

		std::map<LmCommand::Code, CommandStatistics> statMap;

		for(auto const& p : lmCommands)
		{
			const LmCommand& lmc = p.second;

			if (lmc.code == LmCommand::Code::NoCommand)
			{
				continue;
			}

			statMap.insert({lmc.code, CommandStatistics(lmc.code) });
		}

		for(const CodeItem& ci : m_code)
		{
			if (ci.isCommand() == false)
			{
				continue;
			}

			auto it = statMap.find(ci.getOpcode());

			if (it == statMap.end())
			{
				Q_ASSERT(false);
				continue;
			}

			CommandStatistics& cs = it->second;

			cs.usedCount++;
			cs.codeSizeW += ci.sizeW();
			cs.execTime += ci.waitTime() + ci.execTime();
		}

		stat->reserve(statMap.size());

		for(auto const& p : statMap)
		{
			stat->emplace_back(p.second);
		}

		return true;
	}

	int AppLogicCode::startFbExec(int fbOpCode, int fbRuntime)
	{
		int waitTime = 0;

		auto it = m_runningAfbs.find(fbOpCode);

		if (it == m_runningAfbs.end())
		{
			// FB with fbOpCode is NOT running now!
			//
			m_runningAfbs.insert({fbOpCode, fbRuntime});
			waitTime = 0;
		}
		else
		{
			Q_ASSERT(it->second > 0);

			// FB with fbOpCode is running now!
			//
			waitTime = it->second;			// fb remaining exec time
			it->second = fbRuntime;
		}

		return waitTime;
	}

	void AppLogicCode::decFbExecTime(int time)
	{
		if (m_runningAfbs.empty() == true)
		{
			return;
		}

		std::map<int, int> stillRunningAfbs;

		for(auto& p : m_runningAfbs)
		{
			Q_ASSERT(p.second > 0);

			p.second -= time;

			if (p.second > 0)
			{
				stillRunningAfbs.emplace(p);
			}
		}

		m_runningAfbs.swap(stillRunningAfbs);
	}

	int AppLogicCode::getFbRemainingExecTime(int fbOpCode)
	{
		int remainingTime = 0;

		auto it = m_runningAfbs.find(fbOpCode);

		if (it != m_runningAfbs.end())
		{
			remainingTime = it->second;
			m_runningAfbs.erase(it);
		}

		return remainingTime;
	}

	int AppLogicCode::getMaxFbRemainingExecTimeAndClear()
	{
		int maxTime = 0;

		for(auto& p : m_runningAfbs)
		{
			if (p.second > maxTime)
			{
				maxTime = p.second;
			}
		}

		m_runningAfbs.clear();

		return maxTime;
	}

	// -----------------------------------------------------------------------------------------------
	//
	// CodeSnippetMetrics struct implementation
	//
	// -----------------------------------------------------------------------------------------------

	void CodeSnippetMetrics::setEndAddr(int endAddr)
	{
		m_endAddr = endAddr;

		Q_ASSERT(false);			// wrong calc !~!! 65536.0!!!!
		m_codePercent = static_cast<double>(m_endAddr - m_startAddr) * 100.0 / 65536.0 ;
	}


	QString CodeSnippetMetrics::codePercentStr() const
	{
		return QString("%1%%").arg(m_codePercent, 0, 'g', 2);
	}

}
