#include <QCoreApplication>
#include "ArchUtils.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QStringList args;

	for(int i = 1; i < argc; i++)
	{
		args.append(QString(argv[i]).toLower());
	}

	if (args.count() < 2)
	{
		std::cout << "Use u7arch.exe -d file name [-lt | -st | -pt]";
		return 1;
	}

	ArchUtils utils(QDir::currentPath());

	if (args[0] == "-d")
	{
		utils.dump(args[1], args.contains("-lt"), args.contains("-st"), args.contains("-pt"));
	}

	return 1;
}
