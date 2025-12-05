#include <CommonLib/ConstStrings.h>
#include <CommonStdLib/u7_vld.h>

#include "version.h"

#include <LicenseLib/RpctLicense.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>


bool generateBlacklist(QString dirName, QString fileName)
{
	// Generate blacklist
	// Recursively go through the directory and get all .rls files
	// If the rls file is expired, then add its UUID to the blacklist
	//
	struct LicenseEntry
	{
		QUuid uuid;
		bool revoked{};
		bool expired{};
		QString fileName;
	};

	std::vector<LicenseEntry> blacklist;
	blacklist.reserve(256);

	QDir dir{dirName};
	if (dir.exists() == false)
	{
		qDebug() << "Directory does not exist:" << dirName;
		return false;
	}

	QDate currentDate = QDate::currentDate();

	auto processDir = [&](const QDir& d, auto& processDirRef) -> void
	{
		QFileInfoList entries = d.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

		for (const QFileInfo& entry : entries)
		{
			if (entry.isDir() == true)
			{
				processDirRef(QDir{entry.absoluteFilePath()}, processDirRef);
			}
			else
			{
				if (entry.suffix().compare("rls", Qt::CaseInsensitive) == 0)
				{
					// Load the license
					//
					QFile file{entry.absoluteFilePath()};
					if (file.open(QIODevice::ReadOnly) == false)
					{
						continue;
					}

					QString errorMessage;

					auto license =
						LicenseLib::RpctLicense::fromRawData(file.readAll(), QString{":/LicenseLib/public_key_inst1.pem"}, &errorMessage);

					if (errorMessage.isEmpty() == false)
					{
						qCritical() << "Failed to load license from file: " << entry.absoluteFilePath() << ", error:" << errorMessage;
						continue;
					}

					if (license.isNull() == true)
					{
						qCritical() << "License is null in file: " << entry.absoluteFilePath();
						continue;
					}

					if (license.revoked() == true || currentDate > license.endDate())
					{
						LicenseEntry le = {.uuid = license.uuid(),
										   .revoked = license.revoked(),
										   .expired = currentDate > license.endDate(),
										   .fileName = entry.absoluteFilePath()};
						blacklist.push_back(le);
					}
				}
			}
		}

		// Write the blacklist to file, file .hpp
		// file has inlined and constant `std::unordered_map<QUuid, BlacklistItem> EmbeddedBlacklist;`
		// where BlacklistItem is already included via precompiled header
		//
		QFile outFile{fileName};
		if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		{
			qDebug() << "Failed to open output file:" << fileName << ", error:" << outFile.errorString();
			return;
		}

		QTextStream outStream{&outFile};
		outStream << "#pragma once\n\n";
		outStream << "#include <unordered_map>\n\n";
		outStream << "namespace LicenseLib\n{\n";
		outStream << "\tinline const std::unordered_map<QUuid, BlacklistItem> EmbeddedBlacklist =\n\t{\n";

		for (const auto& item : blacklist)
		{
			if (item.revoked == true)
			{
				outStream << "\t\t{ QUuid(\"" << item.uuid.toString(QUuid::WithoutBraces) << "\"), BlacklistItem{ QUuid(\""
						  << item.uuid.toString(QUuid::WithoutBraces) << "\"), BlacklistReason::Revoked } }, // " << item.fileName << "\n";
				continue;
			}
			else if (item.expired == true)
			{
				outStream << "\t\t{ QUuid(\"" << item.uuid.toString(QUuid::WithoutBraces) << "\"), BlacklistItem{ QUuid(\""
						  << item.uuid.toString(QUuid::WithoutBraces) << "\"), BlacklistReason::Expired } }, // " << item.fileName << "\n";
				continue;
			}

			Q_ASSERT(item.revoked == true || item.expired == true);
		}

		outStream << "\t};\n";
		outStream << "}\n";
		outFile.close();

		return;
	};

	processDir(dir, processDir);

	return true;
}

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	QCoreApplication app(argc, argv);

	QCoreApplication::setOrganizationName(Manufacturer::RADIY);
	QCoreApplication::setOrganizationDomain(Manufacturer::SITE);
	QCoreApplication::setApplicationName("Licenser");

	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// Init LicenseLib resources
	//
	Q_INIT_RESOURCE(LicenseLib);


	// --generate-blacklist <directory> <output_file>
	//
	QStringList args = app.arguments();

	if (args.size() >= 4 && args[1] == "--generate-blacklist")
	{
		QString dirName = args[2];
		QString fileName = args[3];

		if (dirName.isEmpty() == true || fileName.isEmpty() == true)
		{
			qWarning("No directory or file name specified for blacklist generation.");
			return 1;
		}

		if (generateBlacklist(dirName, fileName) == false)
		{
			qWarning("Failed to generate blacklist to file: %s", qUtf8Printable(fileName));
			return 1;
		}

		qInfo("Blacklist generated to file: %s", qUtf8Printable(fileName));
		return 0;
	}

	// Report application version and exit
	//
	if (args.size() >= 2 && (args[1] == "--version" || args[1] == "-v"))
	{
		qInfo() << qApp->applicationName() + " version " + qApp->applicationVersion();
		return 0;
	}

	// Print usage
	//
	qInfo() << qApp->applicationName() + " version " + qApp->applicationVersion();
	qInfo() << "Usage:";
	qInfo() << "  " + qApp->applicationName() + " [--generate-blacklist <directory> <output_file>]";
	qInfo() << "  " + qApp->applicationName() + " [--version|-v]";

	return 1;
}
