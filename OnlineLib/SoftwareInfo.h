#pragma once

#include <CommonLib/Types.h>

namespace Network
{
	class SoftwareInfo;
}

class SoftwareInfo
{
public:
	SoftwareInfo() = default;
	SoftwareInfo(E::SoftwareType softwareType, const QString& equipmentID);

	void clear();

	void serializeTo(Network::SoftwareInfo* info) const;
	void serializeFrom(const Network::SoftwareInfo& info);

	E::SoftwareType softwareType() const { return m_softwareType; }
	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	int majorVersion() const { return m_majorVersion; }
	int minorVersion() const { return m_minorVersion; }
	int patchVersion() const { return m_patchVersion; }

	QString releaseType() const { return m_releaseType; }

	QString branchName() const { return m_branchName; }
	QString commitHash() const { return m_commitHash; }
	QString buildUserName() const { return m_buildUserName; }
	QString buildDate() const { return m_buildDate; }
	QString buildHostname() const { return m_buildHostname; }
	int pipelineID() const { return m_pipelineID; }

	QString hostname() const { return m_hostname; }
	void setHostname(const QString& hostName) { m_hostname = hostName; }

	QString osUsername() const { return m_osUsername; }

private:
	E::SoftwareType m_softwareType = E::SoftwareType::Unknown;
	QString m_equipmentID;

	// initialization by values from version.h
	//
	int m_majorVersion = 0;
	int m_minorVersion = 0;
	int m_patchVersion = 0;

	QString m_releaseType;

	QString m_branchName;
	QString m_commitHash;
	QString m_buildUserName;
	QString m_buildDate;
	QString m_buildHostname;
	int m_pipelineID = 0;

	QString m_hostname;			// Hostname on which software running
	QString m_osUsername;		// OS logged in user
};
