#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QPoint>

// ==============================================================================================

#define					SOURCE_REG_KEY			"Options/Source/"

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
	QString				m_signalsFilePath;
	QString				m_sourceCfgFilePath;
	QString				m_sourcesFilePath;

	QString				m_appDataSrvIP;
	QString				m_ualTesterIP;

public:

	void				clear();

	// path
	//
	QString				buildDirPath() const { return m_buildDirPath; }
	void				setBuildDirPath(const QString& path) { m_buildDirPath = path; }

	QString				signalsFilePath() const { return m_signalsFilePath; }
	void				setSignalsFilePath(const QString& path) { m_signalsFilePath = path; }

	QString				sourceCfgFilePath() const { return m_sourceCfgFilePath; }
	void				setSourceCfgFilePath(const QString& path) { m_sourceCfgFilePath = path; }

	QString				sourcesFilePath() const { return m_sourcesFilePath; }
	void				setSourcesFilePath(const QString& path) { m_sourcesFilePath = path; }

	// ip
	//
	QString				appDataSrvIP() const { return m_appDataSrvIP; }
	void				setAppDataSrvIP(const QString& ip) { m_appDataSrvIP = ip; }

	QString				ualTesterIP() const { return m_ualTesterIP; }
	void				setUalTesterIP(const QString& ip) { m_ualTesterIP = ip; }

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
