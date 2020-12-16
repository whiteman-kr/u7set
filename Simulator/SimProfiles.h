#pragma once

namespace Sim
{
	//
	// ProfileProperties
	//

	struct ProfileProperties
	{
		std::map<QString, QVariant> properties;
	};

	//
	// Profile
	//

	struct Profile
	{
		std::vector<QString> equipment() const;

		const ProfileProperties& properties(const QString& equipmentId) const;

		std::map<QString, ProfileProperties> equipmentProperties;
	};

	//
	// Profiles
	//

	class Profiles
	{
	public:
		Profiles();

		bool load(const QByteArray& data, QString* errorMsg);
		bool parse(const QString& string, QString* errorMsg);

		std::vector<QString> profiles() const;

		const Profile& profile(const QString& profileId) const;

		QString dump() const;

	private:

		std::map<QString, Profile> m_profiles;
	};
}

