#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>

// ==============================================================================================

#define					SOURCE_REG_KEY			"Options/Source/"

// ----------------------------------------------------------------------------------------------

class PathOption : public QObject
{
	Q_OBJECT

public:

	explicit	PathOption(QObject *parent = nullptr);
				PathOption(const PathOption& from, QObject *parent = nullptr);
	virtual		~PathOption();

private:

	QString				m_signalPath;
	QString				m_sourcePath;
	QString				m_localIP;

public:

	void				clear();

	QString				signalPath() const { return m_signalPath; }
	void				setSignalPath(const QString& path) { m_signalPath = path; }

	QString				sourcePath() const { return m_sourcePath; }
	void				setSourcePath(const QString& path) { m_sourcePath = path; }

	QString				localIP() const { return m_localIP; }
	void				setLocalIP(const QString& ip) { m_localIP = ip; }

	void				load();
	void				save();

	PathOption&			operator=(const PathOption& from);
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

	PathOption			m_path;

public:

	PathOption&			path() { return m_path; }
	void				setPath(const PathOption& path) { m_path = path; }

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
