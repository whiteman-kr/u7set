#include "UserManager.h"
#include <QSettings>

//
// User
//

User::User()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "StrID", true, User::name, User::setName);
	ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, User::description, User::setDescription);
	ADD_PROPERTY_GETTER_SETTER(QString, "Password", true, User::password, User::setPassword);
	ADD_PROPERTY_GETTER_SETTER(bool, "Administrator", true, User::admin, User::setAdmin);
}

User::User(const QString& name, const QString& description, const QString& password, bool admin)
	:User()
{
	m_name = name;
	m_description = description;
	m_password = password;
	m_admin = admin;
}

User& User::operator=(const User& That)
{
	m_name = That.m_name;
	m_description = That.m_description;
	m_password = That.m_password;
	m_admin = That.m_admin;
	return *this;
}


QString User::name() const
{
	return m_name;
}

void User::setName(const QString& value)
{
	m_name = value;
}

QString User::description() const
{
	return m_description;
}

void User::setDescription(const QString& value)
{
	m_description = value;

}

QString User::password() const
{
	return m_password;
}

void User::setPassword(const QString& value)
{
	m_password = value;
}

bool User::admin() const
{
	return m_admin;
}

void User::setAdmin(bool value)
{
	m_admin = value;
}

//
// UserManager
//

UserManager::UserManager()
{
}

bool UserManager::requestPassword()
{
	return true;
}


void UserManager::Restore()
{
	QSettings s(QSettings::IniFormat, QSettings::SystemScope, qApp->organizationName(), qApp->applicationName() + "Users");

	int count = s.value("Users/count", 0).toInt();
	m_users.clear();
	m_users.reserve(count);
	for (int index = 0; index < count; index++)
	{
		User user;
		user.setName(s.value(QString("Users/User%1/name").arg(index)).toString());
		user.setDescription(s.value(QString("Users/User%1/description").arg(index)).toString());
		user.setPassword(s.value(QString("Users/User%1/password").arg(index)).toString());
		user.setAdmin(s.value(QString("Users/User%1/admin").arg(index)).toBool());
		m_users.push_back(user);
	}

	if (m_users.empty() == true)
	{
		m_users.push_back(User("Administrator", "Built-in administrator", "", true));
	}
}

void UserManager::Store()
{
	QSettings s(QSettings::IniFormat, QSettings::SystemScope, qApp->organizationName(), qApp->applicationName() + "Users");

	s.setValue("Users/count", static_cast<quint64>(m_users.size()));

	int index = 0;
	for (const User& u : m_users)
	{
		s.setValue(QString("Users/User%1/name").arg(index), u.name());
		s.setValue(QString("Users/User%1/description").arg(index), u.description());
		s.setValue(QString("Users/User%1/password").arg(index), u.password());
		s.setValue(QString("Users/User%1/admin").arg(index), u.admin());
		index++;
	}
}

