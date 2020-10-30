#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/DbController.h"

#include <optional>

class DbController;

class ReportGenerator;

class ReportSchemaView;

struct ReportObjectContext
{
	ReportGenerator* reportGenerator = nullptr;
	QTextDocument* textDocument = nullptr;
	QPainter* painter = nullptr;
	QTextCursor* textCursor = nullptr;
	ReportSchemaView* schemaView = nullptr;
};

//
// ReportSchemaView
//

class ReportSchemaView : public VFrame30::SchemaView
{
public:
	ReportSchemaView(QWidget* parent);

	void adjust(QPainter* painter, double startX, double startY, double zoom) const;
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
// ReportBreak
//

class ReportBreak : public ReportObject
{
public:
	ReportBreak();

	void render(const ReportObjectContext& context) const override;
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
// ReportSchema
//

class ReportSchema : public ReportObject
{
public:
	ReportSchema(std::shared_ptr<VFrame30::Schema> schema);

	void render(const ReportObjectContext& context) const override;

private:
	std::shared_ptr<VFrame30::Schema> m_schema;
};

//
// ReportSection
//

class ReportSection
{
public:
	ReportSection(const ReportObjectContext& context);
	virtual ~ReportSection();

	bool isEmpty() const;

	// Add object functions

	void addText(const QString& text);
	void addSchema(std::shared_ptr<VFrame30::Schema> schema);

	void addTable(std::shared_ptr<ReportTable> table);
	std::shared_ptr<ReportTable> addTable(const QStringList& headerLabels);

	void addNewPage();

	// Formatting functions

	void saveFormat();
	void restoreFormat();

	void setFont(const QFont& font);
	void setTextForeground(const QBrush& brush);
	void setTextBackground(const QBrush& brush);
	void setTextAlignment(Qt::Alignment alignment);

	// Render functions

	void render() const;

private:
	std::vector<std::shared_ptr<ReportObject>> m_objects;

	ReportObjectContext m_context;

	QTextCharFormat m_currentCharFormat;
	QTextBlockFormat m_currentBlockFormat;

	QTextCharFormat m_currentCharFormatSaved;
	QTextBlockFormat m_currentBlockFormatSaved;
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

public:
	void printSchema(QTextDocument* textDocument, QPainter* painter, ReportSchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema);
	void newPage(QTextDocument* textDocument, QPainter* painter);
	void flushDocument(QTextDocument* textDocument, QPainter* painter);
	void clearDocument(QTextDocument* textDocument);

protected:
	void addMarginItem(const ReportMarginItem& item);

private:
	void drawMarginItems(int page, QPainter* painter);

protected:
	QPdfWriter m_pdfWriter;
	std::shared_ptr<ReportSchemaView> m_schemaView;


	int m_currentPage = 1;

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
	// Statistics
	int totalSignals() const;
	int currentSignalIndex() const;
	int totalFiles() const;
	int currentFileIndex() const;
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
					 ReportSection& diffDataSet,
					 ReportTable* const headerTable);

	void compareFile(const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 ReportSection& diffDataSet,
					 ReportTable* const headerTable);

	void compareFileContents(const std::shared_ptr<DbFile>& sourceFile,
							 const std::shared_ptr<DbFile>& targetFile,
							 const QString& fileName,
							 ReportSection& diffDataSet,
							 ReportTable* const headerTable);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile,
							  const std::shared_ptr<Hardware::DeviceObject>& sourceObject, const std::shared_ptr<Hardware::DeviceObject>& targetObject,
							  ReportSection& diffDataSet, ReportTable* const headerTable);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable);
	void compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable);
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable);
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);

	void compareSignal(const int signalID, const CompareData& compareData, ReportSection& diffDataSet, ReportTable* const headerTable);
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, ReportSection& diffDataSet, ReportTable* const headerTable);

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

	void generateTitlePage(QTextCursor* textCursor);
	void createMarginItems(QTextCursor* textCursor);

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
	mutable QMutex m_statisticsMutex;
	int m_signalsCount = 0;
	int m_currentSignalIndex = 0;

	int m_filesCount = 0;
	int m_currentFileIndex = 0;

	QString m_currentSection;
	QString m_currentObjectName;

	bool m_stop = false;	// Stop processing flag, set by stop()

	std::map<int, QString> m_fileTypesNamesMap;

};

#endif // PROJECTDIFFGENERATOR_H
