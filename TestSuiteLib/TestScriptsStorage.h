#pragma once

#include "../CommonLib/Hash.h"

namespace TestSuite
{
	class TestScript
	{
	public:
		TestScript() = default;
		TestScript(const QString& name, const QString& contents):
			m_fileName(name), m_script(contents), m_hash(::calcHash(name))
		{
		}
		Hash hash() const
		{
			return m_hash;
		}
		const QString fileName() const
		{
			return m_fileName;
		}
		void setFileName(const QString& name)
		{
			m_fileName = name;
			m_hash = ::calcHash(name);
		}
		const QString& script() const
		{
			return m_script;
		}
		void setScript(const QString& contents)
		{
			m_script = contents;
		}

	private:
		Hash m_hash = UNDEFINED_HASH;
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
		const TestScript& script(Hash hash) const;

		qsizetype count() const;
		QStringList scriptList() const;

		// Script add/remove operations
		//
		void clear();
		void add(const TestScript& script);
		void setScripts(std::vector<TestScript>&& scripts);	// Sets scripts by moving them from source

		bool loadFromPath(const QString& path, QString* errorMsg);

	private:
		std::vector<TestScript> m_scripts;
	};

}

