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
		bool applyToObject(std::shared_ptr<PropertyObject> object, QString* errorMessage);
		bool restoreObject();

	private:
		std::shared_ptr<PropertyObject> m_savedObject;	// This property is saved in applyToObject
		std::map<QString, QVariant> m_savedPropertirs;	// Key - property caption, Value - value to override
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
		[[nodiscard]] QStringList equipment() const;
		bool applyToObject(QString equipmentId, std::shared_ptr<PropertyObject> object, QString* errorMessage);

		[[nodiscard]] const ProfileProperties& properties(const QString& equipmentId) const;
		[[nodiscard]] ProfileProperties& properties(const QString& equipmentId);

		bool restoreObjects();
	};

	// Profiles
	//
	class Profiles
	{
	public:
		Profiles();

	public:
		void clear();

		bool load(const QByteArray& data, QString* errorMessage);
		bool parse(const QString& string, QString* errorMessage);

		[[nodiscard]] QStringList profiles() const;

		[[nodiscard]] const Profile& profile(const QString& profileId) const;
		[[nodiscard]] Profile& profile(const QString& profileId);

		QString dump() const;

	private:
		std::map<QString, Profile> m_profiles;
	};
}

