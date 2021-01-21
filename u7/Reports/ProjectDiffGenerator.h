#pragma once

#include "ReportTools.h"

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/DbController.h"

//
// FileDiff
//

class FileDiff
{
public:
	struct FileLine
	{
		QString text;
		int line = -1;
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

struct ProjectDiffFileTypeParams
{
	ProjectDiffFileTypeParams(int fileId, const QString& caption, bool selected)
	{
		this->fileId = fileId;
		this->caption = caption;
		this->selected = selected;
	}

	ProjectDiffFileTypeParams(int fileId, const QString& caption, bool selected, QPageSize pageSize, QPageLayout::Orientation orientation, QMarginsF margins)
		:ProjectDiffFileTypeParams(fileId, caption, selected)
	{
		this->pageSize = pageSize;
		this->orientation = orientation;
		this->margins = margins;
	}

	int fileId = -1;
	QString caption;
	bool selected = false;

	// Multiple-file report section page options
	//
	QPageSize pageSize = QPageSize(QPageSize::A3);
	QPageLayout::Orientation orientation = QPageLayout::Orientation::Portrait;
	QMarginsF margins = QMarginsF(15, 15, 15, 15);
};

struct ProjectDiffReportParams
{
	CompareData compareData;
	std::vector<ProjectDiffFileTypeParams> fileTypeParams;

	bool expertProperties = false;
	bool multipleFiles = false;

	// Single-file report page options
	//
	QPageSize albumPageSize = QPageSize(QPageSize::A3);
	QPageLayout::Orientation albumOrientation = QPageLayout::Orientation::Portrait;
	QMarginsF albumMargins = QMarginsF(15, 15, 15, 15);
};

class ProjectDiffGeneratorThread
{
public:
	static void run(const QString& fileName,
					const ProjectDiffReportParams& settings,
					const QString& projectName,
					const QString& userName,
					const QString& userPassword,
					QWidget* parent);

};

//
// ProjectDiffGenerator
//

class ProjectDiffGenerator : public ReportGenerator
{
	Q_OBJECT

public:
	ProjectDiffGenerator(const QString& fileName,
					  const ProjectDiffReportParams& settings,
					  ReportSchemaView* schemaView,
					  const QString& projectName,
					  const QString& userName,
					  const QString& userPassword);
	virtual ~ProjectDiffGenerator();

	static std::vector<ProjectDiffFileTypeParams> defaultFileTypeParams(DbController* db);
	static int applicationSignalsTypeId() { return -256; }

public slots:
	void process();
	void stop();

	void progressRequested();

public:
	const QString& filePath() const;

private:
	// Statistics

	enum class WorkerStatus
	{
		Idle,
		Analyzing,
		RequestingSignals,
		Rendering,
		Printing
	};

	WorkerStatus currentStatus() const;

	int signalsCount() const;
	int signalIndex() const;

	int filesCount() const;
	int fileIndex() const;

	int sectionCount() const;
	int sectionIndex() const;

	int pagesCount() const;
	int pageIndex() const;

	QString renderingReportName() const;
	QString currentSection() const;
	QString currentObjectName() const;

signals:
	void progressChanged(int progress, int progressMin, int progressMax, const QStringList& status);
	void finished(const QString& errorMessage);

private:
	DbController* db();

	void compareProject(std::map<int, std::vector<std::shared_ptr<ReportSection> > >& reportContents);

	void compareFilesRecursive(int rootFileId,
							   const DbFileTree& filesTree,
							   const std::shared_ptr<DbFileInfo>& fi,
							   const CompareData& compareData,
							   ReportTable* const headerTable,
							   std::vector<std::shared_ptr<ReportSection> >* sectionsArray);

	void compareFile(int rootFileId,
					 const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 ReportTable* const headerTable,
					 std::vector<std::shared_ptr<ReportSection> >* sectionsArray);

	void compareFileContents(int rootFileId,
							 const std::shared_ptr<DbFile>& sourceFile,
							 const std::shared_ptr<DbFile>& targetFile,
							 const QString& fileName,
							 ReportTable* const headerTable,
							 std::vector<std::shared_ptr<ReportSection> >* sectionsArray);

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap) const;

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile,
							  const std::shared_ptr<Hardware::DeviceObject>& sourceObject, const std::shared_ptr<Hardware::DeviceObject>& targetObject,
							  ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray, bool presets);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);
	void compareSchemas(const QString& fileName, const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);

	void compareSignals(const CompareData& compareData, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, ReportTable* const headerTable, std::vector<std::shared_ptr<ReportSection> >* sectionsArray);

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

	void generateTitlePage(QTextDocument* textDocument, const CompareData& compareData, const QString& projectName, const QString& userName, const QString& subreportName) const;

	void generateReportFilesPage(QTextDocument* textDocument, const QStringList& subreportFiles);

	void createMarginItems(const CompareData& compareData, const QString& subreportName);

	void fillDiffTable(ReportTable* diffTable, const std::vector<PropertyDiff>& diffs);

	void addHeaderTableItem(ReportTable* const headerTable, const QString& caption, const QString& action, std::shared_ptr<DbFile> file);
	void addHeaderTableItem(ReportTable* const headerTable, const QString& caption, const QString& action, const Signal& signal);

	QString changesetString(const std::shared_ptr<DbFile>& file);
	QString changesetString(const Signal& signal);

	void renderReport(std::map<int, std::vector<std::shared_ptr<ReportSection> > > reportContents);

private:
	DbController m_db;

	ProjectDiffReportParams m_reportParams;
	QString m_filePath;

	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	QFont m_headerFont;
	QFont m_normalFont;
	QFont m_tableFont;
	QFont m_marginFont;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;

	// Statistics data
	//

	WorkerStatus m_currentStatus = WorkerStatus::Idle;

	mutable QMutex m_statisticsMutex;

	int m_signalsCount = 0;
	int m_signalIndex = 0;

	int m_filesCount = 0;
	int m_fileIndex = 0;

	int m_sectionCount = 0;
	int m_sectionIndex = 0;

	int m_pagesCount = 0;	// Calculated after text rendering
	int m_pageIndex = 0;

	QString m_currentSectionName;
	QString m_currentObjectName;
	QString m_currentReportName;

	bool m_stop = false;	// Stop processing flag, set by stop()

};
