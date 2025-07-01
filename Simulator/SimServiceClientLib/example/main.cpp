#include "Actions.hpp"

#include <CommonLib/Types.h>
#include <SimServiceClientLib/SimServiceClient.h>

#include <QRegularExpression>

#include <functional>
#include <iostream>


int main()
{
	Sim::SimServiceClient client{"localhost:50051"};

	Actions::help();

	do
	{
		std::cout << ">";

		std::string command;
		std::getline(std::cin, command); // Read the full line including spaces

		QString qcommand = QString::fromStdString(command);
		QStringList args = qcommand.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

		if (args.isEmpty() == true || Actions::actions.contains(args[0]) == false)
		{
			Actions::help();
			continue;
		}

		auto& actionFunc = Actions::actions.at(args[0]);

		{
			QElapsedTimer timer;
			timer.start();

			actionFunc(client, args);

			auto elapsed = timer.elapsed();
			std::cout << "Time, ms: " << elapsed << "\n";
		}

	} while (true);

	return 0;
}