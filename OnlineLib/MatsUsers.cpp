#ifndef ONLINE_LIB_DOMAIN
	#error Do not include this file in the project! Link DbLib instead.
#endif

#include "MatsUsers.h"

namespace OnlineLib
{

	MatsUser::MatsUser(const QString& login, const QString& description) :
		m_login(login),
		m_description(description)
	{
	}

	bool MatsUser::save(QXmlStreamWriter& writer) const
	{
		writer.writeStartElement("User");
		writer.writeAttribute("Login", login());
		writer.writeAttribute("Description", description());
		writer.writeAttribute("Enabled", enabled() ? "true" : "false");
		writer.writeAttribute("TuningTags", tuningTagsToString());
		writer.writeEndElement();
		return true;
	}

	bool MatsUser::load(QXmlStreamReader& reader)
	{
		if (reader.name() != QLatin1String("User"))
		{
			return false;
		}

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
			setTuningTagsFromString(reader.attributes().value(QLatin1String("TuningTags")).toString());
		}

		QXmlStreamReader::TokenType endToken = reader.readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return true;
	}

	const QString& MatsUser::login() const
	{
		return m_login;
	}

	void MatsUser::setLogin(const QString& value)
	{
		m_login = value;
	}

	const QString& MatsUser::description() const
	{
		return m_description;
	}

	void MatsUser::setDescription(const QString& value)
	{
		m_description = value;
	}

	bool MatsUser::enabled() const
	{
		return m_enabled;
	}

	void MatsUser::setEnabled(bool value)
	{
		m_enabled = value;
	}

	const std::set<QString>& MatsUser::tuningTags() const
	{
		return m_tuningTags;
	}

	void MatsUser::setTuningTags(const std::set<QString>& value)
	{
		m_tuningTags = value;
	}

	QString MatsUser::tuningTagsToString() const
	{
		QStringList result;
		for (const auto& tag : m_tuningTags)
		{
			result.push_back(tag);
		}
		return result.join(';');
	}

	void MatsUser::setTuningTagsFromString(QString value)
	{
		m_tuningTags.clear();
		value.remove('\r');
		value.replace('\n', ';');
		value.replace(',', ';');
		value.replace(' ', ';');
		auto list = value.split(';', Qt::SkipEmptyParts);
		for (const QString& s : list)
		{
			m_tuningTags.insert(s.trimmed());
		}
	}

	//
	// MatsUserStorage
	//
	MatsUserStorage::MatsUserStorage()
	{
	}

	void MatsUserStorage::add(const MatsUser& user)
	{
		m_users.push_back(user);
	}

	int MatsUserStorage::count() const
	{
		return static_cast<int>(m_users.size());
	}

	const MatsUser& MatsUserStorage::get(int index) const
	{
		if (index < 0 || index >= count())
		{
			static MatsUser err;
			assert(false);
			return err;
		}
		return m_users[index];
	}

	void MatsUserStorage::clear()
	{
		m_users.clear();
	}

	const std::vector<MatsUser>& MatsUserStorage::users() const
	{
		return m_users;
	}

	const std::set<QString>& MatsUserStorage::tuningTags(const QString& login, bool* found) const
	{

		auto it = std::find_if(m_users.begin(), m_users.end(), [&login](const MatsUser& user)
							   {
								   return user.login() == login;
							   });
		if (it == m_users.end())
		{
			if (found != nullptr)
			{
				*found = false;
			}
			static std::set<QString> err;
			return err;
		}
		if (found != nullptr)
		{
			*found = true;
		}
		return it->tuningTags();
	}

	bool MatsUserStorage::load(const QByteArray& data, QString& errorCode)
	{
		// Load subsystems from XML
		//

		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != QLatin1String("MatsUsers"))
		{
			reader.raiseError(QObject::tr("The file is not an MatsUsers file."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			MatsUser user;
			if (user.load(reader) == true)
			{
				m_users.push_back(user);
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				errorCode = reader.errorString();
				reader.skipCurrentElement();
			}
		}
		return !reader.hasError();
	}

	bool MatsUserStorage::save(QByteArray& data, const QString& comment) const
	{
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);
		writer.writeStartDocument();

		writer.writeStartElement("MatsUsers");
		for (const auto& s : m_users)
		{
			s.save(writer);
		}
		writer.writeEndElement();

		writer.writeEndDocument();

		return true;
	}

} // namespace OnlineLib