#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/DbController.h"

#include "EditSchemaWidget.h"

#include <optional>

class DbController;

class ReportGenerator;

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
	ReportTable(const QStringList& headerLabels, const QTextCharFormat& charFormat);

	QStringList headerLabels() const;
	void setHeaderLabels(const QStringList& headerLabels);

	int columnCount() const;
	int rowCount() const;

	const QStringList& rowAt(int index) const;

	void insertRow(const QStringList& row);

	void render(const ReportObjectContext& context) const override;

private:

	QStringList m_headerLabels;
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
	ReportSection();
	virtual ~ReportSection();

	bool isEmpty() const;

	// Add object functions

	void addText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat);

	void addTable(std::shared_ptr<ReportTable> table);
	std::shared_ptr<ReportTable> addTable(const QStringList& headerLabels, const QTextCharFormat& charFormat);

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
// ReportGenerator
//

class ReportGenerator : public QObject
{
public:
	ReportGenerator(const QString& fileName, std::shared_ptr<ReportSchemaView> schemaView);


protected:
	void addMarginItem(const ReportMarginItem& item);

	// Rendering functions

	void printDocument(QPdfWriter* pdfWriter, QTextDocument* textDocument, QPainter* painter);
	void printSchema(QTextDocument* textDocument, QPainter* painter, ReportSchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schemas, const std::map<QUuid, CompareAction>& itemActions);

	// Formatting functions

	void saveFormat();
	void restoreFormat();

	void setFont(const QFont& font);
	void setTextForeground(const QBrush& brush);
	void setTextBackground(const QBrush& brush);
	void setTextAlignment(Qt::Alignment alignment);

private:
	void drawMarginItems(int page, QPainter* painter);

protected:
	QPdfWriter m_pdfWriter;
	std::shared_ptr<ReportSchemaView> m_schemaView;

	QTextCharFormat m_currentCharFormat;
	QTextBlockFormat m_currentBlockFormat;

	QTextCharFormat m_currentCharFormatSaved;
	QTextBlockFormat m_currentBlockFormatSaved;

	std::vector<std::shared_ptr<ReportSection>> m_sections;

	mutable QMutex m_statisticsMutex;

	int m_pagesCount = 0;	// Calculated after text rendering
	int m_pageIndex = 0;

private:
	std::vector<ReportMarginItem> m_marginItems;

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
		int line;
		Hash hash;

		bool operator == (const FileLine& That) const
		{
			return hash == That.hash;
		}
	};

	static bool diff(const QByteArray& sourceData, const QByteArray& targetData, std::vector<FileLine>* sourceDiff, std::vector<FileLine>* targetDiff);
	static void loadFileData(const QByteArray& fileData, std::vector<FileLine>* fileLines);
	template<typename T> static void calculateLcs(const std::vector<T>& X, const std::vector<T>& Y, std::vector<T>* result);
};

//
// PropertyDiff
//

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


class ProjectDiffGenerator
{
public:
	static void run(const CompareData& compareData,
					std::map<int, QString> fileTypesMap,
					const QString& projectName,
					const QString& userName,
					const QString& userPassword,
					QWidget* parent);

	static std::map<int, QString> filesTypesNamesMap(DbController* db);

	static int applicationSignalsTypeId() { return -256; }
};

//
// ProjectDiffWorker
//

class ProjectDiffWorker : public ReportGenerator
{
	Q_OBJECT

public:
	ProjectDiffWorker(const QString& fileName,
					  const CompareData& compareData,
					  std::map<int, QString> fileTypesMap,
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

	QString currentSection() const;
	QString currentObjectName() const;

signals:
	void finished();

private:
	DbController* const db();

	void compareProject();
	void compareFilesRecursive(const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 ReportTable* const headerTable);

	void compareFile(const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 ReportTable* const headerTable);

	void compareFileContents(const std::shared_ptr<DbFile>& sourceFile,
							 const std::shared_ptr<DbFile>& targetFile,
							 const QString& fileName,
							 ReportTable* const headerTable);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile,
							  const std::shared_ptr<Hardware::DeviceObject>& sourceObject, const std::shared_ptr<Hardware::DeviceObject>& targetObject,
							  ReportTable* const headerTable);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable);
	void compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable);
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable);
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);

	void compareSignal(const int signalID, const CompareData& compareData, ReportTable* const headerTable);
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, ReportTable* const headerTable);

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

	void generateTitlePage(QTextCursor* textCursor);
	void createMarginItems(QTextCursor* textCursor);

	void fillDiffTable(ReportTable* diffTable, const std::vector<PropertyDiff>& diffs);

	QString changesetString(const std::shared_ptr<DbFile>& file);
	QString changesetString(const Signal& signal);

private:
	DbController m_db;
	QString m_projectName;
	QString m_userName;
	QString m_userPassword;

	CompareData m_compareData;

	QFont m_headerFont;
	QFont m_normalFont;
	QFont m_tableFont;
	QFont m_errorFont;
	QFont m_marginFont;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;


	// Statistics data
	//

	WorkerStatus m_currentStatus = WorkerStatus::Idle;

	int m_signalsCount = 0;
	int m_signalIndex = 0;

	int m_filesCount = 0;
	int m_fileIndex = 0;

	int m_sectionCount = 0;
	int m_sectionIndex = 0;

	QString m_currentSectionName;
	QString m_currentObjectName;


	bool m_stop = false;	// Stop processing flag, set by stop()

	std::map<int, QString> m_fileTypesNamesMap;

};

#endif // PROJECTDIFFGENERATOR_H
