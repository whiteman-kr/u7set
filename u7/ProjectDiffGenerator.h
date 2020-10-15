#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaView.h"

#include <optional>

class DbController;

class PdfSchemaView : public VFrame30::SchemaView
{
public:
	PdfSchemaView(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent);

	void Adjust(QPainter* painter, double startX, double startY, double zoom) const;
};


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

class DiffReportTable
{
public:
	DiffReportTable();

	QString caption() const;
	void setCaption(const QString& caption);

	QStringList headerLabels();
	void setHeaderLabels(const QStringList& headerLabels);

	int columnCount() const;
	int rowCount() const;

	const QStringList& rowAt(int index) const;

	void insertRow(const QStringList& row);

	void render(QTextCursor& textCursor) const;
	void clear();

private:
	QString m_caption;

	QStringList m_headerLabels;
	std::vector<QStringList> m_rows;
};

class DiffReportObject
{
public:
	DiffReportObject(const QString& text);
	DiffReportObject(const DiffReportTable& table);

	bool isText() const;
	bool isTable() const;

	void render(QTextCursor& textCursor) const;

private:
	enum class DiffDataObjectType
	{
		Text,
		Table
	};

	DiffDataObjectType m_type = DiffDataObjectType::Text;

	// Data
	//
	QString m_text;
	DiffReportTable m_table;

};

class DiffReportObjectSet
{
public:
	bool empty() const;

	QString caption() const;
	void setCaption(const QString& caption);

	DiffReportTable& headerTable();

	void addText(const QString& text);
	void addTable(const DiffReportTable& table);

	void render(QTextCursor& textCursor) const;
	void clear();

private:
	QString m_caption;

	DiffReportTable m_headerTable;

	std::vector<DiffReportObject> m_objects;
};

class ProjectDiffGenerator : public QObject
{
	Q_OBJECT
public:
	static void run(const CompareData& compareData, DbController* dbc, QWidget* parent);

private:
	ProjectDiffGenerator(const QString& fileName, DbController* dbc, QWidget* parent);
public:
	virtual ~ProjectDiffGenerator();

private:
	void compareProject(CompareData compareData);
	bool compareFile(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData, DiffReportObjectSet& diffData);
	bool compareFileContents(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, const QString& fileName, DiffReportObjectSet& diffData);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject> >* deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, DiffReportObjectSet& diffData);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, DiffReportObjectSet& diffData) const;
	void compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, DiffReportObjectSet& diffData);
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, DiffReportObjectSet& diffData);
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);

	void compareSignal(const int signalID, const CompareData& compareData, DiffReportObjectSet& diffData);
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, DiffReportObjectSet& diffData);

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

	bool writePdf(const QTextDocument& doc, const QString& fileName) const;
	void generateTitlePage();

	void printSchema(std::shared_ptr<VFrame30::Schema> schema);

	void newPage();
	void flushDocument();
	void clearDocument();

private:
	DbController* m_db = nullptr;
	QWidget* m_parent = nullptr;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;

	int m_filesTotal = 0;

	QPdfWriter m_pdfWriter;
	QPainter m_pdfPainter;
	QTextDocument m_textDocument;
	QTextCursor m_textCursor;

	std::map<int, QString> m_filesTypesNamesMap;

	bool m_openBlock = false;

};

#endif // PROJECTDIFFGENERATOR_H
