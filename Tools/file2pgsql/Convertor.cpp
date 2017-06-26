#include "Convertor.h"
#include <QDir>
#include <QFile>
#include <iostream>
#include <stdint.h>

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

bool Convertor::createQueryFiles(const QString& inputFileName, const QString& parentFile, std::vector<FileQuery>& queries)
{
	if (convert(inputFileName, parentFile, queries) == false)
	{
		std::cout << "ERROR converting signle file" << std::endl;
		return false;
	}

	QString dirName = inputFileName + ".files";


	QFileInfo fi(inputFileName);

	QString parent = parentFile + "/" + fi.fileName();

	if (convertFiles(dirName, parent, queries) == false)
	{
		std::cout << "ERROR in recursive file search" << std::endl;
		return false;
	}


	return true;
}

bool Convertor::convert(const QString& inputFilePath, const QString& parentFile, std::vector<FileQuery>& queries)
{
	// Read file
	//
	QFile input(inputFilePath);

	bool ok = input.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		std::cout << "Cannot read input file" << inputFilePath.toStdString() << std::endl;
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

	FileQuery fq;
	fq.path = parentFile + "/" + inputFileName;
	fq.addQuery = "SELECT * FROM add_or_update_file(1, \'" + parentFile + "\', \'" + inputFileName + "\', \'Update: Adding file " + inputFileName + "\', " + str + ", '{}');\n\n\n";
	fq.deleteQuery = "SELECT * FROM public.delete_file_on_update(1, '" + fq.path + "', 'Delete file " + inputFileName + "');\n\n\n";

	queries.push_back(fq);

	return true;
}

bool Convertor::convertFiles(const QString& dirName, const QString& parentFile, std::vector<FileQuery>& queries)
{
	QDir dir(dirName);

	QStringList listOfFiles = dir.entryList(QStringList("*.*"), QDir::Files | QDir::NoDotAndDotDot);

	for (QString file : listOfFiles)
	{
		// working with files inside dir

		QString fileFromDir = dirName + QDir::separator() + file;   //making a path to file in dir
		if (convert(fileFromDir, parentFile, queries) == false)
		{
			return false;
		}

		QDir checkDir(dirName + QDir::separator() + file + ".files");

		if (checkDir.exists())
		{
			if (convertFiles(dirName + QDir::separator() + file + ".files", parentFile + "/" + file, queries) == false)
			{
				return false;
			}
		}
	}
	return true;
}

bool Convertor::writeToFile(const QString& outputFileName, const QString& header, const std::vector<FileQuery>& queries, FileQueryType queryType)
{
	QFile outputFile(outputFileName);											// creating file
	if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		std::cout << "Cannot open output file for writing" << std::endl;
		return false;
	}
	QTextStream out(&outputFile);

	out << header;

	for (const FileQuery& fq : queries)
	{
		switch (queryType)
		{
		case FileQueryType::Add:
			out << fq.addQuery;
			break;

		case FileQueryType::Delete:
			out << fq.deleteQuery;
			break;

		default:
			Q_ASSERT(false);
			return false;
		}
	}

	return true;
}
