#pragma once

#include <vector>


namespace BuildCompLib
{
	struct CompareResult
	{
		bool isSame = false;

		bool projectName = true;
		QString projectNameLeft;
		QString projectNameRight;

		bool userName = true;
		QString userNameLeft;
		QString userNameRight;

		bool buildNumber = true;
		int buildNumberLeft = -1;
		int buildNumberRight = -1;

		struct Subsystem
		{
			enum SideResult
			{
				NotModified,
				Modified,
				NotExists
			};

			QString subsystemId;
			SideResult left = Modified;
			SideResult right = Modified;

			QStringList leftModules;  // Modules on the left side for this subsystem
			QStringList rightModules; // Modules on the right side for this subsystem
		};

		std::vector<Subsystem> subsystems;
	};
} // namespace BuildCompLib