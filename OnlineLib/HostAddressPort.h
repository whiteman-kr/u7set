#pragma once

#include <QtGlobal>
#include <QHostAddress>

class HostAddressPort
{
public:
	HostAddressPort();
	HostAddressPort(const HostAddressPort &copy);

	HostAddressPort(const QHostAddress& addr, quint16 port);
	HostAddressPort(const QHostAddress& addr, int port);
	HostAddressPort(quint32 ip4Addr, quint16 port);
	HostAddressPort(quint8* ip6Addr, quint16 port);
	HostAddressPort(const Q_IPV6ADDR& ip6Addr, quint16 port);
	HostAddressPort(const sockaddr* sockaddr, quint16 port);
	HostAddressPort(const QString& address, quint16 port);
	HostAddressPort(const QString& address, int port);

	HostAddressPort& operator=(const HostAddressPort &other);
	bool operator==(const HostAddressPort &other);
	bool operator!=(const HostAddressPort &other);

	void setAddress(quint32 ip4Addr);
	void setAddress(quint8* ip6Addr);
	void setAddress(const Q_IPV6ADDR& ip6Addr);
	void setAddress(const sockaddr* sockaddr);
	bool setAddress(const QString& address);
	void setPort(quint16 port);
	void setPort(int port);

	bool setAddressPort(const QString& addressStr, quint16 Port);
	bool setAddressPort(const QString& addressStr, int port);

	bool setAddressPortStr(const QString& addressPortStr, quint16 defaultPort);

	quint32 address32() const;
	QHostAddress address() const;

	quint16 port() const;

	QString addressPortStr() const;
	QString addressPortStrIfSet() const;

	QString addressStr() const;
	QString addressStrIfSet() const;

	void clear();
	bool isEmpty() const;
	bool isSet() const;

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

	static const char* const NOT_SET;
};
