#pragma once

#include "../Builder/SchemasReportGenerator.h"
#include "../ReportLib/Report.h"
#include "../ReportLib/ReportAppSignalProvider.h"
#include "../ReportLib/ReportDiagStateProvider.h"
#include "../ReportLib/ReportPrinter.h"

#include <HardwareLib/DeviceObject.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/DiagStateController.h>

#include "GlobalMessanger.h"

//
// FileDiff
//

class FileDiff
{
public:
	struct FileLine
	{
		QString text;
		qsizetype line = -1;
		Hash hash = UNDEFINED_HASH;

		bool operator == (const FileLine& That) const
		{
			return hash == That.hash;
		}
		bool operator != (const FileLine& That) const
		{
			return hash != That.hash;
		}
	};

	enum class FileDiffAction
	{
		Added,
		Removed,
		Modified,
		Match
	};

	static void loadFileData(const QByteArray& fileData, std::vector<FileLine>* fileLines);

	template<typename T> static void calculateLcs(const std::vector<T>& source, const std::vector<T>& target, std::vector<T>* result);

	template<typename T>
	static void alignResults(const std::vector<T>& source, const std::vector<T>& target,
							 const std::vector<T>& lcs,
							 std::vector<T>* sourceAligned, std::vector<T>* targetAligned,
							 std::vector<FileDiffAction>* actions,
							 int* addedCount,
							 int* removedCount,
							 int* alignedCount);
};

struct FileDiffPair
{
	QString sourceText;
	QString targetText;
};

struct PropertyDiff
{
	enum class Action
	{
		Added,
		Removed,
		Modified
	};

	QString caption;
	Action action = Action::Modified;

	QVariant oldValue;
	QVariant newValue;

	QString oldValueText;
	QString newValueText;
};

struct ProjectDiffReportParams
{
	CompareData compareData;
	std::vector<Builder::SchemaTypesParams> schemaTypesParams;

	bool expertProperties = false;
	bool singleFile = true;

	QPageLayout singleFilePageLayout() const;
};

class ProjectDiffGeneratorThread
{
public:
	static void run(const QString& fileName,
					const ProjectDiffReportParams& settings,
					const QString& projectName,
					const QString& userName,
					const QString& userPassword,
					const AppSignalSet *signalSet,
					QWidget* parent);

};

//
// ProjectDiffGenerator
//

class ProjectDiffGenerator : public QObject
{
	Q_OBJECT

public:
	ProjectDiffGenerator(const QString& fileName,
						 const ProjectDiffReportParams& settings,
						 std::shared_ptr<ReportLib::ReportSchemaView> schemaView,
						 const AppSignalSet* signalSet,
						 const QString& projectName,
						 const QString& userName,
						 const QString& userPassword);
	virtual ~ProjectDiffGenerator();

	static std::vector<Builder::SchemaTypesParams> defaultFileTypeParams(DbController* db);
	static int applicationSignalsTypeId() { return -256; }

public slots:
	void process();
	void stop();

	void progressRequested();

public:
	const QString& filePath() const;

private:
	// Statistics data
	//
	enum class WorkerStatus
	{
		Idle,
		Comparing,
		RequestingSignals,
		Printing
	};

	struct Statistics
	{
		WorkerStatus m_state = WorkerStatus::Idle;

		int m_signalsCount = 0;
		int m_signalIndex = 0;

		int m_filesCount = 0;
		int m_fileIndex = 0;

		int m_sectionCount = 0;
		int m_sectionIndex = 0;

		QString m_currentSectionName;
		QString m_currentObjectName;
		QString m_printingReportName;
	};

	Statistics statistics() const;

signals:
	void progressChanged(int progress, int progressMin, int progressMax, const QStringList& status);
	void finished(const QString& errorMessage);

private:
	DbController* db();

	void compareProject();

	void compareFilesRecursive(int rootFileId,
							   const DbFileTree& filesTree,
							   const std::shared_ptr<DbFileInfo>& fi,
							   const CompareData& compareData,
							   std::shared_ptr<ReportLib::ReportSection> section,
							   ReportLib::ReportTable& headerTable);

	void compareFile(int rootFileId,
					 const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 std::shared_ptr<ReportLib::ReportSection> section,
					 ReportLib::ReportTable& headerTable);

