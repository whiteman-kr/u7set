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

CfgServer::CfgServer(const QString& buildFolder) :
	Tcp::FileServer(buildFolder)
{
}


void CfgServer::onServerThreadStarted()
{
	Tcp::FileServer::onServerThreadStarted();

	onRootFolderChange();
}


void CfgServer::onRootFolderChange()
{
	m_buildXmlPathFileName = m_rootFolder + "/build.xml";

	readBuildXml();
}


void CfgServer::readBuildXml()
{
	QDir dir(m_buildXmlPathFileName);

	if (dir.exists(m_buildXmlPathFileName) == false)
	{
		m_errorCode = ErrorCode::BuildNotFound;
        qDebug() << "File not found: " << m_buildXmlPathFileName;
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

		Builder::BuildFileInfo bfi;

		bfi.readFromXml(xmlReader);

		m_buildFileInfo.insert(bfi.pathFileName, bfi);
	}

	qDebug() << "File " << m_buildXmlPathFileName << " has been readed";
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


void CfgLoader::shutdown()
{
}


void CfgLoader::onConnection()
{
	resetStatuses();

	startConfigurationXmlLoading();
}


bool CfgLoader::startConfigurationXmlLoading()
{
	if (isConnected() == false || m_transferInProgress == true)
	{
		return true;
	}

	m_configurationXmlReady = false;

	FileDownloadRequest fdr;

	fdr.pathFileName = m_configurationXmlPathFileName;

	m_downloadQueue.push_front(fdr);

	startDownload();

	return true;
}


void CfgLoader::startDownload()
{
	if (m_downloadQueue.isEmpty())
	{
		assert(false);
		return;
	}

	m_currentDownloadRequest = m_downloadQueue.first();

	qDebug() << "start request " << m_currentDownloadRequest.pathFileName;

	m_downloadQueue.removeFirst();

	slot_downloadFile(m_currentDownloadRequest.pathFileName);	// TcpFileTransfer::slot_downloadFile
}


bool CfgLoader::downloadCfgFile(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	fileData->clear();

	WaitForSignalHelper wsh(this, SIGNAL(signal_endCfgFileDownload()));

	QByteArray localFileData;
	Tcp::FileTransferResult errorCode = Tcp::FileTransferResult::Ok;

	emit signal_downloadCfgFile(pathFileName, &localFileData, &errorCode);

	if (wsh.wait(5000) == true)
	{
		*errorStr = getErrorStr(errorCode);

		if (errorCode == Tcp::FileTransferResult::Ok)
		{
			fileData->swap(localFileData);
			return true;
		}
		else
		{
			return false;
		}
	}

	*errorStr = tr("File reading timeout");

	return false;
}


void CfgLoader::slot_downloadCfgdFile(const QString& fileName, QByteArray* fileData, Tcp::FileTransferResult* errorCode)
{
	if (fileData == nullptr || errorCode == nullptr)
	{
		assert(false);
		return;
	}

	fileData->clear();

	if (m_configurationXmlReady == false)
	{
		*errorCode = Tcp::FileTransferResult::ConfigurationIsNotReady;
		emit signal_endCfgFileDownload();
		return;
	}

    if (m_cfgFilesInfo.contains(fileName) == false)
	{
		*errorCode = Tcp::FileTransferResult::NotFoundRemoteFile;
		emit signal_endCfgFileDownload();
		return;
	}

    if (readCfgFileIfExists(fileName, fileData, m_cfgFilesInfo[fileName].md5) == true)
	{
        qDebug() << "File " << fileName << " allready exists, md5 = " << m_cfgFilesInfo[fileName].md5;

		*errorCode = Tcp::FileTransferResult::Ok;
		emit signal_endCfgFileDownload();

		return;
	}

	// file is not exists
	//

	FileDownloadRequest fdr;

	fdr.pathFileName = fileName;
	fdr.isAutoRequest = false;			// manual request
	fdr.fileData = fileData;
	fdr.errorCode = errorCode;
    fdr.etalonMD5 = m_cfgFilesInfo[fileName].md5;

	m_downloadQueue.append(fdr);

	if (isTransferInProgress() == false)
	{
		startDownload();
	}
}


void CfgLoader::onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5)
{
	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		QString msg = QString("File '%1' download error - %2").
				arg(m_currentDownloadRequest.pathFileName).
				arg(getErrorStr(errorCode));

		qDebug() << msg;

		emit signal_endCfgFileDownload();

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
			assert(false);

			m_currentDownloadRequest.setErrorCode(Tcp::FileDataCorrupted);
			emit signal_endCfgFileDownload();

			return;
		}
	}

	if (fileName == m_configurationXmlPathFileName)
	{
		// configuration.xml is loaded
		//
		if (m_currentDownloadRequest.isTestCfgRequest)
		{
            if (m_cfgFilesInfo[CONFIGURATION_XML].md5 != md5)
			{
				// configuration changed !!!!!!!!!!!!
			}
		}
		else
		{
			qDebug() << "Downloaded configuration.xml";

			if (readConfigurationXml() == true)
			{
				m_configurationXmlReady = true;

				BuildFileInfoArray bfiArray;

                for(const CfgFileInfo& cfi : m_cfgFilesInfo)
				{
					Builder::BuildFileInfo bfi;

					bfi.pathFileName = cfi.pathFileName;
					bfi.size = cfi.size;
					bfi.md5 = cfi.md5;

					bfiArray.append(bfi);
				}

				qDebug() << "Readed configuration.xml";

                emit signal_configurationReady(m_cfgFilesInfo[CONFIGURATION_XML].fileData, bfiArray);
			}
		}
	}
	else
	{
		qDebug() << "Downloaded " << (m_currentDownloadRequest.isAutoRequest ? "(auto) :" : "(manual) :")  << fileName;

		if (m_currentDownloadRequest.isAutoRequest == false)
		{
			// emit signal_endFileDownload for "manual" requests only!
			//
			if(m_currentDownloadRequest.fileData == nullptr)
			{
				assert(false);
				m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::InternalError);
				emit signal_endCfgFileDownload();
			}
			else
			{
				if (readCfgFile(fileName, m_currentDownloadRequest.fileData) == false)
				{
					m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::LocalFileReadingError);
					emit signal_endCfgFileDownload();
				}
				else
				{
					emit signal_endCfgFileDownload();
				}
			}
		}
	}

    if (m_autoDownloadIndex == m_cfgFilesInfo.count())
	{
		m_allFilesLoaded = true;
	}

	if (m_downloadQueue.isEmpty() == false)
	{
		startDownload();
	}
}


