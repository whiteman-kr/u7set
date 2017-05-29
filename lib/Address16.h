#pragma once

#include <cassert>
#include "../lib/Types.h"

// ----------------------------------------------------------------------------------------
//
// Class represent address in word-memory
//
// offset	- in memory in 16-bit words
// bitNo	- in word in bits 0..15
//
// ----------------------------------------------------------------------------------------

const int BAD_ADDRESS = -1;

class Address16
{
private:
	int m_offset = BAD_ADDRESS;
	int m_bit = BAD_ADDRESS;

public:
	Address16() {}
	Address16(const Address16& addr) : m_offset(addr.m_offset), m_bit(addr.m_bit) {}
	Address16(int offset, int bit) : m_offset(offset), m_bit(bit) {}

	void set(int offset, int bit) { m_offset = offset; m_bit = bit; }
	void setOffset(int offset) { m_offset = offset; }
	void setBit(int bit) { m_bit = bit; }

	int addWord(int wordCount)
	{
		assert(isValid());

		m_offset += wordCount;
		return wordCount;
	}

	int addBit(int bitCount)
	{
		assert(isValid());

		int old_offset = m_offset;
		int totalBitCount = m_offset * 16 + m_bit + bitCount;

		m_offset = totalBitCount / 16;
		m_bit = totalBitCount % 16;

		return m_offset - old_offset;
	}

	void add1Word() { addBit(16); }
	void add1Bit() { addBit(1); }

	int wordAlign()
	{
		assert(isValid());

		int offset = 0;

		if (m_bit != 0)
		{
			m_offset++;
			m_bit = 0;

			offset = 1;
		}

		return offset;
	}

	int offset() const
	{
		return m_offset;
	}

	int bit() const
	{
		return m_bit;
	}

	void reset() { 	m_offset = BAD_ADDRESS; m_bit = BAD_ADDRESS; }

	bool isValid() const { return m_offset != BAD_ADDRESS && m_bit != BAD_ADDRESS; }

	QString toString() const
	{
		return QString("%1:%2").arg(m_offset).arg(m_bit);
	}

	void fromString(QString str)
	{
		assert(isValid());

		const QStringList& list = str.split(":");
		m_offset = list[0].toInt();
		m_bit = list[1].toInt();
	}

	int bitAddress() const { return m_offset * 16 + m_bit; }
};


class SignalAddress16 : public Address16
{
public:
	SignalAddress16() {}

	SignalAddress16(const Address16& addr16) :
		Address16(addr16)
	{
	}

	SignalAddress16(const SignalAddress16& addr) :
		Address16(addr)
	{
		m_signalType = addr.m_signalType;
		m_dataFormat = addr.m_dataFormat;
		m_dataSize = addr.m_dataSize;
		m_byteOrder = addr.m_byteOrder;
	}

	SignalAddress16& operator = (const Address16& addr16)
	{
		setOffset(addr16.offset());
		setBit(addr16.bit());

		return *this;
	}

	void setAddress(const Address16& addr) { (*this) = addr; }

	void setSignalType(E::SignalType st) { m_signalType = st; }
	E::SignalType signalType() const { return m_signalType; }

	void setDataFormat(E::DataFormat df) { m_dataFormat = df; }
	E::DataFormat dataFormat() const { return m_dataFormat; }

	void setDataSize(int ds) { m_dataSize = ds; }
	int dataSize() const { return m_dataSize; }

	void setByteOrder(E::ByteOrder bo) { m_byteOrder = bo; }
	E::ByteOrder byteOrder() const { return m_byteOrder; }

private:
	E::SignalType m_signalType = E::SignalType::Discrete;
	E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
	int m_dataSize = 0;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
};
