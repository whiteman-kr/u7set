#include "SimScriptRamAddress.h"

namespace Sim
{

	RamAddress::RamAddress(const Address16& addr16) :
		m_offset(addr16.isValid() ? addr16.offset() : BadAddress),
		m_bit(addr16.isValid() ? addr16.bit() : BadAddress)
	{
	}

	RamAddress::RamAddress(quint32 offset, quint32 bit) :
		m_offset(offset),
		m_bit(bit)
	{
	}

	bool RamAddress::isValid() const
	{
		return m_offset != BadAddress && m_bit != BadAddress;
	}

	quint32 RamAddress::offset() const
	{
		return m_offset;
	}

	void RamAddress::setOffset(quint32 value)
	{
		m_offset = value;
	}

	quint32 RamAddress::bit() const
	{
		return m_bit;
	}

	void RamAddress::setBit(quint32 value)
	{
		m_bit = value;
	}

	QString RamAddress::toString() const
	{
		return QString("Offset: %1 (%2), bit: %3, Access: %4");
	}

	quint32 RamAddress::bitAddress() const
	{
		return m_offset * 16 + m_bit;
	}


}
