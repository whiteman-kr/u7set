#pragma once
#include "../lib/PropertyObject.h"

namespace Sim
{
	//
	// ProfileProperties
	//
	struct ProfileProperties
	{
		QString equipmentId;
		std::map<QString, QVariant> properties;			// Key - property caption, Value - value to override

		// --
		//
		bool applyToObject(PropertyObject* object, QString* errorMessage) const;
	};

	//
	// Profile
	//
	struct Profile
	{
		QString profileName;
		std::map<QString, ProfileProperties> equipmentProperties;	// Key is EquipmentID, value is properties for this object

		// --
		//
		[[nodiscard]] std::vector<QString> equipment() const;
		[[nodiscard]] const ProfileProperties& properties(const QString& equipmentId) const;
	};

	// Profiles
	//
	class Profiles
	{
	public:
		Profiles();

	public:
		bool load(const QByteArray& data, QString* errorMessage);
		bool parse(const QString& string, QString* errorMessage);

		[[nodiscard]] std::vector<QString> profiles() const;
		[[nodiscard]] const Profile& profile(const QString& profileId) const;

		QString dump() const;

	private:
		std::map<QString, Profile> m_profiles;
	};
}

