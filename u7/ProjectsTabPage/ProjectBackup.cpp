#include "ProjectBackup.h"
#include "../AppSettings.h"

namespace
{
	struct ExecuteProcessResult
	{
		bool ok = true;
		QString output;
		QString error;
	};

	struct EnvVariable
	{
		QString name;
		QString value;
	};

	[[nodiscard]] ExecuteProcessResult executeProcess(QString executable,
													  QStringList arguments,
													  const std::vector<EnvVariable>& envVariables = {})
	{
		QProcess process;
		QString programOutput;
		bool errorOutput = false;

		try
		{
			// Capture standard output
			//
			QObject::connect(&process,
							 &QProcess::readyReadStandardOutput,
							 [&process, &programOutput]()
							 {
								 programOutput += process.readAllStandardOutput();
							 });

			// Capture error output
			//
			QObject::connect(&process,
							 &QProcess::readyReadStandardError,
							 [&process, &programOutput, &errorOutput]()
							 {
								 errorOutput = true;
								 programOutput += process.readAllStandardError();
							 });

			// Set additional environment variables
			//
			auto env = process.processEnvironment();
			for (const auto& e : envVariables)
			{
				env.insert(e.name, e.value);
			}
			process.setProcessEnvironment(env);

			// Start process.
			//
			process.start(executable, arguments);

			if (process.waitForStarted() == false)
			{
				throw std::runtime_error(QString{"Failed to start %1 process."}.arg(executable).toStdString());
			}

			QEventLoop loop;
			QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
			loop.exec(QEventLoop::ExcludeUserInputEvents);

			int exitCode = process.exitCode();

			if (exitCode > 0)
			{
				throw std::runtime_error(
					QString("%1 process failed with exit code %2, %3").arg(executable).arg(exitCode).arg(programOutput).toStdString());
			}

			if (exitCode == -1)
			{
				throw std::runtime_error(QString{"%1 crashed during execution."}.arg(executable).toStdString());
			}

			if (exitCode == -2)
			{
				throw std::runtime_error(QString{"%1 cannot be started."}.arg(executable).toStdString());
			}
		}
		catch (std::exception& e)
		{
			ExecuteProcessResult result;
			result.ok = false;
			result.output = programOutput;
			result.error = e.what();

			return result;
		}

		ExecuteProcessResult result;
		result.ok = !errorOutput;
		result.output = programOutput;

		return result;
	}
} // namespace

bool ProjectBackup::backup(QString db, QString fileName, const ProjectBackup::Server& server, QString& errorText) const
{
	if (canBackup() == false)
	{
		errorText = "Backup executable is not set.";
		return false;
	}

	std::vector<EnvVariable> envVariables;
	envVariables.emplace_back("PGPASSWORD", server.password);

	QStringList arguments;
	arguments << QString{"--username=%1"}.arg(server.username);
	arguments << QString{"--host=%1"}.arg(server.host);
	arguments << QString{"--port=%1"}.arg(server.port);
	arguments << QString{"--dbname=%1"}.arg(db);
	arguments << QString{"--format=plain"};
	arguments << QString{"--no-password"};
	arguments << QString{"--no-comments"};
	arguments << QString{"--no-owner"};
	arguments << QString{"--exclude-table-data=public.log"};
	arguments << QString{"--exclude-table-data=public.build"};
	arguments << QString{"--file=%1"}.arg(fileName);

	auto result = executeProcess(m_pgDumpCommand, arguments, envVariables);
	errorText = result.error;

	return result.ok;
}

