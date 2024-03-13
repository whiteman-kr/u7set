#pragma once

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

		virtual bool expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance = 0) const = 0;
	};

	class InputControllerStub : public IInputController
	{
		virtual bool waitForConnection(qint64 /*timeoutMs*/) const override {return false;}
		virtual bool signalExists(const QString& /*signalId*/) const override {return false;}
		virtual AppSignalParam signalParam(const QString& /*appSignalId*/, bool* /*found*/) const override {return {};}
		//		virtual bool isDiscrete(const QString& signalId) const = 0;
		//		virtual bool isAnalog(const QString& signalId) const = 0;
		//		virtual int precision(const QString& signalId) const = 0;
		virtual AppSignalState signalState(const QString& /*appSignalId*/, bool* /*found*/) const override {return {};}
		virtual bool expectSignalValue(QString /*appSignalId*/, qint64 /*timeoutMs*/, double /*value*/, double /*tolerance*/ = 0) const override { return false; }
	};
}
