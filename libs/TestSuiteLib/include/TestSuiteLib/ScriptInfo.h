#pragma once

#include <map>

namespace TestSuite
{
	struct ScriptInfo
	{
		QString fileName;
		QString scriptCaption;
		QStringList scriptTags;

		QStringList testsList;
		std::map<QString, QString> testsCaptions; // Key is function name, value is function caption

		QString globalAllowFunction;              // Name of global allow function (allowGlobal())
		QString allowFunction;                    // Name of local allow function (allow<SCRIPT_FILE_NAME>())

		explicit ScriptInfo(const QString& fileName);

		[[nodiscard]] bool empty() const;
		[[nodiscard]] qsizetype testsCount() const;
		[[nodiscard]] QString testCaption(const QString& function, bool* found = nullptr) const;
		[[nodiscard]] bool checkScriptTags(const QStringList& tagsProperty) const;
	};
} // namespace TestSuite