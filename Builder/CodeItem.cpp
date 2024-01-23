#include "CodeItem.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// BinCommandCode class implementation
	//
	// ---------------------------------------------------------------------------------------

	BinCommandCode::BinCommandCode()
	{
		setNoCommand();
	}

	BinCommandCode::BinCommandCode(const BinCommandCode& cCode)
	{
		*this = cCode;
	}

	BinCommandCode& BinCommandCode::operator = (const BinCommandCode& cCode)
	{
		word1 = cCode.word1;
		word2 = cCode.word2;
		word3 = cCode.word3;
		word4 = cCode.word4;

		return *this;
	}

	void BinCommandCode::setOpCode(LmCommandCode code, quint16 cmdCodeMask)
	{
		Q_ASSERT((code & (~cmdCodeMask)) == 0);

		word1 &= ~cmdCodeMask;
		word1 |= (code & cmdCodeMask);
	}

	void BinCommandCode::setFbType(int fbType)
	{
		if (fbType > MAX_FB_TYPE)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			opCode.fbType = CHECK_AND_CAST_TO_QUINT16(fbType);
		}
	}

	void BinCommandCode::setFbInstance(int fbInstance)
	{
		param.fbInstance = CHECK_AND_CAST_TO_QUINT16(fbInstance);
	}

	void BinCommandCode::setFbParamNo(int fbParamNo)
	{
		if (fbParamNo > MAX_FB_PARAM_NO)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			param.fbParamNo = CHECK_AND_CAST_TO_QUINT16(fbParamNo);
		}
	}

	void BinCommandCode::setBitNo(int bitNo)
	{
		if (bitNo < 0 || bitNo > MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			word4 = static_cast<quint16>(bitNo);
		}
	}

	void BinCommandCode::setBitNo1(int bitNo)
	{
		if (bitNo < 0 || bitNo > MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b1 = static_cast<quint8>(bitNo);
		}
	}

	void BinCommandCode::setBitNo2(int bitNo)
	{
		if (bitNo < 0 || bitNo > MAX_BIT_NO_16)
		{
			Q_ASSERT(false);
			setNoCommand();
		}
		else
		{
			this->bitNo.b2 = static_cast<quint8>(bitNo);
		}
	}

	quint16 BinCommandCode::getWord(int index) const
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

	void BinCommandCode::calcCrc5()
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

	void BinCommandCode::clear()
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

	CodeItem& CodeItem::startafb(int fbType, int fbInstance, const QString& fbCaption, int fbRunTime,
						 const QString& comment)
	{
		initCommand();

		m_result = true;

		Q_ASSERT(fbRunTime != 0);		// fbRunTime can't be 0

		m_fbExecTime = fbRunTime;

		m_lmCmdCode = LmCommand::STARTAFB;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);

		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::stop(const QString& comment)
	{
		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::STOP;

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::mov(int addrTo, int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOV;

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

	CodeItem& CodeItem::movAddrAcc(int addrTo, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOV_ADDR_ACC;

		m_code.setWord2(addrTo);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movAddrAcc(Address16 addrTo, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		return movAddrAcc(addrTo.offset(), comment);
	}

	CodeItem& CodeItem::movAccAddr(int addrFrom, const QString& comment)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOV_ACC_ADDR;

		m_code.setWord2(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movAccAddr(Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrFrom.isValid() == true);
		Q_ASSERT(addrFrom.bit() == 0);

		return movAccAddr(addrFrom.offset(), comment);
	}

	CodeItem& CodeItem::movMem(int addrTo, int addrFrom, int sizeW, const QString& comment)
	{
		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_lmCmdCode = LmCommand::MOVMEM;

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

		m_lmCmdCode = LmCommand::MOVC;

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

	CodeItem& CodeItem::movAccConst(int constVal, const QString& comment)
	{
		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVC_ACC;

		m_code.setWord2(constVal);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movBitConst(int addrTo, int bitNo, int constBit, const QString& comment)
	{
		Q_ASSERT(addrTo >=0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVBC;

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

		m_lmCmdCode = LmCommand::WRFB;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_fbCaption = fbCaption;

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

		m_lmCmdCode = LmCommand::RDFB;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);

		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockConst(int fbType, int fbInstance, int fbParamNo, int constVal,
											const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::WRFBC;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(constVal);

		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockBit(int fbType, int fbInstance, int fbParamNo, int addrFrom, int bitNo,
										  const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::WRFBB;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);
		m_code.setBitNo(bitNo);

		setFbCaption(fbCaption);
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

		m_lmCmdCode = LmCommand::RDFBB;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);
		m_code.setBitNo(bitNo);

		setFbCaption(fbCaption);
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

		m_lmCmdCode = LmCommand::RDFBCMP;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(CHECK_AND_CAST_TO_QUINT16(testValue));

		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::setMem(int addr, int constValue, int sizeW, const QString& comment)
	{
		Q_ASSERT(addr >=0);

		initCommand();

		m_result = true;

		Q_ASSERT(sizeW > 0);

		m_lmCmdCode = LmCommand::SETMEM;

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

		m_lmCmdCode = LmCommand::MOVB;

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

	CodeItem& CodeItem::movBitAccAddr(int addrFrom, int bitNo, const QString& comment)
	{
		Q_ASSERT(addrFrom >=0);
		Q_ASSERT(bitNo >= 0 && bitNo < 16);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVB_ACC_ADDR;

		m_code.setWord1(bitNo & 0x0F);
		m_code.setWord2(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movBitAccAddr(Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrFrom.isValid() == true);

		return movBitAccAddr(addrFrom.offset(), addrFrom.bit(), comment);
	}

	CodeItem& CodeItem::movBitAddrAcc(int addrTo, int bitNo, const QString& comment)
	{
		Q_ASSERT(addrTo >=0);
		Q_ASSERT(bitNo >= 0 && bitNo < 16);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVB_ADDR_ACC;

		m_code.setWord1(bitNo & 0x0F);
		m_code.setWord2(addrTo);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movBitAddrAcc(Address16 addrTo, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);

		return movBitAddrAcc(addrTo.offset(), addrTo.bit(), comment);
	}

	CodeItem& CodeItem::nstart(int fbType, int fbInstance, int startCount,
							   const QString& fbCaption, int fbRunTime, const QString& comment)
	{
		initCommand();

		m_result = true;

		Q_ASSERT(fbRunTime != 0);		// fbRunTime can't be 0

		m_fbExecTime = fbRunTime;

		m_lmCmdCode = LmCommand::NSTART;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setWord3(startCount);

		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::appStart(int appStartAddr, const QString& comment)
	{
		Q_ASSERT(appStartAddr >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::APPSTART;

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

		m_lmCmdCode = LmCommand::MOV32;

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

		m_lmCmdCode = LmCommand::MOVC32;

		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);

		setConstInt32(constInt32);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movConstUInt32(int addrTo, quint32 constUInt32, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVC32;
		m_code.setWord2(addrTo);
		m_code.setWord3(static_cast<quint16>((constUInt32 >> 16) & 0xFFFF));
		m_code.setWord4(static_cast<quint16>(constUInt32 & 0xFFFF));

		setConstUInt32(constUInt32);
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

		m_lmCmdCode = LmCommand::MOVC32;

		m_code.setWord2(addrTo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);

		setConstFloat(constFloat);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlock32(int fbType, int fbInstance, int fbParamNo, int addrFrom,
										 const QString& fbCaption, const QString& comment)
	{
		Q_ASSERT(addrFrom >= 0);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::WRFB32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrFrom);

		setFbCaption(fbCaption);
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

		m_lmCmdCode = LmCommand::RDFB32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3(addrTo);

		setFbCaption(fbCaption);
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

		m_lmCmdCode = LmCommand::WRFBC32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);

		setConstInt32(constInt32);
		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::writeFuncBlockConstFloat(int fbType, int fbInstance, int fbParamNo, float constFloat,
												 const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		qint32 constInt32 = std::bit_cast<qint32>(constFloat);		// map binary code of float to qint32

		m_lmCmdCode = LmCommand::WRFBC32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((constInt32 >> 16) & 0xFFFF);
		m_code.setWord4(constInt32 & 0xFFFF);

		setConstFloat(constFloat);
		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlockCompareInt32(int fbType, int fbInstance, int fbParamNo, qint32 testInt32,
												  const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::RDFBCMP32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);

		setConstInt32(testInt32);
		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::readFuncBlockCompareFloat(int fbType, int fbInstance, int fbParamNo, float testFloat,
												  const QString& fbCaption, const QString& comment)
	{
		initCommand();

		m_result = true;

		qint32 testInt32 = std::bit_cast<qint32>(testFloat);		// map binary code of float to qint32

		m_lmCmdCode = LmCommand::RDFBCMP32;

		m_code.setFbType(fbType);
		m_code.setFbInstance(fbInstance);
		m_code.setFbParamNo(fbParamNo);
		m_code.setWord3((testInt32 >> 16) & 0xFFFF);
		m_code.setWord4(testInt32 & 0xFFFF);

		setConstFloat(testFloat);
		setFbCaption(fbCaption);
		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::movCompareFlag(int addrTo, int bitNo, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);
		Q_ASSERT(bitNo >= 0 && bitNo <= BinCommandCode::MAX_BIT_NO_16);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::MOVCMPF;

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

		m_lmCmdCode = LmCommand::PMOV;

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

		m_lmCmdCode = LmCommand::PMOV32;

		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::fillb(int addrTo, int addrFrom, int addrFromBit, const QString& comment)
	{
		Q_ASSERT(addrTo >= 0);

		Q_ASSERT(addrFrom >= 0);
		Q_ASSERT(addrFromBit >= 0 && addrFromBit <= BinCommandCode::MAX_BIT_NO_16);

		initCommand();

		m_result = true;

		m_lmCmdCode = LmCommand::FILLB;

		m_code.setWord2(addrTo);
		m_code.setWord3(addrFrom);
		m_code.setWord4(addrFromBit);

		setComment(comment);

		return *this;
	}

	CodeItem& CodeItem::fillb(Address16 addrTo, Address16 addrFrom, const QString& comment)
	{
		Q_ASSERT(addrTo.isValid() == true);
		Q_ASSERT(addrTo.bit() == 0);

		Q_ASSERT(addrFrom.isValid() == true);

		return fillb(addrTo.offset(), addrFrom.offset(), addrFrom.bit(), comment);
	}

	CodeItem& CodeItem::resetAcc()
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::RESET;
		return *this;
	}

	CodeItem& CodeItem::setAcc()
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::SET;
		return *this;

	}
	CodeItem& CodeItem::orAcc(const QString& comment)
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::OR;
		setComment(comment);
		return *this;
	}

	CodeItem& CodeItem::andAcc(const QString& comment)
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::AND;
		setComment(comment);
		return *this;
	}

	CodeItem& CodeItem::notAcc(const QString& comment)
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::NOT;
		setComment(comment);
		return *this;
	}

	CodeItem& CodeItem::lshift0Acc(const QString& comment)
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::LSHIFT0;
		setComment(comment);
		return *this;
	}

	CodeItem& CodeItem::lshift1Acc(const QString& comment)
	{
		initCommand();
		m_result = true;
		m_lmCmdCode = LmCommand::LSHIFT1;
		setComment(comment);
		return *this;
	}

	CodeItem& CodeItem::setComment(const QString& comment)
	{
		m_comment = comment;

		return *this;
	}

	bool CodeItem::setBinParams(LmDescriptionConstShared lmDesc, int address)
	{
		TEST_PTR_RETURN_FALSE(lmDesc);

		if (m_isCommand == false ||
			m_lmCmdCode == LmCommand::NO_COMMAND)
		{
			Q_ASSERT(false);
			return false;
		}

		const LmCommand* lmCmd = lmDesc->commandPtr(m_lmCmdCode);

		TEST_PTR_RETURN_FALSE(lmCmd);

		m_lmCmd = lmCmd;

		Q_ASSERT(lmCmd->code == m_lmCmdCode);

		m_address = address;
		m_code.setOpCode(lmCmd->code, lmCmd->codeMask);

		return true;
	}

	int CodeItem::address() const
	{
		Q_ASSERT(m_isCommand == true);

		if (m_address == -1)
		{
			Q_ASSERT(false);		// setBinParams should be called first!
		}

		return m_address;
	}

	int CodeItem::sizeW() const
	{
		if (m_isCommand == false)
		{
			return 0;
		}

		if (m_lmCmd == nullptr)
		{
			Q_ASSERT(false);		// setBinParams should be called first!
			return 0;
		}

		return m_lmCmd->codeSize;
	}

	bool CodeItem::isWaitingForFbExecution() const
	{
		if (m_lmCmd == nullptr)
		{
			Q_ASSERT(false);		// setBinParams should be called first!
			return false;
		}

		return m_lmCmd->waitFbExecution;
	}

	quint16 CodeItem::srcAddr() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOV:
		case LmCommand::PMOV:
		case LmCommand::WRFB:

		case LmCommand::MOV32:
		case LmCommand::PMOV32:
		case LmCommand::WRFB32:

		case LmCommand::MOVMEM:

			return getWord3();

		case LmCommand::MOV_ACC_ADDR:
			return getWord2();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	Address16 CodeItem::srcBitAddr() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOVB:
		case LmCommand::WRFBB:
		case LmCommand::FILLB:
			return Address16(getWord3(), getBitNo1());

		case LmCommand::MOVB_ACC_ADDR:
			return Address16(getWord2(), getWord1BitNo());

		default:
			Q_ASSERT(false);
		}

		return Address16();
	}

	quint16 CodeItem::destAddr() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOV:
		case LmCommand::MOV_ADDR_ACC:
		case LmCommand::MOVC:
		case LmCommand::PMOV:
		case LmCommand::WRFB:

		case LmCommand::MOV32:
		case LmCommand::MOVC32:
		case LmCommand::PMOV32:
		case LmCommand::WRFB32:

		case LmCommand::MOVMEM:
		case LmCommand::SETMEM:
		case LmCommand::FILLB:
			return getWord2();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	Address16 CodeItem::destBitAddr() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOVBC:
			return Address16(getWord2(), getBitNo1());

		case LmCommand::MOVB:
			return Address16(getWord2(), getBitNo2());

		case LmCommand::RDFBB:
			return Address16(getWord3(), getBitNo1());

		case LmCommand::MOVB_ADDR_ACC:
			return Address16(getWord2(), getWord1BitNo());

		default:
			Q_ASSERT(false);
		}

		return Address16();
	}

	quint16 CodeItem::getMoveSizeW() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOV:
		case LmCommand::MOVC:
		case LmCommand::PMOV:
			return 1;

		case LmCommand::MOV32:
		case LmCommand::MOVC32:
		case LmCommand::PMOV32:
			return 2;

		case LmCommand::MOVMEM:
		case LmCommand::SETMEM:
			return getWord4();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint16 CodeItem::getConst16() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOVC:
		case LmCommand::WRFBC:
		case LmCommand::SETMEM:
			return getWord3();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint32 CodeItem::getConst32() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOVC32:
		case LmCommand::WRFBC32:
				return (static_cast<quint32>(getWord3()) << 16) | getWord4();
		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint16 CodeItem::getConstBit() const
	{
		switch(lmCommandCode())
		{
		case LmCommand::MOVBC:
		case LmCommand::WRFBC:
			return getWord3();

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	bool CodeItem::isMoveCmd() const
	{
		return m_lmCmdCode == LmCommand::MOV;
	}

	bool CodeItem::isMove32Cmd() const
	{
		return m_lmCmdCode == LmCommand::MOV32;
	}

	bool CodeItem::isMoveMemCmd() const
	{
		return m_lmCmdCode == LmCommand::MOVMEM;
	}

	bool CodeItem::isMoveBitCmd() const
	{
		return m_lmCmdCode == LmCommand::MOVB;
	}

	bool CodeItem::isMoveBitConstCmd() const
	{
		return m_lmCmdCode == LmCommand::MOVBC;
	}

	bool CodeItem::isMoveConstCmd() const
	{
		return m_lmCmdCode == LmCommand::MOVC;
	}

	bool CodeItem::isMoveConst32Cmd() const
	{
		return m_lmCmdCode == LmCommand::MOVC32;
	}

	bool CodeItem::isSetMemCmd() const
	{
		return m_lmCmdCode == LmCommand::SETMEM;
	}

	bool CodeItem::isWriteFuncBlockBitCmd() const
	{
		return m_lmCmdCode == LmCommand::WRFBB;
	}

	bool CodeItem::isWriteFuncBlockConstCmd() const
	{
		return m_lmCmdCode == LmCommand::WRFBC;
	}

	bool CodeItem::isStartAfbCmd() const
	{
		return m_lmCmdCode == LmCommand::STARTAFB;
	}

	bool CodeItem::isReadFuncBlockBitCmd() const
	{
		return m_lmCmdCode == LmCommand::RDFBB;
	}

	bool CodeItem::generateBinCode(QByteArray* binCode) const
	{
		TEST_PTR_RETURN_FALSE(binCode);

		binCode->clear();

		if (isComment() == true)
		{
			return true;
		}

		if (m_lmCmd == nullptr)
		{
			Q_ASSERT(false);			// setBinParams should be called first!
			return false;
		}

		E::ByteOrder byteOrder = E::ByteOrder::BigEndian;

		binCode->resize(m_lmCmd->codeSize * WORD_SIZE_IN_BYTES);

		BinCommandCode cmdCode = m_code;

		cmdCode.calcCrc5();

		for(int i = 0; i < m_lmCmd->codeSize; i++)
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

	QString CodeItem::getAsmCode(LmDescriptionConstShared lmDesc, bool printCmdCode, bool printTime) const
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

		if (printTime == true)
		{
			char cstr[32];
			snprintf(cstr, 32, "[%02d:%02d %6d]", m_waitTime, m_execTime, m_clockCount);
			cmdStr += QString(cstr).leftJustified(16, ' ');
		}

		QString mnemo = mnemoCode(lmDesc);

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

	QString CodeItem::mnemoCode(LmDescriptionConstShared lmDesc) const
	{
		TEST_PTR_RETURN_VALUE(lmDesc, QString("Nullptr!"));

		const LmCommand* lmCmd = lmDesc->commandPtr(m_lmCmdCode);

		if (lmCmd == nullptr)
		{
			Q_ASSERT(false);
			return QString("Command with code %1 not found in %2").
							arg(m_lmCmdCode).arg(lmDesc->name());
		}

		QString mnemoCode = lmCmd->caption.leftJustified(10, ' ', false).toUpper();

		auto it = m_getMnemoFuncMap.find(lmCmd->getMnemoFunc);

		if (it == m_getMnemoFuncMap.end())
		{
			Q_ASSERT(false);
			return QString("GetMnemoFunction '%1' not found for command %2").
							arg(lmCmd->getMnemoFunc).arg(m_lmCmdCode);
		}

		GetMnemoFuncPtr getMnemoFunc = it->second;

		QString params = (this->*getMnemoFunc)();

		return mnemoCode + params.toUpper();
	}

	QString CodeItem::getConstValueString() const
	{
		switch(m_constDataFormat)
		{
		case E::DataFormat::Float:
			return QString("%1").arg(getConstFloat());

		case E::DataFormat::SignedInt:
			return QString("%1").arg(getConstInt32());

		case E::DataFormat::UnsignedInt:
			return QString("%1").arg(getConstUInt32());

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	bool CodeItem::calcRunTime(LmDescriptionConstShared lmDesc,
							   int prevCmdExecTime,
							   int waitFbTime,
							   int* waitTime,
							   int* execTime,
							   int* fbExecTime,
							   bool firstAlpCommand)
	{
		TEST_PTR_RETURN_FALSE(m_lmCmd);

		*waitTime = 0;
		*execTime = 0;
		*fbExecTime = 0;

		m_waitTime = 0;
		m_execTime = 0;

		int cmdReadTime = m_lmCmd->readTime;

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

		auto it = m_calcExecTimeFuncMap.find(m_lmCmd->calcExecTimeFunc);

		if (it == m_calcExecTimeFuncMap.end())
		{
			Q_ASSERT(false);
			m_execTime = 0;
			return false;
		}

		CalcExecTimeFuncPtr calcExecTimeFuncPtr = it->second;

		(this->*calcExecTimeFuncPtr)(lmDesc, waitFbTime, fbExecTime);

/*		switch(m_lmCmdCode)
		{
		case LmCommand::NO_COMMAND:
			Q_ASSERT(false);
			break;

			// commands with const runtime
			//
		case LmCommand::NOP:
		case LmCommand::STOP:
		case LmCommand::WRFB:
		case LmCommand::WRFBC:
		case LmCommand::WRFBB:
		case LmCommand::APPSTART:
		case LmCommand::MOV32:
		case LmCommand::MOVC32:
		case LmCommand::WRFB32:
		case LmCommand::WRFBC32:
		case LmCommand::RDFBCMP:
		case LmCommand::RDFBCMP32:
		case LmCommand::PMOV32:
			Q_ASSERT(lmCommand.constRuntime != LmCommand::CALC_RUNTIME);
			cmdExecTime = lmCommand.constRuntime;
			break;

			// specific commands START, NSTART
			//
		case LmCommand::START:
			{
				Q_ASSERT(lmCommand.constRuntime != LmCommand::CALC_RUNTIME);

				cmdExecTime = lmCommand.constRuntime;

				if (waitFbTime != 0)
				{
					m_waitTime += waitFbTime;
				}

				*fbExecTime = m_fbExecTime;
			}
			break;

		case LmCommand::NSTART:
			{
				Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord3();

				cmdExecTime = 2 + (m_fbExecTime + 3) * n + 2;
			}
			break;

			// commands with calculated runtime
			//
		case LmCommand::RDFB:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = calcRdFbRuntime(m_waitTime, 2, waitFbTime, 7);
			break;

		case LmCommand::RDFB32:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = calcRdFbRuntime(m_waitTime, 2, waitFbTime, 18);
			break;

		case LmCommand::RDFBB:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			if (isAddrInBitMem(lmDesc, m_code.getWord3()) == true)
			{
				cmdExecTime = calcRdFbRuntime(m_waitTime, 4, waitFbTime, 7);
			}
			else
			{
				cmdExecTime = calcRdFbRuntime(m_waitTime, 6, waitFbTime, 8);
			}
			break;

		case LmCommand::MOV:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 70 : 10;
			break;

		case LmCommand::MOVMEM:
			{
				Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 8 + (n - 1) * 8 + 2;
			}
			break;

		case LmCommand::MOVC:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 67 : 6;
			break;

		case LmCommand::MOVBC:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 6 : 14;
			break;

		case LmCommand::SETMEM:
			{
				Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);

				quint16 n = m_code.getWord4();

				Q_ASSERT(n > 0);

				cmdExecTime = 5 + (n - 1) * 4 + 1;
			}
			break;

		case LmCommand::MOVB:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 10 : 17;
			break;

		case LmCommand::MOVCMPF:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 6 : 14;
			break;

		case LmCommand::PMOV:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 69 : 10;
			break;

		case LmCommand::FILLB:
			Q_ASSERT(lmCommand.constRuntime == LmCommand::CALC_RUNTIME);
			cmdExecTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ? 69 : 11;
			break;

		default:
			Q_ASSERT(false);								// unknown command code
		}*/

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
		m_fbCaption.clear();
		m_comment.clear();
	}

	void CodeItem::setConstFloat(float floatValue)
	{
		m_const.floatValue = floatValue;
		m_constDataFormat = E::DataFormat::Float;
	}

	float CodeItem::getConstFloat() const
	{
		if (m_constDataFormat == E::DataFormat::Float)
		{
			return m_const.floatValue;
		}

		Q_ASSERT(false);

		return 0;
	}

	void CodeItem::setConstInt32(qint32 int32Value)
	{
		m_const.int32Value = int32Value;
		m_constDataFormat = E::DataFormat::SignedInt;
	}

	qint32 CodeItem::getConstInt32() const
	{
		if (m_constDataFormat == E::DataFormat::SignedInt)
		{
			return m_const.int32Value;
		}

		Q_ASSERT(false);

		return 0;
	}

	void CodeItem::setConstUInt32(quint32 uint32Value)
	{
		m_const.uint32Value = uint32Value;
		m_constDataFormat = E::DataFormat::UnsignedInt;
	}

	quint32 CodeItem::getConstUInt32() const
	{
		if (m_constDataFormat == E::DataFormat::UnsignedInt)
		{
			return m_const.uint32Value;
		}

		Q_ASSERT(false);

		return 0;
	}

	QString CodeItem::getCodeWordStr(int wordNo) const
	{
		QString str;

		QByteArray binCode;

		generateBinCode(&binCode);

		if (binCode.size() < (wordNo + 1) * WORD_SIZE_IN_BYTES)
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

	bool CodeItem::isAddrInBitMem(LmDescriptionConstShared lmDesc, quint32 addr) const
	{
		return (addr >= lmDesc->memory().m_appLogicBitDataOffset &&
				addr < (lmDesc->memory().m_appLogicBitDataOffset +
						lmDesc->memory().m_appLogicBitDataSize));
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

	QString CodeItem::mnemo_nop() const
	{
		return QStringLiteral("");
	}

	QString CodeItem::mnemo_acc() const
	{
		return QStringLiteral("ACC");
	}

	QString CodeItem::mnemo_startafb() const
	{
		return QString("%1.%2").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt());
	}

	QString CodeItem::mnemo_stop() const
	{
		return QStringLiteral("");
	}

	QString CodeItem::mnemo_mov() const
	{
		return QString("%1, %2").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_mov_addr_acc() const
	{
		return QString("%1, ACC").
					arg(m_code.getWord2());
	}

	QString CodeItem::mnemo_mov_acc_addr() const
	{
		return QString("ACC, %1").
					arg(m_code.getWord2());
	}

	QString CodeItem::mnemo_movmem() const
	{
		return QString("%1, %2, %3").
					arg(m_code.getWord2()).
					arg(m_code.getWord3()).
					arg(m_code.getWord4());
	}

	QString CodeItem::mnemo_movc() const
	{
		return QString("%1, #%2").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_movc_acc() const
	{
		return QString("ACC, #%1").
					arg(m_code.getWord2());
	}

	QString CodeItem::mnemo_movbc() const
	{
		return QString("%1[%2], #%3").
					arg(m_code.getWord2()).
					arg(m_code.getWord4()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_wrfb() const
	{
		return QString("%1.%2[%3], %4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_rdfb() const
	{
		return QString("%1, %2.%3[%4]").
					arg(m_code.getWord3()).
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt());
	}

	QString CodeItem::mnemo_wrfbc() const
	{
		return QString("%1.%2[%3], #%4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_wrfbb() const
	{
		return QString("%1.%2[%3], %4[%5]").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(m_code.getWord3()).
					arg(m_code.getWord4());
	}

	QString CodeItem::mnemo_rdfbb() const
	{
		return QString("%1[%2], %3.%4[%5]").
					arg(m_code.getWord3()).
					arg(m_code.getWord4()).
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt());
	}

	QString CodeItem::mnemo_rdfbcmp() const
	{
		return QString("%1.%2[%3], #%4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_setmem() const
	{
		return QString("%1, #%2, %3").
					arg(m_code.getWord2()).
					arg(m_code.getWord3()).
					arg(m_code.getWord4());
	}

	QString CodeItem::mnemo_movb() const
	{
		return QString("%1[%2], %3[%4]").
					arg(m_code.getWord2()).
					arg(m_code.getBitNo2()).
					arg(m_code.getWord3()).
					arg(m_code.getBitNo1());
	}

	QString CodeItem::mnemo_movb_acc_addr() const
	{
		return QString("ACC, %1[%2]").
					arg(m_code.getWord2()).
					arg(m_code.getWord1BitNo());
	}

	QString CodeItem::mnemo_movb_addr_acc() const
	{
		return QString("%1[%2], ACC").
					arg(m_code.getWord2()).
					arg(m_code.getWord1BitNo());
	}

	QString CodeItem::mnemo_nstart() const
	{
		return QString("%1.%2, %3").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_appstart() const
	{
		return QString("%1").
					arg(m_code.getWord2());
	}

	QString CodeItem::mnemo_mov32() const
	{
		return QString("%1, %2").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_movc32() const
	{
		return QString("%1, #%2").
					arg(m_code.getWord2()).
					arg(getConstValueString());
	}

	QString CodeItem::mnemo_wrfb32() const
	{
		return QString("%1.%2[%3], %4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_rdfb32() const
	{
		return QString("%1, %2.%3[%4]").
					arg(m_code.getWord3()).
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt());
	}

	QString CodeItem::mnemo_wrfbc32() const
	{
		return QString("%1.%2[%3], #%4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(getConstValueString());
	}

	QString CodeItem::mnemo_rdfbcmp32() const
	{
		return QString("%1.%2[%3], #%4").
					arg(m_fbCaption).
					arg(m_code.getFbInstanceInt()).
					arg(m_code.getFbParamNoInt()).
					arg(getConstValueString());
	}

	QString CodeItem::mnemo_movcmpf() const
	{
		return QString("%1[%2]").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_pmov() const
	{
		return QString("%1, %2").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_pmov32() const
	{
		return QString("%1, %2").
					arg(m_code.getWord2()).
					arg(m_code.getWord3());
	}

	QString CodeItem::mnemo_fillb() const
	{
		return QString("%1, %2[%3]").
					arg(m_code.getWord2()).
					arg(m_code.getWord3()).
					arg(m_code.getWord4());
	}

	void CodeItem::exectime_const(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(waitFbTime);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->constRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::NOP
		// LmCommand::RESET
		// LmCommand::SET
		// LmCommand::OR
		// LmCommand::AND
		// LmCommand::NOT
		// LmCommand::STOP
		// LmCommand::WRFB
		// LmCommand::WRFBC
		// LmCommand::WRFBB
		// LmCommand::APPSTART
		// LmCommand::MOV32
		// LmCommand::MOVC32
		// LmCommand::MOVB_ACC_ADDR
		// LmCommand::MOVC_ACC
		// LmCommand::MOV_ACC_ADDR
		// LmCommand::MOV_ADDR_ACC
		// LmCommand::WRFB32
		// LmCommand::WRFBC32
		// LmCommand::RDFBCMP
		// LmCommand::RDFBCMP32
		// LmCommand::PMOV32

		m_execTime = m_lmCmd->constRuntime;
	}

	void CodeItem::exectime_startafb(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->constRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::STARTAFB

		m_execTime = m_lmCmd->constRuntime;

		if (waitFbTime != 0)
		{
			m_waitTime += waitFbTime;
		}

		*fbExecTime = m_fbExecTime;
	}

	void CodeItem::exectime_nstart(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(waitFbTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->constRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::NSTART

		quint16 n = m_code.getWord3();

		m_execTime =  m_lmCmd->constRuntime + (m_fbExecTime + 3) * n;

		*fbExecTime = m_fbExecTime;
	}

	void CodeItem::exectime_write_bit_or_word_mem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(waitFbTime);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->writeToBitMemRuntime != LmCommand::UNDEFINED_PARAM);
		Q_ASSERT(m_lmCmd->writeToWordMemRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::MOV
		// LmCommand::MOVC
		// LmCommand::MOVBC
		// LmCommand::MOVB
		// LmCommand::MOVB_ADDR_ACC
		// LmCommand::MOVCMPF
		// LmCommand::PMOV
		// LmCommand::FILLB

		m_execTime = isAddrInBitMem(lmDesc, m_code.getWord2()) == true ?
							m_lmCmd->writeToBitMemRuntime :
							m_lmCmd->writeToWordMemRuntime;
	}

	void CodeItem::exectime_movmem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(waitFbTime);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->constRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::MOVMEM

		quint16 n = m_code.getWord4();

		m_execTime = m_lmCmd->constRuntime + (n - 1) * 8;
	}

	void CodeItem::exectime_setmem(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(waitFbTime);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->constRuntime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::SETMEM

		quint16 n = m_code.getWord4();

		m_execTime = m_lmCmd->constRuntime + (n - 1) * 4;
	}

	void CodeItem::exectime_rdfb(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(lmDesc);
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->preFbReadWordTime != LmCommand::UNDEFINED_PARAM);
		Q_ASSERT(m_lmCmd->postFbReadWordTime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::RDFB
		// LmCommand::RDFB32

		m_execTime = calcRdFbRuntime(m_waitTime, m_lmCmd->preFbReadWordTime,
									 waitFbTime, m_lmCmd->postFbReadWordTime);
	}

	void CodeItem::exectime_rdfb_bit(LmDescriptionConstShared lmDesc, int waitFbTime, int* fbExecTime)
	{
		Q_UNUSED(fbExecTime);

		TEST_PTR_RETURN(m_lmCmd);

		Q_ASSERT(m_lmCmd->preFbReadWordTime != LmCommand::UNDEFINED_PARAM);
		Q_ASSERT(m_lmCmd->postFbReadWordTime != LmCommand::UNDEFINED_PARAM);
		Q_ASSERT(m_lmCmd->preFbReadBitTime != LmCommand::UNDEFINED_PARAM);
		Q_ASSERT(m_lmCmd->postFbReadBitTime != LmCommand::UNDEFINED_PARAM);

		// LmCommand::RDFBB

		if (isAddrInBitMem(lmDesc, m_code.getWord3()) == true)
		{
			m_execTime = calcRdFbRuntime(m_waitTime, m_lmCmd->preFbReadBitTime,
										 waitFbTime, m_lmCmd->postFbReadBitTime);
		}
		else
		{
			m_execTime = calcRdFbRuntime(m_waitTime, m_lmCmd->preFbReadWordTime,
										 waitFbTime, m_lmCmd->postFbReadWordTime);
		}
	}
}
