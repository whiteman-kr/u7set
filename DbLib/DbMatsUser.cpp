#ifndef DB_LIB_DOMAIN
	#error Do not include this file in the project! Link DbLib instead.
#endif

#include "DbMatsUser.h"

DbMatsUser::DbMatsUser(const QString& login, const QString& description):
	m_login(login),
	m_description(description)
{
}

bool DbMatsUser::save(QXmlStreamWriter& writer) const
{
	writer.writeAttribute("Login", login());
	writer.writeAttribute("Description", description());
	writer.writeAttribute("Enabled", enabled() ? "true" : "false");
	writer.writeAttribute("TuningTags", tuningTags());
	return true;
}

bool DbMatsUser::load(QXmlStreamReader& reader)
{
if (reader.attributes().hasAttribute(QLatin1String("Login")) == true)
	{
		setLogin(reader.attributes().value(QLatin1String("Login")).toString());
	}

	if (reader.attributes().hasAttribute(QLatin1String("Description")))
	{
		setDescription(reader.attributes().value(QLatin1String("Description")).toString());
	}

	if (reader.attributes().hasAttribute(QLatin1String("Enabled")))
	{
		setEnabled(reader.attributes().value(QLatin1String("Enabled")).toString() == "true" ? true : false);
	}

	if (reader.attributes().hasAttribute(QLatin1String("TuningTags")))
	{
		setTuningTags(reader.attributes().value(QLatin1String("TuningTags")).toString());
	}

	QXmlStreamReader::TokenType endToken = reader.readNext();
	Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

	return true;
}

const QString& DbMatsUser::login() const
{
	return m_login;
}

void DbMatsUser::setLogin(const QString& value)
{
	m_login = value;
}

const QString& DbMatsUser::description() const
{
	return m_description;
}

void DbMatsUser::setDescription(const QString& value)
{
	m_description = value;
}

bool DbMatsUser::enabled() const
{
	return m_enabled;
}

void DbMatsUser::setEnabled(bool value)
{
	m_enabled = value;
}

const QString& DbMatsUser::tuningTags() const
{
	return m_tuningTags;
}

void DbMatsUser::setTuningTags(const QString& value)
{
	m_tuningTags = value;
}
