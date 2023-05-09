#include "ApplicationLogicCode.h"
#include "ModuleLogicCompiler.h"

#include "../HardwareLib/Afb.h"
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

	CodeItem &CodeItem::nop(const QString& comment)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::NOP);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::start(int fbType, int fbInstance, const QString& fbCaption, int fbRunTime,
						 const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::stop(const QString& comment)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::STOP);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::mov(int addrTo, int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::mov(Address16 addrTo, Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		return mov(addrTo.offset(), addrFrom.offset(), comment);
	}

	CodeItem& CodeItem::movMem(int addrTo, int addrFrom, int sizeW, const QString& comment)
	{
		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::MOVMEM);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(sizeW);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movMem(Address16 addrTo, Address16 addrFrom, int sizeW, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		return movMem(addrTo.offset(), addrFrom.offset(), sizeW, comment);
	}

	CodeItem& CodeItem::movConst(int addrTo, int constVal, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constVal);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movConst(Address16 addrTo, int constVal, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		return movConst(addrTo.offset(), constVal, comment);
	}

	CodeItem& CodeItem::movBitConst(int addrTo, int bitNo, int constBit, const QString& comment)
	{
		Q_ASSERT(addrTo >=0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVBC);
		m_code.setWord2(addrTo);
		m_code.setWord3(constBit);
		m_code.setBitNo(bitNo);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movBitConst(Address16 addr16, int constBit, const QString& comment)
	{
		Q_ASSERT(addr16.isValid() == true);

		return movBitConst(addr16.offset(), addr16.bit(), constBit, comment);
	}

	CodeItem& CodeItem::writeFuncBlock(int fbType, int fbInstance, int fbParamNo, const Address16& addrFrom,
									   const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrFrom.bit() == 0);

		return writeFuncBlock(fbType, fbInstance, fbParamNo, addrFrom.offset(), fbCaption, comment);
	}

	CodeItem& CodeItem::writeFuncBlock(int fbType, int fbInstance, int fbParamNo, int addrFrom,
									   const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlock(const Address16& addrTo, int fbType, int fbInstance, int fbParamNo,
									  const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrTo.bit() == 0);

		return readFuncBlock(addrTo.offset(), fbType, fbInstance, fbParamNo, fbCaption, comment);
	}

	CodeItem& CodeItem::readFuncBlock(int addrTo, int fbType, int fbInstance, int fbParamNo,
									  const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal,
											const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::WRFBC);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(constVal);
		m_code.setFbCaption(fbCaption);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo,
										  const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom,
										  const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrFrom.isValid() == true);

		return writeFuncBlockBit(fbType, fbInstance, fbParamNo, addrFrom.offset(), addrFrom.bit(),
								 fbCaption, comment);
	}

	CodeItem& CodeItem::readFuncBlockBit(int addrTo, int bitNo, int fbType, int fbInstance, int fbParamNo,
										 const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlockBit(Address16 addrTo, int fbType, int fbInstance, int fbParamNo,
										 const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);

		return readFuncBlockBit(addrTo.offset(), addrTo.bit(), fbType, fbInstance, fbParamNo,
								fbCaption, comment);
	}

	CodeItem& CodeItem::readFuncBlockCompare(int fbType, int fbInstance, int fbParamNo, int testValue,
											 const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::RDFBCMP);
		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(CHECK_AND_CAST_TO_QUINT16(testValue));
		m_code.setFbCaption(fbCaption);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::setMem(int addr, int constValue, int sizeW, const QString& comment)
	{
		Q_ASSERT(addr >=0);

		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_code.setOpCode(LmCommand::Code::SETMEM);
		m_code.setWord2(addr);
		m_code.setWord3(constValue);
		m_code.setWord4(sizeW);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::setMem(Address16 addr, int constValue, int sizeW, const QString& comment)
	{
		Q_ASSERT(addr.isValid() == true);
		Q_ASSERT(addr.bit() == 0);

		return setMem(addr.offset(), constValue, sizeW, comment);
	}

	CodeItem& CodeItem::movBit(int addrTo, int bitTo, int addrFrom, int bitFrom, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movBit(Address16 addrTo, Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrFrom.isValid() == true);

		return movBit(addrTo.offset(), addrTo.bit(), addrFrom.offset(), addrFrom.bit(), comment);
	}

	CodeItem& CodeItem::nstart(int fbType, int fbInstance, int startCount,
							   const QString& fbCaption, int fbRunTime, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::appStart(int appStartAddr, const QString& comment)
	{
		Q_ASSERT(appStartAddr >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::APPSTART);
		m_code.setWord2(CHECK_AND_CAST_TO_QUINT16(appStartAddr));

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::mov32(int addrTo, int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::mov32(Address16 addrTo, Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		return mov32(addrTo.offset(), addrFrom.offset(), comment);
	}

	CodeItem& CodeItem::movConstInt32(int addrTo, qint32 constInt32, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);
		m_code.setConstInt32(constInt32);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movConstUInt32(int addrTo, quint32 constUInt32, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVC32);
		m_code.setWord2(addrTo);
		m_code.setWord3(static_cast<quint16>((constUInt32 >> 16) & 0xFFFF));
		m_code.setWord4(static_cast<quint16>(constUInt32 & 0xFFFF));
		m_code.setConstUInt32(constUInt32);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movConstUInt32(Address16 addrTo, quint32 constUInt32, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		return movConstUInt32(addrTo.offset(), constUInt32, comment);
	}

	CodeItem& CodeItem::movConstFloat(int addrTo, float constFloat, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, int addrFrom,
										 const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, Address16 addrFrom,
										 const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		return writeFuncBlock32(fbType, fbInstance, fbParamNo, addrFrom.offset(), fbCaption, comment);
	}

	CodeItem& CodeItem::readFuncBlock32(int addrTo, int fbType, int fbInstance, int fbParamNo,
										const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlock32(Address16 addrTo, int fbType, int fbInstance, int fbParamNo,
										const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		return readFuncBlock32(addrTo.offset(), fbType, fbInstance, fbParamNo, fbCaption, comment);
	}

	CodeItem& CodeItem::writeFuncBlockConstInt32(int fbType, int fbInstance, int fbParamNo, qint32 constInt32,
												 const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockConstFloat(int fbType, int fbInstance, int fbParamNo, float constFloat,
												 const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlockCompareInt32(int fbType, int fbInstance, int fbParamNo, qint32 testInt32,
												  const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlockCompareFloat(int fbType, int fbInstance, int fbParamNo, float testFloat,
												  const QString& fbCaption, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movCompareFlag(int addrTo, int bitNo, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(bitNo >= 0 && bitNo <= LmCommand::MAX_BIT_NO_16);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::MOVCMPF);
		m_code.setWord2(addrTo);
		m_code.setWord3(bitNo);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::prevMov(int addrTo, int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::prevMov32(int addrTo, int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_code.setOpCode(LmCommand::Code::PMOV32);
		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::fill(int addrTo, int addrFrom, int addrFromBit, const QString& comment)
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

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::fill(Address16 addrTo, Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);

		return fill(addrTo.offset(), addrFrom.offset(), addrFrom.bit(), comment);
	}

	CodeItem& CodeItem::setComment(const QString& comment)
	{
		m_comment = comment;

		return *this;
	}

	bool CodeItem::isWaitingForFbExecution() const
	{
/*		return lmCommands[m_code.getOpCodeInt()].;

		if (lmCommand.waitFbExecution == true)
		{
			int fbType = m_code.getFbType();*/

		return lmCommands[m_code.getOpCodeInt()].waitFbExecution;
	}

	quint16 CodeItem::srcAddr() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOV:
		case LmCommand::Code::PMOV:
		case LmCommand::Code::WRFB:

		case LmCommand::Code::MOV32:
		case LmCommand::Code::PMOV32:
		case LmCommand::Code::WRFB32:

		case LmCommand::Code::MOVMEM:

			return getWord3();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	Address16 CodeItem::srcBitAddr() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOVB:
		case LmCommand::Code::WRFBB:
		case LmCommand::Code::FILL:
			return Address16(getWord3(), getBitNo1());

		default:
			Q_ASSERT(false);
		}

		return Address16();
	}

	quint16 CodeItem::destAddr() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOV:
		case LmCommand::Code::MOVC:
		case LmCommand::Code::PMOV:
		case LmCommand::Code::WRFB:

		case LmCommand::Code::MOV32:
		case LmCommand::Code::MOVC32:
		case LmCommand::Code::PMOV32:
		case LmCommand::Code::WRFB32:

		case LmCommand::Code::MOVMEM:
		case LmCommand::Code::SETMEM:
		case LmCommand::Code::FILL:
			return getWord2();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	Address16 CodeItem::destBitAddr() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOVBC:
			return Address16(getWord2(), getBitNo1());

		case LmCommand::Code::MOVB:
			return Address16(getWord2(), getBitNo2());

		case LmCommand::Code::RDFBB:
			return Address16(getWord3(), getBitNo1());

		default:
			Q_ASSERT(false);
		}

		return Address16();
	}

	quint16 CodeItem::getMoveSizeW() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOV:
		case LmCommand::Code::MOVC:
		case LmCommand::Code::PMOV:
			return 1;

		case LmCommand::Code::MOV32:
		case LmCommand::Code::MOVC32:
		case LmCommand::Code::PMOV32:
			return 2;

		case LmCommand::Code::MOVMEM:
		case LmCommand::Code::SETMEM:
			return getWord4();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint16 CodeItem::getConst16() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOVC:
		case LmCommand::Code::WRFBC:
		case LmCommand::Code::SETMEM:
			return getWord3();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint32 CodeItem::getConst32() const
	{
		switch(getOpcode())
		{
		case LmCommand::Code::MOVC32:
		case LmCommand::Code::WRFBC32:
				return (static_cast<quint32>(getWord3()) << 16) | getWord4();
		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	bool CodeItem::isMoveCmd() const
	{
		return getOpcode() == LmCommand::Code::MOV;
	}

	bool CodeItem::isMove32Cmd() const
	{
		return getOpcode() == LmCommand::Code::MOV32;
	}

	bool CodeItem::isMoveMemCmd() const
	{
		return getOpcode() == LmCommand::Code::MOVMEM;
	}

	bool CodeItem::isMoveBitCmd() const
	{
		return getOpcode() == LmCommand::Code::MOVB;
	}

	bool CodeItem::isMoveConstCmd() const
	{
		return getOpcode() == LmCommand::Code::MOVC;
	}

	bool CodeItem::isMoveConst32Cmd() const
	{
		return getOpcode() == LmCommand::Code::MOVC32;
	}

	bool CodeItem::isSetMemCmd() const
	{
		return getOpcode() == LmCommand::Code::SETMEM;
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

		char cstr[32];
		snprintf(cstr, 32, "[%02d:%02d %6d]", m_waitTime, m_execTime, m_clockCount);
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

	bool CodeItem::calcRunTime(const LmDescription& lmDesc,
							   int prevCmdExecTime,
							   int waitFbTime,
							   int* waitTime,
							   int* execTime,
							   int* fbExecTime,
							   bool firstAlpCommand)
	{
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

		//

		if (prevCmdExecTime >= cmdReadTime)
		{
			m_waitTime = 0;
		}
		else
		{
			m_waitTime = cmdReadTime - prevCmdExecTime;
		}

		if (m_waitTime < 2)
		{
			m_waitTime = 2;
		}

		m_waitTime += (firstAlpCommand == true ? 3 : 0);

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
		case LmCommand::Code::WRFBC:
		case LmCommand::Code::WRFBB:
		case LmCommand::Code::APPSTART:
		case LmCommand::Code::MOV32:
		case LmCommand::Code::MOVC32:
		case LmCommand::Code::WRFB32:
		case LmCommand::Code::WRFBC32:
		case LmCommand::Code::RDFBCMP:
		case LmCommand::Code::RDFBCMP32:
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

				if (waitFbTime != 0)
				{
					m_waitTime += waitFbTime;
				}

				*fbExecTime = m_fbExecTime;
			}
			break;

		case LmCommand::Code::NSTART:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord3();

				cmdExecTime = 2 + (m_fbExecTime + 3) * n + 2;
			}
			break;

			// commands with calculated runtime
			//
		case LmCommand::Code::RDFB:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = calcRdFbRuntime(m_waitTime, 2, waitFbTime, 7);
			break;

		case LmCommand::Code::RDFB32:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = calcRdFbRuntime(m_waitTime, 2, waitFbTime, 18);
			break;

		case LmCommand::Code::RDFBB:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			if (isAddrInBitMem(lmDesc, m_code.getWord3()) == true)
			{
				cmdExecTime = calcRdFbRuntime(m_waitTime, 4, waitFbTime, 7);
			}
			else
			{
				cmdExecTime = calcRdFbRuntime(m_waitTime, 6, waitFbTime, 8);
			}
			break;

		case LmCommand::Code::MOV:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 70 : 10;
			break;

		case LmCommand::Code::MOVMEM:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 8 + (n - 1) * 8 + 2;
			}
			break;

		case LmCommand::Code::MOVC:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 67 : 6;
			break;

		case LmCommand::Code::MOVBC:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 6 : 14;
			break;

		case LmCommand::Code::SETMEM:
			{
				Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 5 + (n - 1) * 4 + 1;
			}
			break;

		case LmCommand::Code::MOVB:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 10 : 17;
			break;

		case LmCommand::Code::MOVCMPF:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 6 : 14;
			break;

		case LmCommand::Code::PMOV:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 69 : 10;
			break;

		case LmCommand::Code::FILL:
			Q_ASSERT(lmCommand.runTime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 69 : 11;
			break;

		default:
			Q_ASSERT(false);								// unknown command code
		}

		m_execTime = cmdExecTime;

		*waitTime = m_waitTime;
		*execTime = m_execTime;

		return true;
	}

	void CodeItem::addExecTime(int execTime)
	{
		Q_ASSERT(m_execTime != -1);
		m_execTime += execTime;
		m_clockCount += execTime;
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

	bool CodeItem::isAddrInBitMem(const LmDescription& lmDesc, quint32 addr) const
	{
		return (addr >= lmDesc.memory().m_appLogicBitDataOffset &&
				addr < (lmDesc.memory().m_appLogicBitDataOffset +
						lmDesc.memory().m_appLogicBitDataSize));
	}

	bool CodeItem::isAddrInWordMem(const LmDescription& lmDesc, quint32 addr) const
	{
		return (addr >= lmDesc.memory().m_appLogicWordDataOffset &&
				addr < (lmDesc.memory().m_appLogicWordDataOffset +
						lmDesc.memory().m_appLogicWordDataSize));
	}

	int CodeItem::calcRdFbRuntime(int cmdWaitTime,
								  int preFbReadTime,
								  int fbExecTime,
								  int postFbReadTime) const
	{
		fbExecTime -= cmdWaitTime;

		int runtime = preFbReadTime;

		fbExecTime -= preFbReadTime;

		if (fbExecTime > 0)
		{
			runtime += fbExecTime;
		}

		runtime += postFbReadTime;

		return runtime;
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

	CodeSnippet& CodeSnippet::operator << (const QString& commentStr)
	{
		comment(commentStr);
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

	void CodeSnippet::swap(CodeSnippet& code)
	{
		m_code.swap(code.m_code);
	}

	bool CodeSnippet::isEmpty() const
	{
		return m_code.empty();
	}

	int CodeSnippet::itemsCount() const
	{
		return static_cast<int>(m_code.size());
	}

	int CodeSnippet::codeSizeW() const
	{
		int sizeW = 0;

		for(const auto& ci : m_code)
		{
			sizeW += ci.sizeW();
		}

		return sizeW;
	}

	void CodeSnippet::getAsmCode(QStringList* asmCode) const
	{
		TEST_PTR_RETURN(asmCode);

		asmCode->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QString str = codeItem.getAsmCode(true);

			asmCode->append(str);
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

	const std::vector<CodeItem>& CodeSnippet::code() const
	{
		return m_code;
	}

	CodeSnippetIterator CodeSnippet::begin()
	{
		return m_code.begin();
	}

	CodeSnippetConstIterator CodeSnippet::begin() const
	{
		return m_code.begin();
	}

	CodeSnippetIterator CodeSnippet::end()
	{
		return m_code.end();
	}

	CodeSnippetConstIterator CodeSnippet::end() const
	{
		return m_code.end();
	}

	// ----------------------------------------------------------------------------------
	//
	// AppLogicCode class implementation
	//
	// ----------------------------------------------------------------------------------

	AppLogicCode::AppLogicCode(Type type, bool optimized) :
		m_codeType(type),
		m_optimized(optimized)
	{
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

	bool AppLogicCode::finalize(std::shared_ptr<const LmDescription> lmDesc)
	{
		TEST_PTR_RETURN_FALSE(lmDesc);

		m_codeSizeW = 0;
		m_clockCount = 0;
		m_commandsCount = 0;

		m_lmCodeMemUsage = 0;
		m_execTimeMcs = 0;
		m_lmCycleTimeUsage = 0;

		if (m_code.empty() == true)
		{
			return true;
		}

		// read commands and calculate code runtime
		//
		int prevCmdExecTime = 0;
		int waitTime = 0;
		int execTime = 0;
		int fbExecTime = 0;
		int waitFbTime = 0;
		int phaseClockCount = 0;
		bool firstAlpCommand = false;

		const LmDescription& lmDescRef = *lmDesc.get();

		for(CodeItem& codeItem : m_code)
		{
			if (codeItem.isCommand() == false)
			{
				continue;
			}

			m_commandsCount++;

			codeItem.setAddress(m_codeSizeW);

			m_codeSizeW += codeItem.sizeW();

			waitFbTime = 0;

			if (codeItem.isWaitingForFbExecution() == true)
			{
				waitFbTime = getFbRemainingExecTime(codeItem.getFbType());
			}

			codeItem.calcRunTime(lmDescRef, prevCmdExecTime, waitFbTime,
								 &waitTime, &execTime, &fbExecTime, firstAlpCommand);

			firstAlpCommand = false;

			m_clockCount += (waitTime + execTime);
			phaseClockCount += (waitTime + execTime);

			codeItem.setClockCount(phaseClockCount);

			decFbExecTime(waitTime + execTime);

			prevCmdExecTime = execTime;

			if (fbExecTime != 0)
			{
				Q_ASSERT(codeItem.getOpcode() == LmCommand::Code::START ||
						 codeItem.getOpcode() == LmCommand::Code::NSTART);

				startFbExec(codeItem.getFbType(), fbExecTime);
			}

			if (codeItem.getOpcode() == LmCommand::Code::STOP)
			{
				int addTime = getMaxFbRemainingExecTimeAndClear() - prevCmdExecTime;

				if (addTime > 0)
				{
					codeItem.addExecTime(addTime);
					m_clockCount += addTime;
				}

				prevCmdExecTime = 0;
				phaseClockCount = 0;

				firstAlpCommand = true;
			}
		}

		if (lmDesc->memory().m_codeMemorySize != 0)
		{
			m_lmCodeMemUsage = static_cast<double>(m_codeSizeW * 100) /
								static_cast<double>(lmDesc->memory().m_codeMemorySize);
		}

		m_execTimeMcs = m_clockCount * lmDesc->logicUnit().clockTimeSecs() * 1000000.0;

		double totalTimeMcs = 0;

		switch(m_codeType)
		{
		case Type::IDR_Code:
			totalTimeMcs = lmDesc->logicUnit().m_idrPhaseTime;
			break;

		case Type::ALP_Code:
			totalTimeMcs = lmDesc->logicUnit().m_alpPhaseTime;
			break;

		case Type::AllCode:
			totalTimeMcs =	lmDesc->logicUnit().m_idrPhaseTime +
							lmDesc->logicUnit().m_alpPhaseTime;
			break;

		default:
			Q_ASSERT(false);
			return false;
		}

		if (totalTimeMcs !=  0)
		{
			m_lmCycleTimeUsage = static_cast<double>(m_execTimeMcs * 100) / totalTimeMcs;
		}

		return true;
	}

	void AppLogicCode::clear()
	{
		CodeSnippet::clear();

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

	bool AppLogicCode::optimized() const
	{
		return m_optimized;
	}

	void AppLogicCode::setOptimized( bool optimized)
	{
		m_optimized = optimized;
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

	void AppLogicCode::removeStopCommand()
	{
		auto it = std::find_if(m_code.rbegin(), m_code.rend(),
								[] (const CodeItem& ci)
								{
									return ci.getOpcode() == LmCommand::Code::STOP;
								});

		if (it == m_code.rend())
		{
			Q_ASSERT(false);
			return;
		}

		m_code.erase((it + 1).base(), m_code.end());
	}

	void AppLogicCode::startFbExec(int fbOpCode, int fbRuntime)
	{
		auto it = m_runningAfbs.find(fbOpCode);

		if (it == m_runningAfbs.end())
		{
			// FB with fbOpCode is NOT running now
			//
			m_runningAfbs.insert({fbOpCode, fbRuntime});
		}
		else
		{
			Q_ASSERT(false);
		}
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
