#ifndef TESTSCRIPTSSTORAGE_H
#define TESTSCRIPTSSTORAGE_H


class TestScriptsStorage
{
public:
	TestScriptsStorage();

	void move(TestScriptsStorage& That);

	void add(const QString& name, QByteArray& data);
	void clear();

	qsizetype testScriptCount() const;
	QStringList testScriptList() const;
	const QByteArray& testScript(const QString& fileName) const;

private:
	std::map<QString, QByteArray> m_scripts;

};

#endif // TESTSCRIPTSSTORAGE_H
