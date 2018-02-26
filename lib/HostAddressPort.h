#pragma once

#include <QtGlobal>
#include <QHostAddress>

class HostAddressPort
{
private:
	QHostAddress m_hostAddress;
	quint16 m_port = 0;

public:
	HostAddressPort() {}

	HostAddressPort(const QHostAddress& addr, quint16 port);
	HostAddressPort(quint32 ip4Addr, quint16 port);
	HostAddressPort(quint8* ip6Addr, quint16 port);
	HostAddressPort(const Q_IPV6ADDR& ip6Addr, quint16 port);
	HostAddressPort(const sockaddr* sockaddr, quint16 port);
	HostAddressPort(const QString& address, quint16 port);

	HostAddressPort(const HostAddressPort &copy);

	HostAddressPort& operator=(const HostAddressPort &other);
	bool operator==(const HostAddressPort &other);
	bool operator!=(const HostAddressPort &other);

	void setAddress(quint32 ip4Addr);
	void setAddress(quint8* ip6Addr);
	void setAddress(const Q_IPV6ADDR& ip6Addr);
	void setAddress(const sockaddr* sockaddr);
	bool setAddress(const QString& address);
	void setPort(quint16 port);

	bool setAddressPort(const QString& addressPortStr, quint16 defaultPort);

	quint32 address32() const;
	QHostAddress address() const;

	quint16 port() const;

	QString addressPortStr() const;
	QString addressStr() const;

	void clear();

	static bool isValidIPv4(const QString& ipAddressStr);
	static bool isValidPort(const QString& portStr);

	// splitAddressPortStr parse strings matching: 192.168.1.1:1234
	//
	// return true if converion success
	// return false if converion failed
	// if port number is not pesent in addressPortStr, variable port is set to defaultPort
	//
	static bool splitAddressPortStr(const QString& addressPortStr, QString* addressStr, quint16* port, quint16 defaultPort);
};

