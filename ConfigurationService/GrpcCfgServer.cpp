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
	readBuildXml();
}

const OnlineLib::BuildInfo& GrpcCfgServer::buildInfo() const
{
	return m_buildInfo;
}

const std::unordered_map<QString, OnlineLib::BuildFileInfo>& GrpcCfgServer::buildFilesInfo() const
{
	return m_buildFilesInfo;
}

void GrpcCfgServer::readBuildXml()
{
	m_buildReadOK = false;

	QString buildXmlFileName = rootFolder() + File::SLASH_BUILD_XML;

	QDir dir(buildXmlFileName);

	if (dir.exists(buildXmlFileName) == false)
	{
		logErr(QString("file %1 not found!").arg(buildXmlFileName));
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
		logErr(QString("сan't read <BuildInfo> section in file %1!").arg(buildXmlFileName));
		return;
	}

	res = xmlReader.findElement(XmlElement::FILES);

	if (res == false)
	{
		logErr(QString("сan't read <Files> section in file %1!").arg(buildXmlFileName));
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

			m_buildFilesInfo.emplace(bfi.pathFileName, bfi);
		}
		while(true);
	}

	if (res == false)
	{
		logErr(QString("File %1 reading error!").arg(buildXmlFileName));
		return;
	}

	logMsg(QString("file %1 has been read").arg(buildXmlFileName));

	m_buildReadOK = true;
}

bool GrpcCfgServer::checkFile(const QString& pathFileName, const QByteArray& fileData, QString& md5) const
{
	GrpcFileSrv::checkFile(pathFileName, fileData, md5);

	auto it = m_buildFilesInfo.find(pathFileName);

	if (it == m_buildFilesInfo.end())
	{
		return false;
	}

	const OnlineLib::BuildFileInfo& bfi = it->second;

	if (bfi.size != fileData.size() || md5 != bfi.md5)
	{
		return false;
	}

	return true;
}

void GrpcCfgServer::getSessionParams(Network::SessionParams* params) const
{
	m_sessionParams.saveTo(params);
}
