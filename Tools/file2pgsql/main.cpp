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
	if (argc != 4)
	{
		std::cout << "Parameters error, usage: files2 inputfile outputfile parentfile";
		return 1;
	}

	QString inputFileName = QString::fromLocal8Bit(argv[1]);
	QString outputFileName = QString::fromLocal8Bit(argv[2]);
	QString parentFolder = QString::fromLocal8Bit(argv[3]);

	QFile outputFile(outputFileName);							// creating file

	QString userName;                                           // setting user
	userName = qgetenv("USER");									// get the user name in Linux

	if(userName.isEmpty())
	{
		userName = qgetenv("USERNAME"); // get the name in Windows
	}

	if(userName.isEmpty())
	{
		userName = "Unknown user";
	}

	if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		std::cout << "Cannot open output file for writing";
		return 1;
	}

	QTextStream out(&outputFile);

	out << "---------------------------------------------------------------------------\n";
	out << "--\n";
	out << "-- Automaicaly generated file by file2pgsql, version 1.0\n";
	out << "-- Host: "<< QHostInfo::localHostName() <<", User: " << userName << ", Date: " << QDateTime::currentDateTime().toString("dd/MM/yyyy") << "\n";
	out << "-- FileName: " << inputFileName << "\n";
	out << "--\n";
	out << "---------------------------------------------------------------------------\n\n";

	bool result = Convertor::start(inputFileName, parentFolder, out);

	if (result == false)
	{
		std::cout << "Convert error!";
	}

	return (result == true) ? 0 : 1;
}
