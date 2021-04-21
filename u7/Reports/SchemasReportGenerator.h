#include "ReportTools.h"
#include "../../DbLib/DbController.h"

class SchemasReportDialog : public QDialog
{
	Q_OBJECT

public:
	static bool getReportFileName(QString* fileName,
							 QPageLayout* pageLayout,
							 QWidget *parent);

	static bool getReportFilesPath(QString* path,
							 std::vector<ReportFileTypeParams>* reportFileTypeParams,
							 const std::vector<ReportFileTypeParams>& defaultFileTypeParams,
							 QWidget *parent);
private:
	enum class Type
	{
		SelectFile,
		SelectPath
	};

	SchemasReportDialog(Type type, QString* path,  QWidget *parent);

	SchemasReportDialog(QString* path,
						std::vector<ReportFileTypeParams>* reportFileTypeParams,
						const std::vector<ReportFileTypeParams>& defaultFileTypeParams,
						QWidget *parent);

	SchemasReportDialog(QString* fileName,
						QPageLayout* pageLayout,
						QWidget *parent);

private slots:
	void okClicked();
	void browseClicked();
	void pageSetupClicked();

private:

	Type m_type = Type::SelectFile;

	QString* m_reportPath = nullptr;

	QPageLayout* m_pageLayout = nullptr;

	std::vector<ReportFileTypeParams>* m_reportFileTypeParams = nullptr;
	std::vector<ReportFileTypeParams> m_defaultFileTypeParams;

	QLineEdit* m_editReportPath = nullptr;
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
						   QWidget* parent);

	void exportSchemasToPdf(const QString& filePath, const std::vector<DbFileInfo>& files);
	void exportSchemasToAlbum(const QString& filePath, const std::vector<DbFileInfo>& files, const QPageLayout& pageLayout);
	void exportAllSchemasToAlbum(const QString& filePath, const std::vector<ReportFileTypeParams>& reportFileTypeParams);

private:
	enum class TaskType
	{
		ExportFilesToPdf,
		ExportFilesToAlbum,
		ExportAllSchemasToAlbum
	};
	void run(TaskType task, const QString& filePath, const std::vector<DbFileInfo>& files, const QPageLayout& albumPageLayout, const std::vector<ReportFileTypeParams>& albumsFileTypeParams);

private:
	QString m_serverIp;
	int m_serverPort = -1;
	QString m_serverUserName;
	QString m_serverPassword;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	QWidget* m_parent = nullptr;
};

//
// ProjectDiffWorker
//

class SchemasReportGenerator : public ReportGenerator
{
	Q_OBJECT

public:
	SchemasReportGenerator(ReportSchemaView* schemaView,
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

	void setReportFileTypeParams(const std::vector<ReportFileTypeParams>& reportFileTypeParams);

	static std::vector<ReportFileTypeParams> defaultFileTypeParams(DbController* db);

public slots:
	void exportFilesToPdf();
	void exportFilesToAlbum();
	void exportAllSchemasToAlbum();

	void stop();
	void progressRequested();

signals:
	void progressChanged(int progress, int progressMin, int progressMax, const QString& status);
	void finished(const QString& errorMessage);

public:
	enum class WorkerStatus
	{
		Idle,
		Loading,
		Parsing,
		Rendering
	};

	// Statistics
	//
	WorkerStatus currentStatus() const;
	int schemasCount() const;
	int schemaIndex() const;
	QString currentSchemaType() const;
	QString currentSchemaId() const;

private:
	struct SchemaFilesInfo
	{
		SchemaFilesInfo(int fileId, const QString& caption)
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
	void loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema>>* schemas);

private:
	DbController m_db;

	std::vector<ReportFileTypeParams> m_reportFileTypeParams;

	// Input files for exportFilesToPdf() and exportFilesToAlbum()
	//
	std::vector<DbFileInfo> m_inputFiles;

	// Output file path
	//
	QString m_filePath;

	// Report parameters

	bool m_stop = false;	// Stop processing flag, set by stop()

	// Connection information

	QString m_serverIp;
	int m_serverPort = -1;
	QString m_serverUserName;
	QString m_serverPassword;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	QFont m_font;
	QFont m_marginFont;

	// Statistics data
	//

	WorkerStatus m_currentStatus = WorkerStatus::Idle;

	mutable QMutex m_statisticsMutex;
	int m_schemasCount = 0;	// Calculated after text rendering
	int m_schemaIndex = 0;
	QString m_currentSchemaType;
	QString m_currentSchemaId;

};
