#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>

// ==============================================================================================

#define					SOURCE_REG_KEY			"Options/Source/"

// ----------------------------------------------------------------------------------------------

class SourceOption : public QObject
{
	Q_OBJECT

public:

	explicit	SourceOption(QObject *parent = 0);
				SourceOption(const SourceOption& from, QObject *parent = 0);
	virtual		~SourceOption();

private:

	QString				m_path;

public:

	void				clear();

	QString				path() const { return m_path; }
	void				setPath(const QString& path) { m_path = path; }

	void				load();
	void				save();

	SourceOption&		operator=(const SourceOption& from);
};

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit	Options(QObject *parent = 0);
				Options(const Options& from, QObject *parent = 0);
	virtual		~Options();

private:

	QMutex				m_mutex;

	SourceOption		m_source;

public:

	SourceOption&		source() { return m_source; }
	void				setSource(const SourceOption& source) { m_source = source; }

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
