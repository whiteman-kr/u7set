#pragma once

#include "../CommonLib/Hash.h"

namespace TestSuite
{
	class TestScriptSelection
	{
	public:
		TestScriptSelection() = default;
		explicit TestScriptSelection(const QString& testMasks);

		// Tests masks operations
		//
		const QStringList& testMasks() const;

		// Selected files and tests operations
		//
		bool isEmpty() const;	// returns true if list of selected functions is empty
		QStringList selectedFiles() const;
		
		const QStringList& selectedFunctions(const QString& scriptName) const;
		void setSelectedFunctions(const QString& scriptName, const QStringList& functons);

	private:
		QStringList m_testMasks;

		std::map<QString, QStringList> m_testFunctions;	// Key is script filename, value is list of functions
	};

	class TestScript
	{
	public:
		TestScript() = default;
		TestScript(const QString& name, const QString& contents):
			m_fileName(name), m_fileNameHash(::calcHash(name)), m_script(contents)
		{
		}
		Hash fileNameHash() const
		{
			return m_fileNameHash;
		}
		const QString fileName() const
		{
			return m_fileName;
		}
		const QString shortFileName() const
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
		void setFileName(const QString& name)
		{
			m_fileName = name;
			m_fileNameHash = ::calcHash(name);
		}
		const QString& script() const
		{
			return m_script;
		}
		void setScript(const QString& contents)
		{
			m_script = contents;
		}
		bool isGlobalScript() const
		{
			return m_fileName.contains(GlobalScriptID, Qt::CaseInsensitive);
		}

		static inline QString GlobalScriptID = "GlobalScript";

	private:
		Hash m_fileNameHash = UNDEFINED_HASH;
		QString m_fileName;
		QString m_script;
	};

	class TestScriptsStorage
	{
	public:
		TestScriptsStorage() = default;

		// Script access operations
		//
		//std::vector<TestScript>& scripts();
		const std::vector<TestScript>& scripts() const;

		const TestScript* globalScript() const;

		bool hasScript(Hash hash) const;

		const TestScript& script(Hash hash) const;
		const TestScript& script(int index) const;

		qsizetype count() const;
		QStringList scriptList() const;

		// Script add/remove operations
		//
		void clear();
		void add(const TestScript& script);
		void setScripts(const std::vector<TestScript>& scripts);	// Sets scripts by moving them from source

		bool loadFromPath(const QString& path, QString* errorMsg);

	private:
		bool loadScriptsFromPath(const QString& path, QString* errorMsg);


	private:
		std::vector<TestScript> m_scripts;
	};

}

