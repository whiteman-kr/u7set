#pragma once

namespace TestSuite
{
	class IOutputController
	{
	public:
		virtual ~IOutputController() = default;

		virtual bool waitForConnection(qint64 timeoutMs) const = 0;
	};
}

