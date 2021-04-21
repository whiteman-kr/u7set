#pragma once

#include "../lib/Types.h"
#include "../lib/ConstStrings.h"

// ----------------------------------------------------------------------------------------
//
// Class represent address in word-memory
//
// offset	- in memory in 16-bit words
// bitNo	- bit no in word 0..15
//
// ----------------------------------------------------------------------------------------

const int BAD_ADDRESS = -1;

class Address16
{
public:
	Address16();
	Address16(const Address16& addr);
	Address16(int offset, int bit);

	Address16& operator = (const Address16& addr);

	void set(int offset, int bit);
	void reset();

	void setOffset(int offset);
	int offset() const;

	void setBit(int bit);
	int bit() const;

	void setBitAddress(int bitAddress);
	int bitAddress() const;

	bool isValid() const;

	void addWord(int wordCount);
	void addBit(int bitCount);

	void add1Word();
	void add1Bit();

	int wordAlign();

	QString toString(bool zeroPadded = false) const;
	void fromString(const QString& str, bool* ok = nullptr);

private:
	int m_offset = BAD_ADDRESS;
	int m_bit = BAD_ADDRESS;
};

//

class SignalAddress16 : public Address16
{
public:
	SignalAddress16();
	SignalAddress16(const Address16& addr16);
	SignalAddress16(const SignalAddress16& addr);

	SignalAddress16& operator = (const Address16& addr16);

	void setAddress(const Address16& addr);

	void setSignalType(E::SignalType st);
	E::SignalType signalType() const;

	void setDataFormat(E::DataFormat df);
	E::DataFormat dataFormat() const;

	void setDataSize(int ds);
	int dataSize() const;

	void setByteOrder(E::ByteOrder bo);
	E::ByteOrder byteOrder() const;

private:
	E::SignalType m_signalType = E::SignalType::Discrete;
	E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
	int m_dataSize = 0;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
};

// ------------------------------------------------------------------------------------
//
// Address16 class implementation
//
// ------------------------------------------------------------------------------------

inline Address16::Address16()
{
}

inline Address16::Address16(const Address16& addr) :
	m_offset(addr.m_offset),
	m_bit(addr.m_bit)
{
}

inline Address16::Address16(int offset, int bit) :
	m_offset(offset),
	m_bit(bit)
{
}

inline Address16& Address16::operator = (const Address16& addr)
{
	m_offset = addr.m_offset;
	m_bit = addr.m_bit;

	return *this;
}

inline void Address16::set(int offset, int bit)
{
	m_offset = offset;
	m_bit = bit;
}

inline void Address16::reset()
{
	m_offset = BAD_ADDRESS;
	m_bit = BAD_ADDRESS;
}

inline void Address16::setOffset(int offset)
{
	m_offset = offset;
}

inline int Address16::offset() const
{
	return m_offset;
}

inline void Address16::setBit(int bit)
{
	m_bit = bit;
}

inline int Address16::bit() const
{
	return m_bit;
}

inline void Address16::setBitAddress(int bitAddress)
{
	m_offset = bitAddress / SIZE_16BIT;
	m_bit = bitAddress % SIZE_16BIT;
}

inline int Address16::bitAddress() const
{
	return m_offset * SIZE_16BIT + m_bit;
}

inline bool Address16::isValid() const
{
	return m_offset != BAD_ADDRESS && m_bit != BAD_ADDRESS;
}

inline void Address16::addWord(int wordCount)
{
	Q_ASSERT(isValid() == true);

	m_offset += wordCount;
}

inline void Address16::addBit(int bitCount)
{
	Q_ASSERT(isValid() == true);

	setBitAddress(bitAddress() + bitCount);
}

inline void Address16::add1Word()
{
	addWord(1);
}

inline void Address16::add1Bit()
{
	addBit(1);
}

inline int Address16::wordAlign()
{
	Q_ASSERT(isValid() == true);

	int offset = 0;

	if (m_bit != 0)
	{
		m_offset++;
		m_bit = 0;

		offset = 1;
	}

	return offset;
}

inline QString Address16::toString(bool zeroPadded) const
{
	if (zeroPadded == true)
	{
		return QString("%1:%2").arg(m_offset, 5, 10, Latin1Char::ZERO).arg(m_bit, 2, 10, Latin1Char::ZERO);
	}

	return QString("%1:%2").arg(m_offset).arg(m_bit);
}

inline void Address16::fromString(const QString& str, bool* ok)
{
	const QStringList& list = str.split(":");

	bool ok1 = false;

	m_offset = list[0].toInt(&ok1);

	bool ok2 = false;

	m_bit = list[1].toInt(&ok2);

	if (ok != nullptr)
	{
		*ok = ok1 && ok2;
	}
}

inline bool operator == (const Address16& addr1, const Address16& addr2)
{
	return addr1.offset() == addr2.offset() && addr1.bit() == addr2.bit();
}

inline bool operator != (const Address16& addr1, const Address16& addr2)
{
	return addr1.offset() != addr2.offset() || addr1.bit() != addr2.bit();
}

// ------------------------------------------------------------------------------------
//
// SignalAddress16 class implementation
//
// ------------------------------------------------------------------------------------

inline SignalAddress16::SignalAddress16()
{
}

inline SignalAddress16::SignalAddress16(const Address16& addr16) :
	Address16(addr16)
{
}

inline SignalAddress16::SignalAddress16(const SignalAddress16& addr) :
	Address16(addr),
	m_signalType(addr.m_signalType),
	m_dataFormat(addr.m_dataFormat),
	m_dataSize(addr.m_dataSize),
	m_byteOrder(addr.m_byteOrder)
{
}

inline SignalAddress16& SignalAddress16::operator = (const Address16& addr16)
{
	setOffset(addr16.offset());
	setBit(addr16.bit());

	return *this;
}

inline void SignalAddress16::setAddress(const Address16& addr)
{
	(*this) = addr;
}

inline void SignalAddress16::setSignalType(E::SignalType st)
{
	m_signalType = st;
}

inline E::SignalType SignalAddress16::signalType() const
{
	return m_signalType;
}

inline void SignalAddress16::setDataFormat(E::DataFormat df)
{
	m_dataFormat = df;
}

inline E::DataFormat SignalAddress16::dataFormat() const
{
	return m_dataFormat;
}

inline void SignalAddress16::setDataSize(int ds)
{
	m_dataSize = ds;
}

inline int SignalAddress16::dataSize() const
{
	return m_dataSize;
}

inline void SignalAddress16::setByteOrder(E::ByteOrder bo)
{
	m_byteOrder = bo;
}

inline E::ByteOrder SignalAddress16::byteOrder() const
{
	return m_byteOrder;
}
