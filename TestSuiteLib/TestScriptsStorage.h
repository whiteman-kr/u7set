#ifndef TESTSCRIPTSSTORAGE_H
#define TESTSCRIPTSSTORAGE_H


struct TestScript
{
	QString fileName;
	QByteArray script;
};


class TestScriptsStorage
{
public:
	TestScriptsStorage();

	// Script access operations
	//
	std::vector<TestScript>& scripts();
	const std::vector<TestScript>& scripts() const;
	const TestScript& script(int index) const;

	qsizetype count() const;
	QStringList scriptList() const;

	// Script add/remove operations
	//
	void clear();
	void add(const TestScript &script);
	void move(std::vector<TestScript>& scripts);	// Sets scripts by moving them from source

	bool loadFromPath(const QString& path, QString *errorMsg);

	bool isLoadedFromFiles() const;

private:
	mutable QReadWriteLock m_lock;

	std::vector<TestScript> m_scripts;

	bool m_loadedFromFiles = false;

};

#endif // TESTSCRIPTSSTORAGE_H
