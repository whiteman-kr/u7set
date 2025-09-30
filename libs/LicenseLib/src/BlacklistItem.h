#pragma once
#include <QUuid>

namespace LicenseLib
{
	enum class BlacklistReason
	{
		Revoked,
		Compromised,
		Expired
	};

	struct BlacklistItem
	{
		QUuid licenseUuid;
		BlacklistReason reason;
	};
} // namespace LicenseLib