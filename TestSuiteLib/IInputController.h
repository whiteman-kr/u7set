#pragma once

namespace TestSuite
{
	class IInputController
	{
	public:
		virtual ~IInputController() = default;

		virtual bool waitForConnection(qint64 timeoutMs) const = 0;
	};
}
