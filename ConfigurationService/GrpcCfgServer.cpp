#include "GrpcCfgServer.h"

#include "../UtilsLib/XmlHelper.h"

// -------------------------------------------------------------------------------------
//
// GrpcCfgServer class implementation
//
// -------------------------------------------------------------------------------------

GrpcCfgServer::GrpcCfgServer(const SoftwareInfo& softwareInfo,
							const SessionParams& sessionParams,
							const std::vector<ClientInfo> &clients,
							bool checkClientHostname,
							const HostAddressPort& listenIP,
							const QString& buildFolder,
							CircularLoggerShared logger) :
	GrpcFileSrv(softwareInfo, false, clients, checkClientHostname, listenIP, buildFolder, logger),
	m_sessionParams(sessionParams)
{
	readBuildXml(buildFolder);
}

const OnlineLib::BuildInfo& GrpcCfgServer::buildInfo() const
{
	return m_buildInfo;
}

void GrpcCfgServer::readBuildXml(const QString& buildFolder)
{
	m_buildReadOK = false;

	QString buildXmlFileName = rootFolder() + File::SLASH_BUILD_XML;

	QDir dir(buildXmlFileName);

	if (dir.exists(buildXmlFileName) == false)
	{
		DEBUG_LOG_ERR(m_log, QString("file %1 not found!").arg(buildXmlFileName));
		return;
	}

	QFile buildXml(buildXmlFileName);

	if (buildXml.open(QIODevice::ReadOnly) == false)
	{
		return;
	}

	QByteArray data = buildXml.readAll();

	buildXml.close();

	if (data.isEmpty())
	{
		return;
	}

	XmlReadHelper xmlReader(data);

	bool res = m_buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		DEBUG_LOG_ERR(m_log, QString("сan't read <BuildInfo> section in file %1!").arg(buildXmlFileName));
		return;
	}

	res = xmlReader.findElement(XmlElement::FILES);

	if (res == false)
	{
		DEBUG_LOG_ERR(m_log, QString("сan't read <Files> section in file %1!").arg(buildXmlFileName));
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

	if (res == false)
	{
		DEBUG_LOG_ERR(m_log, QString("File %1 reading error!").arg(buildXmlFileName));
		return;
	}

	DEBUG_LOG_MSG(m_log, QString("file %1 has been read").arg(buildXmlFileName));

	m_buildReadOK = true;
}

bool GrpcCfgServer::checkFile(const QString& pathFileName, const QByteArray& fileData) const
{
	auto it = m_buildFileInfo.find(pathFileName);

	if (it == m_buildFileInfo.end())
	{
		return false;
	}

	const OnlineLib::BuildFileInfo& bfi = it->second;

	QByteArray dataMd5 = GrpcFileBase::getMd5(fileData);

	if (bfi.size != fileData.size() ||
		QByteArray::fromHex(bfi.md5.toUtf8()) != dataMd5.toHex())
	{
		return false;
	}

	return true;
}

void GrpcCfgServer::getSessionParams(Network::SessionParams* params) const
{
	m_sessionParams.saveTo(params);
}
