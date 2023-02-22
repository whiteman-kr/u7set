#include <QCoreApplication>
#include "ArchUtils.h"

int main(int argc, char *argv[])
{
	//QCoreApplication a(argc, argv);

	QStringList args;

	for(int i = 1; i < argc; i++)
	{
		args.append(QString(argv[i]).toLower());
	}

	if (args.count() < 2)
	{
		std::cout << "\n";
		std::cout << "Use u7arch.exe -d achive_file_name [-lt | -st | -pt]\n\n";
		std::cout << "Options:\n";
		std::cout << "\t-d\tdump archive file\n";
		std::cout << "\t-lt\tprint local time in dump\n";
		std::cout << "\t-st\tprint system time (UTC+0) in dump\n";
		std::cout << "\t-pt\tprint plant time in dump\n";
		std::cout << "\n\n";
		return 1;
	}

	std::cout << "\n";

	ArchUtils utils(QDir::currentPath());

	if (args[0] == "-d")
	{
		utils.dump(args[1], args.contains("-lt"), args.contains("-st"), args.contains("-pt"));
	}

	std::cout << "\n\n";

	return 1;
}
