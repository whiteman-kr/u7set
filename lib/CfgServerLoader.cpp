#include "../include/CfgServerLoader.h"
#include <QXmlStreamReader>

// -------------------------------------------------------------------------------------
//
// CfgServer class implementation
//
// -------------------------------------------------------------------------------------

CfgServer::CfgServer(const QString& rootFolder) :
	Tcp::FileServer(rootFolder)
{
}


void CfgServer::onServerThreadStarted()
{
	onRootFolderChange();
}


void CfgServer::onRootFolderChange()
{
	m_buildXmlPathFileName = m_rootFolder + QDir::separator() + "build.xml";

	readBuildXml();
}


void CfgServer::readBuildXml()
{
	QDir dir(m_buildXmlPathFileName);

	if (dir.exists(m_buildXmlPathFileName) == false)
	{
		m_errorCode = ErrorCode::BuildNotFound;
		return;
	}

	QFile buildXml(m_buildXmlPathFileName);

	if (buildXml.open(QIODevice::ReadOnly) == false)
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	QByteArray data = buildXml.readAll();

	if (data.isEmpty())
	{
		m_errorCode = ErrorCode::BuildCantRead;
		buildXml.close();
		return;
	}

	QXmlStreamReader xml(data);

	while(xml.atEnd() == false)
	{
		if (xml.readNextStartElement() == false)
		{
			continue;
		}

		// find "file" element
		//
		if (xml.name() != "file")
		{
			continue;
		}

		QString fileName(xml.attributes().value("name").toString());
		QString md5(xml.attributes().value("md5").toString());
		qint64 size = xml.attributes().value("size").toInt();;
	}

	buildXml.close();

}


// -------------------------------------------------------------------------------------
//
// CfgLoader class implementation
//
// -------------------------------------------------------------------------------------

CfgLoader::CfgLoader(const QString& appStrID, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::FileClient("", serverAddressPort1, serverAddressPort2)
{
	QString rootFolder;

	// construct path to root folder

	setRootFolder(rootFolder);
}



void CfgLoader::onClientThreadStarted()
{

}

