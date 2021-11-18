#include "Session.h"

namespace VFrame30
{

	Session::Session() :
		Session({}, {}, {})
	{
	}

	Session::Session(QString projectId, QString username, QString host) :
		m_project(std::move(projectId)),
		m_username(std::move(username)),
		m_host(std::move(host))
	{
		ADD_PROPERTY_GETTER(QString, "Project", true, Session::project);
		ADD_PROPERTY_GETTER(QString, "Username", true, Session::username);
		ADD_PROPERTY_GETTER(QString, "Host", true, Session::host);

		ADD_PROPERTY_GETTER(QString, "Date", true, Session::date);
		ADD_PROPERTY_GETTER(QString, "Time", true, Session::time);

		return;
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
		m_project = std::move(value);
	}

	QString Session::username() const
	{
		return m_username;
	}

	void Session::setUsername(QString value)
	{
		m_username = std::move(value);
	}

	QString Session::host() const
	{
		return m_host;
	}

	void Session::setHost(QString value)
	{
		m_host = std::move(value);
	}

	QString Session::date() const
	{
		QDateTime t = QDateTime::currentDateTime();
		return t.toString("dd MMM yyyy");
	}

	QString Session::time() const
	{
		QDateTime t = QDateTime::currentDateTime();
		return t.toString("hh:mm");
	}

}
