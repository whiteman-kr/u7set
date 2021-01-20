#include "ReportTools.h"
#include "../../lib/DbController.h"

class SchemasReportDialog : public QDialog
{
	Q_OBJECT
public:
	SchemasReportDialog(const QString& path, const QPageSize& pageSize, const QPageLayout::Orientation orientation, const QMarginsF& margins, QWidget *parent);

public:
	QString reportPath() const;

	QPageSize pageSize() const;
	QPageLayout::Orientation orientation() const;
	QMarginsF margins() const;

private slots:
	void okClicked();
	void browseClicked();
	void pageSetupClicked();

private:
	QString m_reportPath;
	QPageSize m_pageSize;
	QPageLayout::Orientation m_orientation = QPageLayout::Orientation::Portrait;
	QMarginsF m_margins;

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

	void run(const std::vector<DbFileInfo>& files, bool album);

private:
	QString m_serverIp;
	int m_serverPort = -1;
	QString m_serverUserName;
	QString m_serverPassword;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	QWidget* m_parent = nullptr;

	static QPageSize m_albumPageSize;
	static QPageLayout::Orientation m_albumOrientation;
	static QMarginsF m_albumMargins;
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


	/*
	SchemasReportGenerator(ReportSchemaView* schemaView,
						std::vector<std::shared_ptr<VFrame30::Schema>> schemas,
						std::vector<std::shared_ptr<QBuffer>>* outputBuffers,
						bool singleFile);*/

	virtual ~SchemasReportGenerator();

public slots:
	void createPDFs();
	void createAlbum();
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
	QString currentSchemaId() const;

private:
	DbController* db();

	const QString& filePath() const;

	void loadSchemas();

private:
	DbController m_db;

	// Input data (files or pointer to schemas)
	//
	std::map<QString, std::shared_ptr<VFrame30::Schema>> m_schemas;	// Key is full path to schema file
	std::vector<DbFileInfo> m_files;

	// Output data (file path or output buffers)
	//
	std::vector<std::shared_ptr<QBuffer>>* m_outputBuffers = nullptr;
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
	QString m_currentSchemaId;

};
