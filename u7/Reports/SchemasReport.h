#include "../Builder/SchemasReportGenerator.h"

class SchemasReportDialog : public QDialog
{
	Q_OBJECT

public:
	static bool getReportFileName(QString* fileName,
							 QPageLayout* pageLayout,
							 QWidget *parent);

	static bool getReportFilesPath(QString* path,
							 std::vector<ReportLib::ReportFileTypeParams>* reportFileTypeParams,
							 const std::vector<ReportLib::ReportFileTypeParams>& defaultFileTypeParams,
							 QWidget *parent);
private:
	enum class Type
	{
		SelectFile,
		SelectPath
	};

	SchemasReportDialog(Type type, QString* path,  QWidget *parent);

	SchemasReportDialog(QString* path,
						std::vector<ReportLib::ReportFileTypeParams>* reportFileTypeParams,
						const std::vector<ReportLib::ReportFileTypeParams>& defaultFileTypeParams,
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

	std::vector<ReportLib::ReportFileTypeParams>* m_reportFileTypeParams = nullptr;
	std::vector<ReportLib::ReportFileTypeParams> m_defaultFileTypeParams;

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
								 const AppSignalSet* signalSet,
								 QWidget *parent);

	void exportSchemasToPdf(const QString& filePath, const std::vector<DbFileInfo>& files);
	void exportSchemasToAlbum(const QString& filePath, const std::vector<DbFileInfo>& files, const QPageLayout& pageLayout);
	void exportAllSchemasToAlbum(const QString& filePath, const std::vector<ReportLib::ReportFileTypeParams>& reportFileTypeParams);

private:
	enum class TaskType
	{
		ExportFilesToPdf,
		ExportFilesToAlbum,
		ExportAllSchemasToAlbum
	};
	void run(TaskType task,
			 const QString& filePath,
			 const std::vector<DbFileInfo>& files,
			 const QPageLayout& albumPageLayout,
			 const std::vector<ReportLib::ReportFileTypeParams>& albumsFileTypeParams);

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
};

