#include <QCoreApplication>
#include "Metaparser.h"

int main(int argc, char *argv[])
{
	// Checking input parameters
	//
	if (argc != 3)
	{
		qDebug() << "Parameters error, usage: metaparser inputfile outputfile";
		return 1;
	}

	// If parameters are OK - set program values
	//
	QString inputFileName = QString::fromLocal8Bit(argv[1]);
	QString outputFileName = QString::fromLocal8Bit(argv[2]);

	// Create parse class, set Input and Output files, then call "searchBlocks" function
	//
	Metaparser parse;

	int errCode = 0;

	parse.setInputFile(inputFileName);
	parse.setOutputFile(outputFileName);

	errCode = parse.searchBlocks();

	if (errCode != 0)
	{
		qDebug() << "Exit code: " << errCode;
	}

	errCode = parse.writeToHtml();

	if (errCode != 0)
	{
		qDebug() << "Exit code: " << errCode;
	}

	return 0;
}