void CfgLoader::onTimer()
{
	if (m_configurationXmlReady == false)
	{
		startConfigurationXmlLoading();
		return;
	}

    if (m_autoDownloadIndex >= m_cfgFilesInfo.count() ||
		isConnected() == false ||
		m_configurationXmlReady == false ||
		isTransferInProgress() ||
		m_allFilesLoaded == true ||
		m_downloadQueue.isEmpty() == false)
	{
		return;
	}

    while(m_autoDownloadIndex < m_cfgFilesInfo.count())
	{
        CfgFileInfo& cfi = m_cfgFilesInfo[m_autoDownloadIndex];

		if (isCfgFileIsExists(cfi.pathFileName, cfi.md5) == true)
		{
			// file exists from previous downloads
			// nothing to do
			//
			qDebug() << "File " << cfi.pathFileName << " allready exists, md5 = " << cfi.md5;
		}
		else
		{
			FileDownloadRequest fdr;

            fdr.pathFileName = m_cfgFilesInfo[m_autoDownloadIndex].pathFileName;
            fdr.etalonMD5 = m_cfgFilesInfo[m_autoDownloadIndex].md5;
			fdr.isAutoRequest = true;

			m_downloadQueue.append(fdr);

			startDownload();

			m_autoDownloadIndex++;

			break;
		}

		m_autoDownloadIndex++;
	}
}


void CfgLoader::changeApp(const QString& appStrID, int appInstance)
{
	shutdown();

	m_appStrID = appStrID;
	m_appInstance = appInstance;

	m_appDataPath = "/" + m_appStrID + "-" + QString::number(m_appInstance);

	m_rootFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + m_appDataPath;

	//m_rootFolder = "d:/cfgloader" + m_appDataPath;		// for debugging only!!!

	setRootFolder(m_rootFolder);

	m_configurationXmlPathFileName = "/" + m_appStrID + "/configuration.xml";

    readSavedConfiguration();

	resetStatuses();
}


void CfgLoader::readSavedConfiguration()
{
    // if exists, read previously loaded configuration
    //

   // if ()
}


void CfgLoader::resetStatuses()
{
	m_downloadQueue.clear();
	m_currentDownloadRequest.clear();

	m_configurationXmlMd5 = "";

	m_configurationXmlReady = false;
	m_autoDownloadIndex = 1;		// index 0 - configuration.xml
	m_allFilesLoaded = false;
}


bool CfgLoader::isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5)
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


bool CfgLoader::readConfigurationXml()
{
	QByteArray fileData;

	if (readCfgFile(m_configurationXmlPathFileName, &fileData) == false)
	{
		return false;
	}

    m_cfgFilesInfo.clear();

	CfgFileInfo cfi;

	cfi.pathFileName = m_configurationXmlPathFileName;
	cfi.size = fileData.size();
	cfi.md5 = Md5Hash::hashStr(fileData);
	cfi.fileData.swap(fileData);

	// configuration.xml info always first item in m_cfgFileInfo
	//
    m_cfgFilesInfo.insert(cfi.pathFileName, cfi);

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

        m_cfgFilesInfo.insert(cfi.pathFileName, cfi);
	}

	return true;
}


bool CfgLoader::readCfgFile(const QString& pathFileName, QByteArray* fileData)
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


bool CfgLoader::readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

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

	file.close();

	Md5Hash md5Hash;

	md5Hash.addData(*fileData);

	if (md5Hash.resultStr() == etalonMd5)
	{
		return true;
	}

	fileData->clear();

	return false;
}



CfgLoaderThread::CfgLoaderThread(CfgLoader* cfgLoader) :
    Tcp::Thread(cfgLoader),
    m_cfgLoader(cfgLoader)
{
}


bool CfgLoaderThread::downloadCfgFile(const QString& pathFileName, QByteArray* fileData, QString *errorStr)
{
    if (fileData == nullptr || errorStr == nullptr)
    {
        assert(false);
        return false;
    }

    if (m_cfgLoader == nullptr)
    {
        assert(false);
        *errorStr = tr("CfgLoaderThread vember m_cfgLoader is not initialized");
        return false;
    }

    return m_cfgLoader->downloadCfgFile(pathFileName, fileData, errorStr);
}





