#pragma once

#include <ClientLib/ITestObserver.h>
#include <CommonLib/expected.hpp>

#include <memory>


namespace TestSuite
{
	class IInputController
	{
	public:
		virtual ~IInputController() = default;

		virtual bool init(qint64 timeoutMs) = 0;
		virtual bool shutdown() = 0;

		[[nodiscard]] virtual bool signalExists(const QString& signalId) const = 0;

		[[nodiscard]] virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;
		[[nodiscard]] virtual AppSignalState signalState(const QString& appSignalId, bool* found) const = 0;

		[[nodiscard]] virtual bool expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance = 0) const = 0;

		//! \brief Create a test observer if supported by the input controller.
		[[nodiscard]] virtual tl::expected<std::unique_ptr<ITestObserver>, QString> createTestObserver() = 0;
	};


	class InputControllerStub : public IInputController
	{
	public:
		bool init(qint64 /*timeoutMs*/) override { return true; }
		bool shutdown() override { return true; }

		bool signalExists(const QString& /*signalId*/) const override { return false; }

		AppSignalParam signalParam(const QString& /*appSignalId*/, bool* found) const override
		{
			if (found != nullptr)
			{
				*found = false;
			}
			return {};
		}

		AppSignalState signalState(const QString& /*appSignalId*/, bool* found) const override
		{
			if (found != nullptr)
			{
				*found = false;
			}
			return {};
		}

		bool expectSignalValue(QString /*appSignalId*/, qint64 /*timeoutMs*/, double /*value*/, double /*tolerance*/ = 0) const override
		{
			return false;
		}

		tl::expected<std::unique_ptr<ITestObserver>, QString> createTestObserver() override
		{
			return tl::make_unexpected("Not implemented");
		}
	};
} // namespace TestSuite
