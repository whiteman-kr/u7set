#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/DbController.h"

#include "SchemaEditor/EditSchemaWidget.h"

#include <optional>

class DbController;

class ReportGeneratorCore;

class ReportSchemaView;

struct ReportObjectContext
{
	QTextDocument* textDocument = nullptr;
	QTextCursor* textCursor = nullptr;
};

//
// ReportSchemaView
//

class ReportSchemaView : public VFrame30::SchemaView
{
public:
	ReportSchemaView(QWidget* parent);

	void adjust(QPainter* painter, double startX, double startY, double zoom) const;

	void drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, CompareAction>& compareActions);
};

//
// ReportObject
//

class ReportObject
{
public:
	ReportObject();	// New page constructor
	virtual ~ReportObject();

	bool isText() const;
	bool isSchema() const;
	bool isTable() const;
	bool isNewPage() const;

	virtual void render(const ReportObjectContext& context) const = 0;
};

//
// ReportTable
//

class ReportTable : public ReportObject
{
public:
	ReportTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

	int columnCount() const;
	int rowCount() const;

	const QStringList& rowAt(int index) const;

	void insertRow(const QStringList& row);

	void sortByColumn(int column);

	void render(const ReportObjectContext& context) const override;

private:

	QStringList m_headerLabels;
	std::vector<int> m_columnWidths;

	std::vector<QStringList> m_rows;

	// Format
	//
	QTextCharFormat m_charFormat;
};

//
// ReportText
//

class ReportText : public ReportObject
{
public:
	ReportText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

	void render(const ReportObjectContext& context) const override;

private:
	QString m_text;

	// Format
	//
	QTextCharFormat m_charFormat;
	QTextBlockFormat m_blockCharFormat;
};

//
// ReportSection
//

class ReportSection
{
public:
	ReportSection(const QString& caption);
	virtual ~ReportSection();

	bool isEmpty() const;

	const QString& caption() const;

	// Add object functions

	void addText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

	void addTable(std::shared_ptr<ReportTable> table);
	std::shared_ptr<ReportTable> addTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

	static std::shared_ptr<ReportTable> createTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat);

	// Schema functions

	std::shared_ptr<VFrame30::Schema> schema() const;
	void setSchema(std::shared_ptr<VFrame30::Schema> schema);

	const std::map<QUuid, CompareAction>& compareItemActions() const;
	void setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions);

	// Render functions

	void render(QSizeF pageSize);

	int pageCount() const;	// filled after render()!!!

	QTextDocument* textDocument();

private:
	QString m_caption;

	std::vector<std::shared_ptr<ReportObject>> m_objects;

	std::shared_ptr<VFrame30::Schema> m_schema;

	std::map<QUuid, CompareAction> m_itemsActions;

	QTextDocument m_textDocument;

	int m_pageCount = 0; // filled after render()!!!
};

// ReportPageFooter

class ReportMarginItem
{
public:
	ReportMarginItem(const QString& text, int pageFrom, int pageTo, Qt::Alignment alignment, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

public:
	int m_pageFrom = -1;
	int m_pageTo = -1;

	QString m_text;

	Qt::Alignment m_alignment = Qt::AlignTop | Qt::AlignHCenter;

	// Format
	//
	QTextCharFormat m_charFormat;
	QTextBlockFormat m_blockFormat;

};

//
// ReportGeneratorTools
//

class ReportGeneratorCore : public QObject
{
public:
	ReportGeneratorCore(const QString& fileName, std::shared_ptr<ReportSchemaView> schemaView);

public:
	QString fileName() const;
	int resolution() const;

protected:
	void addMarginItem(const ReportMarginItem& item);
	void clearMarginItems();

	// Rendering functions

	void printDocument(QPdfWriter* pdfWriter, QTextDocument* textDocument, QPainter* painter, int* pageIndex, QMutex* pageIndexMutex, int pageCount);

	void printSchema(QPdfWriter* pdfWriter,
					 QTextDocument* textDocument,
					 QPainter* painter,
					 std::shared_ptr<VFrame30::Schema> schema,
					 const std::map<QUuid, CompareAction>& itemActions);

	// Formatting functions

	void saveFormat();
	void restoreFormat();

	void setFont(const QFont& font);
	void setTextForeground(const QBrush& brush);
	void setTextBackground(const QBrush& brush);
	void setTextAlignment(Qt::Alignment alignment);

	const QTextCharFormat& currentCharFormat() const;
	const QTextBlockFormat& currentBlockFormat() const;

private:
	void drawMarginItems(int page, int totalPages, QPdfWriter* pdfWriter, QPainter* painter);

private:
	QString m_fileName;

	int m_resolution = 300;

	std::vector<ReportMarginItem> m_marginItems;
	std::shared_ptr<ReportSchemaView> m_schemaView;

	QTextCharFormat m_currentCharFormat;
	QTextBlockFormat m_currentBlockFormat;

	QTextCharFormat m_currentCharFormatSaved;
	QTextBlockFormat m_currentBlockFormatSaved;
};

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

struct ProjectFileType
{
	ProjectFileType(int fileId, const QString& fileName, bool selected)
	{
		this->fileId = fileId;
		this->fileName = fileName;
		this->selected = selected;
	}

	int fileId = -1;
	QString fileName;
	bool selected = false;
};

struct ProjectDiffParams
{
	CompareData compareData;
	std::vector<ProjectFileType> projectFileTypes;
	bool expertProperties = false;
	bool multipleFiles = false;
};

class ProjectDiffGenerator
{
public:
	static void run(const ProjectDiffParams& settings,
					const QString& projectName,
					const QString& userName,
					const QString& userPassword,
					QWidget* parent);

	static std::vector<ProjectFileType> defaultProjectFileTypes(DbController* db);

	static int applicationSignalsTypeId() { return -256; }
};

//
// ProjectDiffWorker
//

class ProjectDiffWorker : public ReportGeneratorCore
{
	Q_OBJECT

public:
	ProjectDiffWorker(const QString& fileName,
					  const ProjectDiffParams& settings,
					  std::shared_ptr<ReportSchemaView> schemaView,
					  const QString& projectName,
					  const QString& userName,
					  const QString& userPassword);
	virtual ~ProjectDiffWorker();

public slots:
	void process();
	void stop();

public:

	enum class WorkerStatus
	{
		Idle,
		Analyzing,
		RequestingSignals,
		Rendering,
		Printing
	};

	// Statistics
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
	void finished();

private:
	DbController* db();

	void compareProject(std::map<QString, std::vector<std::shared_ptr<ReportSection>>>& reportContents);

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

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

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

	void generateTitlePage(QTextCursor* textCursor, const CompareData& compareData, const QString& projectName, const QString& userName, const QString& subreportName);

	void generateReportFilesPage(QTextCursor* textCursor, const QStringList& subreportFiles);

	void createMarginItems(QTextCursor* textCursor, const CompareData& compareData, const QString& subreportName);

	void fillDiffTable(ReportTable* diffTable, const std::vector<PropertyDiff>& diffs);

	QString changesetString(const std::shared_ptr<DbFile>& file);
	QString changesetString(const Signal& signal);

	void renderReport(std::map<QString, std::vector<std::shared_ptr<ReportSection>>> reportContents);

private:
	DbController m_db;

	ProjectDiffParams m_diffParams;

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

#endif // PROJECTDIFFGENERATOR_H
