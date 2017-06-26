#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <iostream>
#include <QDateTime>
#include <QHostInfo>
#include "Convertor.h"


int main(int argc, char *argv[])
{
	if (argc < 4 || argc > 5)
	{
		std::cout << "Parameters error, usage: files2 inputfile outputfile parentfile [previousfile]";
		return 1;
	}

	QString inputFileName = QString::fromLocal8Bit(argv[1]);
	QString outputFileName = QString::fromLocal8Bit(argv[2]);
	QString parentFolder = QString::fromLocal8Bit(argv[3]);

	QString previousInputFileName;
	if (argc == 5)
	{
		previousInputFileName = QString::fromLocal8Bit(argv[4]);
	}

	QString userName;                                           // setting user
	userName = qgetenv("USER");									// get the user name in Linux

	if(userName.isEmpty()) {
		userName = qgetenv("USERNAME"); // get the name in Windows
	}

	if(userName.isEmpty()) {
		userName = "Unknown user";
	}

	QString header;

	header += "---------------------------------------------------------------------------\n";
	header += "--\n";
	header += "-- Automaicaly generated file by file2pgsql, version 1.0\n";
	header += ("-- Host: " + QHostInfo::localHostName() + ", User: " + userName + ", Date: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") + "\n");
	header += ("-- FileName: " + inputFileName + "\n");
	header += "--\n";
	header += "---------------------------------------------------------------------------\n\n";

	Convertor conv;

	std::vector<FileQuery> queries;

	if (conv.createQueryFiles(inputFileName, parentFolder, queries) == false)
	{
		return 1;
	}

	std::cout << "Total files: " << queries.size() << "\r\n";

	if (conv.writeToFile(outputFileName, header, queries, FileQueryType::Add) == false)
	{
		return 1;
	}

	if (previousInputFileName.isEmpty() == false)
	{
		std::vector<FileQuery> previousQueries;

		if (conv.createQueryFiles(previousInputFileName, parentFolder, previousQueries) == false)
		{
			return 1;
		}

		if (conv.writeToFile(outputFileName + ".prev", header, previousQueries, FileQueryType::Add) == false)
		{
			return 1;
		}

		// Deleted

		std::vector<FileQuery> deletedQueries;

		for (const FileQuery& fqPrevious : previousQueries)
		{
			bool found = false;

			for (const FileQuery& fq : queries)
			{
				if (fqPrevious.path == fq.path)
				{
					found = true;
					break;
				}
			}

			if (found == false)
			{
				deletedQueries.push_back(fqPrevious);
			}
		}

		std::cout << "Deleted files: " << deletedQueries.size() << "\r\n";

		if (conv.writeToFile(outputFileName + ".deleted", header, deletedQueries, FileQueryType::Delete) == false)
		{
			return 1;
		}

		// Added/Modified

		std::vector<FileQuery> addedModifiedQueries;

		for (const FileQuery& fq : queries)
		{
			bool modified = false;
			bool added = true;

			for (const FileQuery& fqPrevious : previousQueries)
			{
				if (fqPrevious.path == fq.path)
				{
					added = false;
				}

				if (fqPrevious.path == fq.path && fqPrevious.addQuery != fq.addQuery)
				{
					modified = true;
				}
			}

			if (added == true || modified == true)
			{
				addedModifiedQueries.push_back(fq);
			}
		}

		std::cout << "Added/Modified files: " << addedModifiedQueries.size() << "\r\n";

		if (conv.writeToFile(outputFileName + ".modified", header, addedModifiedQueries, FileQueryType::Add) == false)
		{
			return 1;
		}
	}

	return 0;
}
