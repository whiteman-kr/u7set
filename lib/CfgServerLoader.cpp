#include "../include/CfgServerLoader.h"
#include <QXmlStreamReader>
#include <QStandardPaths>


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServerLoaderBase::m_BuildFileInfoArrayRegistered = false;


CfgServerLoaderBase::CfgServerLoaderBase()
{
	if (m_BuildFileInfoArrayRegistered == false)
	{
		qRegisterMetaType<BuildFileInfoArray>("BuildFileInfoArray");
		m_BuildFileInfoArrayRegistered = true;
	}
}


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
	//m_timer.start();			!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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

	resetStatuses();
}


void CfgLoader::resetStatuses()
{
	m_downloadQueue.clear();
	m_currentDownload.clear();

	m_configurationXmlMd5 = "";

	m_configurationReady = false;
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
		m_configurationReady == false ||
		isTransferInProgress() ||
		m_allFilesLoaded == true ||
		m_downloadQueue.isEmpty() == false)
	{
		return;
	}

	while(m_autoDownloadIndex < m_cfgFileInfo.count())
	{
		CfgFileInfo& cfi = m_cfgFileInfo[m_autoDownloadIndex];

		if (cfi.loaded == false)
		{
			if (cfgFileIsExists(cfi.pathFileName, cfi.md5) == true)
			{
				// file exists from previous downloads
				// nothing to do
				//
				cfi.loaded = true;
			}
			else
			{
				FileDownloadRequest fdr;

				fdr.pathFileName = m_cfgFileInfo[m_autoDownloadIndex].pathFileName;
				fdr.isAutoRequest = true;

				m_downloadQueue.append(fdr);

				startDownload();

				m_autoDownloadIndex++;

				break;
			}
		}

		m_autoDownloadIndex++;
	}
}


