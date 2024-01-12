#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "SoftwareInfo.h"
#include "version.h"

SoftwareInfo::SoftwareInfo(E::SoftwareType softwareType,
						   const QString& equipmentID) :
	m_softwareType(softwareType),
	m_equipmentID(equipmentID),
	m_majorVersion(U7SET_MAJOR_VERSION),
	m_minorVersion(U7SET_MINOR_VERSION),
	m_patchVersion(U7SET_PATCH_VERSION),
	m_releaseType(U7SET_RELEASE_TYPE),
	m_branchName(U7SET_BRANCH_NAME),
	m_commitHash(U7SET_COMMIT_HASH),
	m_userName(U7SET_USER_NAME),
	m_buildDate(U7SET_BUILD_DATE),
	m_hostname(U7SET_HOSTNAME),
	m_pipelineID(U7SET_PIPELINE_ID)
{
}

void SoftwareInfo::clear()
{
	m_softwareType = E::SoftwareType::Unknown;
	m_equipmentID.clear();

	m_majorVersion = 0;
	m_minorVersion = 0;
	m_patchVersion = 0;

	m_releaseType.clear();

	m_branchName.clear();
	m_commitHash.clear();
	m_userName.clear();
	m_buildDate.clear();
	m_hostname.clear();
	m_pipelineID = 0;
}

void SoftwareInfo::serializeTo(Network::SoftwareInfo* info) const
{
	if (info == nullptr)
	{
		assert(false);
		return;
	}

	info->set_softwaretype(TO_INT(m_softwareType));
	info->set_equipmentid(m_equipmentID.toStdString());

	info->set_majorversion(m_majorVersion);
	info->set_minorversion(m_minorVersion);
	info->set_patchversion(m_patchVersion);

	info->set_releasetype(m_releaseType.toStdString());

	info->set_branchname(m_branchName.toStdString());
	info->set_commithash(m_commitHash.toStdString());
	info->set_username(m_userName.toStdString());
	info->set_builddate(m_buildDate.toStdString());
	info->set_hostname(m_hostname.toStdString());
	info->set_pipelineid(m_pipelineID);
}

void SoftwareInfo::serializeFrom(const Network::SoftwareInfo& info)
{
	m_softwareType = static_cast<E::SoftwareType>(info.softwaretype());
	m_equipmentID = QString::fromStdString(info.equipmentid());

	m_majorVersion = info.majorversion();
	m_minorVersion = info.minorversion();
	m_patchVersion = info.patchversion();

	m_releaseType = QString::fromStdString(info.releasetype());

	m_branchName = QString::fromStdString(info.branchname());
	m_commitHash = QString::fromStdString(info.commithash());
	m_userName = QString::fromStdString(info.username());
	m_buildDate = QString::fromStdString(info.builddate());
	m_hostname = QString::fromStdString(info.hostname());
	m_pipelineID = info.pipelineid();
}
