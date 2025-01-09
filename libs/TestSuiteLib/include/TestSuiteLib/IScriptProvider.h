#pragma once
#include "TestScript.h"

#include <optional>
#include <vector>


namespace TestSuite
{
	class IScriptProvider
	{
	public:
		virtual ~IScriptProvider() = default;

		[[nodiscard]] virtual QStringList getScriptFileNames() const = 0;
		[[nodiscard]] virtual std::vector<TestScript> getScripts() const = 0;

		[[nodiscard]] virtual std::optional<TestScript> getGloablScript() const = 0;
		[[nodiscard]] virtual std::optional<TestScript> getScriptByFileName(const QString& fileName) const = 0;
	};


	class IScriptProviderStub : public IScriptProvider
	{
	public:
		[[nodiscard]] QStringList getScriptFileNames() const override { return {}; }
		[[nodiscard]] std::vector<TestScript> getScripts() const override { return {}; }

		[[nodiscard]] std::optional<TestScript> getGloablScript() const override { return {}; }
		[[nodiscard]] std::optional<TestScript> getScriptByFileName(const QString& /*fileName*/) const override { return {}; }
	};
} // namespace TestSuite