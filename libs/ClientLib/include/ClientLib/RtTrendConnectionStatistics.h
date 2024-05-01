#pragma once

namespace ClientLib
{
	struct RtTrendConnectionStatistics
	{
		QString text;
		int requestQueueSize = 0;
		int requestCount = 0;
		int replyCount = 0;
		int isConnected = 0;		// It must be int for summing up statistics for several connections.
	};
}