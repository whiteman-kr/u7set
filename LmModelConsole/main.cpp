#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "../LmModel/LmModel.h"

QByteArray readFile(QString fileName)
{
	QFile file(fileName);

	bool ok = file.open(QIODevice::ReadOnly);
	if (ok == false)
	{
		return QByteArray();
	}

	return file.readAll();
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	bool ok = true;

	// Loading files
	//
	QByteArray lmDescription = readFile("D:/Develop/build/LogicModule0000.xml");
	QByteArray tub = readFile("D:/Develop/build/LM-1.tub");
	QByteArray mcb = readFile("D:/Develop/build/LM-1.mcb");
	QByteArray alb = readFile("D:/Develop/build/LM-1.alb");

	// --
	//
	QTextStream textStream(stdout);
	LmModel::LogicModule lm(&textStream);

	ok = lm.load(lmDescription, tub, mcb, alb);
	if (ok == false)
	{
		return 1;
	}

	ok = lm.powerOn(0, true);
	if (ok == false)
	{
		return 2;
	}

	while (lm.isFaultMode() == false || lm.isPowerOn() == true)
	{
		QThread::yieldCurrentThread();
	};

	return 0;
}