bool ProjectBackup::restore(QString projectName, QString fileName, const ProjectBackup::Server& server, QString& errorText) const
{
	if (canRestore() == false)
	{
		errorText = "Restore executable (psql) is not set.";
		return false;
	}

	QString dbName = projectName.toLower();
	if (dbName.startsWith("u7_", Qt::CaseInsensitive) == false)
	{
		dbName = "u7_" + dbName;
	}

	// 1. Create database
	// $env:PGPASSWORD = "xxxxxx"
	// psql --username=u7 --host=localhost --port=123 --dbname=postgres --command="CREATE DATABASE u7_cdu2;"
	//
	std::vector<EnvVariable> envVariables;
	envVariables.emplace_back("PGPASSWORD", server.password);

	QStringList arguments;
	arguments << QString{"--username=%1"}.arg(server.username);
	arguments << QString{"--host=%1"}.arg(server.host);
	arguments << QString{"--port=%1"}.arg(server.port);
	arguments << QString{"--dbname=postgres"};
	arguments << QString{"--command=CREATE DATABASE %1;"}.arg(dbName);

	auto result = executeProcess(m_psqlCommand, arguments, envVariables);
	errorText = result.error;

	if (result.ok == false)
	{
		return false;
	}

	// 2. Restore project to it
	// $env:PGPASSWORD = "xxxxxx"
	// psql --username=u7 --host=localhost --port=123 --dbname=u7_cdu2 --no-password --file=D:/Develop/Temp/pg/dump.sql
	//
	arguments.clear();

	arguments << QString{"--username=%1"}.arg(server.username);
	arguments << QString{"--host=%1"}.arg(server.host);
	arguments << QString{"--port=%1"}.arg(server.port);
	arguments << QString{"--dbname=%1"}.arg(dbName);
	arguments << QString{"--no-password"};
	arguments << QString{"--single-transaction"};
	arguments << QString{"--set=ON_ERROR_STOP=1"};
	arguments << QString{"--file=%1"}.arg(fileName);

	result = executeProcess(m_psqlCommand, arguments, envVariables);

	if (result.ok == false)
	{
		errorText = result.error.isEmpty() ? result.output : result.error;
	}

	if (result.ok == false)
	{
		// Delete just created database
		//
		arguments.clear();
		arguments << QString{"--username=%1"}.arg(server.username);
		arguments << QString{"--host=%1"}.arg(server.host);
		arguments << QString{"--port=%1"}.arg(server.port);
		arguments << QString{"--dbname=postgres"};
		arguments << QString{"--command=DROP DATABASE %1;"}.arg(dbName);
		[[maybe_unused]] auto dropResult = executeProcess(m_psqlCommand, arguments, envVariables);
		return false;
	}

	return result.ok;
}

bool ProjectBackup::canBackup() const
{
	return m_pgDumpCommand.isEmpty() == false;
}

bool ProjectBackup::canRestore() const
{
	return m_psqlCommand.isEmpty() == false;
}

QString ProjectBackup::backupExecutable() const
{
	return m_pgDumpCommand;
}

void ProjectBackup::setBackupExecutable(QString file)
{
	m_pgDumpCommand = file;
}

QString ProjectBackup::restoreExecutable() const
{
	return m_psqlCommand;
}

void ProjectBackup::setRestoreExecutable(QString file)
{
	m_psqlCommand = file;
}

QString ProjectBackup::autoDetectExecutable(QString name)
{
	std::list<QString> paths = {
		QCoreApplication::applicationDirPath() + "/pg/" + name,
		QCoreApplication::applicationDirPath() + "/" + name,
		name,
	};

	for (const auto& path : paths)
	{
		auto r = executeProcess(path, QStringList() << "--version");
		if (r.ok == true)
		{
			return QDir::toNativeSeparators(path);
		}
	}

#ifdef Q_OS_WIN32
	// Downloads binaries:
	// https://get.enterprisedb.com/postgresql/postgresql-16.8-1-windows-x64-binaries.zip
	//

	QStringList otherPaths;
	// get path by env variable "ProgramFiles"
	//
	QString programFiles = qgetenv("ProgramFiles");
	QString localAppData = qgetenv("LOCALAPPDATA");

	otherPaths << programFiles + "/pgAdmin 4/runtime/" + name;
	otherPaths << localAppData + "/pgAdmin 4/runtime/" + name;
	otherPaths << localAppData + "/Programs/pgAdmin 4/runtime/" + name;

	for (int i = 99; i >= 10; i--)
	{
		otherPaths << programFiles + "/PostgreSQL/" + QString::number(i) + "/bin/" + name;
	}

	for (const auto& path : otherPaths)
	{
		auto r = executeProcess(path, QStringList() << "--version");
		if (r.ok == true)
		{
			return QDir::toNativeSeparators(path);
		}
	}
#endif //  Q_OS_WIN32

	return {};
}

std::optional<QString> ProjectBackup::executableOutput(const QString& executable, const QStringList& arguments)
{
	auto r = executeProcess(executable, arguments);

	std::optional<QString> result;
	if (r.ok == true)
	{
		result = r.output;
	}

	return result;
}
