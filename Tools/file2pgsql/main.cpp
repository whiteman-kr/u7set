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
	if (argc != 4) {
		std::cout << "Parameters error, usage: files2 inputfile outputfile parentfile";
		return 1;
	}

	QString inputFileName = QString::fromLocal8Bit(argv[1]);
	QString outputFileName = QString::fromLocal8Bit(argv[2]);
	QString parentFolder = QString::fromLocal8Bit(argv[3]);

	QString userName;                                           // setting user
	userName = qgetenv("USER");									// get the user name in Linux

	if(userName.isEmpty()) {
		userName = qgetenv("USERNAME"); // get the name in Windows
	}

	if(userName.isEmpty()) {
		userName = "Unknown user";
	}

	QString output="";

	output+= "---------------------------------------------------------------------------\n";
	output+= "--\n";
	output+= "-- Automaicaly generated file by file2pgsql, version 1.0\n";
	output+= ("-- Host: " + QHostInfo::localHostName() + ", User: " + userName + ", Date: " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:s") + "\n");
	output+= ("-- FileName: " + inputFileName + "\n");
	output+= "--\n";
	output+= "---------------------------------------------------------------------------\n\n";

	QString query = Convertor::start(inputFileName, parentFolder);

	if (query == "ERROR") {
		return 1;
	}

	if (Convertor::writeToFile(outputFileName, output+query)==false) {
		return 1;
	}

	return 0;
}
