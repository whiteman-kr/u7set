#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "../LmModel/LmModel.h"

QByteArray readFile(QString fileName)
{
	QFile file(fileName);

	bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
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
	//QByteArray lmDescription = readFile("D:\\Develop\\build\\LM1_SR01.xml");
	QByteArray lmDescription = readFile("D:/Develop/u7set/u7/LogicModuleDescription/LM1_SR01.xml");
	QByteArray tub = readFile("D:/Develop/build/test_simulator-debug/build/SUBSYSID00/LM1-SR01.tub");
	QByteArray mcb = readFile("D:/Develop/build/test_simulator-debug/build/SUBSYSID00/LM1-SR01.mcb");
	QByteArray alb = readFile("D:/Develop/build/test_simulator-debug/build/SUBSYSID00/LM1-SR01.alb");

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


	getc(stdin);

//	while (lm.isFaultMode() == false || lm.isPowerOn() == true)
//	{
//		QApp

//		QThread::yieldCurrentThread();
//	};

	return 0;
}
