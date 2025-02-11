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
		int buildNumberLeft;
		int buildNumberRight;

		struct Subsystem
		{
			enum SideResult
			{
				NotModified,
				Modified,
				NotExists
			};

			QString subsystemId;
			SideResult left;
			SideResult right;
		};

		std::vector<Subsystem> subsystems;
	};
} // namespace BuildCompLib