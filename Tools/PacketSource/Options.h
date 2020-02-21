#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QPoint>

#include "BuildOpt.h"

// ==============================================================================================

#define					BUILD_REG_KEY			"Options/Build/"

// ----------------------------------------------------------------------------------------------

class BuildOption : public QObject
{
	Q_OBJECT

public:

	explicit	BuildOption(QObject *parent = nullptr);
				BuildOption(const BuildOption& from, QObject *parent = nullptr);
	virtual		~BuildOption();

private:

	BuildInfo			m_buildInfo;
	QString				m_signalsStatePath;

public:

	void				clear();

	//
	//
	BuildInfo&			info() { return m_buildInfo; }
	void				setInfo(const BuildInfo& buildInfo) { m_buildInfo = buildInfo; }

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

#endif // OPTIONS_H
