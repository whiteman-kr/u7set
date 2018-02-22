#pragma once

#include "Types.h"
#include "../Proto/network.pb.h"

class SoftwareInfo
{
public:
	static const int UNDEFINED_BUILD_NO;

	SoftwareInfo();
	SoftwareInfo(const SoftwareInfo& si);

	void clear();

	void init(E::SoftwareType softwareType,
			  const QString& equipmentID,
			  int majorVersion,
			  int minorVersion,
			  int buildNo);

	void init(E::SoftwareType softwareType,
			  const QString& equipmentID,
			  int majorVersion,
			  int minorVersion);

	void serializeTo(Network::SoftwareInfo* info) const;
	void serializeFrom(const Network::SoftwareInfo& info);

	E::SoftwareType softwareType() const { return m_softwareType; }

	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }
	QString equipmentID() const { return m_equipmentID; }

	int majorVersion() const { return m_majorVersion; }
	int minorVersion() const { return m_minorVersion; }
	int commitNo() const { return m_commitNo; }
	QString buildBranch() const { return m_buildBranch; }
	QString commitSHA() const { return m_commitSHA; }
	QString userName() const { return m_userName; }
	int buildNo() const { return m_buildNo; }
	quint32 crc() const { return m_crc; }

private:
	E::SoftwareType m_softwareType = E::SoftwareType::Unknown;
	QString m_equipmentID;
	int m_majorVersion = 0;
	int m_minorVersion = 0;
	int m_commitNo = 0;
	QString m_buildBranch;
	QString m_commitSHA;
	QString m_userName;
	int m_buildNo = UNDEFINED_BUILD_NO;
	quint32 m_crc = 0;
};
