#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <stdint.h>

// Convert any file (binaary or text) to the string which can be used to send file via
// SQL request to PostgreSql
//
// Usage:
//	file2pgsql inputfile outputfile
//


const static char* rawhex = {"000102030405060708090a0b0c0d0e0f"
						"101112131415161718191a1b1c1d1e1f"
						"202122232425262728292a2b2c2d2e2f"
						"303132333435363738393a3b3c3d3e3f"
						"404142434445464748494a4b4c4d4e4f"
						"505152535455565758595a5b5c5d5e5f"
						"606162636465666768696a6b6c6d6e6f"
						"707172737475767778797a7b7c7d7e7f"
						"808182838485868788898a8b8c8d8e8f"
						"909192939495969798999a9b9c9d9e9f"
						"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
						"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
						"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
						"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
						"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
						"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc != 3)
	{
		std::cout << "Parameters error, usage: file2pgsql inputfile outputfile";
		return 1;
	}

	QString inputFileName = QString::fromLocal8Bit(argv[1]);
	QString outputFileName = QString::fromLocal8Bit(argv[2]);

	// Read file
	//
	QFile inputFile(inputFileName);

	bool ok = inputFile.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		std::cout << "Cannot read input file";
		return 2;
	}

	QByteArray data = inputFile.readAll();

	inputFile.close();

	// Convert to PostgreSQL string
	//
	QString str;
	str.reserve(data.size() * 2 + 256);
	str.append("E'\\\\x");

	QString hex(rawhex);
	const QChar* hexptr = hex.data();

	int fileSize = data.size();
	const char* dataptr = data.constData();

	for (int i = 0; i < fileSize; i++)
	{
		unsigned int asbyte = static_cast<uint8_t>(*dataptr) & 0xFF;
		str.append(hexptr + asbyte*2, 2);

		dataptr ++;
	}

	str.append("'");

	// Write result to file
	//
	QFile outputFile(outputFileName);

	if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		std::cout << "Cannot open output file for writing";
		return 3;
	}

	QTextStream out(&outputFile);
	out << str;

	outputFile.close();

	return 0;
}
