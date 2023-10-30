#include "TestScriptsStorage.h"

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

	const std::vector<TestScript>& TestScriptsStorage::scripts() const
	{
		return m_scripts;
	}

	const TestScript* TestScriptsStorage::globalScript() const
	{
		for (const auto& ts : m_scripts)
		{
			if (ts.isGlobalScript() == true)
			{
				return &ts;
			}
		}
		return nullptr;
	}

	bool TestScriptsStorage::hasScript(Hash hash) const
	{
		for (const auto& ts : m_scripts)
		{
			if(ts.fileNameHash() == hash)
			{
				return true;
			}
		}

		return false;
	}

	const TestScript& TestScriptsStorage::script(Hash hash) const
	{

		for (const auto& ts : m_scripts)
		{
			if(ts.fileNameHash() == hash)
			{
				return ts;
			}
		}

		static TestScript err;
		return err;
	}

	const TestScript& TestScriptsStorage::script(int index) const
	{
		if (index >= m_scripts.size())
		{
			static TestScript err;
			return err;
		}

		return m_scripts[index];
	}

	qsizetype TestScriptsStorage::count() const
	{
		return m_scripts.size();
	}

	QStringList TestScriptsStorage::scriptList() const
	{
		QStringList result;
		result.reserve(m_scripts.size());

		for (const TestScript& ts : m_scripts)
		{
			result.push_back(ts.fileName());
		}

		return result;
	}

	void TestScriptsStorage::clear()
	{
		m_scripts.clear();
		return;
	}

	void TestScriptsStorage::add(const TestScript& script)
	{
		m_scripts.push_back(script);
		return;
	}

	void TestScriptsStorage::setScripts(const std::vector<TestScript>& scripts)
	{
		m_scripts = scripts;
	}

	bool TestScriptsStorage::loadFromPath(const QString& path, QString* errorMsg)
	{
		if (errorMsg == nullptr)
		{
			Q_ASSERT(errorMsg);
			return false;
		}

		QDir dir(path);
		if (dir.exists() == false)
		{
			*errorMsg = QObject::tr("Error: Scripts path \"%1\" does not exist!").arg(path);
			return false;
		}

		clear();

		return loadScriptsFromPath(path, errorMsg);
	}

	bool TestScriptsStorage::loadScriptsFromPath(const QString& path, QString* errorMsg)
	{
		QDir dir(path);

		QStringList files = dir.entryList(QStringList() << "*.js", QDir::Files, QDir::Name);
		for (const QString& file : files)
		{
			TestScript ts;
			ts.setFileName(path + QDir::separator() + file);

			QFile f(ts.fileName());
			if (f.open(QFile::ReadOnly) == false)
			{
				*errorMsg = QObject::tr("Error: Can't open file \"%1\" for reading!").arg(ts.fileName());
				return false;
			}
			ts.setScript(f.readAll());

			add(ts);
		}

		QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		for (const QString& sd : subdirs)
		{	
			loadScriptsFromPath(path + QDir::separator() + sd, errorMsg);
		}

		return true;
	}
}
