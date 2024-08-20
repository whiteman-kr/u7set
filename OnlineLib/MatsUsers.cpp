#ifndef ONLINE_LIB_DOMAIN
	#error Do not include this file in the project! Link DbLib instead.
#endif

#include "MatsUsers.h"
#include <CommonLib/ConstStrings.h>

namespace OnlineLib
{

	MatsUser::MatsUser(const QString& login, const QString& description) :
		m_login(login),
		m_description(description)
	{
	}

	bool MatsUser::save(QXmlStreamWriter& writer) const
	{
		writer.writeStartElement(XmlElement::MATS_USER);
		writer.writeAttribute(XmlAttribute::LOGIN, login());
		writer.writeAttribute(XmlAttribute::DESCRIPTION, description());
		writer.writeAttribute(XmlAttribute::ENABLED, enabled() ? "true" : "false");
		writer.writeAttribute(XmlAttribute::APP_SIGNAL_TAGS, appSignalTagsToString());
		writer.writeEndElement();
		return true;
	}

	bool MatsUser::load(QXmlStreamReader& reader)
	{
		if (reader.name() != XmlElement::MATS_USER)
		{
			return false;
		}

		if (reader.attributes().hasAttribute(XmlAttribute::LOGIN) == true)
		{
			setLogin(reader.attributes().value(XmlAttribute::LOGIN).toString());
		}

		if (reader.attributes().hasAttribute(XmlAttribute::DESCRIPTION))
		{
			setDescription(reader.attributes().value(XmlAttribute::DESCRIPTION).toString());
		}

		if (reader.attributes().hasAttribute(XmlAttribute::ENABLED))
		{
			setEnabled(reader.attributes().value(XmlAttribute::ENABLED).toString() == "true" ? true : false);
		}

		if (reader.attributes().hasAttribute(XmlAttribute::APP_SIGNAL_TAGS))
		{
			setAppSignalTagsFromString(reader.attributes().value(XmlAttribute::APP_SIGNAL_TAGS).toString());
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

	const std::set<QString>& MatsUser::appSignalTags() const
	{
		return m_appSignalTags;
	}

	void MatsUser::setAppSignalTags(const std::set<QString>& tags)
	{
		m_appSignalTags = tags;
	}

	QString MatsUser::appSignalTagsToString(QChar separator) const
	{
		QStringList result;
		for (const auto& tag : m_appSignalTags)
		{
			result.push_back(tag);
		}
		return result.join(separator);
	}

	void MatsUser::setAppSignalTagsFromString(QString value)
	{
		m_appSignalTags.clear();
		value.remove('\r');
		value.replace('\n', ';');
		value.replace(',', ';');
		value.replace(' ', ';');
		auto list = value.split(';', Qt::SkipEmptyParts);
		for (const QString& s : list)
		{
			m_appSignalTags.insert(s.trimmed());
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

	const std::set<QString>& MatsUserStorage::appSignalTags(const QString& login, bool* found) const
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
		return it->appSignalTags();
	}

	bool MatsUserStorage::loadFromByteArray(const QByteArray& data, QString& errorCode)
	{
		clear();

		// Load MATS users from XML
		//
		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != XmlElement::MATS_USERS)
		{
			reader.raiseError(QObject::tr("The file is not an MatsUsers file."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		while (reader.readNextStartElement())
		{
			MatsUser user;

			if (user.load(reader) == true)
			{
				m_users.push_back(user);
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown XML tag: ") + reader.name().toString());
				errorCode = reader.errorString();
				reader.skipCurrentElement();
			}
		}
		return !reader.hasError();
	}

	bool MatsUserStorage::saveToByteArray(QByteArray& data) const
	{
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);
		writer.writeStartDocument();

		writer.writeStartElement(XmlElement::MATS_USERS);

		for (const auto& s : m_users)
		{
			s.save(writer);
		}

		writer.writeEndElement();

		writer.writeEndDocument();

		return true;
	}

} // namespace OnlineLib
