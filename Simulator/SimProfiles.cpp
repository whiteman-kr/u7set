#include "SimProfiles.h"

namespace Sim
{
	//
	// ProfileProperties
	//
	bool ProfileProperties::applyToObject(PropertyObject* object, QString* errorMessage) const
	{
		if (object == nullptr || errorMessage == nullptr)
		{
			Q_ASSERT(object);
			Q_ASSERT(errorMessage);
			return false;
		}

		errorMessage->clear();

		for (const auto&[propertyCaption, value] : properties)
		{
			auto p = object->propertyByCaption(propertyCaption);
			if (p == nullptr)
			{
				if (errorMessage->isEmpty() == false)
				{
					*errorMessage += "\n";
				}

				*errorMessage = QObject::tr("Property %1 not found.").arg(propertyCaption);
			}
			else
			{
				p->setValue(value);
			}
		}

		return errorMessage->isEmpty();
	}

	//
	// Profile
	//
	std::vector<QString> Profile::equipment() const
	{
		std::vector<QString> result;
		result.reserve(equipmentProperties.size());

		for(auto const& it: equipmentProperties)
		{
			result.push_back(it.first);
		}

		return result;
	}

	const ProfileProperties& Profile::properties(const QString& equipmentId) const
	{
		const auto& itEquipment = equipmentProperties.find(equipmentId);

		if (itEquipment == equipmentProperties.end())
		{
			static const ProfileProperties empty;
			return empty;
		}

		return itEquipment->second;
	}

	//
	// Profiles
	//
	Profiles::Profiles()
	{
	}

	bool Profiles::load(const QByteArray& data, QString* errorMsg)
	{
		return parse(data, errorMsg);
	}

	bool Profiles::parse(const QString& string, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return false;
		}

		m_profiles.clear();

		QStringList strings = string.split(QChar::LineFeed, Qt::SkipEmptyParts);

		QRegExp regExpProfile("\\[[a-zA-Z\\d_]+\\]$");
		QRegExp regExpProperty("[A-Z\\d_]+.\\w+\\s*=\\s*\"?[\\w.]+\"?;$");

		QString currentProfile = "Generic";

		for (const QString& dataStr : strings)
		{
			QString str = dataStr;

			int commentPos = str.indexOf("//");
			if (commentPos != -1)
			{
				str = str.left(commentPos);
			}

			str = str.trimmed();
			if (str.isEmpty() == true)
			{
				continue;
			}

			if (regExpProfile.exactMatch(str) == true)
			{
				str = str.remove(QRegExp("[\\[\\]]"));

				if (str.isEmpty() == true)
				{
					*errorMessage = QObject::tr("Can't parse string '%1', profile name is empty").arg(dataStr.trimmed());
					return false;
				}

				currentProfile = str;
				continue;
			}

			if (regExpProperty.exactMatch(str) == true)
			{
				int ptPos = str.indexOf('.');
				int eqPos = str.indexOf('=');

				QString equipmentId = str.left(ptPos).trimmed();
				QString propertyName = str.mid(ptPos + 1, eqPos - ptPos - 1).trimmed();
				QString propertyValue = str.right(str.length() - eqPos - 1).trimmed();

				if (propertyValue.endsWith(';') == true)
				{
					propertyValue.remove(propertyValue.length() - 1, 1);
				}

				if (propertyValue.endsWith('\"') == true)
				{
					propertyValue.remove(propertyValue.length() - 1, 1);
				}

				if (propertyValue.startsWith('\"') == true)
				{
					propertyValue.remove(0, 1);
				}

				Profile& profile = m_profiles[currentProfile];
				profile.profileName = currentProfile;

				ProfileProperties& eqp = profile.equipmentProperties[equipmentId];
				eqp.equipmentId = equipmentId;

				if (eqp.properties.count(propertyName) != 0)
				{
					*errorMessage = QObject::tr("Ambiguous property value, property %1, profile %2 Object %3")
									.arg(propertyName)
									.arg(currentProfile)
									.arg(equipmentId);
					return false;
				}

				eqp.properties[propertyName] = propertyValue;

				continue;
			}

			*errorMessage = QObject::tr("Can't parse string '%1'").arg(dataStr.trimmed());
			return false;
		}

		return true;
	}

	std::vector<QString> Profiles::profiles() const
	{
		std::vector<QString> result;
		result.reserve(m_profiles.size());

		for(auto const& it: m_profiles)
		{
			result.push_back(it.first);
		}

		return result;
	}

	const Profile& Profiles::profile(const QString& profileId) const
	{
		auto it = m_profiles.find(profileId);
		if (it == m_profiles.end())
		{
			static const Profile empty;
			return empty;
		}

		return it->second;
	}

	QString Profiles::dump() const
	{
		QString result;

		std::vector<QString> allProfiles = profiles();

		for (const QString& profileId: allProfiles)
		{
			result += QObject::tr("\nProfile: %1\n").arg(profileId);

			const Profile& p = profile(profileId);
			std::vector<QString> allEquipment = p.equipment();

			for (const QString& equipmentId : allEquipment)
			{
				result += QObject::tr("\nEquipment: %1\n\n").arg(equipmentId);

				const ProfileProperties& eqp = p.properties(equipmentId);

				for (auto it = eqp.properties.begin(); it != eqp.properties.end(); it++)
				{
					result += QObject::tr("%1 = %2\n").arg(it->first).arg(it->second.toString());
				}
			}
		}

		return result;
	}
}

