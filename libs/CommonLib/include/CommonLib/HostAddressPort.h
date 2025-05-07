#pragma once

#include <QHostAddress>

class HostAddressPort
{
public:
	HostAddressPort() = default;
	HostAddressPort(const HostAddressPort &copy) = default;
	~HostAddressPort() = default;

	HostAddressPort(const QHostAddress& addr, quint16 port);
	HostAddressPort(const QHostAddress& addr, int port);
	HostAddressPort(quint32 ip4Addr, quint16 port);
	HostAddressPort(quint8* ip6Addr, quint16 port);
	HostAddressPort(const Q_IPV6ADDR& ip6Addr, quint16 port);
	HostAddressPort(const sockaddr* sockaddr, quint16 port);
	HostAddressPort(const QString& address, quint16 port);
	HostAddressPort(const QString& address, int port);

	HostAddressPort& operator=(const HostAddressPort &other) = default;
	bool operator == (const HostAddressPort &other) const;
	bool operator != (const HostAddressPort &other) const;
	bool operator < (const HostAddressPort &other) const;

	void clear();

	void setAny();
	bool isAny() const;

	void setAddress(quint32 ip4Addr);
	void setAddress(quint8* ip6Addr);
	void setAddress(const Q_IPV6ADDR& ip6Addr);
	void setAddress(const sockaddr* sockaddr);
	bool setAddress(const QString& address);
	void setSpecAddress(QHostAddress::SpecialAddress specAddr);
	void setPort(quint16 port);
	void setPort(int port);

	bool setAddressPort(const QString& addressStr, quint16 Port);
	bool setAddressPort(const QString& addressStr, int port);

	bool setAddressPortStr(const QString& addressPortStr, quint16 defaultPort);

	[[nodiscard]] quint32 address32() const;
	[[nodiscard]] quint32 address32IfSet() const;
	[[nodiscard]] QHostAddress address() const;

	[[nodiscard]] quint16 port() const;
	[[nodiscard]] quint16 portIfSet() const;

	[[nodiscard]] QString toString() const;
	[[nodiscard]] std::string toStdString() const;
	[[nodiscard]] QString addressPortStr() const;
	[[nodiscard]] QString addressPortStrWithoutPort0() const;
	[[nodiscard]] QString addressPortStrIfSet() const;

	[[nodiscard]] QString addressStr() const;
	[[nodiscard]] QString addressStrIfSet() const;

	[[nodiscard]] QString portStr() const;

	[[nodiscard]] quint64 addressPortValue() const;

	[[nodiscard]] bool isNull() const;
	[[nodiscard]] bool isSet() const;

	static bool isValidIPv4(const QString& ipAddressStr);
	static bool isValidPort(const QString& portStr);

	// splitAddressPortStr parse strings matching: 192.168.1.1:1234
	//
	// return true if converion success
	// return false if converion failed
	// if port number is not pesent in addressPortStr, variable port is set to defaultPort
	//
	static bool splitAddressPortStr(const QString& addressPortStr, QString* addressStr, quint16* port, quint16 defaultPort);

private:
	QHostAddress m_hostAddress;
	quint16 m_port = 0;

	static const QString NOT_SET;
};

inline const QString HostAddressPort::NOT_SET = "NotSet";

inline HostAddressPort::HostAddressPort(const QHostAddress& addr, quint16 port) :
	m_hostAddress(addr),
	m_port(port)
{
}

inline HostAddressPort::HostAddressPort(const QHostAddress& addr, int port) :
	m_hostAddress(addr),
	m_port(static_cast<quint16>(port))
{
	Q_ASSERT(port >= 0 && port <= 65535);
}

inline HostAddressPort::HostAddressPort(quint32 ip4Addr, quint16 port)
{
	m_hostAddress.setAddress(ip4Addr);
	m_port = port;
}

inline HostAddressPort::HostAddressPort(quint8* ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}

inline HostAddressPort::HostAddressPort(const Q_IPV6ADDR& ip6Addr, quint16 port)
{
	m_hostAddress.setAddress(ip6Addr);
	m_port = port;
}

inline HostAddressPort::HostAddressPort(const sockaddr* sockaddr, quint16 port)
{
	m_hostAddress.setAddress(sockaddr);
	m_port = port;
}

inline HostAddressPort::HostAddressPort(const QString& address, quint16 port)
{
	m_hostAddress.setAddress(address);
	m_port = port;
}

inline HostAddressPort::HostAddressPort(const QString& address, int port)
{
	Q_ASSERT(port >= 0 && port <= 65535);

	m_hostAddress.setAddress(address);
	m_port = static_cast<quint16>(port);
}

inline bool HostAddressPort::operator == (const HostAddressPort& other) const
{
	return m_hostAddress == other.m_hostAddress && m_port == other.m_port;
}

inline bool HostAddressPort::operator != (const HostAddressPort& other) const
{
	return m_hostAddress != other.m_hostAddress || m_port != other.m_port;
}

inline bool HostAddressPort::operator < (const HostAddressPort& other) const
{
	return addressPortValue() < other.addressPortValue();
}

inline void HostAddressPort::clear()
{
	m_hostAddress.clear();
	m_port = 0;
}

inline void HostAddressPort::setAny()
{
	m_hostAddress = QHostAddress::AnyIPv4;
}

inline bool HostAddressPort::isAny() const
{
	return (m_hostAddress == QHostAddress::AnyIPv4);
}

