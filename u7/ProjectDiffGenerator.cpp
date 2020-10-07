#include "ProjectDiffGenerator.h"
#include "../lib/DbController.h"

#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/Connection.h"
#include "../VFrame30/DrawParam.h"
#include "../lib/TypesAndEnums.h"

#include <QPrinter>

PdfSchemaView::PdfSchemaView(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent):
	VFrame30::SchemaView(schema, parent)
{
}

void PdfSchemaView::Adjust(QPainter* painter, double startX, double startY, double zoom) const
{
	Ajust(painter, startX, startY, zoom);
}

bool FileDiff::diff(const QByteArray& sourceData, const QByteArray& targetData, std::vector<FileLine>* sourceDiff, std::vector<FileLine>* targetDiff)
{
	if (sourceDiff == nullptr || targetDiff == nullptr)
	{
		Q_ASSERT(sourceDiff);
		Q_ASSERT(targetDiff);
		return false;
	}

	std::vector<FileLine> fileLinesSource;
	std::vector<FileLine> fileLinesTarget;
	std::vector<FileLine> fileLinesCommon;

	loadFileData(sourceData, &fileLinesSource);
	loadFileData(targetData, &fileLinesTarget);

	fileLinesCommon.reserve(static_cast<int>(fileLinesSource.size()));

	calculateLcs(fileLinesSource, fileLinesTarget, &fileLinesCommon);

	for (const FileLine& fileLine : fileLinesSource)
	{
		bool commonLineExists = false;

		for (const FileLine& commonLine : fileLinesCommon)
		{
			if (fileLine.hash == commonLine.hash)
			{
				commonLineExists = true;
				break;
			}
		}

		if (commonLineExists == false)
		{
			sourceDiff->push_back(fileLine);
		}
	}

	for (const FileLine& fileLine : fileLinesTarget)
	{
		bool commonLineExists = false;

		for (const FileLine& commonLine : fileLinesCommon)
		{
			if (fileLine.hash == commonLine.hash)
			{
				commonLineExists = true;
				break;
			}
		}

		if (commonLineExists == false)
		{
			targetDiff->push_back(fileLine);
		}
	}

	return true;
}

void FileDiff::loadFileData(const QByteArray& fileData, std::vector<FileLine>* fileLines)
{
	if (fileLines == nullptr)
	{
		Q_ASSERT(fileLines);
		return;
	}

	QString string = fileData;

	QStringList strings = string.split(QChar::LineFeed);

	int count =  strings.size();

	fileLines->resize(count);

	for (int i = 0; i < count; i++)
	{
		FileLine& fileLine = (*fileLines)[i];

		fileLine.text = strings[i];
		fileLine.line = i + 1;
		fileLine.hash = ::calcHash(fileLine.text);
	}

	return;
}

template<typename T> void FileDiff::calculateLcs(const std::vector<T>& X, const std::vector<T>& Y, std::vector<T>* result)
{
	if (result == NULL)
	{
		Q_ASSERT(result);
		return;
	}

	int m = static_cast<int>(X.size());
	int n = static_cast<int>(Y.size());

	std::vector<std::vector<int>> L(m + 1, std::vector<int>(n + 1));

	/* Following steps build L[m+1][n+1] in bottom up fashion. Note
		that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
	for (int i = 0; i <= m; i++)
	{
		for (int j = 0; j <= n; j++)
		{
			if (i == 0 || j == 0)
			{
				L[i][j] = 0;
			}
			else
			{
				if (X[i - 1] == Y[j - 1])
				{
					L[i][j] = L[i - 1][j - 1] + 1;
				}
				else
				{
					L[i][j] = std::max<int>(L[i - 1][j], L[i][j - 1]);
				}
			}
		}
	}

	// Following code is used to print LCS
	int index = L[m][n];

	// Create a character array to store the lcs string
	result->resize(index/* + 1*/);
	//(*result)[index] = '\0'; // Set the terminating character

	// Start from the right-most-bottom-most corner and
	// one by one store characters in lcs[]
	int i = m;
	int j = n;
	while (i > 0 && j > 0)
	{
		// If current character in X[] and Y are same, then
		// current character is part of LCS
		if (X[i - 1] == Y[j - 1])
		{
			(*result)[index-1] = X[i-1]; // Put current character in result
			i--;
			j--;
			index--;     // reduce values of i, j and index
		}

		// If not same, then find the larger of two and
		// go in the direction of larger value
		else
		{
			if (L[i-1][j] > L[i][j-1])
			{
				i--;
			}
			else
			{
				j--;
			}
		}
	}
}