bool CfgLoader::cfgFileIsExists(const QString& filePathName, const QString& etalonMd5)
{
	QString fileName = m_rootFolder + filePathName;

	QFile file(fileName);

	if (file.exists() == false)
	{
		return false;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	Md5Hash md5Hash;

	md5Hash.addData(&file);

	file.close();

	if (md5Hash.resultStr() == etalonMd5)
	{
		return true;
	}

	return false;
}


bool CfgLoader::readFile(const QString& filePathName, QByteArray* fileData)
{
	QString fileName = m_rootFolder + filePathName;

	QFile file(fileName);

	if (file.exists() == false)
	{
		return false;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	*fileData = file.readAll();

	return true;
}


void CfgLoader::onConnection()
{
	resetStatuses();

	FileDownloadRequest fdr;

	fdr.pathFileName = m_configurationXmlPathFileName;

	m_downloadQueue.push_front(fdr);

	startDownload();
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
		emit signal_endFileDownload(fileName, getErrorStr(errorCode));
		return;
	}

	if (m_currentDownloadRequest.etalonMD5.isEmpty() == true)
	{
		// can be empty for configuration.xml file only!!!
		//
		assert(m_currentDownloadRequest.pathFileName == m_configurationXmlPathFileName);
	}
	else
	{
		if (m_currentDownloadRequest.etalonMD5 != md5)
		{
			errorCode = Tcp::FileDataCorrupted;
			assert(false);
			emit signal_endFileDownload(fileName, getErrorStr(errorCode));
			return;
		}
	}

	if (fileName == m_configurationXmlPathFileName)
	{
		// configuration.xml is loaded
		//
		if (m_currentDownload.isTestCfgRequest)
		{
			if (m_cfgFileInfo[CONFIGURATION_XML].md5 != md5)
			{
				// configuration changed !!!!!!!!!!!!
			}
		}
		else
		{
			qDebug() << "Downloaded configuration.xml";

			if (readConfigurationXml() == true)
			{
				m_configurationReady = true;

				BuildFileInfoArray bfiArray;

				for(const CfgFileInfo& cfi : m_cfgFileInfo)
				{
					Builder::BuildFileInfo bfi;

					bfi.pathFileName = cfi.pathFileName;
					bfi.size = cfi.size;
					bfi.md5 = cfi.md5;

					bfiArray.append(bfi);
				}

				qDebug() << "Readed configuration.xml";

				emit signal_configurationReady(m_cfgFileInfo[CONFIGURATION_XML].fileData, bfiArray);
			}
		}
	}
	else
	{
		qDebug() << "Downloaded " << (m_currentDownloadRequest.isAutoRequest ? "(auto) :" : "(manual) :")  << fileName;

		if (m_cfgFileInfo.contains(m_currentDownloadRequest.pathFileName))
		{
			m_cfgFileInfo[m_currentDownloadRequest.pathFileName].loaded = true;
		}
		else
		{
			assert(false);
		}

		if (m_currentDownloadRequest.isAutoRequest == false)
		{
			// emit signal for "manual" requests only!
			//

			if(m_currentDownloadRequest.fileData == nullptr)
			{
				assert(false);
				emit signal_endFileDownload(fileName, "Internal error");
			}
			else
			{
				if (readFile(fileName, m_currentDownloadRequest.fileData) == false)
				{
					emit signal_endFileDownload(fileName, "Read file error");
				}
				else
				{
					emit signal_endFileDownload(fileName, "");
				}
			}
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


bool CfgLoader::readConfigurationXml()
{
	QByteArray fileData;

	if (readConfigurationFile(m_configurationXmlPathFileName, &fileData) == false)
	{
		return false;
	}

	m_cfgFileInfo.clear();

	CfgFileInfo cfi;

	cfi.pathFileName = m_configurationXmlPathFileName;
	cfi.size = fileData.size();
	cfi.md5 = Md5Hash::hashStr(fileData);
	cfi.loaded = true;
	cfi.fileData.swap(fileData);

	// configuration.xml info always first item in m_cfgFileInfo
	//
	m_cfgFileInfo.insert(cfi.pathFileName, cfi);

	QXmlStreamReader xmlReader(cfi.fileData);

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

	return true;
}


bool CfgLoader::readConfigurationFile(const QString& pathFileName, QByteArray* fileData)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	QFile file(m_rootFolder + pathFileName);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		assert(false);
		return false;
	}

	*fileData = file.readAll();

	file.close();

	if (fileData->isEmpty())
	{
		assert(false);
		return false;
	}

	return true;
}


void CfgLoader::slot_downloadCfgdFile(const QString& fileName, QByteArray* fileData)
{
	if (fileData == nullptr)
	{
		assert(false);
		return;
	}

	fileData->clear();

	if (m_cfgFileInfo.contains(fileName) == false)
	{
		emit signal_endFileDownload(fileName, "File not found in configuration.xml");
		return;
	}

	if (m_cfgFileInfo[fileName].loaded)
	{
		// file allready loaded
		// read file data
		//
		if (readFile(fileName, fileData) == false)
		{
			emit signal_endFileDownload(fileName, "Read file error");
		}
		else
		{
			emit signal_endFileDownload(fileName, "");
		}

		return;
	}

	// file is not loaded
	//

	FileDownloadRequest fdr;

	fdr.pathFileName = fileName;
	fdr.isAutoRequest = false;		// manual request
	fdr.fileData = fileData;

	m_downloadQueue.append(fdr);

	if (isTransferInProgress() == false)
	{
		startDownload();
	}
}


bool CfgLoader::downloadCfgFile(const QString& pathFileName, QByteArray* fileData)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	WaitForSignalHelper wsh(this, SIGNAL(signal_endFileDownload));

	QByteArray localFileData;

	emit signal_downloadCfgFile(pathFileName, &localFileData);

	if (wsh.wait(5000) == true)
	{
		fileData->swap(localFileData);
		return true;
	}

	fileData->clear();
	return false;
}






