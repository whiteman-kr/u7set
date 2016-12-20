#include "Session.h"

namespace VFrame30
{

	Session::Session() :
		Session(QString(), QString(), QString())
	{
	}

	Session::Session(QString projectId, QString username, QString host) :
		m_project(projectId),
		m_username(username),
		m_host(host)
	{
		ADD_PROPERTY_GETTER(QString, "Project", true, Session::project);
		ADD_PROPERTY_GETTER(QString, "Username", true, Session::username);
		ADD_PROPERTY_GETTER(QString, "Host", true, Session::host);

		ADD_PROPERTY_GETTER(QString, "Date", true, Session::date);
		ADD_PROPERTY_GETTER(QString, "Time", true, Session::time);

	}

	Session& Session::operator= (const Session& src)
	{
		m_project = src.m_project;
		m_username = src.m_username;
		m_host = src.m_host;

		return *this;
	}

	QString Session::project() const
	{
		return m_project;
	}

	void Session::setProject(QString value)
	{
		m_project = value;
	}

	QString Session::username() const
	{
		return m_username;
	}

	void Session::setUsername(QString value)
	{
		m_username = value;
	}

	QString Session::host() const
	{
		return m_host;
	}

	void Session::setHost(QString value)
	{
		m_host = value;
	}

	QString Session::date() const
	{
		QDateTime t = QDateTime::currentDateTime();
		return t.toString("d MMM yyyy");
	}

	QString Session::time() const
	{
		QDateTime t = QDateTime::currentDateTime();
		return t.toString("hh:mm");
	}

}
