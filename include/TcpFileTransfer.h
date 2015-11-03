#pragma once

#include "../include/Tcp.h"

namespace Tcp
{

	class FileClient : public Client
	{
		Q_OBJECT

	private:
		QString m_rootFolder;

		bool isWriteable(const QString& folder);

	public:
		FileClient(const QString &rootFolder, const HostAddressPort &serverAddressPort);
		FileClient(const QString &rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);


	};

}
