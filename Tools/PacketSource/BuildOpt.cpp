#include "BuildOpt.h"

#include <QDir>
#include <QFile>
#include <QCryptographicHash>

#include "../../lib/XmlHelper.h"
#include "../../lib/ConstStrings.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BuildFile::BuildFile()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

BuildFile::~BuildFile()
{
}

// -------------------------------------------------------------------------------------------------------------------

void BuildFile::clear()
{
	m_path.clear();
	m_fileName.clear();
	m_size = 0;
	m_md5.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildFile::setPath(const QString& path)
{
	m_path = path;

	if (m_path.isEmpty() == true)
	{
		return;
	}

	QStringList list = m_path.split(BUILD_FILE_SEPARATOR);
	if (list.count() >= 2)
	{
		m_fileName = BUILD_FILE_SEPARATOR + list[ list.count() - 2 ] + BUILD_FILE_SEPARATOR + list[ list.count() - 1 ];
	}

	QFile file(m_path);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		return;
	}

	m_size = file.size();

	QCryptographicHash md5Generator(QCryptographicHash::Md5);
	md5Generator.addData(&file);
	m_md5 = md5Generator.result().toHex();

	file.close();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BuildInfo::BuildInfo(QObject* parent) :
	QObject(parent)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

BuildInfo::BuildInfo(const BuildInfo& from, QObject* parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

BuildInfo::~BuildInfo()
{
}

// -------------------------------------------------------------------------------------------------------------------

void BuildInfo::clear()
{
	m_buildDirPath.clear();
	for (int type = 0; type < BUILD_FILE_TYPE_COUNT; type ++)
	{
		m_buildFile[type].clear();
	}

	m_enableReload = true;
	m_timeoutReload = BUILD_INFO_RELOAD_TIMEOUT;

	m_appDataSrvIP.clear();
	m_ualTesterIP.clear();

	m_sourcesForRunList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildInfo::setBuildDirPath(const QString& path)
{
	m_buildDirPath = path;
	m_buildDirPath.replace("\\", BUILD_FILE_SEPARATOR);

	loadBuildFiles();
}

// -------------------------------------------------------------------------------------------------------------------

void BuildInfo::loadBuildFiles()
{
	if (m_buildDirPath.isEmpty() == true)
	{
		return;
	}

	QString signalsFile = m_buildDirPath + BUILD_FILE_SEPARATOR + Directory::COMMON + BUILD_FILE_SEPARATOR + File::APP_SIGNALS_ASGS;
	signalsFile.replace("\\", BUILD_FILE_SEPARATOR);
	m_buildFile[BUILD_FILE_TYPE_SIGNALS].setPath(signalsFile);

	// find path of AppDataSrv directory
	//
	QString appDataSrvDirPath;

	QDir dir(m_buildDirPath);
	QStringList listPathSubDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString pathSubDir, listPathSubDirs)
	{
		QString fileCfg = m_buildDirPath + BUILD_FILE_SEPARATOR + pathSubDir + BUILD_FILE_SEPARATOR + File::CONFIGURATION_XML;

		QFile fileCfgXml(fileCfg);
		if (fileCfgXml.open(QIODevice::ReadOnly) == false)
		{
			continue;
		}

		QByteArray&& cfgData = fileCfgXml.readAll();
		XmlReadHelper xmlCfg(cfgData);

		if (xmlCfg.findElement("Software") == false)
		{
			continue;
		}

		int softwareType = 0;
		if (xmlCfg.readIntAttribute("Type", &softwareType) == false)
		{
			continue;
		}

		if (softwareType == E::SoftwareType::AppDataService)
		{
			appDataSrvDirPath = m_buildDirPath + BUILD_FILE_SEPARATOR + pathSubDir;
			break;
		}

		fileCfgXml.close();
	}

	if (appDataSrvDirPath.isEmpty() == true)
	{
		return;
	}

	QString sourceCfgFile = appDataSrvDirPath + BUILD_FILE_SEPARATOR + File::CONFIGURATION_XML;
	sourceCfgFile.replace("\\", BUILD_FILE_SEPARATOR);
	m_buildFile[BUILD_FILE_TYPE_SOURCE_CFG].setPath(sourceCfgFile);

	QString sourcesFile = appDataSrvDirPath + BUILD_FILE_SEPARATOR + File::APP_DATA_SOURCES_XML;
	sourcesFile.replace("\\", BUILD_FILE_SEPARATOR);
	m_buildFile[BUILD_FILE_TYPE_SOURCES].setPath(sourcesFile);
}

// -------------------------------------------------------------------------------------------------------------------

BuildFile BuildInfo::buildFile(int type) const
{
	if (type < 0 || type >= BUILD_FILE_TYPE_COUNT)
	{
		return BuildFile();
	}

	return m_buildFile[type];
}

// -------------------------------------------------------------------------------------------------------------------

void BuildInfo::setBuildFile(int type, const BuildFile& buildFile)
{
	if (type < 0 || type >= BUILD_FILE_TYPE_COUNT)
	{
		return;
	}

	m_buildFile[type] = buildFile;
}

// -------------------------------------------------------------------------------------------------------------------

BuildInfo& BuildInfo::operator=(const BuildInfo& from)
{
	m_buildDirPath = from.m_buildDirPath;
	for (int type = 0; type < BUILD_FILE_TYPE_COUNT; type ++)
	{
		m_buildFile[type] = from.m_buildFile[type];
	}

	m_enableReload = from.m_enableReload;
	m_timeoutReload = from.m_timeoutReload;

	m_appDataSrvIP = from.m_appDataSrvIP;
	m_ualTesterIP = from.m_ualTesterIP;

	m_sourcesForRunList = from.m_sourcesForRunList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
