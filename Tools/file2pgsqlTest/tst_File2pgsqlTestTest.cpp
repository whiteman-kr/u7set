#include <QString>
#include <QtTest>
#include <QTextStream>
#include "../file2pgsql/Convertor.h"

class FindFileTest : public QObject
{
    Q_OBJECT

public:
	FindFileTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
	void findFiles_data();
	void findFiles();
};

class ConvertFileTest : public QObject
{
	Q_OBJECT

public:
	ConvertFileTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void convertFile_data();
	void convertFile();
};

class StartProgramTest : public QObject
{
	Q_OBJECT

public:
	StartProgramTest();

private Q_SLOTS:
	void startProgramTests_data();
	void startProgramTests();
};

class WriteFileTest : public QObject
{
	Q_OBJECT

public:
	WriteFileTest();

private Q_SLOTS:
	void writeFile_data();
	void writeFile();
};

WriteFileTest::WriteFileTest()
{
}

void WriteFileTest::writeFile_data()
{
	QTest::addColumn<QString>("testFileName");
	QTest::addColumn<QString>("query");
	QTest::addColumn<bool>("out");
	QTest::newRow("writeFileTest") << "writeFileTestOutput" << "Awesome query" << true;
}

void WriteFileTest::writeFile()
{
	QFETCH (QString, testFileName);
	QFETCH (QString, query);
	QFETCH (bool, out);

	QCOMPARE (Convertor::writeToFile(testFileName, query), out);
}

StartProgramTest::StartProgramTest()
{
}

void StartProgramTest::startProgramTests_data()
{
	QTest::addColumn<QString>("inputFilePath");
	QTest::addColumn<QString>("parentFile");
	QTest::addColumn<QString>("result");

	QTest::newRow("StartTest") << "../file2pgsqlTest/TestFiles/TestStartFunction.txt" << "root" << "SELECT * FROM add_or_update_file(1, 'root', 'TestStartFunction.txt', 'Update: Adding file TestStartFunction.txt', E'\\\\x31', '{}');\n\n\n";
	QTest::newRow("StartTestError") << "../file2pgsqlTest/TestFiles/TestStartFunction1.txt" << "root" << "ERROR";

}

void StartProgramTest::startProgramTests()
{
	QFETCH (QString, inputFilePath);
	QFETCH (QString, parentFile);
	QFETCH (QString, result);

	QCOMPARE(Convertor::start(inputFilePath, parentFile), result);
}

ConvertFileTest::ConvertFileTest()
{
}

void ConvertFileTest::initTestCase()
{
}

void ConvertFileTest::cleanupTestCase()
{
}

void ConvertFileTest::convertFile_data()
{
	QTest::addColumn<QString>("path");
	QTest::addColumn<QString>("parentFile");
	QTest::addColumn<QString>("query");
	QTest::addColumn<QString>("result");

	QTest::newRow("TestFile1") << "../file2pgsqlTest/TestFiles/Test0/test3.txt" << "root" << "" << "SELECT * FROM add_or_update_file(1, 'root', 'test3.txt', 'Update: Adding file test3.txt', E'\\\\x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff', '{}');\n\n\n";
}

void ConvertFileTest::convertFile()
{
	QFETCH(QString, path);
	QFETCH(QString, parentFile);
	QFETCH(QString, query);
	QFETCH(QString, result);

	QCOMPARE(Convertor::convert(path, parentFile, query), result);
}

FindFileTest::FindFileTest()
{
}

void FindFileTest::initTestCase()
{
}

void FindFileTest::cleanupTestCase()
{
}

void FindFileTest::findFiles_data()
{
	QTest::addColumn<QString>("dirName");
	QTest::addColumn<QString>("parentFile");
	QTest::addColumn<QString>("query");
	QTest::addColumn<QString>("filePathes");
    QTest::addColumn<QString>("output");

	QTest::newRow("OneDirSearch") << "../file2pgsqlTest/TestFiles/Test0" << "\"root\"" << "" << "" << "../file2pgsqlTest/TestFiles/Test0/test3.txt//";
	QTest::newRow("MultiDirSearch") << "../file2pgsqlTest/TestFiles/Test1" << "\"root\"" << "" << "" << "../file2pgsqlTest/TestFiles/Test1/test2.txt//../file2pgsqlTest/TestFiles/Test1/test2.txt.files/test3.txt//../file2pgsqlTest/TestFiles/Test1/test2.txt.files/test3.txt.files/test4.txt//";
	QTest::newRow("CheckingUncheckedDirs") << "../file2pgsqlTest/TestFiles/Test2" << "\"root\"" << "" << "" << "../file2pgsqlTest/TestFiles/Test2/test.txt//../file2pgsqlTest/TestFiles/Test2/test.txt.files/dfsfdsfsdfdsfsdf.txt//../file2pgsqlTest/TestFiles/Test2/test1.txt//../file2pgsqlTest/TestFiles/Test2/test1.txt.files/dfsfsdfsdf.txt//";
	QTest::newRow("MultiFileTest") << "../file2pgsqlTest/TestFiles/Test3" << "\"root\"" << "" << "" << "../file2pgsqlTest/TestFiles/Test3/dfsdfdsf.txt//../file2pgsqlTest/TestFiles/Test3/logic.preview.png//../file2pgsqlTest/TestFiles/Test3/sdfsdfdsfs.txt//";
}

void FindFileTest::findFiles()
{
	QFETCH(QString, dirName);
	QFETCH(QString, parentFile);
	QFETCH(QString, query);
	QFETCH(QString, filePathes);
    QFETCH(QString, output);

	QCOMPARE(Convertor::findFiles(dirName, parentFile, query, filePathes), output);
}

int main(int argc, char *argv[])
{
	FindFileTest testFind;
	ConvertFileTest testConv;
	StartProgramTest startTest;
	WriteFileTest testFile;
	if (QTest::qExec(&testFind, argc, argv) != 0)
	{
		return 1;
	}

	if (QTest::qExec(&testConv, argc, argv) != 0)
	{
		return 1;
	}

	if (QTest::qExec(&startTest, argc, argv) != 0)
	{
		return 1;
	}

	if (QTest::qExec(&testFile, argc, argv) != 0)
	{
		return 1;
	}

	return 0;
}

#include "tst_File2pgsqlTestTest.moc"
