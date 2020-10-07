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
} ;

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
	bool compareFile(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData);
	bool compareFileContents(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, const QString& fileName);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject> >* deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile) const;
	void compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile);

	void compareSignal(const int signalID, const CompareData& compareData);
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal);

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

	bool writePdf(const QTextDocument& doc, const QString& fileName) const;
	virtual void generateHeader();
	//bool exportToTextDocument(QTableView* tableView, QTextDocument* doc, bool onlySelectedRows);

	void printSchema(std::shared_ptr<VFrame30::Schema> schema);

	void newPage();
	void flushDocument();

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

};

#endif // PROJECTDIFFGENERATOR_H
