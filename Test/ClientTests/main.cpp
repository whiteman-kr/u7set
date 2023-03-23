#include <QCoreApplication>
#include "ConnectionPorts.h"

int main(int argc, char *argv[])
{
	QCoreApplication app{argc, argv};

	QStringList arguments = app.arguments();
	QString buildPath;
	QString profileName;

	for (const QString& a : arguments)
	{
		if (a.startsWith("-build=") == true || a.startsWith("--build=") == true)
		{
			buildPath = a;
			buildPath.replace("--build=", "", Qt::CaseInsensitive);
			buildPath.replace("-build=", "", Qt::CaseInsensitive);
			continue;
		}

		if (a.startsWith("-profile=") == true || a.startsWith("--profile=") == true)
		{
			profileName = a;
			profileName.replace("--profile=", "", Qt::CaseInsensitive);
			profileName.replace("-profile=", "", Qt::CaseInsensitive);
		}
	}

	if (buildPath.isEmpty() == true || profileName.isEmpty() == true)
	{
		std::cout << "Warning: Build path and/or profile name are not specified, functional tests will fail.\n";
		std::cout << "./ClientTest [-build=build_dir] [-profile=profile_name]\n\n";
	}

	if (buildPath.isEmpty() == false && profileName.isEmpty() == false)
	{
		std::filesystem::path profileFile = std::filesystem::path(buildPath.toStdString()) / "Common/SimProfiles.txt";
		QString error;

		Sim::Profiles profiles;
		bool ok = profiles.load(profileFile, &error);

		if (ok == false)
		{
			std::cout << "Load profiles " << profileFile << " error:\n";
			std::cout << error.toStdString() << "\n";
			return 1;
		}

		if (profiles.hasProfile(profileName) == false)
		{
			std::cout << "Profile " << profileName.toStdString() << " is not found in file " << profileFile << "\n";
			return 1;
		}

		g_profile = profiles.profile(profileName);

		if (auto[initResult, message] = g_connectionPorts.init(g_profile);
			initResult == false)
		{
			std::cout << message.toStdString() << "\n";
			return 1;
		}
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
