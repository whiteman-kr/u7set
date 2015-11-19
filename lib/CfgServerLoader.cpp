#include "../include/CfgServerLoader.h"
#include <QXmlStreamReader>
#include <QStandardPaths>

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
	Tcp::FileServer::onServerThreadStarted();

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

	buildXml.close();

	if (data.isEmpty())
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	QXmlStreamReader xmlReader(data);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == "build")
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != "file")
		{
			continue;
		}

		Builder::BuildFileInfo bfi;

		bfi.readFromXml(xmlReader);

		m_buildFileInfo.insert(bfi.pathFileName, bfi);
	}

}


// -------------------------------------------------------------------------------------
//
// CfgLoader class implementation
//
// -------------------------------------------------------------------------------------

CfgLoader::CfgLoader(const QString& appStrID, int appInstance, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::FileClient("", serverAddressPort1, serverAddressPort2),
	m_timer(this)
{
	changeApp(appStrID, appInstance);
}


void CfgLoader::onClientThreadStarted()
{
	Tcp::FileClient::onClientThreadStarted();

	connect(&m_timer, &QTimer::timeout, this, &CfgLoader::onTimer);
	connect(this, &CfgLoader::signal_downloadCfgFile, this, &CfgLoader::slot_downloadCfgdFile);

	m_timer.setInterval(2000);
	m_timer.start();
}


void CfgLoader::changeApp(const QString& appStrID, int appInstance)
{
	shutdown();

	m_appStrID = appStrID;
	m_appInstance = appInstance;

	m_appDataPath = "/" + m_appStrID + "-" + QString::number(m_appInstance);
	m_rootFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + m_appDataPath;

	setRootFolder(m_rootFolder);

	m_configurationXmlPathFileName = "/" + m_appStrID + "/configuration.xml";

	m_configurationlReady = false;

	m_autoDownloadIndex = 0;

	m_allFilesLoaded = false;
}



void CfgLoader::shutdown()
{

}


void CfgLoader::onTimer()
{
	if (m_autoDownloadIndex >= m_cfgFileInfo.count() ||
		isConnected() == false ||
		m_configurationlReady == false ||
		isTransferInProgress() ||
		m_allFilesLoaded == true ||
		m_downloadQueue.isEmpty() == false)
	{
		return;
	}

	while(m_autoDownloadIndex < m_cfgFileInfo.count())
	{
		if (m_cfgFileInfo[m_autoDownloadIndex].loaded == false)
		{
			FileDownloadRequest fdr;

			fdr.pathFileName = m_cfgFileInfo[m_autoDownloadIndex].pathFileName;
			fdr.isAutoRequest = true;

			m_downloadQueue.append(fdr);

			startDownload();

			m_autoDownloadIndex++;

			break;
		}

		m_autoDownloadIndex++;
	}
}


void CfgLoader::onConnection()
{
	downloadFile(m_configurationXmlPathFileName);
}


void CfgLoader::startDownload()
{
	if (m_downloadQueue.isEmpty())
	{
		assert(false);
		return;
	}

	m_currentDownloadRequest = m_downloadQueue.first();

	m_downloadQueue.removeFirst();

	downloadFile(m_currentDownloadRequest.pathFileName);
}


void CfgLoader::onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5)
{
	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		assert(false);
		return;		//	???
	}

	if (m_currentDownloadRequest.etalonMD5 != md5)
	{
		errorCode = Tcp::FileDataCorrupted;
		assert(false);
		return;		//  ??
	}

	qDebug() << "Downloaded:  " << fileName;

	if (fileName == m_configurationXmlPathFileName)
	{
		readConfigurationXml();

		m_configurationlReady = true;

		emit configurationReady();
	}
	else
	{
		if (m_currentDownloadRequest.isAutoRequest == false)
		{
//			emit endFileDownload(fileName, )
		}
	}

	if (m_autoDownloadIndex == m_cfgFileInfo.count())
	{
		m_allFilesLoaded = true;
	}

	if (m_downloadQueue.isEmpty() == false)
	{
		startDownload();
	}
}


void CfgLoader::readConfigurationXml()
{
	QFile xmlFile(m_rootFolder + m_configurationXmlPathFileName);

	if (xmlFile.open(QIODevice::ReadOnly) == false)
	{
		assert(false);
		return;
	}

	QByteArray data = xmlFile.readAll();

	xmlFile.close();

	if (data.isEmpty())
	{
		assert(false);
		return;
	}

	QXmlStreamReader xmlReader(data);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == "BuildInfo")
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != "File")
		{
			continue;
		}

		CfgFileInfo cfi;

		cfi.readFromXml(xmlReader);

		m_cfgFileInfo.insert(cfi.pathFileName, cfi);
	}
}


void CfgLoader::slot_downloadCfgdFile(const QString& fileName)
{
	if (m_cfgFileInfo.contains(fileName) == false)
	{
		emit endFileDownload(fileName, "File not found in configuration.xml", "");
		return;
	}

	if (m_cfgFileInfo[fileName].loaded)
	{
		emit endFileDownload(fileName, "", m_cfgFileInfo[fileName].md5);			// file allready loaded
		return;
	}

	// file is not loaded
	//

	FileDownloadRequest fdr;

	fdr.pathFileName = fileName;
	fdr.isAutoRequest = false;		// manual request

	m_downloadQueue.append(fdr);

	if (isTransferInProgress() == false)
	{
		startDownload();
	}
}







