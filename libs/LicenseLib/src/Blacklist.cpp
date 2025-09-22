#include "Blacklist.h"

namespace LicenseLib
{
	Blacklist::Blacklist()
	{
#ifdef HAVE_EMBEDDED_BLACKLIST
#endif
	}

	tl::expected<bool, BlacklistReason> Blacklist::check(const QUuid& licenseUuid) const
	{
#ifdef HAVE_EMBEDDED_BLACKLIST
		qDebug() << "Blacklist::check: Checking embedded blacklist...";

		{
			auto it = ::LicenseLib::EmbeddedBlacklist.find(licenseUuid);
			if (it != ::LicenseLib::EmbeddedBlacklist.end())
			{
				return tl::unexpected(it->second.reason);
			}
		}
#endif
		qDebug() << "Blacklist::check: Checking runtime blacklist...";

		{
			auto it = m_blacklist.find(licenseUuid);
			if (it != m_blacklist.end())
			{
				return tl::unexpected(it->second.reason);
			}
		}

		return true;
	}
} // namespace LicenseLib