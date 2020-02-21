#ifndef BUILDINFO_H
#define BUILDINFO_H

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

const int				BUILD_FILE_TYPE_SIGNALS			= 0,
						BUILD_FILE_TYPE_SOURCE_CFG		= 1,
						BUILD_FILE_TYPE_SOURCES			= 2;

const int				BUILD_FILE_TYPE_COUNT			= 3;

// ----------------------------------------------------------------------------------------------

#define					BUILD_FILE_SEPARATOR			"/"

// ----------------------------------------------------------------------------------------------

class BuildFile
{
public:

	BuildFile();
	virtual ~BuildFile();

private:

	QString				m_path;
	QString				m_fileName;
	qint64				m_size = 0;
	QString				m_md5;

public:

	void				clear();

	QString				path() const { return m_path; }
	void				setPath(const QString& path);

	QString				fileName() const { return m_fileName; }
	qint64				size() const { return m_size; }
	QString				md5() const { return m_md5; }

};

// ----------------------------------------------------------------------------------------------

const int				BUILD_INFO_RELOAD_TIMEOUT		= 3;	// seconds

// ----------------------------------------------------------------------------------------------

class BuildInfo : public QObject
{
	Q_OBJECT

public:

	explicit	BuildInfo(QObject *parent = nullptr);
				BuildInfo(const BuildInfo& from, QObject *parent = nullptr);
	virtual		~BuildInfo();

private:

	QString				m_buildDirPath;
	BuildFile			m_buildFile[BUILD_FILE_TYPE_COUNT];

	bool				m_enableReload = true;
	int					m_timeoutReload = BUILD_INFO_RELOAD_TIMEOUT;

	HostAddressPort		m_appDataSrvIP;
	HostAddressPort		m_ualTesterIP;

	QStringList			m_sourcesForRunList;
	
public:

	void				clear();

	// path
	//
	QString				buildDirPath() const { return m_buildDirPath; }
	void				setBuildDirPath(const QString& path);

	void				loadBuildFiles();
	BuildFile			buildFile(int type) const;
	void				setBuildFile(int type, const BuildFile& buildFile);
	
	// timer for update buildFiles
	//
	bool				enableReload() const { return m_enableReload; }
	void				setEnableReload(bool enable) { m_enableReload = enable; }

	int					timeoutReload() const { return m_timeoutReload; }
	void				setTimeoutReload(int sec) { m_timeoutReload = sec; }

	// ip
	//
	HostAddressPort		appDataSrvIP() const { return m_appDataSrvIP; }
	void				setAppDataSrvIP(const HostAddressPort& ip) { m_appDataSrvIP = ip; }

	HostAddressPort		ualTesterIP() const { return m_ualTesterIP; }
	void				setUalTesterIP(const HostAddressPort& ip) { m_ualTesterIP = ip; }
	
	//
	//
	QStringList			sourcesForRunList() const { return m_sourcesForRunList; }
	void				setSourcesForRunList(const QStringList& list) { m_sourcesForRunList = list; }
	void				appendSourcesForRunToList(const QString& sourceEquipmentID) { m_sourcesForRunList.append(sourceEquipmentID); }
	
	//
	//
	BuildInfo&			operator=(const BuildInfo& from);
};

// ==============================================================================================

#endif // BUILDINFO_H
