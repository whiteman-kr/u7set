#include "SimProfiles.h"

namespace Sim
{
	//
	// ProfileProperties
	//
	bool ProfileProperties::applyToObject(std::shared_ptr<PropertyObject> object, QString* errorMessage)
	{
		m_savedObject = object;
		m_savedPropertirs.clear();

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

				*errorMessage = QObject::tr("Property %1 not found").arg(propertyCaption);
			}
			else
			{
				QVariant propValue = p->value();

				if ((value.type() == QVariant::String && propValue.type() != QVariant::String) ||
					(value.type() != QVariant::String && propValue.type() == QVariant::String) ||
					value.canConvert(propValue.type()) == false)
				{
					*errorMessage = QObject::tr("Property %1 has incompatible type").arg(propertyCaption);
					continue;
				}

				m_savedPropertirs[propertyCaption] = propValue;		// Save old value property
				p->setValue(value);
			}
		}

		return errorMessage->isEmpty();
	}

	bool ProfileProperties::restoreObject()
	{
		if (m_savedObject == nullptr)
		{
			Q_ASSERT(m_savedObject);
			return false;
		}

		bool ok = true;

		for (const auto&[propertyCaption, value] : m_savedPropertirs)
		{
			auto p = m_savedObject->propertyByCaption(propertyCaption);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				ok = false;
			}
			else
			{
				p->setValue(value);
			}
		}

		m_savedObject = nullptr;
		m_savedPropertirs.clear();

		return ok;
	}

	//
	// Profile
	//
	QStringList Profile::equipment() const
	{
		QStringList result;
		result.reserve(static_cast<int>(equipmentProperties.size()));

		for(auto const& it: equipmentProperties)
		{
			result.push_back(it.first);
		}

		return result;
	}

	bool Profile::applyToObject(QString equipmentId, std::shared_ptr<PropertyObject> object, QString* errorMessage)
	{
		auto it = equipmentProperties.find(equipmentId);
		if (it == equipmentProperties.end())
		{
			return false;
		}

		ProfileProperties& pp = it->second;
		Q_ASSERT(equipmentId == pp.equipmentId);

		return pp.applyToObject(object, errorMessage);
	}

	const ProfileProperties& Profile::properties(const QString& equipmentId) const
	{
		auto itEquipment = equipmentProperties.find(equipmentId);

		if (itEquipment == equipmentProperties.end())
		{
			static const ProfileProperties empty;
			return empty;
		}

		return itEquipment->second;
	}

	ProfileProperties& Profile::properties(const QString& equipmentId)
	{
		auto itEquipment = equipmentProperties.find(equipmentId);

		if (itEquipment == equipmentProperties.end())
		{
			static ProfileProperties empty;
			return empty;
		}

		return itEquipment->second;
	}

	bool Profile::restoreObjects()
	{
		bool ok = true;

		for (auto&[equipmentId, profileProps] : equipmentProperties)
		{
			Q_ASSERT(equipmentId == profileProps.equipmentId);
			Q_UNUSED(equipmentId);

			ok &= profileProps.restoreObject();
		}

		return ok;
	}

	//
	// Profiles
	//
	Profiles::Profiles()
	{
	}

	void Profiles::clear()
	{
		*this = {};
	}

	bool Profiles::load(const QByteArray& data, QString* errorMsg)
	{
		return parse(data, errorMsg);
	}

	bool Profiles::parse(const QString& string, QString* errorMessage)
	{
		clear();

		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return false;
		}

		std::map<QString, Profile> profiles;

		QStringList strings = string.split(QChar::LineFeed, Qt::SkipEmptyParts);

		QRegExp regExpProfile("^\\[[a-zA-Z\\d_]+\\]");
		QRegExp regExpProperty("^[A-Z\\d_]+.\\w+\\s*=\\s*\"?[\\w\\s.\\+\\-/\\\\:]+\"?;");	// OBJECT1.a_1 = " hello+-./\: ";
		QRegExp regExpUInt("^(?:0|[1-9][0-9]*)$");
		QRegExp regExpInt("^-?(?:0|[1-9][0-9]*)$");
		QRegExp regExpDouble("^-?(?:0|[1-9][0-9]*)\\.?[0-9]+([e|E][+-]?[0-9]+)?$");


		QString currentProfile = "Generic";

		for (const QString& dataStr : strings)
		{
			QString str = dataStr.trimmed();

			if (str.isEmpty() == true)
			{
				continue;
			}

			if (str.startsWith("//"))
			{
				continue;
			}

			int pos = regExpProfile.indexIn(str);
			if (pos != -1 && regExpProfile.capturedTexts().size() == 1)
			{
				str = regExpProfile.cap(0);

				str = str.remove(QRegExp("[\\[\\]]"));

				if (str.isEmpty() == true)
				{
					*errorMessage = QObject::tr("Can't parse string '%1', profile name is empty").arg(dataStr.trimmed());
					return false;
				}

				currentProfile = str;
				continue;
			}

			if (currentProfile.compare("Default", Qt::CaseInsensitive) == 0)
			{
				*errorMessage = QObject::tr("Profile name 'Default' is reserved, please use another profile name");
				return false;
			}

			pos = regExpProperty.indexIn(str);
			if (pos != -1 && regExpProperty.capturedTexts().size() == 1)
			{
				str = regExpProperty.cap(0);

				int ptPos = str.indexOf('.');
				int eqPos = str.indexOf('=');

				QString equipmentId = str.left(ptPos).trimmed();
				QString propertyName = str.mid(ptPos + 1, eqPos - ptPos - 1).trimmed();
				QString propertyString = str.right(str.length() - eqPos - 1).trimmed();

				if (propertyString.endsWith(';') == true)
				{
					propertyString.remove(propertyString.length() - 1, 1);
				}

				bool quoted = false;

				if (propertyString.endsWith('\"') == true && propertyString.startsWith('\"') == true)
				{
					// Property value is string in quotes
					//
					propertyString.remove(propertyString.length() - 1, 1);
					propertyString.remove(0, 1);

					quoted = true;
				}

				QVariant propertyValue = propertyString;

				// Check if propertyValue is a number or boolean value
				//
				do
				{
					if (regExpUInt.exactMatch(propertyString) == true)
					{
						propertyValue = propertyValue.toUInt();
						break;
					}

					if (regExpInt.exactMatch(propertyString) == true)
					{
						propertyValue = propertyValue.toInt();
						break;
					}

					if (regExpDouble.exactMatch(propertyString) == true)
					{
						propertyValue = propertyValue.toDouble();
						break;
					}

					if (propertyString.compare("true", Qt::CaseInsensitive) == 0)
					{
						propertyValue = true;
						break;
					}

					if (propertyString.compare("false", Qt::CaseInsensitive) == 0)
					{
						propertyValue = false;
						break;
					}

					if (quoted == true)
					{
						propertyValue = propertyValue.toString();
						break;
					}

					*errorMessage = QObject::tr("Can't parse property value '%1'\nin\n'%2'").arg(propertyString).arg(dataStr.trimmed());
					return false;

				}while(false);

				Profile& profile = profiles[currentProfile];
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

		m_profiles = std::move(profiles);

		return true;
	}

	QStringList Profiles::profiles() const
	{
		QStringList result;
		result.reserve(static_cast<int>(m_profiles.size()));

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

	Profile& Profiles::profile(const QString& profileId)
	{
		auto it = m_profiles.find(profileId);
		if (it == m_profiles.end())
		{
			static Profile empty;
			return empty;
		}

		return it->second;
	}

	QString Profiles::dump() const
	{
		QString result;
		QStringList allProfiles = profiles();

		for (const QString& profileId: allProfiles)
		{
			result += QObject::tr("\nProfile: %1\n").arg(profileId);

			const Profile& p = profile(profileId);
			QStringList allEquipment = p.equipment();

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