	void compareFileContents(int rootFileId,
							 const std::shared_ptr<DbFile>& sourceFile,
							 const std::shared_ptr<DbFile>& targetFile,
							 const QString& fileName,
							 std::shared_ptr<ReportLib::ReportSection> section,
							 ReportLib::ReportTable& headerTable);

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap) const;

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile,
							  const std::shared_ptr<Hardware::DeviceObject>& sourceObject,
							  const std::shared_ptr<Hardware::DeviceObject>& targetObject,
							  std::shared_ptr<ReportLib::ReportSection> section,
							  ReportLib::ReportTable& headerTable,
							  bool fileTypeIsPreset);

	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile,
						 const std::shared_ptr<DbFile>& targetFile,
						 std::shared_ptr<ReportLib::ReportSection> section,
						 ReportLib::ReportTable& headerTable);

	void compareSchemas(const QString& fileName,
						const std::shared_ptr<DbFile>& sourceFile,
						const std::shared_ptr<DbFile>& targetFile,
						std::shared_ptr<ReportLib::ReportSection> section,
						ReportLib::ReportTable& headerTable);

	void compareConnections(const std::shared_ptr<DbFile>& sourceFile,
							const std::shared_ptr<DbFile>& targetFile,
							std::shared_ptr<ReportLib::ReportSection> section,
							ReportLib::ReportTable& headerTable);

	void compareDiagSignalTypes(const std::shared_ptr<DbFile>& sourceFile,
							const std::shared_ptr<DbFile>& targetFile,
							std::shared_ptr<ReportLib::ReportSection> section,
							ReportLib::ReportTable& headerTable);

	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile,
						  const std::shared_ptr<DbFile>& targetFile,
						  std::shared_ptr<ReportLib::ReportSection> section,
						  ReportLib::ReportTable& headerTable);

	void compareSignals(const CompareData& compareData,
						std::shared_ptr<ReportLib::ReportSection> section,
						ReportLib::ReportTable& headerTable);

	void compareSignalContents(const AppSignal& sourceSignal,
							   const AppSignal& targetSignal,
							   std::shared_ptr<ReportLib::ReportSection> section,
							   ReportLib::ReportTable& headerTable);

	void comparePropertyObjects(const PropertyObject& sourceObject,
								const PropertyObject& targetObject,
								std::vector<PropertyDiff>* const result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isDiagSignalTypeFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;


	// Generating title pages functions
	//
	std::shared_ptr<ReportLib::ReportSection> generateTitlePage(const QPageLayout& layout, const CompareData& compareData, const QString& projectName, const QString& userName, const QString& subreportName) const;

	void generateSummaryReport(const QPageLayout& pageLayout);
	std::shared_ptr<ReportLib::ReportSection> generateSummaryReportFilesPage(const QPageLayout& layout, const QStringList& subreportFiles);

	//
	void createMarginItems(ReportLib::Report& report, const CompareData& compareData, const QString& subreportName);

	void fillDiffTable(ReportLib::ReportTable& diffTable, const std::vector<PropertyDiff>& diffs);

	void addHeaderTableItem(ReportLib::ReportTable& headerTable, const QString& caption, const QString& action, std::shared_ptr<DbFile> file);
	void addHeaderTableItem(ReportLib::ReportTable& headerTable, const QString& caption, const QString& action, const AppSignal& signal);

	QString changesetString(const std::shared_ptr<DbFile>& file);
	QString changesetString(const AppSignal& signal);

private:
	DbController m_db;
	const std::shared_ptr<ReportLib::ReportSchemaView> m_schemaView;
	ReportLib::ReportPrinter m_reportPrinter;

	ReportLib::ReportDiagStateProvider m_diagStateProvider;
	VFrame30::DiagStateController m_diagStateController;

	ReportLib::ReportAppSignalProvider m_appSignalProvider;
	VFrame30::AppSignalController m_appSignalController;

	std::vector<std::shared_ptr<ReportLib::Report>> m_generatedReports;

	ProjectDiffReportParams m_reportParams;
	QString m_filePath;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	int m_resolution{600};

	ReportLib::ReportFont m_headerFont;
	ReportLib::ReportFont m_normalFont;
	ReportLib::ReportFont m_tableFont;
	ReportLib::ReportFont m_marginFont;

    ReportLib::TextFormat m_headerFormat;
    ReportLib::TextFormat m_normalFormat;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;

	mutable QMutex m_statisticsMutex;
	Statistics m_statistics;

	std::atomic_bool m_stop = false;	// Stop processing flag, set by stop()

	static inline const QString titlePageName{"TitlePage"};
};
