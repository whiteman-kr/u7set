#include "TestScriptsStorage.h"

namespace TestSuite
{
//	std::vector<TestScript>& TestScriptsStorage::scripts()
//	{
//		QReadLocker l(&m_lock);
//		return m_scripts;
//	}

	const std::vector<TestScript>& TestScriptsStorage::scripts() const
	{
		return m_scripts;
	}

	const TestScript& TestScriptsStorage::script(Hash hash) const
	{

		for (const auto& ts : m_scripts)
		{
			if(ts.hash() == hash)
			{
				return ts;
			}
		}

		static TestScript err;
		return err;
	}

//	const TestScript& TestScriptsStorage::script(int index) const
//	{
//		QReadLocker l(&m_lock);
//		if (index >= m_scripts.size())
//		{
//			static TestScript err;
//			return err;
//		}

//		return m_scripts[index];
//	}

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

	void TestScriptsStorage::setScripts(std::vector<TestScript>&& scripts)
	{
		m_scripts = std::move(scripts);
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

		return true;

	}
}
