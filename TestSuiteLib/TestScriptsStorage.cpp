#include "TestScriptsStorage.h"

TestScriptsStorage::TestScriptsStorage()
{

}

void TestScriptsStorage::move(TestScriptsStorage& That)
{
	m_scripts = std::move(That.m_scripts);
}

void TestScriptsStorage::add(const QString& name, QByteArray& data)
{
	m_scripts[name] = std::move(data);
}

void TestScriptsStorage::clear()
{
	m_scripts.clear();
}

qsizetype TestScriptsStorage::testScriptCount() const
{
	return m_scripts.size();
}

QStringList TestScriptsStorage::testScriptList() const
{
	QStringList result;
	for (const auto& it : m_scripts)
	{
		result.push_back(it.first);
	}
	return result;
}

const QByteArray& TestScriptsStorage::testScript(const QString& fileName) const
{
	auto it = m_scripts.find(fileName);
	if (it == m_scripts.end())
	{
		Q_ASSERT(false);
		static QByteArray err;
		return err;
	}

	return it->second;
}
