#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QPoint>

#include "BuildOption.h"

// ==============================================================================================

class WindowsOption : public QObject
{
	Q_OBJECT

public:

	explicit WindowsOption(QObject *parent = nullptr);
	WindowsOption(const WindowsOption& from, QObject *parent = nullptr);
	virtual ~WindowsOption() override;

public:

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;

	QPoint m_optionsWindowPos;
	QByteArray m_optionsWindowGeometry;

	//
	//
	void load();
	void save();

	//
	//
	WindowsOption& operator=(const WindowsOption& from);
};

// ==============================================================================================

bool compareDouble(double lDouble, double rDouble);

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit Options(QObject *parent = nullptr);
	Options(const Options& from, QObject *parent = nullptr);
	virtual ~Options() override;

public:

	WindowsOption& windows() { return m_windows; }
	void setWindows(const WindowsOption& windows) { m_windows = windows; }

	BuildOption& build() { return m_build; }
	void setBuild(const BuildOption& build) { m_build = build; }

	void load();
	void save();
	void unload();

	bool readFromXml();

	Options& operator=(const Options& from);

private:

	QMutex m_mutex;

	WindowsOption m_windows;
	BuildOption m_build;
};

// ==============================================================================================

#endif // OPTIONS_H
