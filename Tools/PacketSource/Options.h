#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QPoint>

// ==============================================================================================

#define					BUILD_REG_KEY			"Options/Build/"

// ----------------------------------------------------------------------------------------------

const int				BUILD_FILE_TYPE_SIGNALS			= 0,
						BUILD_FILE_TYPE_SOURCE_CFG		= 1,
						BUILD_FILE_TYPE_SOURCES			= 2;

const int				BUILD_FILE_TYPE_COUNT			= 3;

const char* const		BuildFileRegKey[BUILD_FILE_TYPE_COUNT] =
{
						QT_TRANSLATE_NOOP("Options.h", "SignalsFilePath"),
						QT_TRANSLATE_NOOP("Options.h", "SourceCfgFilePath"),
						QT_TRANSLATE_NOOP("Options.h", "SourcesFilePath"),
};

// ----------------------------------------------------------------------------------------------

#define					BUILD_FILE_SEPARATOR			"/"

// ----------------------------------------------------------------------------------------------

const int				BUILD_FILE_RELOAD_TIMEOUT		= 3;	// seconds

// ----------------------------------------------------------------------------------------------

class BuildFile
{
public:

	BuildFile();
	virtual		~BuildFile();

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

class BuildOption : public QObject
{
	Q_OBJECT

public:

	explicit	BuildOption(QObject *parent = nullptr);
				BuildOption(const BuildOption& from, QObject *parent = nullptr);
	virtual		~BuildOption();

private:

	QString				m_buildDirPath;
	BuildFile			m_buildFile[BUILD_FILE_TYPE_COUNT];

	bool				m_enableReload = true;
	int					m_timeoutReload = BUILD_FILE_RELOAD_TIMEOUT;

	QString				m_appDataSrvIP;
	QString				m_ualTesterIP;

	QString				m_signalsStatePath;

public:

	void				clear();

	// path
	//
	QString				buildDirPath() const { return m_buildDirPath; }
	void				setBuildDirPath(const QString& path) { m_buildDirPath = path; }

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
	QString				appDataSrvIP() const { return m_appDataSrvIP; }
	void				setAppDataSrvIP(const QString& ip) { m_appDataSrvIP = ip; }

	QString				ualTesterIP() const { return m_ualTesterIP; }
	void				setUalTesterIP(const QString& ip) { m_ualTesterIP = ip; }

	//
	//
	QString				signalsStatePath() const { return m_signalsStatePath; }
	void				setSignalsStatePath(const QString& path) { m_signalsStatePath = path; }

	//
	//
	void				load();
	void				save();

	//
	//
	BuildOption&		operator=(const BuildOption& from);
};

// ==============================================================================================

class WindowsOption : public QObject
{
	Q_OBJECT

public:

	explicit	WindowsOption(QObject *parent = nullptr);
				WindowsOption(const WindowsOption& from, QObject *parent = nullptr);
	virtual		~WindowsOption();

public:

	QPoint		m_mainWindowPos;
	QByteArray	m_mainWindowGeometry;
	QByteArray	m_mainWindowState;

	QPoint		m_optionsWindowPos;
	QByteArray	m_optionsWindowGeometry;

	//
	//
	void				load();
	void				save();
	//
	//
	WindowsOption&		operator=(const WindowsOption& from);

};

// ==============================================================================================

bool compareDouble(double lDouble, double rDouble);

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit	Options(QObject *parent = nullptr);
				Options(const Options& from, QObject *parent = nullptr);
	virtual		~Options();

private:

	QMutex				m_mutex;

	WindowsOption		m_windows;
	BuildOption			m_build;

public:

	WindowsOption&		windows() { return m_windows; }
	void				setWindows(const WindowsOption& windows) { m_windows = windows; }

	BuildOption&		build() { return m_build; }
	void				setBulid(const BuildOption& build) { m_build = build; }

	void				load();
	void				save();
	void				unload();

	bool				readFromXml();

	Options&			operator=(const Options& from);
};

// ==============================================================================================

extern Options			theOptions;

// ==============================================================================================

#endif // OPTIONS_H
