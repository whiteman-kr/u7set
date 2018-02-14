#include "HostAddressPort.h"
#include <cassert>

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


HostAddressPort::HostAddressPort(quint8* ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const Q_IPV6ADDR& ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const sockaddr* sockaddr, quint16 port)
{
	m_hostAddress.setAddress(sockaddr);
	m_port = port;
}


HostAddressPort::HostAddressPort(const QString& address, quint16 port)
{
	m_hostAddress.setAddress(address);
	m_port = port;
}


HostAddressPort::HostAddressPort(const HostAddressPort& copy)
{
	m_hostAddress = copy.m_hostAddress;
	m_port = copy.m_port;
}


HostAddressPort& HostAddressPort::operator=(const HostAddressPort& other)
{
	m_hostAddress = other.m_hostAddress;
	m_port = other.m_port;

	return *this;
}

bool HostAddressPort::operator==(const HostAddressPort& other)
{
	return m_hostAddress == other.m_hostAddress && m_port == other.m_port;
}

bool HostAddressPort::operator!=(const HostAddressPort& other)
{
	return m_hostAddress != other.m_hostAddress || m_port != other.m_port;
}


void HostAddressPort::setAddress(quint32 ip4Addr)
{
	m_hostAddress.setAddress(ip4Addr);
}


void HostAddressPort::setAddress(quint8* ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}


void HostAddressPort::setAddress(const Q_IPV6ADDR& ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}


void HostAddressPort::setAddress(const sockaddr* sockaddr)
{
	m_hostAddress.setAddress(sockaddr);
}


bool HostAddressPort::setAddress(const QString& address)
{
	return m_hostAddress.setAddress(address);
}


void HostAddressPort::setPort(quint16 port)
{
	m_port = port;
}

bool HostAddressPort::setAddressPort(const QString& addressPortStr, quint16 defaultPort)
{
	QString addrStr;
	quint16 port = 0;

	bool result = HostAddressPort::splitAddressPortStr(addressPortStr, &addrStr, &port, defaultPort);

	if (result == false)
	{
		return false;
	}

	setAddress(addrStr);
	setPort(port);

	return true;
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


bool HostAddressPort::isValidIPv4(const QString& ipAddressStr)
{
	QHostAddress addr;

	return addr.setAddress(ipAddressStr);
}

bool HostAddressPort::isValidPort(const QString& portStr)
{
	bool ok = false;

	int port = portStr.toUInt(&ok);

	if (ok == false)
	{
		return false;
	}

	return port >= 0 && port <= 65535;
}


bool HostAddressPort::splitAddressPortStr(const QString& addressPortStr, QString* addressStr, quint16 *port, quint16 defaultPort)
{
	if (addressStr == nullptr || port == nullptr)
	{
		assert(false);
		return false;
	}

	addressStr->clear();
	*port = 0;

	QStringList strList = addressPortStr.split(":", QString::SplitBehavior::SkipEmptyParts);

	if (strList.size() == 0)
	{
		return false;
	}

	bool addrOk = false;

	if (strList.size() > 0)
	{
		*addressStr = strList.at(0);

		addrOk = isValidIPv4(*addressStr);
	}

	bool portOk = false;

	if (strList.size() > 1)
	{
		if (isValidPort(strList.at(1)) == true)
		{
			*port = static_cast<quint16>(strList.at(1).toInt());
			portOk = true;
		}
	}
	else
	{
		*port = defaultPort;
		portOk = true;
	}

	if (strList.size() > 2)
	{
		return false;
	}

	return addrOk && portOk;
}
