#include "Address16.h"

Address16::Address16()
{
}

Address16::Address16(const Address16& addr) :
	m_offset(addr.m_offset),
	m_bit(addr.m_bit)
{
}

Address16::Address16(int offset, int bit) :
	m_offset(offset),
	m_bit(bit)
{
}

Address16& Address16::operator = (const Address16& addr)
{
	m_offset = addr.m_offset;
	m_bit = addr.m_bit;

	return *this;
}

void Address16::set(int offset, int bit)
{
	m_offset = offset;
	m_bit = bit;
}

void Address16::reset()
{
	m_offset = BAD_ADDRESS;
	m_bit = BAD_ADDRESS;
}

void Address16::setOffset(int offset)
{
	m_offset = offset;
}

int Address16::offset() const
{
	return m_offset;
}

void Address16::setBit(int bit)
{
	m_bit = bit;
}

int Address16::bit() const
{
	return m_bit;
}

void Address16::setBitAddress(int bitAddress)
{
	m_offset = bitAddress / SIZE_16BIT;
	m_bit = bitAddress % SIZE_16BIT;
}

int Address16::bitAddress() const
{
	return m_offset * SIZE_16BIT + m_bit;
}

bool Address16::isValid() const
{
	return m_offset != BAD_ADDRESS && m_bit != BAD_ADDRESS;
}

void Address16::addWord(int wordCount)
{
	assert(isValid() == true);

	m_offset += wordCount;
}

void Address16::addBit(int bitCount)
{
	assert(isValid() == true);

	setBitAddress(bitAddress() + bitCount);
}

void Address16::add1Word()
{
	addWord(1);
}

void Address16::add1Bit()
{
	addBit(1);
}

int Address16::wordAlign()
{
	assert(isValid() == true);

	int offset = 0;

	if (m_bit != 0)
	{
		m_offset++;
		m_bit = 0;

		offset = 1;
	}

	return offset;
}

QString Address16::toString(bool zeroPadded) const
{
	if (zeroPadded == true)
	{
		return QString("%1:%2").arg(m_offset, 5, 10, QLatin1Char('0')).arg(m_bit, 2, 10, QLatin1Char('0'));
	}

	return QString("%1:%2").arg(m_offset).arg(m_bit);
}

void Address16::fromString(const QString& str, bool* ok)
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

bool operator == (const Address16& addr1, const Address16& addr2)
{
	return addr1.offset() == addr2.offset() && addr1.bit() == addr2.bit();
}

bool operator != (const Address16& addr1, const Address16& addr2)
{
	return addr1.offset() != addr2.offset() || addr1.bit() != addr2.bit();
}

//

SignalAddress16::SignalAddress16()
{
}

SignalAddress16::SignalAddress16(const Address16& addr16) :
	Address16(addr16)
{
}

SignalAddress16::SignalAddress16(const SignalAddress16& addr) :
	Address16(addr),
	m_signalType(addr.m_signalType),
	m_dataFormat(addr.m_dataFormat),
	m_dataSize(addr.m_dataSize),
	m_byteOrder(addr.m_byteOrder)
{
}

SignalAddress16& SignalAddress16::operator = (const Address16& addr16)
{
	setOffset(addr16.offset());
	setBit(addr16.bit());

	return *this;
}

void SignalAddress16::setAddress(const Address16& addr)
{
	(*this) = addr;
}

void SignalAddress16::setSignalType(E::SignalType st)
{
	m_signalType = st;
}

E::SignalType SignalAddress16::signalType() const
{
	return m_signalType;
}

void SignalAddress16::setDataFormat(E::DataFormat df)
{
	m_dataFormat = df;
}

E::DataFormat SignalAddress16::dataFormat() const
{
	return m_dataFormat;
}

void SignalAddress16::setDataSize(int ds)
{
	m_dataSize = ds;
}

int SignalAddress16::dataSize() const
{
	return m_dataSize;
}

void SignalAddress16::setByteOrder(E::ByteOrder bo)
{
	m_byteOrder = bo;
}

E::ByteOrder SignalAddress16::byteOrder() const
{
	return m_byteOrder;
}