inline void HostAddressPort::setAddress(quint32 ip4Addr)
{
	m_hostAddress.setAddress(ip4Addr);
}

inline void HostAddressPort::setAddress(quint8* ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}

inline void HostAddressPort::setAddress(const Q_IPV6ADDR& ip6Addr)
{
	m_hostAddress.setAddress(ip6Addr);
}

inline void HostAddressPort::setAddress(const sockaddr* sockaddr)
{
	m_hostAddress.setAddress(sockaddr);
}

inline bool HostAddressPort::setAddress(const QString& address)
{
	return m_hostAddress.setAddress(address);
}

inline void HostAddressPort::setSpecAddress(QHostAddress::SpecialAddress specAddr)
{
	m_hostAddress.setAddress(specAddr);
}

inline void HostAddressPort::setPort(quint16 port)
{
	m_port = port;
}

inline void HostAddressPort::setPort(int port)
{
	assert(port >= 0 && port <= 65535);

	m_port = static_cast<quint16>(port);
}

inline bool HostAddressPort::setAddressPort(const QString& addressStr, quint16 port)
{
	bool result = setAddress(addressStr);

	setPort(port);

	return result;
}

inline bool HostAddressPort::setAddressPort(const QString& addressStr, int port)
{
	return setAddressPort(addressStr, static_cast<quint16>(port));
}

inline bool HostAddressPort::setAddressPortStr(const QString& addressPortStr, quint16 defaultPort)
{
	QString addrStr;
	quint16 port = 0;

	m_hostAddress.clear();

	bool result = HostAddressPort::splitAddressPortStr(addressPortStr, &addrStr, &port, defaultPort);

	if (result == false)
	{
		return false;
	}

	setAddress(addrStr);
	setPort(port);

	return true;
}

inline quint32 HostAddressPort::address32() const
{
	if (m_hostAddress.protocol() != QHostAddress::IPv4Protocol)
	{
		Q_ASSERT(m_hostAddress.protocol() == QHostAddress::IPv4Protocol);
		return 0;
	}

	return m_hostAddress.toIPv4Address();
}

inline quint32 HostAddressPort::address32IfSet() const
{
	if (isSet() == true)
	{
		bool ok = true;
		quint32 ip32 = m_hostAddress.toIPv4Address(&ok);
		return (ok == true ? ip32 : 0);
	}

	return 0;
}

inline QHostAddress HostAddressPort::address() const
{
	return m_hostAddress;
}

inline quint16 HostAddressPort::port() const
{
	return m_port;
}

inline quint16 HostAddressPort::portIfSet() const
{
	if (isSet() == true)
	{
		return m_port;
	}

	return 0;
}

inline QString HostAddressPort::toString() const
{
	return addressPortStr();
}

inline std::string HostAddressPort::toStdString() const
{
	return addressPortStr().toStdString();
}

inline QString HostAddressPort::addressPortStr() const
{
	if (isSet() == true)
	{
		return QString("%1:%2").arg(address().toString()).arg(port());
	}

	return QString();
}

inline QString HostAddressPort::addressPortStrWithoutPort0() const
{
	if (isSet() == true)
	{
		if (port() == 0)
		{
			return address().toString();
		}

		return QString("%1:%2").arg(address().toString()).arg(port());
	}

	return QString();
}

inline QString HostAddressPort::addressPortStrIfSet() const
{
	if (isSet() == true)
	{
		return addressPortStr();
	}

	return NOT_SET;
}

inline QString HostAddressPort::addressStr() const
{
	return address().toString();
}

inline QString HostAddressPort::addressStrIfSet() const
{
	if (isSet() == true)
	{
		return address().toString();
	}

	return NOT_SET;
}

inline QString HostAddressPort::portStr() const
{
	return QString::number(m_port);
}

inline bool HostAddressPort::isNull() const
{
	return m_hostAddress.isNull();
}

inline bool HostAddressPort::isSet() const
{
	return isNull() == false;
}

inline bool HostAddressPort::isValidIPv4(const QString& ipAddressStr)
{
	QHostAddress addr;

	return addr.setAddress(ipAddressStr);
}

inline bool HostAddressPort::isValidPort(const QString& portStr)
{
	bool ok = false;

	int port = portStr.toInt(&ok);

	if (ok == false)
	{
		return false;
	}

	return port >= 0 && port <= 65535;
}

inline bool HostAddressPort::splitAddressPortStr(const QString& addressPortStr, QString* addressStr,
												 quint16* port, quint16 defaultPort)
{
	if (addressStr == nullptr || port == nullptr)
	{
		assert(false);
		return false;
	}

	addressStr->clear();
	*port = 0;

	QStringList strList = addressPortStr.split(":", Qt::KeepEmptyParts);

	if (strList.size() == 0)
	{
		return false;
	}

	bool addrOk = false;

	if (strList.size() > 0)
	{
		const QString& addrStr = strList.at(0);

		if (addrStr.isEmpty() == true)
		{
			return false;
		}

		if (isValidIPv4(addrStr) == false)
		{
			return false;
		}

		*addressStr = addrStr;

		addrOk = true;
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

inline quint64 HostAddressPort::addressPortValue() const
{
	bool ok = false;

	quint64 v1 = m_hostAddress.toIPv4Address(&ok);

	Q_ASSERT(ok == true);

	v1 = (v1 << 16) | m_port;

	return v1;
}