//
// ProjectDiffGenerator
//

void ProjectDiffGenerator::run(const CompareData& compareData, DbController* dbc, QWidget* parent)
{
	// Get filename
	//

	QString fileName = QFileDialog::getSaveFileName(parent, tr("Diff Report"),
													"./",
													tr("PDF documents (*.pdf)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	ProjectDiffGenerator g(fileName, dbc, parent);

	g.compareProject(compareData);

}

ProjectDiffGenerator::ProjectDiffGenerator(const QString& fileName, DbController* dbc, QWidget* parent):
	QObject(parent),
	m_parent(parent),
	m_db(dbc),
	m_pdfWriter(fileName),
	m_textCursor(&m_textDocument)

{
}

ProjectDiffGenerator::~ProjectDiffGenerator()
{
}

void ProjectDiffGenerator::compareProject(CompareData compareData)
{

	// Get and count files
	//
	m_sourceDeviceObjectMap.clear();
	m_targetDeviceObjectMap.clear();

	std::vector<DbFileInfo> rootFiles;

	bool ok = m_db->getFileList(&rootFiles, m_db->rootFileId(), false, m_parent);

	m_filesTotal = 0;

	std::vector<DbFileTree> filesTrees;

	filesTrees.resize(rootFiles.size());

	for (int i = 0; i < static_cast<int>(rootFiles.size()); i++)
	{
		const DbFileInfo& rootFile = rootFiles[i];

		DbFileTree* filesTree = &filesTrees[i];

		ok = m_db->getFileListTree(filesTree, rootFile.fileId(), false/*removeDeleted*/, m_parent);
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}

		m_filesTotal += static_cast<int>(filesTree->files().size());
	}

	// Init document

	m_pdfWriter.setPageSize(QPageSize(QPageSize::A4));
	m_pdfWriter.setPageOrientation(QPageLayout::Portrait);
	m_pdfWriter.setResolution(300);

	m_pdfPainter.begin(&m_pdfWriter);

	QRect pageRectPixels = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());
	m_textDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

	QTextCharFormat charFormat = m_textCursor.charFormat();
	charFormat.setFontPointSize(70);
	m_textCursor.setCharFormat(charFormat);

	generateHeader();

	m_textCursor.insertText(tr("Total files to compare: %1\n").arg(m_filesTotal));

	for (const DbFileTree& filesTree : filesTrees)
	{
		if (compareFile(filesTree, filesTree.rootFile(), compareData) == false)
		{
			m_textCursor.insertText(tr("Could not compare file : %1\n").arg(filesTree.rootFile()->fileName()));
		}
	}

	// Process signals
	//
	QVector<int> signalIDs;

	ok = m_db->getSignalsIDs(&signalIDs, m_parent);

	for (int signalID : signalIDs)
	{
		compareSignal(signalID, compareData);
	}

	// Close file
	//

	flushDocument();

	m_pdfPainter.end();

	// Destroy document

	return;
}

