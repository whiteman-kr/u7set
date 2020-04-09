#pragma once

#include <cassert>
#include "../lib/Types.h"

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

bool operator == (const Address16& addr1, const Address16& addr2);
bool operator != (const Address16& addr1, const Address16& addr2);

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
