#include "../include/TcpFileTransfer.h"

namespace Tcp
{

	FileClient::FileClient(const QString& rootFolder, const HostAddressPort &serverAddressPort) :
		Client(serverAddressPort),
		m_rootFolder(rootFolder)
	{
	}


	FileClient::FileClient(const QString &rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
		Client(serverAddressPort1, serverAddressPort2),
		m_rootFolder(rootFolder)
	{

	}


	bool FileClient::isWriteable(const QString& folder)
	{
		// implement!!!

		return true;
	}


}
