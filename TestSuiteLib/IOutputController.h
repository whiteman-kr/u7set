#pragma once

namespace TestSuite
{
	class IOutputController
	{
	public:
		virtual ~IOutputController() = default;

		virtual bool waitForConnection(qint64 timeoutMs) const = 0;

		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) = 0;
	};
}

