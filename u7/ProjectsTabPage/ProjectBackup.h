#pragma once

#include <optional>

class ProjectBackup final
{
public:
	struct Server
	{
		QString host;
		int port;
		QString username;
		QString password;
	};

	bool backup(QString db, QString fileName, const ProjectBackup::Server& server, QString& errorText, std::atomic<bool>& abort) const;
	bool restore(QString projectName, QString fileName, const ProjectBackup::Server& server, QString& errorText) const;

	bool canBackup() const;
	bool canRestore() const;

	QString backupExecutable() const;
	void setBackupExecutable(QString file);

	QString restoreExecutable() const;
	void setRestoreExecutable(QString file);

public:
	static QString autoDetectExecutable(QString name);
	static std::optional<QString> executableOutput(const QString& executable, const QStringList& arguments);

private:
	QString m_pgDumpCommand;
	QString m_psqlCommand;
};
