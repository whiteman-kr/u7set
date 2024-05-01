#include <TrendView/TrendArchiveServer.h>
#include <TrendView/TrendSignal.h>

namespace TrendLib
{
	bool TrendSignalPlusServerId::operator==(const TrendSignalPlusServerId& that) const
	{
		return appSignalId == that.appSignalId && archiveServerId == that.archiveServerId;
	}

	bool TrendSignalPlusServerId::operator==(const TrendSignalParam& that) const
	{
		return appSignalId == that.appSignalId() && archiveServerId == that.archiveServerId();
	}

	bool TrendSignalPlusServerId::operator<(const TrendSignalPlusServerId& that) const
	{
		return std::tuple{appSignalId, archiveServerId} < std::tuple{that.appSignalId, that.archiveServerId};
	}
}

