#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"

#include <optional>

class DbController;

class ReportGenerator;

//
// ReportSchemaView
//

class ReportSchemaView : public VFrame30::SchemaView
{
public:
	ReportSchemaView(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent);

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

	virtual void render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const = 0;
};

//
// ReportBreak
//

class ReportBreak : public ReportObject
{
public:
	ReportBreak();

	void render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const override;
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

	void render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const override;

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

	void render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const override;

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

	void render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const override;

private:
	std::shared_ptr<VFrame30::Schema> m_schema;
};

//
// ReportSection
//

class ReportSection
{
public:
	ReportSection(ReportGenerator* reportGenerator, QTextCursor* textCursor);
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

	ReportGenerator* m_reportGenerator = nullptr;
	QTextCursor* m_textCursor = nullptr;

	QTextCharFormat m_currentCharFormat;
	QTextBlockFormat m_currentBlockCharFormat;

	QTextCharFormat m_currentCharFormatSaved;
	QTextBlockFormat m_currentBlockCharFormatSaved;
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
	ReportGenerator(const QString& fileName, QWidget* parent);

	QWidget* parentWidget() const;

public:
	void printSchema(std::shared_ptr<VFrame30::Schema> schema);
	void newPage();
	void flushDocument();
	void clearDocument();

protected:
	void addMarginItem(const ReportMarginItem& item);

private:
	void drawMarginItems(int page, QPainter& painter);

protected:
	QPdfWriter m_pdfWriter;
	QPainter m_pdfPainter;
	QTextDocument m_textDocument;
	QTextCursor m_textCursor;

	int m_currentPage = 1;

private:
	QWidget* m_parentWidget = nullptr;

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

//
// ProjectDiffGenerator
//

class ProjectDiffGenerator : public ReportGenerator
{
	Q_OBJECT
public:
	static void run(const CompareData& compareData, DbController* dbc, QWidget* parent);

private:
	ProjectDiffGenerator(const QString& fileName, DbController* dbc, QWidget* parent);
public:
	virtual ~ProjectDiffGenerator();

private:
	void compareProject(const CompareData& compareData);
	bool compareFile(const DbFileTree& filesTree,
					 const std::shared_ptr<DbFileInfo>& fi,
					 const CompareData& compareData,
					 ReportSection& diffDataSet,
					 ReportTable* const headerTable);

	bool compareFileContents(const std::shared_ptr<DbFile>& sourceFile,
							 const std::shared_ptr<DbFile>& targetFile,
							 const QString& fileName,
							 ReportSection& diffDataSet,
							 ReportTable* const headerTable);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable);
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

	void generateTitlePage();

	QString changesetString(const std::shared_ptr<DbFile>& file);
	QString changesetString(const Signal& signal);

private:
	DbController* m_db = nullptr;

	QFont m_headerFont;
	QFont m_normalFont;
	QFont m_tableFont;
	QFont m_marginFont;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;

	int m_filesTotal = 0;

	std::map<int, QString> m_filesTypesNamesMap;
};

#endif // PROJECTDIFFGENERATOR_H
