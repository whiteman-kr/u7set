#include "Convertor.h"
#include <QDir>
#include <QFile>
#include <iostream>
#include <stdint.h>
#include <QDateTime>
#include <QHostInfo>



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


bool Convertor::start(const QString& inputFilePath, const QString& parentFile, QTextStream& out)
{
	if (Convertor::convert(inputFilePath, parentFile, out) == false)				// working with single file
	{
		return 1;
	}

	QString dirName = inputFilePath + ".files";
	QString parent = parentFile + "/" + inputFilePath;

	if (Convertor::findFiles(dirName, parent, out) != 0)
	{
		return 1;
	}

	return 0;
}

bool Convertor::convert(const QString& inputFilePath, const QString& parentFile, QTextStream& out)
{
	// Read file
	//
	QFile input(inputFilePath);

	bool ok = input.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		std::cout << "Cannot read input file" << inputFilePath.toStdString();
		return false;
	}

	QByteArray data = input.readAll();

	input.close();

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

	QFileInfo fi(inputFilePath);
	QString inputFileName = fi.fileName();

	// Write result to file
	//

	out << "SELECT * FROM add_or_update_file(1, \'" << parentFile << "\', \'" << inputFileName <<"\', \'Update: Adding file " << inputFileName << "\', " << str << ");\n\n\n";
	//out << str <<"\n\n";
	return true;
}

int Convertor::findFiles(const QString& dirName, const QString& parentFile, QTextStream& out)
{
	QDir dir(dirName);

	QStringList listOfFiles = dir.entryList(QStringList("*.*"), QDir::Files | QDir::NoDotAndDotDot);

	foreach (QString file, listOfFiles)                         //working with files inside dir
	{
		QString fileFromDir = dirName + QDir::separator() + file;   //making a path to file in dir

		if (convert(fileFromDir, parentFile, out) == false)
		{
			return 1;
		}

		QDir checkDir(dirName + QDir::separator() + file + ".files");

		if (checkDir.exists())
		{
			findFiles(dirName + QDir::separator() + file + ".files", parentFile + "/" + file, out);
		}
	}
	return 0;
}
