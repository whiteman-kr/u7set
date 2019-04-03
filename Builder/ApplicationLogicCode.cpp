#include "ApplicationLogicCode.h"
#include "../VFrame30/Afb.h"
#include "../lib/WUtils.h"

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
				assert(false);			// duplicate command code
				continue;
			}

			insert(TO_INT(lmCommand.code), lmCommand);
		}
	}

	bool LmCommands::isValidCode(LmCommand::Code commandCode)
	{
		return isValidCode(TO_INT(commandCode));
	}

	bool LmCommands::isValidCode(int commandCode)
	{
		bool codeExists = contains(commandCode);

		if (codeExists == false)
		{
			assert(false);
		}

		return codeExists;
	}

	int LmCommands::getSizeW(LmCommand::Code commandCode)
	{
		return getSizeW(TO_INT(commandCode));
	}

	int LmCommands::getSizeW(int commandCode)
	{
		if (isValidCode(commandCode) == false)
		{
			return 0;
		}

		return value(commandCode).sizeW;
	}

	QString LmCommands::getMnemo(LmCommand::Code commandCode)
	{
		return getMnemo(TO_INT(commandCode));
	}

	QString LmCommands::getMnemo(int commandCode)
	{
		if (isValidCode(commandCode) == false)
		{
			return "";
		}

		return QString(value(commandCode).mnemo);
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
		if (fbType > LmCommand::MAX_FB_TYPE)
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
		param.fbInstance = fbInstance;
	}

	void CommandCode::setFbParamNo(quint16 fbParamNo)
	{
		if (fbParamNo > LmCommand::MAX_FB_PARAM_NO)
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
		if (bitNo > LmCommand::MAX_BIT_NO_16)
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
		if (bitNo > LmCommand::MAX_BIT_NO_16)
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
		if (bitNo > LmCommand::MAX_BIT_NO_16)
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

		const quint64 UPPER_BIT = 0x8000000000000000ll;

		const quint64 POLYNOM = 0x05ll << BIT_COUNT;

		quint64 crc5  =  0x1fll << BIT_COUNT;

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

/*	CodeItem::CodeItem()
	{
	}


	CodeItem::CodeItem(const CodeItem& codeItem)
	{
		m_comment = codeItem.m_comment;
		m_binCode = codeItem.m_binCode;
	}*


	// ---------------------------------------------------------------------------------------
	//
	// Comment class implementation
	//
	// ---------------------------------------------------------------------------------------

/*	Comment::Comment()
	{
	}


	Comment::Comment(const Comment& comment) :
		CodeItem(comment)
	{
	}


	Comment::Comment(const QString& comment)
	{
		setComment(comment);
	}


	QString Comment::toString()
	{
		QString comment = getComment();

		if (comment.isEmpty())
		{
			return "";
		}

		return QString("\t-- %1").arg(getComment());
	}*/


	// ---------------------------------------------------------------------------------------
	//
	// CodeItem class implementation
	//
	// ---------------------------------------------------------------------------------------

//	QHash<int, const LmCommand*> CodeItem::m_lmCommands;
	QHash<quint16, int> CodeItem::m_executedFb;

	qint32 CodeItem::m_codeItemsNumerator = 0;

	CodeItem::CodeItem()
	{
		m_numerator = m_codeItemsNumerator;
		m_codeItemsNumerator++;
	}

	void CodeItem::nop()
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::NOP);
	}

	void CodeItem::start(quint16 fbType, quint16 fbInstance, const QString& fbCaption, int fbRunTime)
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
			assert(false);		// fbRunTime can't be 0
		}
	}

	void CodeItem::stop()
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::STOP);
	}


	void CodeItem::mov(quint16 addrTo, quint16 addrFrom)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::mov(Address16 addrTo, Address16 addrFrom)
	{
		assert(addrTo.bit() == 0);
		assert(addrFrom.bit() == 0);

		mov(addrTo.offset(), addrFrom.offset());
	}

	void CodeItem::movMem(quint16 addrTo, quint16 addrFrom, quint16 sizeW)
	{
		initCommand();

		m_result = true;

		assert(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::MOVMEM);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(sizeW);
	}

	void CodeItem::movMem(Address16 addrTo, Address16 addrFrom, quint16 sizeW)
	{
		assert(addrTo.bit() == 0);
		assert(addrFrom.bit() == 0);

		movMem(addrTo.offset(), addrFrom.offset(), sizeW);
	}

	void CodeItem::movConst(quint16 addrTo, quint16 constVal)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constVal);
	}

	void CodeItem::movBitConst(quint16 addrTo, quint16 bitNo, quint16 constBit)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVBC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constBit);
		m_code.setBitNo(bitNo);
	}

	void CodeItem::movBitConst(Address16 addr16, quint16 constBit)
	{
		assert(addr16.isValid() == true);

		movBitConst(addr16.offset(), addr16.bit(), constBit);
	}

	void CodeItem::writeFuncBlock(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::readFuncBlock(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFB);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlockConst(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 constVal, const QString& fbCaption)
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


	void CodeItem::writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, quint16 bitNo, const QString& fbCaption)
	{
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

	void CodeItem::writeFuncBlockBit(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, Address16 addrFrom, const QString& fbCaption)
	{
		writeFuncBlockBit(fbType, fbInstance, fbParamNo, addrFrom.offset(), addrFrom.bit(), fbCaption);
	}

	void CodeItem::readFuncBlockBit(quint16 addrTo, quint16 bitNo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
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

	void CodeItem::readFuncBlockBit(Address16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		readFuncBlockBit(addrTo.offset(), addrTo.bit(), fbType, fbInstance, fbParamNo, fbCaption);
	}

	void CodeItem::readFuncBlockCompare(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 testValue, const QString& fbCaption)
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

	void CodeItem::setMem(quint16 addr, quint16 constValue, quint16 sizeW)
	{
		initCommand();

		m_result = true;

		assert(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::SETMEM);
		m_code.setWord2(addr);
		m_code.setWord3(constValue);
		m_code.setWord4(sizeW);
	}


	void CodeItem::movBit(quint16 addrTo, quint16 bitTo, quint16 addrFrom, quint16 bitFrom)
	{
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
		assert(addrTo.isValid() == true);
		assert(addrFrom.isValid() == true);

		movBit(addrTo.offset(), addrTo.bit(), addrFrom.offset(), addrFrom.bit());
	}

	void CodeItem::nstart(quint16 fbType, quint16 fbInstance, quint16 startCount, const QString& fbCaption, int fbRunTime)
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
			assert(false);		// fbRunTime can't be 0
		}
	}

	void CodeItem::appStart(quint16 appStartAddr)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::APPSTART);
		m_code.setWord2(appStartAddr);
	}


	void CodeItem::mov32(quint16 addrTo, quint16 addrFrom)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::mov32(Address16 addrTo, Address16 addrFrom)
	{
		assert(addrTo.bit() == 0);
		assert(addrFrom.bit() == 0);

		mov32(addrTo.offset(), addrFrom.offset());
	}

	void CodeItem::movConstInt32(quint16 addrTo, qint32 constInt32)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstInt32(constInt32);
	}

	void CodeItem::movConstUInt32(quint16 addrTo, quint32 constUInt32)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constUInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constUInt32 & 0xFFFF);
		m_code.setConstUInt32(constUInt32);
	}

	void CodeItem::movConstFloat(quint16 addrTo, float constFloat)
	{
		initCommand();

		m_result = true;

		qint32 constInt32 = *reinterpret_cast<qint32*>(&constFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstFloat(constFloat);
	}

	void CodeItem::writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, quint16 addrFrom, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::writeFuncBlock32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, Address16 addrFrom, const QString& fbCaption)
	{
		assert(addrFrom.bit() == 0);

		writeFuncBlock32(fbType, fbInstance, fbParamNo, addrFrom.offset(), fbCaption);
	}

	void CodeItem::readFuncBlock32(quint16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFB32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setFbCaption(fbCaption);
	}

	void CodeItem::readFuncBlock32(Address16 addrTo, quint16 fbType, quint16 fbInstance, quint16 fbParamNo, const QString& fbCaption)
	{
		assert(addrTo.bit() == 0);

		readFuncBlock32(addrTo.offset(), fbType, fbInstance, fbParamNo, fbCaption);
	}

	void CodeItem::writeFuncBlockConstInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 constInt32, const QString& fbCaption)
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

	void CodeItem::writeFuncBlockConstFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float constFloat, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		qint32 constInt32 = *reinterpret_cast<qint32*>(&constFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::WRFBC32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(constFloat);
	}

	void CodeItem::readFuncBlockCompareInt32(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, qint32 testInt32, const QString& fbCaption)
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

	void CodeItem::readFuncBlockCompareFloat(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, float testFloat, const QString& fbCaption)
	{
		initCommand();

		m_result = true;

		qint32 testInt32 = *reinterpret_cast<qint32*>(&testFloat);		// map binary code of float to qint32

		m_code.setOpCode(LmCommand::Code::RDFBCMP32);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);
		m_code.setFbCaption(fbCaption);
		m_code.setConstFloat(testFloat);
	}

	void CodeItem::movCompareFlag(quint16 addrTo, quint16 bitNo)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVCMPF);
		m_code.setWord2(addrTo);
		m_code.setWord3(bitNo);
	}

	void CodeItem::prevMov(quint16 addrTo, quint16 addrFrom)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::prevMov32(quint16 addrTo, quint16 addrFrom)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
	}

	void CodeItem::fill(quint16 addrTo, quint16 addrFrom, quint16 addrBit)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::FILL);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(addrBit);
	}

	void CodeItem::fill(Address16 addrTo, Address16 addrFrom)
	{
		assert(addrTo.isValid() == true);
		assert(addrFrom.isValid() == true);
		assert(addrTo.bit() == 0);

		fill(addrTo.offset(), addrFrom.offset(), addrFrom.bit());
	}

	bool CodeItem::checkNop()
	{
		return true;
	}

	bool CodeItem::checkStart()
	{
		// need check fbType and fbInstance
		assert(false);
		return false;
	}

	bool CodeItem::checkStop()
	{
		// need check fbType and fbInstance
		assert(false);
		return false;
	}

	bool CodeItem::checkMov()
	{
		// chek addresses

		//read16(addrFrom);
		//write16(addrTo);
		assert(false);
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
		assert(false);
		return false;
	}

	bool CodeItem::checkMovConst()
	{
		//write16(addrTo);
		assert(false);
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
		assert(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlock()
	{
		//	read16(addrFrom);
		assert(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlock()
	{
		// write16(addrTo);
		assert(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockConst()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockBit()
	{
		// read16(addrFrom);
		assert(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockBit()
	{
		/*
		if (addressInBitMemory(addrTo) == false &&
			addressInWordMemory(addrTo) == false)
		{
			assert(false);			// RDFBB command can write only in bit- or word-addressed memory
			m_result = false;
			return;
		}

		m_memoryMap->write16(addrTo);*/
		assert(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockTest()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkSetMem()
	{
		/*
		if (addressInBitMemory(addr) ||
			addressInBitMemory(addr + sizeW - 1))
		{
			assert(false);			// SETMEM command can't write to bit-addressed memory
			m_result = false;
		}

		writeArea(addr, sizeW);*/
		assert(false);
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
		assert(false);
		return false;
	}

	bool CodeItem::checkNstart()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkAppStart()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkMov32()
	{
		/*read32(addrFrom);
		write32(addrTo);*/
		assert(false);
		return false;
	}

	bool CodeItem::checkMovConst32()
	{
		// write32(addrTo);
		assert(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlock32()
	{
		// read32(addrFrom);
		assert(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlock32()
	{
		// write32(addrTo);
		assert(false);
		return false;
	}

	bool CodeItem::checkWriteFuncBlockConst32()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkReadFuncBlockTest32()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkMovConstIfFlag()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkPrevMov()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkPrevMov32()
	{
		// ??
		assert(false);
		return false;
	}

	bool CodeItem::checkFill()
	{
		// ??
		assert(false);
		return false;
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

	QString CodeItem::getAsmCode(bool printCmdCode) const
	{
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
		cmdStr.sprintf("%05X\t", m_address);

		if (printCmdCode == true)
		{
			// print command code

			for(int w = 0; w < sizeW(); w++)
			{
				QString codeWordStr = getCodeWordStr(w);

				cmdStr += QString("%1 ").arg(codeWordStr);
			}

			int tabLen = 32 - (cmdStr.length() - 1 + 3);

			int tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

			for(int i = 0; i < tabCount; i++)
			{
				cmdStr += "\t";
			}
		}

//		QString str;
//		Execution times printing
//		Commented while refactoring!!!
//
//		assert(m_execTime != 0);			// check that times already calculated
//		str.sprintf("[%02d:%02d]", m_waitTime, m_execTime);
//		str = str.leftJustified(12, ' ');
//		cmdStr += str;

		QString mnemo = mnemoCode();

		cmdStr += mnemo;

		if (m_comment.isEmpty() == false)
		{
			int tabLen = 72 - 32 - mnemo.length();

			if (tabLen <= 0)
			{
				tabLen += 16;
			}

			int tabCount = tabLen / 8 + (tabLen % 8 ? 1 : 0);

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
			assert(false);
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
			assert(false);
		}

		return QString();
	}

	bool CodeItem::getTimes(const LmMemoryMap* lmMemMap, int prevCmdExecTime, int* waitTime, int* execTime) const
	{
		TEST_PTR_RETURN_FALSE(lmMemMap);
		TEST_PTR_RETURN_FALSE(waitTime);
		TEST_PTR_RETURN_FALSE(execTime);

		*waitTime = 0;
		*execTime = 0;

		if (lmCommands.contains(m_code.getOpCodeInt()) == false)
		{
			assert(false);			// unknown command code!
			return false;
		}

		const LmCommand& lmCommand = lmCommands[m_code.getOpCodeInt()];

		int cmdReadTime = lmCommand.readTime;

		if (prevCmdExecTime > cmdReadTime)
		{
			*waitTime = prevCmdExecTime - cmdReadTime;

			decFbExecTime(prevCmdExecTime);
		}
		else
		{
			*waitTime = cmdReadTime - prevCmdExecTime;

			decFbExecTime(cmdReadTime);
		}

		assert(*waitTime >= 0);

		if (lmCommand.waitFbExecution == true)
		{
			int fbType = m_code.getFbType();

			*waitTime += getFbRemainingExecTime(fbType);
		}

		int cmdExecTime = 0;

		switch(m_code.getOpCode())
		{
		case LmCommand::Code::NoCommand:
			assert(false);
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
		case LmCommand::Code::FILL:
			assert(lmCommand.runTime != LmCommand::CALC_RUNTIME);
			cmdExecTime = lmCommand.runTime;
			break;

			// specific commands START, NSTART
			//
		case LmCommand::Code::START:
			{
				assert(lmCommand.runTime != LmCommand::CALC_RUNTIME);

				cmdExecTime = lmCommand.runTime;

				startFbExec(m_code.getFbType(), m_fbExecTime);
			}
			break;

		case LmCommand::Code::NSTART:
			{
				assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord3();

				cmdExecTime = 3 + n * 2;

				startFbExec(m_code.getFbType(), m_fbExecTime * n);
			}
			break;

			// commands with calculated runtime
			//
		case LmCommand::Code::MOV:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 53 : 8;
			break;

		case LmCommand::Code::MOVMEM:
			{
				assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				assert(n > 0);

				cmdExecTime = 7 + (n - 1) * 6 + 1;
			}
			break;

		case LmCommand::Code::MOVC:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 50 : 5;
			break;

		case LmCommand::Code::MOVBC:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 5 : 10;
			break;

		case LmCommand::Code::RDFBB:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord3()) == true ? 7 : 9;
			break;

		case LmCommand::Code::SETMEM:
			{
				assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				assert(n > 0);

				cmdExecTime = 4 + (n - 1) * 3 + 1;
			}
			break;

		case LmCommand::Code::MOVB:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 9 : 13;
			break;

		case LmCommand::Code::PMOV:
			assert(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = lmMemMap->addressInBitMemory(m_code.getWord2()) == true ? 8 : 53;
			break;

		default:
			assert(false);								// unknown command code
		}

		*execTime = cmdExecTime;

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
			assert(false);
			return str;
		}

		unsigned int lowByte = binCode[wordNo * 2];
		unsigned int highByte = binCode[wordNo * 2 + 1];

		lowByte &= 0x00FF;
		highByte &= 0x00FF;

		str.sprintf("%02X%02X", lowByte, highByte);

		return str;
	}

	int CodeItem::startFbExec(quint16 fbType, int fbRuntime)
	{
		int waitTime = 0;

		int fbRemainingExecTime = m_executedFb.value(fbType, -1);

		if (fbRemainingExecTime != -1)
		{
			// FB of fbType is executed now!
			//
			waitTime = fbRemainingExecTime;
		}
		else
		{
			// FB of fbType is NOT executed now!
			//
			waitTime = 0;
		}

		// add FB to exec map
		//
		m_executedFb.insert(fbType, fbRuntime);

		return waitTime;
	}

	void CodeItem::decFbExecTime(int time)
	{
		QHash<quint16, int>::iterator i = m_executedFb.begin();

		while(i != m_executedFb.end())
		{
			int remainingFbExecTime = i.value() - time;

			if (remainingFbExecTime > 0)
			{
				// update FB remaining exec time
				//
				m_executedFb.insert(i.key(), remainingFbExecTime);

				i++;		// move to next elem
			}
			else
			{
				// remove FB from map
				//
				i = m_executedFb.erase(i);
			}
		}
	}

	int CodeItem::getFbRemainingExecTime(quint16 fbType)
	{
		return m_executedFb.value(fbType, 0);
	}

	bool CodeItem::read16(int /*addrFrom*/)
	{
/*		if (m_memoryMap == nullptr)
		{
			assert(false);
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
			assert(false);
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
			assert(false);
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
			assert(false);
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
			assert(false);
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
			assert(false);
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

	void CodeSnippet::init(CodeSnippetMetrics* codeFragmentMetrics)
	{
		if (codeFragmentMetrics == nullptr)
		{
			assert(false);
			return;
		}

		//codeFragmentMetrics->setStartAddr(m_commandAddress);
	}

	void CodeSnippet::calculate(CodeSnippetMetrics* codeFragmentMetrics)
	{
		if (codeFragmentMetrics == nullptr)
		{
			assert(false);
			return;
		}

		//codeFragmentMetrics->setEndAddr(m_commandAddress);
	}

	int CodeSnippet::sizeW() const
	{
		int sizeW = 0;

		for(const CodeItem& codeItem : *this)
		{
			sizeW += codeItem.sizeW();
		}

		return sizeW;
	}

	// -----------------------------------------------------------------------------------------------
	//
	// CodeSnippetMetrics struct implementation
	//
	// -----------------------------------------------------------------------------------------------

	void CodeSnippetMetrics::setEndAddr(int endAddr)
	{
		m_endAddr = endAddr;

		m_codePercent = static_cast<double>(m_endAddr - m_startAddr) * 100.0 / 65536.0 ;
	}


	QString CodeSnippetMetrics::codePercentStr() const
	{
		QString str;

		return str.sprintf("%.2f%%", static_cast<float>(m_codePercent));
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
	}

	void ApplicationLogicCode::setMemoryMap(LmMemoryMap* lmMemory, IssueLogger* log)
	{
		TEST_PTR_RETURN(lmMemory);
		TEST_PTR_RETURN(log);

		m_lmMemoryMap = lmMemory;
	}

	void ApplicationLogicCode::clear()
	{
		m_codeItems.clear();
		m_commandAddress = 0;
	}

	void ApplicationLogicCode::append(const CodeItem& codeItem)
	{
		m_codeItems.append(codeItem);

		int lastIndex = m_codeItems.size() - 1;

		m_codeItems[lastIndex].setAddress(m_commandAddress);

		m_commandAddress += m_codeItems[lastIndex].sizeW();
	}

	void ApplicationLogicCode::append(const CodeSnippet& codeSnippet)
	{
		for(const CodeItem& codeItem : codeSnippet)
		{
			append(codeItem);
		}
	}

	void ApplicationLogicCode::comment(const QString& str)
	{
		CodeItem comment;

		comment.setComment(str);

		append(comment);
	}

	void ApplicationLogicCode::newLine()
	{
		CodeItem emptyComment;

		append(emptyComment);
	}

/*	void ApplicationLogicCode::replaceAt(int commandIndex, const Command &cmd)
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


	void ApplicationLogicCode::comment(const QString& commentStr)
	{
		CodeItem comment;

		comment.setComment(commentStr);

		m_codeItems.append(comment);
	}


	void ApplicationLogicCode::newLine()
	{
		CodeItem emptyComment;

		m_codeItems.append(emptyComment);
	} */


	/*void ApplicationLogicCode::generateBinCode()
	{
		for(const CodeItem& codeItem : m_codeItems)
		{
			codeItem.generateBinCode(m_byteOrder);
		}
	}*/

	void ApplicationLogicCode::getAsmCode(QStringList* asmCode) const
	{
		TEST_PTR_RETURN(asmCode);

		asmCode->clear();

		for(const CodeItem& codeItem : m_codeItems)
		{
			QString str = codeItem.getAsmCode(true);

			asmCode->append(str);
		}
	}

	void ApplicationLogicCode::getBinCode(QByteArray* binCode) const
	{
		TEST_PTR_RETURN(binCode);

		binCode->clear();

		for(const CodeItem& codeItem : m_codeItems)
		{
			QByteArray cmdBinCode;

			codeItem.generateBinCode(&cmdBinCode);

			binCode->append(cmdBinCode);
		}
	}

	void ApplicationLogicCode::getMifCode(QStringList* mifCode) const
	{
		TEST_PTR_RETURN(mifCode);

		mifCode->clear();

		if (m_codeItems.count() < 1)
		{
			return;
		}

		int width = 16;
		int depth = 0;

		// find last command for compute address depth
		//
		int codeItemsCount = m_codeItems.count();

		for(int i = codeItemsCount - 1; i >= 0; i--)
		{
			if (m_codeItems[i].isComment() == true)
			{
				continue;
			}

			depth = m_codeItems[i].address() + m_codeItems[i].sizeW() - 1;
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

		for(const CodeItem& codeItem : m_codeItems)
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

			assert((binCode.count() % 2) == 0);

			int bytesCount = binCode.count();

			for(int i = 0; i < bytesCount; i++)
			{
				if (i == 0)
				{
					str.sprintf("\t%04X : ", codeItem.address());
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

			str = QString("-- %1").arg(codeItem.mnemoCode());

			codeStr += str;

			mifCode->append(codeStr);
		}

		mifCode->append("END;");
	}

	void ApplicationLogicCode::getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const
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

	void ApplicationLogicCode::getAsmMetadata(std::vector<QVariantList>* metadata) const
	{
		TEST_PTR_RETURN(metadata);

		metadata->clear();

		for(const CodeItem& codeItem : m_codeItems)
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

				address = QString().sprintf("%04X", codeItem.address());

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

	bool ApplicationLogicCode::getRunTimes(int* idrPhaseClockCount, int* alpPhaseClockCount) const
	{
		TEST_PTR_RETURN_FALSE(idrPhaseClockCount);
		TEST_PTR_RETURN_FALSE(alpPhaseClockCount);

		*idrPhaseClockCount = 0;
		*alpPhaseClockCount = 0;

		if (m_codeItems.isEmpty() == true)
		{
			return true;
		}

		// find appStart command and read application logic processing code start address
		//
		int appLogicProcessingCodeStartAddress = -1;

		for(const CodeItem& codeItem : m_codeItems)
		{
			if (codeItem.isCommand() == false)
			{
				continue;
			}

			if (codeItem.isOpCode(LmCommand::Code::APPSTART))
			{
				appLogicProcessingCodeStartAddress = codeItem.getWord2();
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

		int prevCmdExecTime = 0;

		for(const CodeItem& codeItem : m_codeItems)
		{
			if (codeItem.isCommand() == false)
			{
				continue;
			}

			if (codeItem.address() == appLogicProcessingCodeStartAddress)
			{
				*idrPhaseClockCount += prevCmdExecTime;

				prevCmdExecTime = 0;

				idrPhaseCode = false;
			}

			int waitTime = 0;
			int execTime = 0;

			codeItem.getTimes(m_lmMemoryMap, prevCmdExecTime, &waitTime, &execTime);

			if (idrPhaseCode == true)
			{
				*idrPhaseClockCount += (waitTime + execTime);
			}
			else
			{
				*alpPhaseClockCount +=  (waitTime + execTime);
			}

			prevCmdExecTime = execTime;
		}

		alpPhaseClockCount += prevCmdExecTime;

		return true;
	}
}
