#include "Options.h"

#include <QSettings>

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceOption::SourceOption(QObject *parent) :
	QObject(parent)
{
	m_serverIP.clear();
	m_serverPort = 0;
	m_path.clear();
}

// -------------------------------------------------------------------------------------------------------------------

SourceOption::SourceOption(const SourceOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SourceOption::~SourceOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOption::load()
{
	QSettings s;

	m_serverIP = s.value(QString("%1ServerIP").arg(SOURCE_REG_KEY), tr("127.0.0.1")).toString();
	m_serverPort = s.value(QString("%1ServerPort").arg(SOURCE_REG_KEY), 0).toInt();
	m_path = s.value(QString("%1Path").arg(SOURCE_REG_KEY), QString()).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOption::save()
{
	QSettings s;

	s.setValue(QString("%1ServerIP").arg(SOURCE_REG_KEY), m_serverIP);
	s.setValue(QString("%1ServerPort").arg(SOURCE_REG_KEY), m_serverPort);
	s.setValue(QString("%1Path").arg(SOURCE_REG_KEY), m_path);
}

// -------------------------------------------------------------------------------------------------------------------

SourceOption& SourceOption::operator=(const SourceOption& from)
{
	m_serverIP = from.m_serverIP;
	m_serverPort = from.m_serverPort;
	m_path = from.m_path;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
	m_source.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
	m_source.save();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::unload()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool Options::readFromXml()
{
	bool result = false;

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
	m_mutex.lock();

		m_source = from.m_source;

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