bool ProjectDiffGenerator::compareFile(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData)
{
	if (fi == nullptr)
	{
		Q_ASSERT(fi);
		return false;
	}

	// Print file name
	//
	QStringList pathList;
	std::shared_ptr<DbFileInfo> f = fi;

	while (f != nullptr)
	{
		pathList.push_front(f->fileName());

		if (f->fileId() == filesTree.rootFileId())
		{
			break;
		}

		f = filesTree.file(f->parentId());
	}

	QString fileName = QChar('/') + pathList.join(QChar('/'));

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = m_db->getFileHistory(*fi, &fileHistory, m_parent);
	if (ok == false)
	{
		return false;
	}

	// Get source file
	//
	std::shared_ptr<DbFile> sourceFile;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		ok = m_db->getLatestVersion(*fi, &sourceFile, m_parent);
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (changesetOpt.has_value() == true)
		{
			ok = m_db->getSpecificCopy(*fi, changesetOpt.value().changeset(), &sourceFile, m_parent);
		}
	}

	if (ok == false)
	{
		Q_ASSERT(false);
		return false;
	}

	// Get target file
	//
	std::shared_ptr<DbFile> targetFile;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		ok = m_db->getLatestVersion(*fi, &targetFile, m_parent);
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (changesetOpt.has_value() == true)
		{
			ok = m_db->getSpecificCopy(*fi, changesetOpt.value().changeset(), &targetFile, m_parent);
		}
	}

	if (ok == false)
	{
		Q_ASSERT(false);
		return false;
	}


	// Target changeset should be later or checked-out - swap files if needed
	//
	if (sourceFile != nullptr && targetFile != nullptr)
	{
		if (sourceFile->changeset() == 0 || targetFile->changeset() == 0)
		{
			// One of files is checked out
			//
			if (sourceFile->changeset() == 0 && targetFile->changeset() != 0)
			{
				sourceFile.swap(targetFile);
			}
		}
		else
		{
			if (sourceFile->changeset() > targetFile->changeset())
			{
				sourceFile.swap(targetFile);
			}
		}
	}

	// Compare files

	compareFileContents(sourceFile, targetFile, fileName);

	// Process children
	//
	int fileId = fi->fileId();

	int childrenCount = filesTree.childrenCount(fileId);
	for (int i = 0; i < childrenCount; i++)
	{
		std::shared_ptr<DbFileInfo> fiChild = filesTree.child(fileId, i);
		if (fiChild == nullptr)
		{
			Q_ASSERT(fiChild);
			return false;
		}

		if (compareFile(filesTree, fiChild, compareData) == false)
		{

			m_textCursor.insertText(tr("Could not compare file : %1\n").arg(fiChild->fileName()));
		}
	}

	return true;
}

bool ProjectDiffGenerator::compareFileContents(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, const QString& fileName)
{
	// No files at all
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		return false;
	}

	// Same changeset
	//
	if (sourceFile != nullptr &&
		targetFile != nullptr &&
		sourceFile->changeset() == targetFile->changeset())
	{
		return true;
	}

	// File was deleted
	//
	if ((sourceFile != nullptr && sourceFile->deleted() == true) ||
		(targetFile != nullptr && targetFile->deleted() == true))
	{
		m_textCursor.insertText(tr("\n%1: Deleted\n").arg(fileName));
		return false;
	}

	// Compare contents
	//
	if (isHardwareFile(fileName) == true)
	{
		compareDeviceObjects(sourceFile, targetFile);
		return true;
	}

	if (isConnectionFile(fileName) == true)
	{
		compareConnections(sourceFile, targetFile);
		return true;
	}

	if (isBusTypeFile(fileName) == true)
	{
		compareBusTypes(sourceFile, targetFile);
		return true;
	}

	if (isSchemaFile(fileName) == true)
	{
		compareSchemas(sourceFile, targetFile);
		return true;
	}

	compareFilesData(sourceFile, targetFile);
	return true;
}

std::optional<DbChangeset> ProjectDiffGenerator::getRecentChangeset(const std::vector<DbChangeset>& history,
																	const CompareVersionType versionType,
																	const int compareChangeset,
																	const QDateTime& compareDate) const
{
	// Get source changeset and file contents
	//
	DbChangeset resultChangeset;
	bool changesetExists = false;

	switch (versionType)
	{
	case CompareVersionType::Changeset:
		{
			for (const DbChangeset& cs: history)
			{
				if (cs.changeset() > compareChangeset)
				{
					continue;
				}

				if (changesetExists == false || resultChangeset.changeset() < cs.changeset())
				{
					resultChangeset = cs;
					changesetExists = true;
				}
			}
		}
		break;
	case CompareVersionType::Date:
		{
			for (const DbChangeset& cs : history)
			{
				if (cs.date() > compareDate)
				{
					continue;
				}

				if (changesetExists == false || resultChangeset.date() < cs.date())
				{
					resultChangeset = cs;
					changesetExists = true;
				}
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Q_ASSERT(false);
		}
		break;
	}

	if (changesetExists == true)
	{
		return resultChangeset;
	}

	return {};

}

std::shared_ptr<Hardware::DeviceObject> ProjectDiffGenerator::loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* deviceObjectMap)
{
	if (deviceObjectMap == nullptr)
	{
		Q_ASSERT(deviceObjectMap);
		return nullptr;
	}

	std::shared_ptr<Hardware::DeviceObject> object = Hardware::DeviceObject::fromDbFile(*file);
	if (object == nullptr)
	{
		Q_ASSERT(object);
		return nullptr;
	}

	// Save object to the map

	(*deviceObjectMap)[file->fileId()] = object;

	// Get pointers to parent and expand Equipment ID

	auto it = deviceObjectMap->find(file->parentId());
	if (it != deviceObjectMap->end())
	{
		std::shared_ptr<Hardware::DeviceObject> parentObject = it->second;
		if (parentObject == nullptr)
		{
			Q_ASSERT(parentObject);
		}
		else
		{
			parentObject->addChild(object);
			object->expandEquipmentId();
		}
	}

	return object;
}

