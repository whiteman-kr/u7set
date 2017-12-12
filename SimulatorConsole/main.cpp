#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "../Simulator/Simulator.h"

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
//	QByteArray lmDescription = readFile("D:/Develop/u7set/u7/LogicModuleDescription/LM1_SR01.xml");
//	QByteArray simScript = readFile("D:/Develop/u7set/LmModel/Scripts/out/LM1_SR01_SIM.js");
//	QByteArray bts = readFile("D:/Develop/build/test_simulator_bts-debug/build/test_simulator_bts-000026.bts");

	// --
	//
	QTextStream textStream(stdout);

	Sim::Simulator simulator(&textStream);

	ok = simulator.load("D:/Develop/build/test_simulator_bts-debug/build");
	if (ok == false)
	{
		return 1;
	}


	//Sim::LogicModule lm(&textStream);

//	ok = lm.load(lmDescription, bts, simScript);
//	if (ok == false)
//	{
//		return 1;
//	}

//	ok = lm.powerOn(0, true);
//	if (ok == false)
//	{
//		return 2;
//	}


	getc(stdin);

//	while (lm.isFaultMode() == false || lm.isPowerOn() == true)
//	{
//		QApp

//		QThread::yieldCurrentThread();
//	};

	return 0;
}
