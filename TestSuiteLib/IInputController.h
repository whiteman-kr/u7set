#pragma once
#include "../AppSignalLib/AppSignalParam.h"

namespace TestSuite
{
	class IInputController
	{
	public:
		virtual ~IInputController() = default;

		virtual bool waitForConnection(qint64 timeoutMs) const = 0;

		virtual bool signalExists(const QString& signalId) const = 0;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

//		virtual bool isDiscrete(const QString& signalId) const = 0;
//		virtual bool isAnalog(const QString& signalId) const = 0;
//		virtual int precision(const QString& signalId) const = 0;

		virtual AppSignalState signalState(const QString& appSignalId, bool* found) const = 0;

		virtual bool expectSignalValue(QString appSignalId, double value, qint64 timeoutMs) const = 0;
	};
}
