#include "SoftwareInfo.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

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

#ifdef GITLAB_CI_BUILD
	m_buildBranch = CI_BUILD_REF_SLUG;
	m_commitSHA = CI_COMMIT_SHA;
	m_commitNo = CI_PIPELINE_ID;
#else
	m_buildBranch = "LOCALBUILD";
	m_commitSHA = "LOCALBUILD";
	m_commitNo = 0;
#endif
	m_buildNo = buildNo;


#ifdef Q_OS_LINUX
	m_userName = getenv("USER");
#endif

#ifdef Q_OS_WIN

	QString qUsername("USERNAME");
	wchar_t username[16];
	int ln = qUsername.toWCharArray(username);
	username[ln] = '\0';

	wchar_t* buf = nullptr;
	size_t len = 0;

	errno_t err = _wdupenv_s(&buf, &len, username);

	Q_ASSERT(err == 0);

	if (err == 0)
	{
		m_userName = QString::fromWCharArray(buf, -1);
	}

	if (buf != nullptr)
	{
		free(buf);
	}

#endif
}

void SoftwareInfo::clear()
{
	m_softwareType = E::SoftwareType::Unknown;
	m_equipmentID.clear();
	m_majorVersion = 0;
	m_minorVersion = 0;
	m_commitNo = 0;
	m_buildBranch.clear();
	m_commitSHA.clear();
	m_userName.clear();
	m_buildNo = UNDEFINED_BUILD_NO;
	m_crc = 0;
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
