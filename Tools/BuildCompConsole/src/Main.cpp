#include <BuildCompLib/BuildComp.h>
#include <CommonLib/ConstStrings.h>
#include <CommonLib/u7_vld.h>

#include "version.h"

#include <QCommandLineParser>

#include <iostream>


#ifdef _WIN32
	#include <windows.h>

void enableAnsiEscapeCodes()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	return;
}
#endif


bool g_colorTerminal = true;
bool g_verbose = false;

std::string g_red = "\033[1;31m";
std::string g_green = "\033[1;32m";
std::string g_orange = "\033[1;33m";
std::string g_reset = "\033[0m";


void printResultDetails(const BuildCompLib::CompareResult& result)
{
	if (result.projectName == false)
	{
		std::cout << g_red;
		std::cout << "Project names are different: " << result.projectNameLeft.toStdString() << " vs "
				  << result.projectNameRight.toStdString() << "\n";
		std::cout << g_reset;
	}
	else
	{
		std::cout << "Project names are the same: " << result.projectNameLeft.toStdString() << "\n";
	}

	if (result.userName == false)
	{
		std::cout << "User names are different: " << result.userNameLeft.toStdString() << " vs " << result.userNameRight.toStdString()
				  << "\n";
	}
	else
	{
		std::cout << "User names are the same: " << result.userNameLeft.toStdString() << "\n";
	}

	if (result.buildNumber == false)
	{
		std::cout << "Build numbers are different: " << result.buildNumberLeft << " vs " << result.buildNumberRight << "\n";
	}
	else
	{
		if (result.projectName == true)
		{
			std::cout << g_orange;
		}
		std::cout << "Build numbers are the same: " << result.buildNumberLeft << "\n";
		std::cout << g_reset;
	}

	auto subsystemSideResultToStr = [](BuildCompLib::CompareResult::Subsystem::SideResult v)
	{
		switch (v)
		{
		case BuildCompLib::CompareResult::Subsystem::SideResult::NotModified:
			return "Not modified";
		case BuildCompLib::CompareResult::Subsystem::SideResult::Modified:
			return "Modified";
		case BuildCompLib::CompareResult::Subsystem::SideResult::NotExists:
			return "Not exists";
		default:
			return "Unknown";
		}
	};

	for (const auto& subsystem : result.subsystems)
	{
		if (subsystem.left == BuildCompLib::CompareResult::Subsystem::NotModified &&
			subsystem.right == BuildCompLib::CompareResult::Subsystem::NotModified)
		{
		}
		else
		{
			std::cout << g_red;
		}

		std::cout << "Subsystem: " << subsystem.subsystemId.toStdString() << "\n";
		std::cout << "\t " << subsystemSideResultToStr(subsystem.left) << "\t|\t" << subsystemSideResultToStr(subsystem.right) << "\n";
		std::cout << g_reset;
	}
}


int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

#ifdef _WIN32
	enableAnsiEscapeCodes();
#endif

	QCoreApplication app(argc, argv);

	QCoreApplication::setOrganizationName(Manufacturer::RADIY);
	QCoreApplication::setOrganizationDomain(Manufacturer::SITE);
	QCoreApplication::setApplicationName("BuildCompConsole");

	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	std::cout << std::boolalpha;

	// BuildCompConsole[.exe] [OPTIONS] <file1> <file2>
	// Arguments:
	//	<file1>				Path to the first file.
	//	<file2>				Path to the second file.
	// Options:
	//	-h, --help			Print this help message and exit.
	//	-v, --verbose		Show detailed comparison results.
	//  -nc, --no-color		Disable color output.
	//
	// Example:
	//	BuildCompConsole file1.bts file2.bts
	//
	QCommandLineParser cmdParser;

	cmdParser.setApplicationDescription("Compares two RPCT output .bts files and reports differences.");
	cmdParser.addHelpOption(); // Automatically adds -h, --help

	// Define options
	//
	QCommandLineOption verboseOption({"v", "verbose"}, "Show detailed comparison results.");
	QCommandLineOption noColorOption("no-color", "Disable color output.");

	cmdParser.addOption(verboseOption);
	cmdParser.addOption(noColorOption);

	// Define positional arguments (file1 and file2)
	//
	cmdParser.addPositionalArgument("file1", "Path to the first file.");
	cmdParser.addPositionalArgument("file2", "Path to the second file.");

	cmdParser.process(app);

	// Retrieve options
	//
	g_verbose = cmdParser.isSet(verboseOption);
	g_colorTerminal = !cmdParser.isSet(noColorOption);

	g_red = g_colorTerminal ? "\033[1;31m" : "";
	g_green = g_colorTerminal ? "\033[1;32m" : "";
	g_orange = g_colorTerminal ? "\033[1;33m" : "";
	g_reset = g_colorTerminal ? "\033[0m" : "";

	// Retrieve positional arguments
	//
	QStringList args = cmdParser.positionalArguments();
	if (args.size() != 2)
	{
		std::cout << g_red << "Error: Two file paths are required.\n\n" << g_reset;
		std::cout << cmdParser.helpText().toStdString() << "\n";
		return EXIT_FAILURE;
	}

	QString file1 = args.at(0);
	QString file2 = args.at(1);

	// Do compare
	//
	std::cout << "Comparing files " << file1.toStdString() << " and " << file2.toStdString() << "\n";

	BuildCompLib::BuildComp buildComp;

	auto result = buildComp.setLeftFile(file1);
	if (!result)
	{
		std::cout << g_red;
		std::cout << "File: " << file1.toStdString() << "\n";
		std::cout << "Error: " << result.error().toStdString() << "\n";
		std::cout << g_reset;
		return EXIT_FAILURE;
	}

	result = buildComp.setRightFile(file2);
	if (!result)
	{
		std::cout << g_red;
		std::cout << "File: " << file2.toStdString() << "\n";
		std::cout << "Error: " << result.error().toStdString() << "\n";
		std::cout << g_reset;
		return EXIT_FAILURE;
	}

	auto compareResult = buildComp.compare();

	if (g_verbose == true)
	{
		printResultDetails(compareResult);
	}
	else
	{
		// Print warning that two files are the same including build number
		//
		if (compareResult.projectName == true && compareResult.buildNumber == true)
		{
			std::cout << g_orange;
			std::cout << "Build numbers are the same: " << compareResult.buildNumberLeft << "\n";
			std::cout << g_reset;
		}
	}

	std::cout << "\n";
	std::cout << "Summary\n";

	if (compareResult.isSame == true)
	{
		std::cout << g_green;
		std::cout << "FC: The payloads of the compared files are identical.\n";
	}
	else
	{
		std::cout << g_red;
		std::cout << "The following subsystem(s) differ:\n";

		for (const auto& subsystem : compareResult.subsystems)
		{
			if (subsystem.left != BuildCompLib::CompareResult::Subsystem::NotModified ||
				subsystem.right != BuildCompLib::CompareResult::Subsystem::NotModified)
			{
				std::cout << "\t" << subsystem.subsystemId.toStdString() << "\n";

				auto modules = subsystem.leftModules + subsystem.rightModules;
				modules.sort();
				modules.removeDuplicates();

				for (const auto& rightModule : modules)
				{
					std::cout << "\t\t" << rightModule.toStdString() << "\n";
				}
			}
		}

		std::cout << "FC: The files are different.\n";
	}

	std::cout << g_reset;
	return compareResult.isSame ? EXIT_SUCCESS : EXIT_FAILURE;
}
