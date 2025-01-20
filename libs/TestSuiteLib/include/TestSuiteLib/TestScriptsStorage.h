#pragma once

#include "IScriptProvider.h"
#include "TestScript.h"

#include <QStringList>

#include <vector>


namespace TestSuite
{
	class TestScriptsStorage : public IScriptProvider
	{
	public:
		// IScriptProvider interface implementation
		//
		[[nodiscard]] QStringList getScriptFileNames() const override;
		[[nodiscard]] std::vector<TestScript> getScripts() const override;

		[[nodiscard]] std::optional<TestScript> getGloablScript() const override;
		[[nodiscard]] std::optional<TestScript> getScriptByFileName(const QString& fileName) const override;

		// End of IScriptProvider interface implementation

	public:
		// Script access operations
		//
		[[nodiscard]] bool hasScript(Hash hash) const;

		[[nodiscard]] const TestScript& script(Hash hash) const;
		[[nodiscard]] const TestScript& script(int index) const;

		[[nodiscard]] qsizetype count() const;

		// Script add/remove operations
		//
		void clear();
		void add(TestScript script);

		void setScripts(const std::vector<TestScript>& scripts); // Sets scripts by moving them from source

		bool loadFromPath(const QString& path, QString* errorMsg);

	private:
		bool loadScriptsFromPath(const QString& path, QString* errorMsg);

	private:
		std::vector<TestScript> m_scripts;
	};

} // namespace TestSuite
