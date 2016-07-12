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

	explicit HostAddressPort(const QHostAddress& addr, quint16 port);
	explicit HostAddressPort(quint32 ip4Addr, quint16 port);
	explicit HostAddressPort(quint8 *ip6Addr, quint16 port);
	explicit HostAddressPort(const Q_IPV6ADDR &ip6Addr, quint16 port);
	explicit HostAddressPort(const sockaddr *sockaddr, quint16 port);
	explicit HostAddressPort(const QString &address, quint16 port);

	HostAddressPort(const HostAddressPort &copy);

	HostAddressPort &operator=(const HostAddressPort &other);

	void setAddress(quint32 ip4Addr);
	void setAddress(quint8 *ip6Addr);
	void setAddress(const Q_IPV6ADDR &ip6Addr);
	void setAddress(const sockaddr *sockaddr);
	bool setAddress(const QString &address);
	void setPort(quint16 port);

	quint32 address32() const;
	QHostAddress address() const;

	quint16 port() const;

	QString addressPortStr() const;
	QString addressStr() const;

	void clear();
};

