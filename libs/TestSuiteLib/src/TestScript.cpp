#include <TestSuiteLib/TestScript.h>

#include <QDir>

namespace TestSuite
{
	TestScript::TestScript(const QString& name, const QString& contents) :
		m_fileName(name),
		m_fileNameHash(::calcHash(name)),
		m_script(contents)
	{
	}

	Hash TestScript::fileNameHash() const
	{
		return m_fileNameHash;
	}

	const QString TestScript::fileName() const
	{
		return m_fileName;
	}

	const QString TestScript::shortFileName() const
	{
		QString shortFileName = m_fileName;
		int nPos1 = shortFileName.lastIndexOf('/');
		int nPos2 = shortFileName.lastIndexOf('\\');
		if (nPos1 != -1)
		{
			shortFileName = shortFileName.right(shortFileName.length() - nPos1 - 1);
		}
		else
		{
			if (nPos2 != -1)
			{
				shortFileName = shortFileName.right(shortFileName.length() - nPos2 - 1);
			}
		}
		return shortFileName;
	}

	void TestScript::setFileName(const QString& name)
	{
		m_fileName = name;
		m_fileNameHash = ::calcHash(name);
	}

	const QString& TestScript::script() const
	{
		return m_script;
	}

	void TestScript::setScript(const QString& contents)
	{
		m_script = contents;
	}

	bool TestScript::isGlobalScript() const
	{
		return m_fileName.contains(GlobalScriptID, Qt::CaseInsensitive);
	}
} // namespace TestSuite
