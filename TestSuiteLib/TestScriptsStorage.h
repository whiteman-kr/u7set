#pragma once

namespace TestSuite
{
	struct TestScript
	{
		QString fileName;
		QString script;
	};

	class TestScriptsStorage
	{
	public:
		TestScriptsStorage() = default;

		// Script access operations
		//
		//std::vector<TestScript>& scripts();
		const std::vector<TestScript>& scripts() const;
		//const TestScript& script(int index) const;

		qsizetype count() const;
		QStringList scriptList() const;

		// Script add/remove operations
		//
		void clear();
		void add(const TestScript& script);
		void setScript(std::vector<TestScript>&& scripts);	// Sets scripts by moving them from source

		bool loadFromPath(const QString& path, QString* errorMsg);

	private:
		mutable QReadWriteLock m_lock;
		std::vector<TestScript> m_scripts;
	};

}

