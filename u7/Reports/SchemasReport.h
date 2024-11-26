#pragma once

#include "../Builder/SchemasReportGenerator.h"

class SchemasAlbumGenerator
{
public:
	static void createSchemasAlbums(DbController* db, const AppSignalSet* signalSet, QWidget* parent);
};

class SchemasReportGeneratorThread
{
public:
	SchemasReportGeneratorThread(const QString& serverIp,
								 int serverPort,
								 const QString& serverUserName,
								 const QString& serverPassword,
								 const QString& projectName,
								 const QString& userName,
								 const QString& userPassword,
								 const AppSignalSet* signalSet,
								 QWidget *parent,
								 const Builder::SchemasReportOptions& options,
								 const std::vector<Builder::SchemaTypesParams>& schemaTypesParams);

	void exportSchemasToMultiplePdf(const QString& filePath, const std::vector<DbFileInfo>& files);
	void exportSchemasToSinglePdf(const QString& fileName, const std::vector<DbFileInfo>& files);
	void exportAllSchemasToAlbum(const QString& pdfPath);

private:
	enum class TaskType
	{
		ExportFilesToMultiplePdf,
		ExportFilesToSinglePdf,
		ExportAllSchemasToAlbum
	};
	void run(TaskType task,
			 const QString& filePath,
			 const std::vector<DbFileInfo>& files);

private:
	QString m_serverIp;
	int m_serverPort = -1;
	QString m_serverUserName;
	QString m_serverPassword;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	const AppSignalSet* m_signalSet = nullptr;

	QWidget* m_parent = nullptr;

	const Builder::SchemasReportOptions& m_options;
	const std::vector<Builder::SchemaTypesParams>& m_schemaTypesParams;
};

