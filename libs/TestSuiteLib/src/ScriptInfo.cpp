#include <TestSuiteLib/ScriptInfo.h>

namespace TestSuite
{
	ScriptInfo::ScriptInfo(const QString& fileName) :
		fileName{fileName}
	{
	}

	bool ScriptInfo::empty() const
	{
		return testsList.empty(); // No functions are in this script. Possibly, it is not evaluated
	}

	qsizetype ScriptInfo::testsCount() const
	{
		return testsList.size();
	}

	QString ScriptInfo::testCaption(const QString& function, bool* found /*= nullptr*/) const
	{
		auto it = testsCaptions.find(function);
		bool foundValue = it != testsCaptions.end();

		if (found != nullptr)
		{
			*found = foundValue;
		}

		return foundValue ? it->second : function;
	}

	bool ScriptInfo::checkScriptTags(const QStringList& tagsProperty) const
	{
		return scriptTags.isEmpty() || std::any_of(scriptTags.begin(),
												   scriptTags.end(),
												   [&tagsProperty](const QString& tag)
												   {
													   return tagsProperty.contains(tag);
												   });
	}
} // namespace TestSuite