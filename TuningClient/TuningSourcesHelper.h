#pragma once

#include <vector>
#include <QWidget>
#include "../ClientLib/TuningConnection.h"


namespace ClientLib
{
	class TuningTcpClient;

	class TuningSourcesHelper
	{
	public:

		// Get Errors and SOR flag count
		//
		static int sourcesErrorsCount(const std::vector<ClientLib::TuningSource>& sourceStates);
		static int sourcesSorCount(const std::vector<ClientLib::TuningSource>& sourceStates, bool* sorActive, bool* sorValid);

		static int sourceErrorsCount(const std::vector<ClientLib::TuningSource>& sourceStates, Hash sourceHash);
		static int sourceSorCount(const std::vector<ClientLib::TuningSource>& sourceStates, Hash sourceHash, bool* sorActive, bool* sorValid);

		// Activate specified tuning source in specified Tuning Services
		//
		static void activateTuningSource(ClientLib::TuningConnection& tuningConnection, const QString& sourceEquipmentId, bool activate, QWidget* parent);
	};
}
