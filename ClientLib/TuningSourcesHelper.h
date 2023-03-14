#pragma once

#include <vector>
#include <QWidget>

namespace ClientLib
{
	class TuningTcpClient;

	class TuningSourcesHelper
	{
	public:

		// Determine if activate or deactivate actions are available for specified tuning source in specified Tuning Services
		//
		static void isActivationActionsAvailable(std::vector<TuningTcpClient*> clients, const QString& sourceEquipmentId, bool* activateEnabled, bool* deactivateEnabled);

		// Activate specified tuning source in specified Tuning Services
		//
		static void activateTuningSourceControl(std::vector<TuningTcpClient*> clients, const QString& sourceEquipmentId, bool enable, QWidget* parent);

		// Take control on specified Tuning Services
		//
		static bool takeServicesControl(std::vector<TuningTcpClient*> clients, QWidget* parent);

		// Returns true if specified Tuning Services control the same tuning source
		//
		static bool clientsHaveSameActiveSource(std::vector<TuningTcpClient*> clients);
	};
}
