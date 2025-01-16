#include <TestSuiteLib/TestScriptsStorage.h>

#include <QDir>

namespace TestSuite
{
	QStringList TestScriptsStorage::getScriptFileNames() const
	{
		QStringList result;
		result.reserve(m_scripts.size());
		std::transform(m_scripts.begin(),
					   m_scripts.end(),
					   std::back_inserter(result),
					   [](const TestScript& ts)
					   {
						   return ts.fileName();
					   });
		return result;
	}

	std::vector<TestScript> TestScriptsStorage::getScripts() const
	{
		return m_scripts;
	}

	std::optional<TestScript> TestScriptsStorage::getGloablScript() const
	{
		auto it = std::find_if(m_scripts.begin(),
							   m_scripts.end(),
							   [](const TestScript& ts)
							   {
								   return ts.isGlobalScript();
							   });

		if (it == m_scripts.end())
		{
			return std::nullopt;
		}

		return *it;
	}
	std::optional<TestScript> TestScriptsStorage::getScriptByFileName(const QString& fileName) const
	{
		auto it = std::find_if(m_scripts.begin(),
							   m_scripts.end(),
							   [fileName](const TestScript& ts)
							   {
								   return ts.fileName() == fileName;
							   });

		if (it == m_scripts.end())
		{
			return std::nullopt;
		}

		return *it;
	}

	bool TestScriptsStorage::hasScript(Hash hash) const
	{
		auto it = std::find_if(m_scripts.begin(),
							   m_scripts.end(),
							   [hash](const TestScript& ts)
							   {
								   return ts.fileNameHash() == hash;
							   });

		return it != m_scripts.end();
	}

	const TestScript& TestScriptsStorage::script(Hash hash) const
	{
		auto it = std::find_if(m_scripts.begin(),
							   m_scripts.end(),
							   [hash](const TestScript& ts)
							   {
								   return ts.fileNameHash() == hash;
							   });

		if (it == m_scripts.end())
		{
			static TestScript err;
			return err;
		}

		return *it;
	}

	const TestScript& TestScriptsStorage::script(int index) const
	{
		if (index < 0 || index >= std::ssize(m_scripts))
		{
			static TestScript err;
			return err;
		}

		return m_scripts[index];
	}

	qsizetype TestScriptsStorage::count() const
	{
		return std::ssize(m_scripts);
	}

	void TestScriptsStorage::clear()
	{
		m_scripts.clear();
		return;
	}

	void TestScriptsStorage::add(TestScript script)
	{
		m_scripts.push_back(std::move(script));
		return;
	}

	void TestScriptsStorage::setScripts(const std::vector<TestScript>& scripts)
	{
		m_scripts = scripts;
	}

	bool TestScriptsStorage::loadFromPath(const QString& path, QString* errorMsg)
	{
		QDir dir(path);

		if (dir.exists() == false)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QObject::tr("Error: Scripts path \"%1\" does not exist!").arg(path);
			}

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
				if (errorMsg != nullptr)
				{
					*errorMsg = QObject::tr("Error: Can't open file \"%1\" for reading!").arg(ts.fileName());
				}

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
} // namespace TestSuite
