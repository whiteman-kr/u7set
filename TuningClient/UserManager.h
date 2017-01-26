#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "../lib/PropertyObject.h"

class User : public PropertyObject
{
public:
	User();
	User(const QString& name, const QString& description, const QString& password, bool admin);

	User& operator=(const User& That);

	QString name() const;
	void setName(const QString& value);

	QString description() const;
	void setDescription(const QString& value);

	QString password() const;
	void setPassword(const QString& value);

	bool admin() const;
	void setAdmin(bool value);

private:
	QString m_name;
	QString m_description;
	QString m_password;

	bool m_admin = false;
};

class UserManager
{
public:
	UserManager();

    bool requestPassword(QWidget *parent, bool adminNeeded);

	void Restore();
	void Store();

	std::vector<User> m_users;

private:
    QString m_emptyMd5;
};

Q_DECLARE_METATYPE(User)


#endif // USERMANAGER_H