void ProjectDiffGenerator::compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile,
												const std::shared_ptr<DbFile>& targetFile)
{
	// No Files
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	std::shared_ptr<Hardware::DeviceObject> sourceObject;
	std::shared_ptr<Hardware::DeviceObject> targetObject;

	// Load objects
	//
	if (sourceFile != nullptr)
	{
		sourceObject = loadDeviceObject(sourceFile, &m_sourceDeviceObjectMap);
		if (sourceObject == nullptr)
		{
			Q_ASSERT(sourceObject);
			return;
		}
	}

	if (targetFile != nullptr)
	{
		targetObject = loadDeviceObject(targetFile, &m_targetDeviceObjectMap);
		if (targetObject == nullptr)
		{
			Q_ASSERT(targetObject);
			return;
		}
	}

	// Single object
	//
	if ((sourceObject != nullptr && targetObject == nullptr) ||
		(sourceObject == nullptr && targetObject != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto singleObject = sourceObject != nullptr ? sourceObject : targetObject;

		m_textCursor.insertText(tr("\n%1: %2\n").arg(singleObject->equipmentId()).arg(singleFile->action().text()));
		return;
	}

	// Both Objects
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceObject, *targetObject, &diffs);

	if (diffs.empty() == false)
	{
		m_textCursor.insertText(tr("\n%1\n").arg(sourceObject->equipmentId()));

		for (const PropertyDiff& diff : diffs)
		{
			switch (diff.action)
			{
			case PropertyDiff::Action::Added:
				{
					m_textCursor.insertText(tr("%1: property added\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Removed:
				{
					m_textCursor.insertText(tr("%1: property removed\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Modified:
				{
					m_textCursor.insertText(tr("%1: property modified: %2 -> %3\n").arg(diff.caption).arg(diff.oldValueText).arg(diff.newValueText));
				}
				break;
			}
		}
	}
}

void ProjectDiffGenerator::compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile) const
{

}

void ProjectDiffGenerator::compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		m_textCursor.insertText(tr("\n%1: %2\n").arg(singleFile->fileName()).arg(singleFile->action().text()));
		return;
	}

	// Both Files
	//
	std::shared_ptr<VFrame30::Schema> sourceSchema = VFrame30::Schema::Create(sourceFile->data());
	if (sourceSchema == nullptr)
	{
		Q_ASSERT(false);
		m_textCursor.insertText(tr("\n%1: source schema loading failed\n").arg(sourceFile->fileName()));
	}

	std::shared_ptr<VFrame30::Schema> targetSchema = VFrame30::Schema::Create(targetFile->data());
	if (targetSchema == nullptr)
	{
		Q_ASSERT(false);
		m_textCursor.insertText(tr("\n%1: target schema loading failed\n").arg(targetFile->fileName()));
	}

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceSchema, *targetSchema, &diffs);

	if (diffs.empty() == true)
	{
		return;
	}

	newPage();

	m_textCursor.insertText(tr("\n%1\n").arg(targetSchema->schemaId()));

	m_textCursor.insertText("Source schema:");

	printSchema(sourceSchema);

	newPage();

	m_textCursor.insertText("Target schema:");

	printSchema(targetSchema);

	newPage();

	for (const PropertyDiff& diff : diffs)
	{
		switch (diff.action)
		{

		case PropertyDiff::Action::Added:
			{
				m_textCursor.insertText(tr("%1: property added\n").arg(diff.caption));
			}
			break;
		case PropertyDiff::Action::Removed:
			{
				m_textCursor.insertText(tr("%1: property removed\n").arg(diff.caption));
			}
			break;
		case PropertyDiff::Action::Modified:
			{
				m_textCursor.insertText(tr("%1: property modified: %2 -> %3\n").arg(diff.caption).arg(diff.oldValueText).arg(diff.newValueText));
			}
			break;
		}
	}

}

void ProjectDiffGenerator::compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	Hardware::Connection sourceConnection;
	Hardware::Connection targetConnection;

	if (sourceFile != nullptr)
	{
		bool ok = sourceConnection.Load(sourceFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}
	if (targetFile != nullptr)
	{
		bool ok = targetConnection.Load(targetFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleConnection = sourceFile != nullptr ? &sourceConnection : &targetConnection;
		m_textCursor.insertText(tr("\n%1: %2\n").arg(singleConnection->connectionID()).arg(singleFile->action().text()));
		return;
	}

	// Both Files
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceConnection, targetConnection, &diffs);

	if (diffs.empty() == false)
	{
		m_textCursor.insertText(tr("\n%1\n").arg(targetConnection.connectionID()));

		for (const PropertyDiff& diff : diffs)
		{
			switch (diff.action)
			{
			case PropertyDiff::Action::Added:
				{
					m_textCursor.insertText(tr("%1: property added\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Removed:
				{
					m_textCursor.insertText(tr("%1: property removed\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Modified:
				{
					m_textCursor.insertText(tr("%1: property modified: %2 -> %3\n").arg(diff.caption).arg(diff.oldValueText).arg(diff.newValueText));
				}
				break;
			}
		}
	}
}

void ProjectDiffGenerator::compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		m_textCursor.insertText(tr("\n%1: %2\n").arg(singleFile->fileName()).arg(singleFile->action().text()));
		return;
	}

	// Both Files
	//
	if (isTextFile(targetFile->fileName()) == true)
	{
		std::vector<FileDiff::FileLine> sourceDiff;
		std::vector<FileDiff::FileLine> targetDiff;

		FileDiff::diff(sourceFile->data(), targetFile->data(), &sourceDiff, &targetDiff);

		if (sourceDiff.empty() == true && targetDiff.empty() == true)
		{
			return;
		}

		m_textCursor.insertText(tr("\n%1n").arg(targetFile->fileName()));
		if (sourceDiff.empty() == false)
		{
			m_textCursor.insertText(tr("Source differences:\n"));
		}

		for (const FileDiff::FileLine& fileLine : sourceDiff)
		{
			m_textCursor.insertText(tr("%1 %2\n").arg(fileLine.line).arg(fileLine.text));
		}

		if (targetDiff.empty() == false)
		{
			m_textCursor.insertText(tr("Target differences:\n"));
		}

		for (const FileDiff::FileLine& fileLine : targetDiff)
		{
			m_textCursor.insertText(tr("%1 %2\n").arg(fileLine.line).arg(fileLine.text));
		}
	}
	else
	{
		// Other file
		//
		if (sourceFile->data() != targetFile->data())
		{
			m_textCursor.insertText(tr("\n%1: Binary data changed\n").arg(targetFile->fileName()));
		}
	}

	if (sourceFile->data().size() != targetFile->data().size())
	{
		m_textCursor.insertText(tr("Size %1 bytes -> %2 bytes\n").arg(sourceFile->data().size()).arg(targetFile->data().size()));
	}
}

void ProjectDiffGenerator::compareSignal(const int signalID, const CompareData& compareData)
{
	// Print signal ID
	//
	QString appSignalID;

	{
		Signal signal;
		bool ok = m_db->getLatestSignal(signalID, &signal, m_parent);
		if (ok == true)
		{
			appSignalID = signal.appSignalID();
		}
	}

	// Get signals history
	//
	std::vector<DbChangeset> signalHistory;

	bool ok = m_db->getSignalHistory(signalID, &signalHistory, m_parent);
	if (ok == false)
	{
		Q_ASSERT(false);
		return;
	}

	// Get source signal
	//

	std::optional<Signal> sourceSignal;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		Signal signal;
		ok = m_db->getLatestSignal(signalID, &signal, m_parent);

		if (ok == true)
		{
			sourceSignal = signal;
		}
	}
	else
	{
		std::optional<DbChangeset> sourceChangesetOpt = getRecentChangeset(signalHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (sourceChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> sourceSignals;	// for getSpecificSignals

			ok = m_db->getSpecificSignals(&signalIDs, sourceChangesetOpt.value().changeset(), &sourceSignals, m_parent);
			if (ok == true)
			{
				if (sourceSignals.size() == 1)
				{
					sourceSignal = sourceSignals[0];
				}
				else
				{
					Q_ASSERT(sourceSignals.size() == 1);
				}
			}
		}
	}

	// Get target signal
	//
	std::optional<Signal> targetSignal;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		Signal signal;
		ok = m_db->getLatestSignal(signalID, &signal, m_parent);
		if (ok == true)
		{
			targetSignal = signal;
		}
	}
	else
	{
		std::optional<DbChangeset> targetChangesetOpt = getRecentChangeset(signalHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (targetChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> targetSignals;	// for getSpecificSignals

			ok = m_db->getSpecificSignals(&signalIDs, targetChangesetOpt.value().changeset(), &targetSignals, m_parent);
			if (ok == true)
			{
				if (targetSignals.size() == 1)
				{
					targetSignal = targetSignals[0];
				}
				else
				{
					Q_ASSERT(targetSignals.size() == 1);
				}
			}
		}
	}

	// Only source file exists
	//
	if (sourceSignal.has_value() == true && targetSignal.has_value() == false)
	{
		m_textCursor.insertText(tr("\n%1: %2\n").arg(appSignalID).arg(sourceSignal.value().instanceAction().text()));
	}

	// Only target file exists
	//
	if (sourceSignal.has_value() == false && targetSignal.has_value() == true)
	{
		m_textCursor.insertText(tr("\n%1: %2\n").arg(appSignalID).arg(targetSignal.value().instanceAction().text()));
	}

	// Both files exist
	//
	if (sourceSignal.has_value() == true &&
		targetSignal.has_value() == true &&
		sourceSignal.value().changesetID() != targetSignal.value().changesetID())
	{
		// Compare contents
		//
		if (sourceSignal.value().deleted() == true || targetSignal.value().deleted() == true)
		{
			m_textCursor.insertText(tr("\n%1: Deleted\n").arg(appSignalID));
		}
		else
		{
			bool swap = false;

			// Target changeset should be later or checked-out
			//
			if (sourceSignal.value().changesetID() == 0 ||  targetSignal.value().changesetID() == 0)
			{
				// One of files is checked out
				//
				if (sourceSignal.value().changesetID() == 0 &&  targetSignal.value().changesetID() != 0)
				{
					swap = true;
				}
			}
			else
			{
				if (sourceSignal.value().changesetID() >  targetSignal.value().changesetID())
				{
					swap = true;
				}
			}

			if (swap == true)
			{
				std::swap(sourceSignal, targetSignal);
			}

			compareSignalContents(sourceSignal.value(), targetSignal.value());
		}
	}

	return;
}

void ProjectDiffGenerator::compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal)
{
	SignalProperties sourceProperties(sourceSignal);
	SignalProperties targetProperties(targetSignal);

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceProperties, targetProperties, &diffs);

	if (diffs.empty() == false)
	{
		m_textCursor.insertText(tr("\n%1\n").arg(targetSignal.appSignalID()));

		for (const PropertyDiff& diff : diffs)
		{
			switch (diff.action)
			{
			case PropertyDiff::Action::Added:
				{
					m_textCursor.insertText(tr("%1: property added\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Removed:
				{
					m_textCursor.insertText(tr("%1: property removed\n").arg(diff.caption));
				}
				break;
			case PropertyDiff::Action::Modified:
				{
					m_textCursor.insertText(tr("%1: property modified: %2 -> %3\n").arg(diff.caption).arg(diff.oldValueText).arg(diff.newValueText));
				}
				break;
			}
		}
	}
}

void ProjectDiffGenerator::comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	std::vector<std::shared_ptr<Property>> sourceProperties = sourceObject.properties();
	std::vector<std::shared_ptr<Property>> targetProperties = targetObject.properties();

	std::map<QString, int> propertyExistMap;

	for (std::shared_ptr<Property> p : sourceProperties)
	{
		propertyExistMap[p->caption()] = 1;
	}
	for (std::shared_ptr<Property> p : targetProperties)
	{
		propertyExistMap[p->caption()] |= 2;
	}

	result->reserve(propertyExistMap.size());

	for (auto it : propertyExistMap)
	{
		const QString& name = it.first;
		int existCode = it.second;

		PropertyDiff diff;
		diff.caption = name;

		if (existCode == 1)
		{
			// Exists only in source
			diff.action = PropertyDiff::Action::Removed;
			result->push_back(diff);
			continue;
		}


		if (existCode == 2)
		{
			// Exists only in target
			diff.action = PropertyDiff::Action::Added;
			result->push_back(diff);
			continue;
		}

		Q_ASSERT(existCode == 3);

		// Exists in both
		diff.action = PropertyDiff::Action::Modified;

		std::shared_ptr<Property> sp = sourceObject.propertyByCaption(name);
		std::shared_ptr<Property> tp = targetObject.propertyByCaption(name);

		if (sp == nullptr || tp == nullptr)
		{
			Q_ASSERT(sp);
			Q_ASSERT(tp);
			continue;
		}

		diff.oldValue = sp->value();
		diff.newValue = tp->value();

		diff.oldValueText = ExtWidgets::PropertyTools::propertyValueText(sp.get(), -1);
		diff.newValueText = ExtWidgets::PropertyTools::propertyValueText(tp.get(), -1);

		// Both are enums
		//
		if (sp->isEnum() == true && tp->isEnum() == true)
		{
			if (sp->value().toInt() != tp->value().toInt())
			{
				result->push_back(diff);
			}
			continue;
		}

		// Types are different
		//
		if (diff.oldValue.userType() != diff.oldValue.userType())
		{
			result->push_back(diff);
			continue;
		}

		// Special type TuningValue
		//
		if (diff.oldValue.userType() == TuningValue::tuningValueTypeId())
		{
			TuningValue tvOld;
			TuningValue tvNew;

			tvOld = diff.oldValue.value<TuningValue>();
			tvNew = diff.newValue.value<TuningValue>();

			if (tvOld != tvNew)
			{
				result->push_back(diff);
			}

			continue;
		}

		// General value
		//
		if (diff.oldValue != diff.newValue)
		{
			result->push_back(diff);
			continue;
		}
	}

	return;
}

bool ProjectDiffGenerator::isHardwareFile(const QString& fileName) const
{
	for (const QString& ext : Hardware::DeviceObjectExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

bool ProjectDiffGenerator::isBusTypeFile(const QString& fileName) const
{
	if (fileName.endsWith(".bus_type") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isConnectionFile(const QString& fileName) const
{
	if (fileName.endsWith(".ocl") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isTextFile(const QString& fileName) const
{
	const std::array<QString, 5> TextExtensions =
		{
			".js",			// Script
			".xml",			// Xml Document
			".xsd",			// Xml Schema
			".csv",			// Table
			".txt"			// Text
		};

	for (const QString& ext : TextExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

bool ProjectDiffGenerator::isSchemaFile(const QString& fileName) const
{
	const std::array<QString, 10> TextExtensions =
	{
		Db::File::AlFileExtension,			// Script
		Db::File::AlTemplExtension,			// Xml Document
		Db::File::UfbFileExtension,			// Xml Schema
		Db::File::UfbTemplExtension,			// Table
		Db::File::MvsFileExtension,
		Db::File::MvsTemplExtension,
		Db::File::TvsFileExtension,
		Db::File::TvsTemplExtension,
		Db::File::DvsFileExtension,
		Db::File::DvsTemplExtension
	};

	for (const QString& ext : TextExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

void ProjectDiffGenerator::generateHeader()
{
	DbProject project = m_db->currentProject();

	QTextBlockFormat headerCenterFormat = m_textCursor.blockFormat();
	headerCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat regularFormat = m_textCursor.blockFormat();
	regularFormat.setAlignment(Qt::AlignLeft);

	//QTextCharFormat headerCharFormat = cursor.charFormat();
	//headerCharFormat.setFontWeight(static_cast<int>(QFont::Bold));
	//headerCharFormat.setFontPointSize(12.0);

	//QTextCharFormat regularCharFormat = cursor.charFormat();
	//headerCharFormat.setFontPointSize(10.0);

	m_textCursor.setBlockFormat(headerCenterFormat);
	//m_textCursor.setCharFormat(headerCharFormat);
	m_textCursor.insertText(QObject::tr("Project Differtences - %1\n").arg(project.projectName()));
	m_textCursor.insertText("\n");

	m_textCursor.setBlockFormat(regularFormat);
	//m_textCursor.setCharFormat(regularCharFormat);
	m_textCursor.insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	//cursor.insertText(tr("RPCT: %1\n").arg(m_configuration->softwareEquipmentId));
	m_textCursor.insertText("\n");

	m_textCursor.insertText("\n");
}

void ProjectDiffGenerator::printSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	// Calculate the upper schema offset

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

	int schemaLeft = 0;

	int schemaTop = contentRect.height() % pageRect.height();

	int schemaMaxHeight = pageRect.height() - schemaTop;

	// Print the rest of the document

	flushDocument();

	// Calculate draw parameters

	PdfSchemaView schemaView(schema, m_parent);

	VFrame30::CDrawParam drawParam(&m_pdfPainter, schema.get(), &schemaView, schema->gridSize(), schema->pinGridStep());
	drawParam.setInfoMode(false);
	drawParam.session() = schemaView.session();

	double schemaWidthInPixel = schema->GetDocumentWidth(m_pdfWriter.resolution(), 100.0);		// Export 100% zoom
	double schemaHeightInPixel = schema->GetDocumentHeight(m_pdfWriter.resolution(), 100.0);		// Export 100% zoom

	double zoom = pageRect.width() / schemaWidthInPixel;

	double schemaHeightInPixelWZoomed = schemaHeightInPixel * zoom;

	if (schemaHeightInPixelWZoomed > schemaMaxHeight)
	{
		// Reduce schema's height, it does not fit vertically

		double yZoom =  schemaMaxHeight / schemaHeightInPixelWZoomed;

		zoom *= yZoom;

		// Center schema horizontally

		int schemaWidthInPixelZoomed = static_cast<int>(schemaHeightInPixel * zoom + 0.5);

		schemaLeft =  (pageRect.width() - schemaWidthInPixelZoomed) / 2;
	}

	// Draw rect

	//m_pdfPainter.fillRect(QRectF(0, schemaTop, pageRect.width(), pageRect.height() - schemaTop), QColor(0xB0, 0xB0, 0xB0));

	// Ajust QPainter
	//

	m_pdfPainter.save();

	m_pdfPainter.setRenderHint(QPainter::Antialiasing);

	schemaView.Adjust(&m_pdfPainter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

	schema->Draw(&drawParam, clipRect);

	m_pdfPainter.restore();
}


void ProjectDiffGenerator::newPage()
{
	flushDocument();

	m_pdfWriter.newPage();
}

void ProjectDiffGenerator::flushDocument()
{
	if (m_textDocument.isEmpty() == true)
	{
		return;
	}

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	// The total extent of the content (there are no page margin in this)
	const QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

	// This is the part of the content we will drop on a page.  It's a sliding window on the content.
	QRect currentRect(0, 0, pageRect.width(), pageRect.height());

	while (currentRect.intersects(contentRect) == true)
	{
			m_pdfPainter.save();
			m_pdfPainter.translate(0, -currentRect.y());
			m_textDocument.drawContents(&m_pdfPainter, currentRect);  // draws part of the document
			m_pdfPainter.restore();

			// Translate the current rectangle to the area to be printed for the next page
			currentRect.translate(0, currentRect.height());

			//Inserting a new page if there is still area left to be printed
			if (currentRect.intersects(contentRect))
			{
				m_pdfWriter.newPage();
			}
	}

	// Clear text document

	QTextCharFormat charFormat = m_textCursor.charFormat();
	//charFormat.setFontPointSize(70);

	m_textDocument.clear();

	m_textCursor.setCharFormat(charFormat);
}
