#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../include/SocketIO.h"

class XmlHelper
{
public:
	static void writeHostAddressPortElement(QXmlStreamWriter& xml, const QString& name, HostAddressPort& hostAddressPort);
	static bool readHostAddressPortElement(QXmlStreamReader& xml, const QString& name, HostAddressPort *hostAddressPort);

	static void writeHostAddressElement(QXmlStreamWriter& xml, const QString& name, QHostAddress& hostAddress);
	static bool readHostAddressElement(QXmlStreamReader& xml, const QString& name, QHostAddress *hostAddress);
};
