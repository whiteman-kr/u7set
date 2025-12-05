#include "CfgServer.h"

#include "../UtilsLib/XmlHelper.h"

// -------------------------------------------------------------------------------------
//
// CfgServer class implementation
//
// -------------------------------------------------------------------------------------

CfgServer::CfgServer(const SoftwareInfo& softwareInfo,
	const SessionParams& sessionParams,
	const QString& buildFolder,
	const std::vector<ClientInfo> &clients,
	bool checkClientHostname,
	CircularLoggerShared logger) :
	Tcp::FileServer(buildFolder, softwareInfo, logger, "CfgServer"),
	m_sessionParams(sessionParams),
	m_knownClients(clients),
	m_checkClientHostname(checkClientHostname)
{
}

Tcp::Server* CfgServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
{
	CfgServer* newServer =  new CfgServer(localSoftwareInfo(),
										 m_sessionParams,
										 m_rootFolder,
										 m_knownClients,
										 m_checkClientHostname,
										 log());
	newServer->setListenAddress(listenAddr);

	qDebug() << "Create CfgServer instance";

	return newServer;
}

void CfgServer::processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	Q_UNUSED(requestData);
	Q_UNUSED(requestDataSize);

	switch(requestID)
	{
	case RQID_GET_SESSION_PARAMS:
		processGetSessionParamsRequest();
		break;

	default:
		logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
	}
}

void CfgServer::onServerThreadStarted()
{
	m_buildXmlPathFileName = m_rootFolder + File::SLASH_BUILD_XML;

	readBuildXml();
}

void CfgServer::onServerThreadFinished()
{
}

Tcp::SetConnectionResult CfgServer::checkClient(const QString& clientEquipmentID, const QString& clientHostname) const
{
	for(const auto& ci : m_knownClients)
	{
		if (clientEquipmentID != ci.equipmentID)
		{
			continue;
		}

		if (m_checkClientHostname == true)
		{
			if (clientHostname != ci.hostname)
			{
				return Tcp::SetConnectionResult::WrongClientHostname;
			}
		}

		return Tcp::SetConnectionResult::Ok;
	}

	return Tcp::SetConnectionResult::UnknownClientID;
}

void CfgServer::readBuildXml()
{
	QDir dir(m_buildXmlPathFileName);

	if (dir.exists(m_buildXmlPathFileName) == false)
	{
		m_errorCode = ErrorCode::BuildNotFound;
		logError(QString("file %1 not found!").arg(m_buildXmlPathFileName));
		return;
	}

	QFile buildXml(m_buildXmlPathFileName);

	if (buildXml.open(QIODevice::ReadOnly) == false)
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	QByteArray data = buildXml.readAll();

	buildXml.close();

	if (data.isEmpty())
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	XmlReadHelper xmlReader(data);

	bool res = m_buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		m_errorCode = ErrorCode::BuildCantRead;
		logError(QString("сan't read <BuildInfo> section in file %1!").arg(m_buildXmlPathFileName));
		return;
	}

	res = xmlReader.findElement(XmlElement::FILES);

	if (res == false)
	{
		m_errorCode = ErrorCode::BuildCantRead;
		logError(QString("сan't read <Files> section in file %1!").arg(m_buildXmlPathFileName));
		return;
	}

	if (res == true)
	{
		do
		{
			bool r = xmlReader.findElement(XmlElement::FILE);

			if (r == false)
			{
				break;
			}

			OnlineLib::BuildFileInfo bfi;

			res &= bfi.readFromXml(xmlReader, false);

			if (res == false)
			{
				break;
			}

			m_buildFileInfo.emplace(bfi.pathFileName, bfi);
		}
		while(true);
	}

	if (res == true)
	{
		logMessage(QString("file %1 has been read").arg(m_buildXmlPathFileName));
	}
	else
	{
		m_errorCode = ErrorCode::BuildCantRead;
		logError(QString("File %1 reading error!").arg(m_buildXmlPathFileName));
	}
}

bool CfgServer::checkFile(QString& pathFileName, QByteArray& fileData)
{
	if (m_buildFileInfo.contains(pathFileName) == false)
	{
		return false;
	}

	QString fileMd5 = m_buildFileInfo[pathFileName].md5;
	QString calculatedMd5 = QCryptographicHash::hash(fileData, QCryptographicHash::Md5).toHex();

	if (fileMd5 != calculatedMd5)
	{
		return false;
	}

	return true;
}

void CfgServer::processGetSessionParamsRequest()
{
	Network::SessionParams sp;

	m_sessionParams.saveTo(&sp);

	sendReply(sp);
}
