#ifndef PROJECTDIFFGENERATOR_H
#define PROJECTDIFFGENERATOR_H

#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"

#include <optional>

class DbController;

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
	ProjectDiffGenerator(DbController* dbc, QWidget* parent);


public slots:
	void compareProject(CompareData compareData);

private:
	bool compareFile(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData, QTextStream* textStream);
	bool compareFileContents(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, const QString& fileName, QTextStream* textStream);

	std::optional<DbChangeset> getRecentChangeset(const std::vector<DbChangeset>& history,
												  const CompareVersionType versionType,
												  const int compareChangeset,
												  const QDateTime& compareDate) const;

	std::shared_ptr<Hardware::DeviceObject> loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject> >* deviceObjectMap);

	void compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, QTextStream* textStream);
	void compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, QTextStream* textStream) const;
	void compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, QTextStream* textStream) const;
	void compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, QTextStream* textStream) const;
	void compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, QTextStream* textStream) const;

	void compareSignal(const int signalID, const CompareData& compareData, QTextStream* textStream) const;
	void compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, QTextStream* textStream) const;

	void comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* result) const;

	bool isHardwareFile(const QString& fileName) const;
	bool isBusTypeFile(const QString& fileName) const;
	bool isConnectionFile(const QString& fileName) const;
	bool isTextFile(const QString& fileName) const;
	bool isSchemaFile(const QString& fileName) const;

private:
	DbController* m_db = nullptr;
	QWidget* m_parent = nullptr;

	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_sourceDeviceObjectMap;
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> m_targetDeviceObjectMap;

	int m_filesTotal = 0;

};

#endif // PROJECTDIFFGENERATOR_H
