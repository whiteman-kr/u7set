#include <TestSuiteLib/TestScriptSelection.h>

namespace TestSuite
{
	TestScriptSelection::TestScriptSelection(const QString& testMasks) :
		m_testMasks(testMasks.split(';', Qt::SkipEmptyParts))
	{
	}

	const QStringList& TestScriptSelection::testMasks() const
	{
		return m_testMasks;
	}

	bool TestScriptSelection::isEmpty() const
	{
		for (auto it : m_testFunctions)
		{
			if (it.second.isEmpty() == false)
			{
				return false;
			}
		}
		return true;
	}

	QStringList TestScriptSelection::selectedFiles() const
	{
		QStringList result;
		for (auto& it : m_testFunctions)
		{
			result.push_back(it.first);
		}
		return result;
	}

	const QStringList& TestScriptSelection::selectedFunctions(const QString& scriptName) const
	{
		const auto& scriptTestFunctionsListIt = m_testFunctions.find(scriptName);
		if (scriptTestFunctionsListIt == m_testFunctions.end())
		{
			static QStringList empty;
			return empty;
		}
		return scriptTestFunctionsListIt->second;
	}

	void TestScriptSelection::setSelectedFunctions(const QString& scriptName, const QStringList& functions)
	{
		m_testFunctions[scriptName] = functions;
	}
} // namespace TestSuite