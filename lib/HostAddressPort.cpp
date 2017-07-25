#include "HostAddressPort.h"

// -------------------------------------------------------------------------------
//
// HostAddressPort class implementation
//
// -------------------------------------------------------------------------------

HostAddressPort::HostAddressPort(const QHostAddress& addr, quint16 port)
{
	m_hostAddress = addr;
	m_port = port;
}


HostAddressPort::HostAddressPort(quint32 ip4Addr, quint16 port)
{
	m_hostAddress.setAddress(ip4Addr);
	m_port = port;
}


HostAddressPort::HostAddressPort(quint8 *ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const Q_IPV6ADDR &ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const sockaddr *sockaddr, quint16 port)
{
	m_hostAddress.setAddress(sockaddr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const QString &address, quint16 port)
{
	m_hostAddress.setAddress(address);
	m_port = port;
}


HostAddressPort::HostAddressPort(const HostAddressPort &copy)
{
	m_hostAddress = copy.m_hostAddress;
	m_port = copy.m_port;
}


HostAddressPort& HostAddressPort::operator=(const HostAddressPort &other)
{
	m_hostAddress = other.m_hostAddress;
	m_port = other.m_port;

	return *this;
}

bool HostAddressPort::operator==(const HostAddressPort &other)
{
	return m_hostAddress == other.m_hostAddress && m_port == other.m_port;
}

bool HostAddressPort::operator!=(const HostAddressPort &other)
{
	return m_hostAddress != other.m_hostAddress && m_port != other.m_port;
}


void HostAddressPort::setAddress(quint32 ip4Addr)
{
	m_hostAddress.setAddress(ip4Addr);
}


void HostAddressPort::setAddress(quint8 *ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}


void HostAddressPort::setAddress(const Q_IPV6ADDR &ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}


void HostAddressPort::setAddress(const sockaddr *sockaddr)
{
	m_hostAddress.setAddress(sockaddr);
}


bool HostAddressPort::setAddress(const QString &address)
{
	return m_hostAddress.setAddress(address);
}


void HostAddressPort::setPort(quint16 port)
{
	m_port = port;
}


quint32 HostAddressPort::address32() const
{
	return m_hostAddress.toIPv4Address();
}

QHostAddress HostAddressPort::address() const
{
	return m_hostAddress;
}


quint16 HostAddressPort::port() const
{
	return m_port;
}


QString HostAddressPort::addressPortStr() const
{
	return QString("%1:%2").arg(address().toString()).arg(port());
}


QString HostAddressPort::addressStr() const
{
	return QString("%1").arg(address().toString());
}


void HostAddressPort::clear()
{
	m_hostAddress.setAddress(static_cast<quint32>(0));
	m_port = 0;
}
