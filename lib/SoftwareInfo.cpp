#include "SoftwareInfo.h"
#include "version.h"

const int SoftwareInfo::UNDEFINED_BUILD_NO = -1;

SoftwareInfo::SoftwareInfo()
{
}

SoftwareInfo::SoftwareInfo(const SoftwareInfo& si)
{
	*this = si;
}

void SoftwareInfo::init(E::SoftwareType softwareType,
							 const QString& equipmentID,
							 int majorVersion,
							 int minorVersion,
							 int buildNo)
{
	m_softwareType = softwareType;
	m_equipmentID = equipmentID;
	m_majorVersion = majorVersion;
	m_minorVersion = minorVersion;
	m_buildBranch = BUILD_BRANCH;
	m_commitSHA = USED_SERVER_COMMIT_SHA;
	m_commitNo = USED_SERVER_COMMIT_NUMBER;
	m_buildNo = buildNo;

#ifdef Q_OS_LINUX
	m_userName = getenv("USER");
#endif

#ifdef Q_OS_WIN
	m_userName = getenv("USERNAME");
#endif
}

void SoftwareInfo::init(E::SoftwareType softwareType,
		  const QString& equipmentID,
		  int majorVersion,
		  int minorVersion)
{
	init(softwareType, equipmentID, majorVersion, minorVersion, UNDEFINED_BUILD_NO);
}


void SoftwareInfo::serializeTo(Network::SoftwareInfo* info) const
{
	if (info == nullptr)
	{
		assert(false);
		return;
	}

	info->set_softwaretype(static_cast<int>(m_softwareType));
	info->set_equipmentid(m_equipmentID.toStdString());
	info->set_majorversion(m_majorVersion);
	info->set_minorversion(m_minorVersion);
	info->set_commitno(m_commitNo);
	info->set_buildbranch(m_buildBranch.toStdString());
	info->set_commitsha(m_commitSHA.toStdString());
	info->set_username(m_userName.toStdString());
	info->set_buildno(m_buildNo);
	info->set_crc(m_crc);
}

void SoftwareInfo::serializeFrom(const Network::SoftwareInfo& info)
{
	m_softwareType = static_cast<E::SoftwareType>(info.softwaretype());
	m_equipmentID = QString::fromStdString(info.equipmentid());
	m_majorVersion = info.majorversion();
	m_minorVersion = info.minorversion();
	m_commitNo = info.commitno();
	m_buildBranch = QString::fromStdString(info.buildbranch());
	m_commitSHA = QString::fromStdString(info.commitsha());
	m_userName = QString::fromStdString(info.username());
	m_buildNo = info.buildno();
	m_crc = info.crc();
}
