#pragma once

#include "../ReportLib/ReportAppSignalProvider.h"
#include "../ReportLib/Report.h"
#include "../DbLib/DbController.h"

namespace Builder
{
	//
	// SchemasReportGenerator
	//

	class SchemasReportGenerator : public QObject
	{
		Q_OBJECT

	public:
		SchemasReportGenerator(std::shared_ptr<ReportLib::ReportSchemaView> schemaView,
								const AppSignalSet* signalSet,
								const QString& serverIp,
								int serverPort,
								const QString& serverUserName,
								const QString& serverPassword,
								const QString& projectName,
								const QString& userName,
								const QString& userPassword,
								std::vector<DbFileInfo> files,
								const QString& filePath);

		virtual ~SchemasReportGenerator();

		void setPageLayout(const QPageLayout& pageLayout);
		void setReportFileTypeParams(const std::vector<ReportLib::ReportFileTypeParams>& reportFileTypeParams);

		static std::vector<ReportLib::ReportFileTypeParams> defaultFileTypeParams(DbController* db);

	public slots:
		void exportFilesToPdf();
		void exportFilesToAlbum();
		void exportAllSchemasToAlbums();

		void stop();
		void progressRequested();

		void getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText);

	signals:
		void progressChanged(int progress, int progressMin, int progressMax, const QString& progressText);
		void finished(const QString& errorMessage);

	public:
		// Access to output data. Output data is filled if fileName is empty
		//
		QStringList outputFilesList() const;
		const QByteArray& outputData(const QString& fileName);

	public:
		// Statistics data
		//
		enum class WorkerStatus
		{
			Idle,
			Loading,
			Parsing,
			Rendering
		};
		struct Statistics
		{
			WorkerStatus m_currentStatus = WorkerStatus::Idle;
			int m_schemasCount = 0;	// Calculated after text rendering
			int m_schemaIndex = 0;
			QString m_currentSchemaType;
			QString m_currentSchemaId;
		};

		Statistics statistics() const;

	private:
		struct SchemaFilesGroup
		{
			SchemaFilesGroup(int fileId, const QString& caption)
			{
				this->fileId = fileId;
				this->caption = caption;
			}

			int fileId = -1;
			QString caption;

			std::vector<DbFileInfo> schemasFiles;
			std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file
		};

	private:
		DbController* db();
		const QString& filePath() const;

		void openProject();
		void closeProject();

		void loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema>>& schemas);
		void renderSchemas(const SchemaFilesGroup& sfg);
		void clearSchemas(SchemaFilesGroup& sfg);

	private:
		DbController m_db;
		const std::shared_ptr<ReportLib::ReportSchemaView> m_schemaView;

		ReportLib::ReportAppSignalProvider m_appSignalProvider;
		VFrame30::AppSignalController m_appSignalController;


		std::vector<ReportLib::ReportFileTypeParams> m_reportFileTypeParams;

		// Input files for exportFilesToPdf() and exportFilesToAlbum()
		//
		std::vector<DbFileInfo> m_inputFiles;

		// Output file path
		//
		QString m_filePath;

		// Output Data
		//
		std::map<QString, QByteArray> m_outputData;

		// Report parameters

		std::atomic_bool m_stop = false;	// Stop processing flag, set by stop()

		// Connection information

		QString m_serverIp;
		int m_serverPort = -1;
		QString m_serverUserName;
		QString m_serverPassword;

		QString m_projectName;
		QString m_userName;
		QString m_userPassword;

		ReportLib::ReportFont m_marginFont;

		QPageLayout m_pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15));

		mutable QMutex m_statisticsMutex;
		Statistics m_statistics;

	};
}
