#include "ProjectDiffGenerator.h"

#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/Connection.h"
#include "../lib/TypesAndEnums.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/Bus.h"

#include "../lib/Ui/DialogProgress.h"

#include "Settings.h"

//
// FileDiff
//

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

template<typename T> void FileDiff::calculateLcs(const std::vector<T>& source, const std::vector<T>& target, std::vector<T>* result)
{
	if (result == NULL)
	{
		Q_ASSERT(result);
		return;
	}

	int m = static_cast<int>(source.size());
	int n = static_cast<int>(target.size());

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
				if (source[i - 1] == target[j - 1])
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
		if (source[i - 1] == target[j - 1])
		{
			(*result)[index-1] = source[i-1]; // Put current character in result
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

template<typename T>
void FileDiff::alignResults(const std::vector<T>& source, const std::vector<T>& target,
							const std::vector<T>& lcs,
							std::vector<T>* sourceAligned, std::vector<T>* targetAligned,
							std::vector<FileDiffAction>* actions,
							int* addedCount,
							int* removedCount,
							int* alignedCount)
{
	int sourceIndex = 0;
	int targetIndex  = 0;
	int lcsIndex = 0;

	*alignedCount = 0;
	*addedCount = 0;
	*removedCount = 0;

	while (sourceIndex < source.size() || targetIndex < target.size())
	{
		if (sourceIndex >= source.size())
		{
			// Source is done, print the rest of target
			//
			while (targetIndex < target.size())
			{
				sourceAligned->push_back(T());
				targetAligned->push_back(target[targetIndex]);
				targetIndex++;

				actions->push_back(FileDiffAction::Added);

				(*addedCount)++;
			}
			break;
		}

		if (targetIndex >= target.size())
		{
			// Target is done, print the rest of source
			//
			while (sourceIndex < source.size())
			{
				sourceAligned->push_back(source[sourceIndex]);
				targetAligned->push_back(T());
				sourceIndex++;

				actions->push_back(FileDiffAction::Added);

				(*addedCount)++;
			}
			break;
		}

		if (lcsIndex >= lcs.size())
		{
			// LCS is done, next lines are all different
			//
			sourceAligned->push_back(source[sourceIndex]);
			targetAligned->push_back(target[targetIndex]);
			sourceIndex++;
			targetIndex++;

			actions->push_back(FileDiffAction::Modified);
			continue;
		}

		const T& sourceLine = source[sourceIndex];
		const T& targetLine = target[targetIndex];
		const T& commonLine = lcs[lcsIndex];

		if (sourceLine == commonLine && targetLine == commonLine)
		{
			sourceAligned->push_back(source[sourceIndex]);
			targetAligned->push_back(target[targetIndex]);

			sourceIndex++;
			targetIndex++;
			lcsIndex++;

			actions->push_back(FileDiffAction::Match);
		}

		if (sourceLine == commonLine && targetLine != commonLine)
		{
			sourceAligned->push_back(T());
			targetAligned->push_back(target[targetIndex]);
			targetIndex++;

			(*addedCount)++;

			actions->push_back(FileDiffAction::Added);
		}

		if (sourceLine != commonLine && targetLine == commonLine)
		{
			sourceAligned->push_back(source[sourceIndex]);
			targetAligned->push_back(T());
			sourceIndex++;

			(*removedCount)++;

			actions->push_back(FileDiffAction::Removed);
		}

		if (sourceLine != commonLine && targetLine != commonLine)
		{
			sourceAligned->push_back(source[sourceIndex]);
			targetAligned->push_back(target[targetIndex]);
			sourceIndex++;
			targetIndex++;

			actions->push_back(FileDiffAction::Modified);

		}
	}

	Q_ASSERT(sourceAligned->size() == targetAligned->size());
	Q_ASSERT(actions->size() == actions->size());

	*alignedCount = static_cast<int>(sourceAligned->size());
}


//
// ProjectDiffThread
//

void ProjectDiffGeneratorThread::run(const QString& fileName,
									 const ProjectDiffReportParams& settings,
									 const QString& projectName,
									 const QString& userName,
									 const QString& userPassword,
									 QWidget* parent)
{
	// Create schema view

	ReportSchemaView* schemaView = new ReportSchemaView(parent);

	schemaView->session().setProject(projectName);
	schemaView->session().setUsername(userName);
	schemaView->session().setHost(QHostInfo::localHostName());

	// Create Worker

	ProjectDiffGenerator* worker = new ProjectDiffGenerator(fileName, settings, schemaView, projectName, userName, userPassword);

	// Create Progress Dialog

	DialogProgress* dialogProgress = new DialogProgress(QObject::tr("Creating Project Differences Report"), 3, parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	QObject::connect(thread, &QThread::started, worker, &ProjectDiffGenerator::process);

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	QObject::connect(dialogProgress, &DialogProgress::getProgress, worker, &ProjectDiffGenerator::progressRequested, Qt::DirectConnection);

	QObject::connect(dialogProgress, &DialogProgress::cancelClicked, worker, &ProjectDiffGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &ProjectDiffGenerator::progressChanged, dialogProgress, &DialogProgress::setProgressMultiple);

	//  Schedule objects deleting

	QObject::connect(worker, &ProjectDiffGenerator::finished, [thread, dialogProgress, worker, schemaView](const QString& errorMessage)
	{
		thread->quit();

		if (errorMessage.isEmpty() == false)
		{
			dialogProgress->setErrorMessage(errorMessage);
		}
		else
		{
			dialogProgress->deleteLater();
		}

		worker->deleteLater();

		schemaView->deleteLater();
	});

	// Start thread

	thread->start();

	dialogProgress->exec();

	return;

}

//
// ProjectDiffWorker
//

ProjectDiffGenerator::ProjectDiffGenerator(const QString& fileName,
									 const ProjectDiffReportParams& settings,
									 ReportSchemaView* schemaView,
									 const QString& projectName,
									 const QString& userName,
									 const QString& userPassword):
	ReportGenerator(schemaView),
	m_filePath(fileName),
	m_reportParams(settings),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword)
{
	// Init fonts

	m_headerFont = QFont("Times", 36, QFont::Bold);
	m_normalFont = QFont("Times", 24);
	m_tableFont = QFont("Times", 24);
	m_marginFont = QFont("Times", static_cast<int>(24 * (96.0 / resolution())));

	return;
}

ProjectDiffGenerator::~ProjectDiffGenerator()
{
	qDebug() << "ProjectDiffGenerator deleted";
}

std::vector<ProjectDiffFileTypeParams> ProjectDiffGenerator::defaultFileTypeParams(DbController* db)
{
	std::vector<ProjectDiffFileTypeParams> result;

	result.push_back({db->hcFileId(),				QObject::tr("Hardware Configuration"),	true});

	result.push_back({applicationSignalsTypeId(),	QObject::tr("Application Signals"),		true});


	//Schemas
	result.push_back({db->mvsFileId(),				QObject::tr("Monitor Schemas"),			true, QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15)});
	result.push_back({db->tvsFileId(),				QObject::tr("Tuning Schemas"),			true, QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15)});
	result.push_back({db->dvsFileId(),				QObject::tr("Diagnostics Schemas"),		true, QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15)});
	result.push_back({db->alFileId(),				QObject::tr("Application Logic"),		true, QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15)});
	result.push_back({db->ufblFileId(),				QObject::tr("UFBL Descriptions"),		true, QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15)});

	result.push_back({db->busTypesFileId(),			QObject::tr("Busses"),					true});
	result.push_back({db->connectionsFileId(),		QObject::tr("Connections"),				true});

	result.push_back({db->simTestsFileId(),			QObject::tr("Simulator Tests"),			true});

	result.push_back({db->afblFileId(),				QObject::tr("AFBL Descriptions"),		false});
	result.push_back({db->hpFileId(),				QObject::tr("Hardware Presets"),		false});
	result.push_back({db->mcFileId(),				QObject::tr("Module Configuration"),	false});
	result.push_back({db->etcFileId(),				QObject::tr("Other Files"),				false});

	return result;

}

void ProjectDiffGenerator::process()
{
	std::map<int, std::vector<std::shared_ptr<ReportSection>>> reportContents;

	try
	{
		compareProject(reportContents);

		renderReport(reportContents);

		emit finished(QString());
	}

	catch(QString errorMessage)
	{
		emit finished(errorMessage);
	}

	return;
}

void ProjectDiffGenerator::stop()
{
	m_stop = true;
}

void ProjectDiffGenerator::progressRequested()
{
	QStringList progressText;

	int progress = 0;
	int progressMax = 0;

	QMutexLocker l(&m_statisticsMutex);

	ProjectDiffGenerator::WorkerStatus status = currentStatus();

	switch (status)
	{
	case ProjectDiffGenerator::WorkerStatus::Idle:
		{
			progressText.push_back(tr("Idle"));
		}
		break;
	case ProjectDiffGenerator::WorkerStatus::RequestingSignals:
		{
			progressText.push_back(tr("Requesting Application Signals..."));
		}
		break;
	case ProjectDiffGenerator::WorkerStatus::Analyzing:
		{
			progressText.push_back(tr("Analyzing: %1").arg(currentSection()));

			progressText.push_back(tr("Signals: %1 / %2, Files: %3 / %4")
								   .arg(signalIndex()).arg(signalsCount())
								   .arg(fileIndex()).arg(filesCount()));

			QString objectName = currentObjectName();
			if (objectName.length() > 48)
			{
				objectName = "..." + objectName.right(48);
			}

			progressText.push_back(tr("Current: %1").arg(objectName));

			progress = fileIndex() + signalIndex();
			progressMax = filesCount() + signalsCount();
		}
		break;
	case ProjectDiffGenerator::WorkerStatus::Rendering:
		{
			if (renderingReportName().isEmpty() == false)
			{
				progressText.push_back(tr("Creating the report '%1'...").arg(renderingReportName()));
			}
			else
			{
				progressText.push_back(tr("Creating a report..."));
			}

			progressText.push_back(tr("Section: %1 / %2").arg(sectionIndex()).arg(sectionCount()));

			progress = sectionIndex();
			progressMax = sectionCount();
		}
		break;
	case ProjectDiffGenerator::WorkerStatus::Printing:
		{
			if (renderingReportName().isEmpty() == false)
			{
				progressText.push_back(tr("Saving the report '%1'...").arg(renderingReportName()));
			}
			else
			{
				progressText.push_back(tr("Saving the report..."));
			}

			progressText.push_back(tr("Page: %1 / %2").arg(pageIndex()).arg(pagesCount()));

			progress = pageIndex();
			progressMax = pagesCount();
		}
		break;
	}

	l.unlock();

	emit progressChanged(progress, 0, progressMax, progressText);

	return;
}

const QString& ProjectDiffGenerator::filePath() const
{
	return m_filePath;
}

ProjectDiffGenerator::WorkerStatus ProjectDiffGenerator::currentStatus() const
{
	return m_currentStatus;
}

int ProjectDiffGenerator::signalsCount() const
{
	return m_signalsCount;
}

int ProjectDiffGenerator::signalIndex() const
{
	return m_signalIndex;
}

int ProjectDiffGenerator::filesCount() const
{
	return m_filesCount;
}

int ProjectDiffGenerator::fileIndex() const
{
	return m_fileIndex;
}

int ProjectDiffGenerator::sectionCount() const
{
	return m_sectionCount;
}

int ProjectDiffGenerator::sectionIndex() const
{
	return m_sectionIndex;
}

int ProjectDiffGenerator::pagesCount() const
{
	return m_pagesCount;
}

int ProjectDiffGenerator::pageIndex() const
{
	return m_pageIndex;
}

QString ProjectDiffGenerator::renderingReportName() const
{
	return m_currentReportName;
}

QString ProjectDiffGenerator::currentSection() const
{
	return m_currentSectionName;
}

QString ProjectDiffGenerator::currentObjectName() const
{
	return m_currentObjectName;
}

DbController* ProjectDiffGenerator::db()
{
	return &m_db;
}

void ProjectDiffGenerator::compareProject(std::map<int, std::vector<std::shared_ptr<ReportSection>>>& reportContents)
{
	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Analyzing;
		m_filesCount = 0;
		m_signalsCount = 0;
		m_currentSectionName.clear();
		m_currentObjectName.clear();
	}

	db()->disableProgress();

	db()->setHost(theSettings.serverIpAddress());
	db()->setPort(theSettings.serverPort());
	db()->setServerUsername(theSettings.serverUsername());
	db()->setServerPassword(theSettings.serverPassword());

	bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
	if (ok == false)
	{
		throw(tr("Failed to open project!"));
	}

	// Place signals to front
	//
	for (int i = 0; i < m_reportParams.fileTypeParams.size(); i++)
	{
		ProjectDiffFileTypeParams ft = m_reportParams.fileTypeParams[i];

		if (ft.fileId == applicationSignalsTypeId() && ft.selected == true && i != 0)
		{
			m_reportParams.fileTypeParams.erase(m_reportParams.fileTypeParams.begin() + i);
			m_reportParams.fileTypeParams.insert(m_reportParams.fileTypeParams.begin(), ft);
			break;
		}
	}

	// Get and count files
	//
	m_sourceDeviceObjectMap.clear();
	m_targetDeviceObjectMap.clear();

	int filesCount = 0;

	std::vector<DbFileTree> filesTrees;

	for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
	{
		if (ft.selected == false)
		{
			continue;
		}

		if (ft.fileId == applicationSignalsTypeId())
		{
			// This is not a file
			continue;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentSectionName = ft.caption;
		}

		filesTrees.push_back({});

		DbFileTree* filesTree = &filesTrees[filesTrees.size() - 1];

		ok = db()->getFileListTree(filesTree, ft.fileId, false/*removeDeleted*/, nullptr);
		if (ok == false)
		{
			throw(tr("DbController::getFileListTree failed on fileId = %1").arg(ft.fileId));
		}

		filesCount += static_cast<int>(filesTree->files().size());

	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_filesCount = filesCount;
	}

	// Process Files

	int fileTreeIndex = 0;

	for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
	{
		if (ft.selected == false)
		{
			continue;
		}

		if (m_stop == true)
		{
			return;
		}

		std::vector<std::shared_ptr<ReportSection>> fileTypeSections;

		// Print section name

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentSectionName = ft.caption;
		}

		std::shared_ptr<ReportSection> headerSection = std::make_shared<ReportSection>(QString());

		setFont(m_normalFont);

		saveFormat();
		setTextAlignment(Qt::AlignHCenter);
		setFont(m_headerFont);
		headerSection->addText(tr("%1\n\n").arg(ft.caption), currentCharFormat(), currentBlockFormat());
		restoreFormat();

		std::shared_ptr<ReportTable> headerTable;

		if (ft.fileId == ProjectDiffGenerator::applicationSignalsTypeId())
		{
			// This is application signals

			{
				QMutexLocker l(&m_statisticsMutex);
				m_currentObjectName.clear();
			}

			saveFormat();
			setFont(m_tableFont);
			headerTable = headerSection->addTable({tr("Signal"), tr("Status"), tr("Changeset"), tr("User"), tr("Date")},
												  {45, 10, 10, 15, 20},
												  currentCharFormat());
			restoreFormat();

			compareSignals(m_reportParams.compareData, headerTable.get(), &fileTypeSections);
		}
		else
		{
			saveFormat();
			setFont(m_tableFont);
			headerTable = headerSection->addTable({tr("Object"), tr("Status"), tr("Changeset"), tr("User"), tr("Date")},
												  {45, 10, 10, 15, 20},
												  currentCharFormat());
			restoreFormat();

			// Compare files

			const DbFileTree& filesTree = filesTrees[fileTreeIndex++];

			const std::vector<std::shared_ptr<DbFileInfo>>& children = filesTree.children(filesTree.rootFileId());

			for (const auto& child : children)
			{
				compareFilesRecursive(ft.fileId, filesTree, child, m_reportParams.compareData, headerTable.get(), &fileTypeSections);
			}
		}

		// Remove header if no data
		//
		if (headerTable->rowCount() > 0)
		{
			headerTable->sortByColumn(0);

			std::sort(fileTypeSections.begin(), fileTypeSections.end(), [](const std::shared_ptr<ReportSection>& a, const std::shared_ptr<ReportSection>& b)
			{

				bool aSchema = a->schema() != nullptr;
				bool bSchema = b->schema() != nullptr;

				if (aSchema != bSchema)
				{
					return aSchema > bSchema;
				}

				return a->caption() < b->caption();
			});

			fileTypeSections.insert(fileTypeSections.begin(), headerSection);

			if (m_reportParams.multipleFiles == false)
			{
				std::vector<std::shared_ptr<ReportSection>>& all = reportContents[0];

				all.insert(all.end(), fileTypeSections.begin(), fileTypeSections.end());
			}
			else
			{
				reportContents[ft.fileId] = fileTypeSections;
			}
		}
	}

	db()->closeProject(nullptr);

	return;
}

void ProjectDiffGenerator::compareFilesRecursive(int rootFileId,
											  const DbFileTree& filesTree,
											  const std::shared_ptr<DbFileInfo>& fi,
											  const CompareData& compareData,
											  ReportTable* const headerTable,
											  std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (m_stop == true)
	{
		return;
	}

	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	if (fi == nullptr)
	{
		Q_ASSERT(fi);
		return;
	}


	compareFile(rootFileId, filesTree, fi, compareData, headerTable, sectionsArray);

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
			return;
		}

		compareFilesRecursive(rootFileId, filesTree, fiChild, compareData, headerTable, sectionsArray);
	}

	return;
}

void ProjectDiffGenerator::compareFile(int rootFileId,
									const DbFileTree& filesTree,
									const std::shared_ptr<DbFileInfo>& fi,
									const CompareData& compareData,
									ReportTable* const headerTable,
									std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
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

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentObjectName = fileName;
		m_fileIndex++;
	}

	// Get source file
	//
	bool sourceOk = false;
	std::shared_ptr<DbFile> sourceFile;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			sourceOk = db()->getLatestVersion(*fi, &sourceFile, nullptr);
		}
		break;
	case CompareVersionType::Changeset:
		{
			sourceOk = db()->getSpecificCopy(*fi, compareData.sourceChangeset, &sourceFile, nullptr);
		}
		break;
	case CompareVersionType::Date:
		{
			sourceOk = db()->getSpecificCopy(*fi, compareData.sourceDate, &sourceFile, nullptr);
		}
		break;
	default:
		Q_ASSERT(false);
		return;
	}

	// Get target file
	//
	bool targetOk = false;
	std::shared_ptr<DbFile> targetFile;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			targetOk = db()->getLatestVersion(*fi, &targetFile, nullptr);
		}
		break;
	case CompareVersionType::Changeset:
		{
			targetOk = db()->getSpecificCopy(*fi, compareData.targetChangeset, &targetFile, nullptr);
		}
		break;
	case CompareVersionType::Date:
		{
			targetOk = db()->getSpecificCopy(*fi, compareData.targetDate, &targetFile, nullptr);
		}
		break;
	default:
		Q_ASSERT(false);
		return;
	}

	if (sourceOk == true && sourceFile == nullptr)
	{
		Q_ASSERT(sourceFile);
		return;
	}

	if (targetOk == true && targetFile == nullptr)
	{
		Q_ASSERT(targetFile);
		return;
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

	compareFileContents(rootFileId, sourceFile, targetFile, fileName, headerTable, sectionsArray);

	return;
}

void ProjectDiffGenerator::compareFileContents(int rootFileId,
											const std::shared_ptr<DbFile>& sourceFile,
											const std::shared_ptr<DbFile>& targetFile,
											const QString& fileName,
											ReportTable* const headerTable,
											std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No files at all
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		return;
	}

	// Load hardware objects
	//
	std::shared_ptr<Hardware::DeviceObject> sourceObject;
	std::shared_ptr<Hardware::DeviceObject> targetObject;

	bool hardwareObject = isHardwareFile(fileName);

	if (hardwareObject == true)
	{
		if (sourceFile != nullptr)
		{
			sourceObject = loadDeviceObject(sourceFile, &m_sourceDeviceObjectMap);
			if (sourceObject == nullptr)
			{
				throw(tr("Failed to load source device object from: '%1'").arg(sourceFile->fileName()));
			}
		}

		if (targetFile != nullptr)
		{
			targetObject = loadDeviceObject(targetFile, &m_targetDeviceObjectMap);
			if (targetObject == nullptr)
			{
				throw(tr("Failed to load target device object from: '%1'").arg(targetFile->fileName()));
			}
		}
	}

	// Same changeset
	//
	if (sourceFile != nullptr &&
		targetFile != nullptr &&
		sourceFile->changeset() == targetFile->changeset())
	{
		return;
	}

	// File was deleted
	//
	if (sourceFile != nullptr && sourceFile->deleted() == true)
	{
		if (hardwareObject == true)
		{
			addHeaderTableItem(headerTable, sourceObject->equipmentId(), tr("Deleted"), sourceFile);
		}
		else
		{
			addHeaderTableItem(headerTable, fileName, tr("Deleted"), sourceFile);
		}
		return;
	}
	else
	{
		if (targetFile != nullptr && targetFile->deleted() == true)
		{
			if (hardwareObject == true)
			{
				addHeaderTableItem(headerTable, targetObject->equipmentId(), tr("Deleted"), targetFile);
			}
			else
			{
				addHeaderTableItem(headerTable, fileName, tr("Deleted"), targetFile);
			}
			return;
		}
	}

	// Compare contents
	//
	if (hardwareObject == true)
	{
		bool preset = rootFileId == db()->hpFileId();

		compareDeviceObjects(sourceFile, targetFile, sourceObject, targetObject, headerTable, sectionsArray, preset);
		return;
	}

	if (isConnectionFile(fileName) == true)
	{
		compareConnections(sourceFile, targetFile, headerTable, sectionsArray);
		return;
	}

	if (isBusTypeFile(fileName) == true)
	{
		compareBusTypes(sourceFile, targetFile, headerTable, sectionsArray);
		return;
	}

	if (isSchemaFile(fileName) == true)
	{
		compareSchemas(fileName, sourceFile, targetFile, headerTable, sectionsArray);
		return;
	}

	compareFilesData(sourceFile, targetFile, headerTable, sectionsArray);
	return;
}

std::shared_ptr<Hardware::DeviceObject> ProjectDiffGenerator::loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap) const
{
	if (deviceObjectMap == nullptr)
	{
		Q_ASSERT(deviceObjectMap);
		return nullptr;
	}

	std::shared_ptr<Hardware::DeviceObject> object = Hardware::DeviceObject::fromDbFile(*file);
	if (object == nullptr)
	{
		return nullptr;
	}

	// Save object to the map

	(*deviceObjectMap)[object->fileId()] = object;

	//qDebug() << object->fileId() << object->equipmentIdTemplate() << object->equipmentId();

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

			//qDebug() << parentObject->fileId() << parentObject->equipmentIdTemplate() << parentObject->equipmentId();

			object->expandEquipmentId();

			//qDebug() << object->fileId() << object->equipmentIdTemplate() << object->equipmentId();
		}
	}
	/*
	else
	{
		qDebug() << "No parent file for " << object->equipmentIdTemplate();
	}
	*/

	return object;
}

void ProjectDiffGenerator::compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile,
											 const std::shared_ptr<DbFile>& targetFile,
											 const std::shared_ptr<Hardware::DeviceObject>& sourceObject,
											 const std::shared_ptr<Hardware::DeviceObject>& targetObject,
											 ReportTable* const headerTable,
											 std::vector<std::shared_ptr<ReportSection>>* sectionsArray,
											 bool presets)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

	// No Files
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	if (sourceFile != nullptr && sourceObject == nullptr)
	{
		Q_ASSERT(sourceObject);
		return;
	}

	if (targetFile != nullptr && targetObject == nullptr)
	{
		Q_ASSERT(targetObject);
		return;
	}

	// Single object
	//
	if (sourceObject != nullptr && targetObject == nullptr)
	{
		addHeaderTableItem(headerTable, tr("%1").arg(sourceObject->equipmentId()), tr("Added"), sourceFile);
		return;
	}
	else
	{
		if (sourceObject == nullptr && targetObject != nullptr)
		{
			addHeaderTableItem(headerTable, tr("%1").arg(targetObject->equipmentId()), tr("Added"), targetFile);
			return;
		}
	}

	// Both Objects
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceObject, *targetObject, &diffs);

	if (diffs.empty() == false)
	{
		addHeaderTableItem(headerTable, tr("%1").arg(targetObject->equipmentId()), targetFile->action().text(), targetFile);

		QString equmipmentType = presets == true ? tr("Preset: ") : tr("Equipment: ");

		std::shared_ptr<ReportSection> deviceDiffSection = std::make_shared<ReportSection>(equmipmentType + targetObject->equipmentId());
		sectionsArray->push_back(deviceDiffSection);

		deviceDiffSection->addText(tr("%1: %2, %3\n\n").arg(equmipmentType).arg(targetObject->equipmentId()).arg(changesetString(targetFile)), currentCharFormat(), currentBlockFormat());

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = deviceDiffSection->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																			 {15, 15, 35, 35},
																			 currentCharFormat());
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}

	return;
}

void ProjectDiffGenerator::compareBusTypes(const std::shared_ptr<DbFile>& sourceFile,
										const std::shared_ptr<DbFile>& targetFile,
										ReportTable* const headerTable,
										std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	VFrame30::Bus sourceBus;
	VFrame30::Bus targetBus;

	bool ok = false;

	if (sourceFile != nullptr)
	{
		ok = sourceBus.Load(sourceFile->data());
		if (ok == false)
		{
			throw(tr("Failed to load source bus from: '%1'").arg(sourceFile->fileName()));
		}
	}

	if (targetFile != nullptr)
	{
		ok = targetBus.Load(targetFile->data());
		if (ok == false)
		{
			throw(tr("Failed to load target bus from: '%1'").arg(targetFile->fileName()));
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleBus= sourceFile != nullptr ? &sourceBus : &targetBus;
		addHeaderTableItem(headerTable, singleBus->busTypeId(), tr("Added"), singleFile);
		return;
	}

	// Both Files
	//
	// Create tables

	saveFormat();
	setFont(m_tableFont);
	std::shared_ptr<ReportTable> busDiffTable = ReportSection::createTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																		   {15, 15, 35, 35},
																		   currentCharFormat());
	restoreFormat();

	saveFormat();
	setFont(m_tableFont);
	std::shared_ptr<ReportTable> busSignalsDiffTable = ReportSection::createTable({tr("SignalID"), tr("Caption"), tr("Status")},
																				  {35, 15, 50},
																				  currentCharFormat());
	restoreFormat();

	std::vector<PropertyDiff> busDiffs;

	// Compare bus properties

	comparePropertyObjects(sourceBus, targetBus, &busDiffs);

	if (busDiffs.empty() == false)
	{
		fillDiffTable(busDiffTable.get(), busDiffs);
	}

	std::map<QString, std::shared_ptr<ReportTable>> busSignalsPropertiesTables;

	// Compare bus signals

	for (const VFrame30::BusSignal& targetBusSignal : targetBus.busSignals())
	{
		bool busSignalFound = false;

		for (const VFrame30::BusSignal& sourceBusSignal : sourceBus.busSignals())
		{
			if (targetBusSignal.signalId() == sourceBusSignal.signalId())
			{
				std::vector<PropertyDiff> busSignalDiffs;

				comparePropertyObjects(sourceBusSignal, targetBusSignal, &busSignalDiffs);

				if (busSignalDiffs.empty() == false)
				{
					saveFormat();
					setFont(m_tableFont);
					std::shared_ptr<ReportTable> busSignalsPropertiesDiffTable = ReportSection::createTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																											{15, 15, 35, 35},
																											currentCharFormat());
					restoreFormat();

					busSignalsPropertiesTables[targetBusSignal.signalId()] = busSignalsPropertiesDiffTable;

					fillDiffTable(busSignalsPropertiesDiffTable.get(), busSignalDiffs);

					busSignalsDiffTable->insertRow({targetBusSignal.signalId(), targetBusSignal.caption(), tr("Modified")});
				}

				busSignalFound = true;
				break;
			}
		}

		if (busSignalFound == false)
		{
			// Bus signal was added
			busSignalsDiffTable->insertRow({targetBusSignal.signalId(), targetBusSignal.caption(), tr("Added")});
		}
	}

	for (const VFrame30::BusSignal& sourceBusSignal : sourceBus.busSignals())
	{
		bool busSignalFound = false;

		for (const VFrame30::BusSignal& targetBusSignal : targetBus.busSignals())
		{
			if (targetBusSignal.signalId() == sourceBusSignal.signalId())
			{
				busSignalFound = true;
				break;
			}
		}

		if (busSignalFound == false)
		{
			// Bus signal was deleted
			busSignalsDiffTable->insertRow({sourceBusSignal.signalId(), sourceBusSignal.caption(), tr("Deleted")});
		}
	}

	// Add tables to section

	if (busDiffTable->rowCount() > 0 || busSignalsDiffTable->rowCount() > 0 || busSignalsPropertiesTables.empty() == false)
	{
		addHeaderTableItem(headerTable, targetBus.busTypeId(), targetFile->action().text(), targetFile);

		// Add tables to section

		std::shared_ptr<ReportSection> busDiffSection = std::make_shared<ReportSection>(tr("Bus: ") + targetBus.busTypeId());
		sectionsArray->push_back(busDiffSection);

		busDiffSection->addText(tr("Bus: %1, %2\n\n").arg(targetBus.busTypeId()).arg(changesetString(targetFile)), currentCharFormat(), currentBlockFormat());

		if (busDiffTable->rowCount() != 0)
		{
			busDiffSection->addTable(busDiffTable);
			busDiffSection->addText("\n", currentCharFormat(), currentBlockFormat());
		}

		if (busSignalsDiffTable->rowCount() != 0)
		{
			busSignalsDiffTable->sortByColumn(0);

			busDiffSection->addText(tr("Bus %1 signals:\n\n").arg(targetBus.busTypeId()), currentCharFormat(), currentBlockFormat());
			busDiffSection->addTable(busSignalsDiffTable);
			busDiffSection->addText("\n", currentCharFormat(), currentBlockFormat());
		}

		for (auto it : busSignalsPropertiesTables)
		{
			const QString& signalId = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			busDiffSection->addText(tr("Bus: %1, signal: %2\n\n").arg(targetBus.busTypeId()).arg(signalId), currentCharFormat(), currentBlockFormat());
			busDiffSection->addTable(itemDiffTable);
		}
	}

	return;
}


void ProjectDiffGenerator::compareSchemas(const QString& fileName,
									   const std::shared_ptr<DbFile>& sourceFile,
									   const std::shared_ptr<DbFile>& targetFile,
									   ReportTable* const headerTable,
									   std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

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
		// Schema was added - just draw it

		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		addHeaderTableItem(headerTable, fileName, tr("Added"), singleFile);

		std::shared_ptr<VFrame30::Schema> singleSchema = VFrame30::Schema::Create(singleFile->data());
		if (singleSchema == nullptr)
		{
			throw(tr("Failed to load schema from: '%1'").arg(singleFile->fileName()));
		}

		// Add schema drawing

		std::shared_ptr<ReportSection> schemaDrawingSection = std::make_shared<ReportSection>(tr("Schema: ") + fileName);
		sectionsArray->push_back(schemaDrawingSection);

		schemaDrawingSection->addText(tr("Schema: %1, %2\n").arg(singleSchema->schemaId()).arg(changesetString(singleFile)), currentCharFormat(), currentBlockFormat());
		schemaDrawingSection->setSchema(singleSchema);

		return;
	}

	// Both Files
	//
	std::shared_ptr<VFrame30::Schema> sourceSchema = VFrame30::Schema::Create(sourceFile->data());
	if (sourceSchema == nullptr)
	{
		throw(tr("Failed to load source schema from: '%1'").arg(sourceFile->fileName()));
	}

	std::shared_ptr<VFrame30::Schema> targetSchema = VFrame30::Schema::Create(targetFile->data());
	if (targetSchema == nullptr)
	{
		throw(tr("Failed to load target schema from: '%1'").arg(targetFile->fileName()));
	}

	// Create tables

	saveFormat();
	setFont(m_tableFont);
	std::shared_ptr<ReportTable> schemaDiffTable = ReportSection::createTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																			  {15, 15, 35, 35},
																			  currentCharFormat());
	restoreFormat();

	saveFormat();
	setFont(m_tableFont);
	std::shared_ptr<ReportTable> schemaItemsDiffTable = ReportSection::createTable({tr("Type"), tr("Label"), tr("Layer"), tr("Status")},
																				   {25, 35, 25, 15},
																				   currentCharFormat());
	restoreFormat();

	// Compare schemas properties

	std::vector<PropertyDiff> schemaDiffs;
	comparePropertyObjects(*sourceSchema, *targetSchema, &schemaDiffs);

	fillDiffTable(schemaDiffTable.get(), schemaDiffs);

	// Compare schemas items properties

	std::map<QUuid, ReportSchemaCompareAction> itemsActions;

	std::map<std::shared_ptr<VFrame30::SchemaItem>, std::shared_ptr<ReportTable>> itemsTables;

	for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : targetSchema->Layers)
	{
		for (SchemaItemPtr targetItem : targetLayer->Items)
		{
			// Look for this item in source
			//
			SchemaItemPtr sourceItem = sourceSchema->getItemById(targetItem->guid());

			if (sourceItem != nullptr)
			{
				// Item is found, so it was modified
				//

				std::vector<PropertyDiff> itemDiffs;

				comparePropertyObjects(*sourceItem, *targetItem, &itemDiffs);

				// Check if properties where modified
				//
				if (itemDiffs.empty() == true)
				{
					// Check if position was changed
					//
					std::vector<VFrame30::SchemaPoint> sourcePoints = sourceItem->getPointList();
					std::vector<VFrame30::SchemaPoint> targetPoints = targetItem->getPointList();

					if (sourcePoints == targetPoints)
					{
						itemsActions[targetItem->guid()] = ReportSchemaCompareAction::Unmodified;
					}
					else
					{
						itemsActions[targetItem->guid()] = ReportSchemaCompareAction::Modified;
					}
				}
				else
				{
					itemsActions[targetItem->guid()] = ReportSchemaCompareAction::Modified;
				}

				// Save properties to table

				if (itemDiffs.empty() == false)
				{
					QString className(targetItem->metaObject()->className());
					className.remove("VFrame30::");

					schemaItemsDiffTable->insertRow({tr("%1").arg(className), targetItem->label(), targetLayer->name(), tr("Modified")});

					saveFormat();
					setFont(m_tableFont);
					std::shared_ptr<ReportTable> itemDiffTable = ReportSection::createTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																							{15, 15, 35, 35},
																							currentCharFormat());
					restoreFormat();

					fillDiffTable(itemDiffTable.get(), itemDiffs);

					itemsTables[targetItem] = itemDiffTable;
				}

				continue;
			}

			if (sourceItem == nullptr)
			{
				// Item was added to targer
				//
				itemsActions[targetItem->guid()] = ReportSchemaCompareAction::Added;

				QString className(targetItem->metaObject()->className());
				className.remove("VFrame30::");

				schemaItemsDiffTable->insertRow({tr("%1").arg(className), targetItem->label(), targetLayer->name(), tr("Added")});

				continue;
			}
		}
	}

	// Look for deteled items (in target)
	//
	for (std::shared_ptr<VFrame30::SchemaLayer> sourceLayer : sourceSchema->Layers)
	{
		for (SchemaItemPtr sourceItem : sourceLayer->Items)
		{
			// Look for this item in source
			//
			SchemaItemPtr targetItem = targetSchema->getItemById(sourceItem->guid());

			if (targetItem == nullptr)
			{
				// Item is found, so it was deleted in target
				//
				itemsActions[sourceItem->guid()] = ReportSchemaCompareAction::Deleted;

				QString className(sourceItem->metaObject()->className());
				className.remove("VFrame30::");

				schemaItemsDiffTable->insertRow({tr("%1").arg(className), sourceItem->label(), sourceLayer->name(), tr("Deleted")});

				// Add item to target
				//
				bool layerFound = false;
				for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : targetSchema->Layers)
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						targetLayer->Items.push_back(sourceItem);
						layerFound = true;
						break;
					}
				}

				Q_ASSERT(layerFound);
			}
		}
	}

	if (schemaDiffTable->rowCount() > 0 || schemaItemsDiffTable->rowCount() > 0 || itemsTables.empty() == false)
	{
		addHeaderTableItem(headerTable, fileName, targetFile->action().text(), targetFile);

		// Add schema differences drawing

		std::shared_ptr<ReportSection> schemaDrawingSection = std::make_shared<ReportSection>(tr("Schema: ") + fileName);
		sectionsArray->push_back(schemaDrawingSection);

		QString schemaId = targetSchema->schemaId();

		schemaDrawingSection->addText(tr("Schema: %1, %2\n").arg(schemaId).arg(changesetString(targetFile)), currentCharFormat(), currentBlockFormat());
		schemaDrawingSection->setSchema(targetSchema);
		schemaDrawingSection->setCompareItemActions(itemsActions);

		// Add schema differences tables

		std::shared_ptr<ReportSection> schemaDiffSection = std::make_shared<ReportSection>(tr("Schema: ") + fileName);
		sectionsArray->push_back(schemaDiffSection);

		if (schemaDiffTable->rowCount() != 0)
		{
			schemaDiffSection->addText(tr("Schema %1 properties:\n\n").arg(schemaId), currentCharFormat(), currentBlockFormat());
			schemaDiffSection->addTable(schemaDiffTable);
			schemaDiffSection->addText("\n", currentCharFormat(), currentBlockFormat());
		}

		if (schemaItemsDiffTable->rowCount() != 0)
		{
			schemaItemsDiffTable->sortByColumn(1);

			schemaDiffSection->addText(tr("Schema %1 items:\n\n").arg(schemaId), currentCharFormat(), currentBlockFormat());
			schemaDiffSection->addTable(schemaItemsDiffTable);
			schemaDiffSection->addText("\n", currentCharFormat(), currentBlockFormat());
		}

		for (auto it : itemsTables)
		{
			const std::shared_ptr<VFrame30::SchemaItem>& item = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			QString className(item->metaObject()->className());
			className.remove("VFrame30::");

			if (item->label().isEmpty() == true)
			{
				schemaDiffSection->addText(tr("%1 (no label):\n\n").arg(className), currentCharFormat(), currentBlockFormat());
			}
			else
			{
				schemaDiffSection->addText(tr("%1 %2:\n\n").arg(className).arg(item->label()), currentCharFormat(), currentBlockFormat());
			}
			schemaDiffSection->addTable(itemDiffTable);
			schemaDiffSection->addText(tr("\n"), currentCharFormat(), currentBlockFormat());
		}
	}
}

void ProjectDiffGenerator::compareConnections(const std::shared_ptr<DbFile>& sourceFile,
										   const std::shared_ptr<DbFile>& targetFile,
										   ReportTable* const headerTable,
										   std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

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
			throw(tr("Failed to load source connection from: '%1'").arg(sourceFile->fileName()));
		}
	}
	if (targetFile != nullptr)
	{
		bool ok = targetConnection.Load(targetFile->data());
		if (ok == false)
		{
			throw(tr("Failed to load target connection from: '%1'").arg(targetFile->fileName()));
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleConnection = sourceFile != nullptr ? &sourceConnection : &targetConnection;
		addHeaderTableItem(headerTable, singleConnection->connectionID(), tr("Added"), singleFile);
		return;
	}

	// Both Files
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceConnection, targetConnection, &diffs);

	if (diffs.empty() == false)
	{
		std::shared_ptr<ReportSection> connectionDiffSection = std::make_shared<ReportSection>(tr("Connection: ") + targetConnection.connectionID());
		sectionsArray->push_back(connectionDiffSection);

		connectionDiffSection->addText(tr("Connection: %1, %2\n\n").arg(targetConnection.connectionID()).arg(changesetString(targetFile)), currentCharFormat(), currentBlockFormat());

		addHeaderTableItem(headerTable, targetConnection.connectionID(), targetFile->action().text(), targetFile);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = connectionDiffSection->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																				 {15, 15, 35, 35},
																				 currentCharFormat());
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}
}

void ProjectDiffGenerator::compareFilesData(const std::shared_ptr<DbFile>& sourceFile,
										 const std::shared_ptr<DbFile>& targetFile,
										 ReportTable* const headerTable,
										 std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

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
		addHeaderTableItem(headerTable, singleFile->fileName(), tr("Added"), singleFile);
		return;
	}

	if (sourceFile->data() == targetFile->data())
	{
		return;
	}

	addHeaderTableItem(headerTable, targetFile->fileName(), targetFile->action().text(), targetFile);

	std::shared_ptr<ReportSection> fileDiffSection = std::make_shared<ReportSection>(tr("File: ") + targetFile->fileName());
	sectionsArray->push_back(fileDiffSection);

	// Both Files
	//
	if (isTextFile(targetFile->fileName()) == true)
	{
		fileDiffSection->addText(tr("File: %1, %2\n\n").arg(targetFile->fileName()).arg(changesetString(targetFile)), currentCharFormat(), currentBlockFormat());

		std::vector<FileDiff::FileLine> fileLinesSource;
		std::vector<FileDiff::FileLine> fileLinesTarget;
		std::vector<FileDiff::FileLine> fileLinesCommon;

		/*
		{
			QFile f1("d:\\Source" + targetFile->fileName());
			f1.open(QFile::WriteOnly);
			f1.write(sourceFile->data());

			QFile f2("d:\\Target" + targetFile->fileName());
			f2.open(QFile::WriteOnly);
			f2.write(targetFile->data());
		}
		*/

		FileDiff::loadFileData(sourceFile->data(), &fileLinesSource);
		FileDiff::loadFileData(targetFile->data(), &fileLinesTarget);

		fileLinesCommon.reserve(static_cast<int>(fileLinesSource.size() + fileLinesTarget.size()));

		FileDiff::calculateLcs(fileLinesSource, fileLinesTarget, &fileLinesCommon);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = fileDiffSection->addTable({tr("Line"), tr("Source"), tr("Line"), tr("Target")},
																		   {10, 40, 10, 40},
																		   currentCharFormat());
		restoreFormat();

		std::vector<FileDiff::FileLine> fileLinesSourceAligned;
		std::vector<FileDiff::FileLine> fileLinesTargetAligned;
		std::vector<FileDiff::FileDiffAction> fileLinesActions;

		int addedCount = 0;
		int removedCount = 0;
		int alignedCount = 0;

		FileDiff::alignResults(fileLinesSource, fileLinesTarget,
							   fileLinesCommon,
							   &fileLinesSourceAligned, &fileLinesTargetAligned,
							   &fileLinesActions,
							   &addedCount,
							   &removedCount,
							   &alignedCount);

		int sourceIndex = 0;
		int targetIndex  = 0;
		int actionIndex = 0;

		while (sourceIndex < fileLinesSourceAligned.size() || targetIndex < fileLinesTargetAligned.size())
		{
			const FileDiff::FileLine& sourceLine = fileLinesSourceAligned[sourceIndex++];
			const FileDiff::FileLine& targetLine = fileLinesTargetAligned[targetIndex++];
			const FileDiff::FileDiffAction action = fileLinesActions[actionIndex++];

			if (action == FileDiff::FileDiffAction::Match)
			{
				continue;
			}

			if (sourceLine.line == -1)
			{
				diffTable->insertRow({QString(), QString(), tr("%1").arg(targetLine.line), targetLine.text.trimmed()});
			}
			else
			{
				if (targetLine.line == -1)
				{
					diffTable->insertRow({tr("%1").arg(sourceLine.line), sourceLine.text.trimmed(), QString(), QString()});
				}
				else
				{
					diffTable->insertRow({tr("%1").arg(sourceLine.line), sourceLine.text.trimmed(), tr("%1").arg(targetLine.line), targetLine.text.trimmed()});
				}
			}
		}
	}
	else
	{
		// Other file
		//

		QString str = tr("File %1: binary data modified.").arg(targetFile->fileName());

		if (sourceFile->data().size() != targetFile->data().size())
		{
			str += tr(" Size changed: %1 -> %2 bytes.").arg(sourceFile->data().size()).arg(targetFile->data().size());
		}

		fileDiffSection->addText(str + "\n", currentCharFormat(), currentBlockFormat());
	}
}

void ProjectDiffGenerator::compareSignals(const CompareData& compareData,
									   ReportTable* const headerTable,
									   std::vector<std::shared_ptr<ReportSection> >* sectionsArray)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	bool ok = false;

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::RequestingSignals;
	}

	// Get source signals
	//

	std::vector<Signal> sourceSignalsVec;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			SignalSet signalsSet;

			ok = db()->getSignals(&signalsSet, true/*excludeDeleted*/, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSignals failed."));
			}

			int count = signalsSet.count();

			sourceSignalsVec.reserve(count);

			for (int i = 0; i < count; i++)
			{
				sourceSignalsVec.push_back(sourceSignalsVec[i]);
			}
		}
		break;
	case CompareVersionType::Changeset:
		{
			ok = db()->getSpecificSignals(compareData.sourceChangeset, &sourceSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSpecificSignals failed."));
			}
		}
		break;
	case CompareVersionType::Date:
		{
			ok = db()->getSpecificSignals(compareData.sourceDate, &sourceSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSpecificSignals failed."));
			}
		}
		break;
	default:
		Q_ASSERT(false);
		return;
	}

	// Get target signals
	//

	std::vector<Signal>  targetSignalsVec;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			SignalSet signalsSet;

			ok = db()->getSignals(&signalsSet, true/*excludeDeleted*/, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSignals failed."));
			}

			int count = signalsSet.count();

			targetSignalsVec.reserve(count);

			for (int i = 0; i < count; i++)
			{
				targetSignalsVec.push_back(signalsSet[i]);
			}
		}
		break;
	case CompareVersionType::Changeset:
		{
			ok = db()->getSpecificSignals(compareData.targetChangeset, &targetSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSpecificSignals failed."));
			}
		}
		break;
	case CompareVersionType::Date:
		{
			ok = db()->getSpecificSignals(compareData.targetDate, &targetSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSpecificSignals failed."));
			}
		}
		break;
	default:
		Q_ASSERT(false);
		return;
	}

	// Build signal maps

	std::map<Hash, Signal*> sourceSignals;
	std::map<Hash, Signal*> targetSignals;

	std::map<Hash, int> allHashesMap;

	for (Signal& s : sourceSignalsVec)
	{
		if (s.deleted() == true)
		{
			continue;
		}

		Hash hash = ::calcHash(s.appSignalID());

		allHashesMap[hash] = 1;

		sourceSignals[::calcHash(s.appSignalID())] = &s;
	}

	for (Signal& s : targetSignalsVec)
	{
		if (s.deleted() == true)
		{
			continue;
		}

		Hash hash = ::calcHash(s.appSignalID());

		allHashesMap[hash] = 1;

		targetSignals[::calcHash(s.appSignalID())] = &s;
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Analyzing;
		m_signalsCount = static_cast<int>(allHashesMap.size());
	}

	for (auto it = targetSignals.begin(); it != targetSignals.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		Hash hash = it->first;

		const Signal* targetSignal = it->second;
		if (targetSignal == nullptr)
		{
			Q_ASSERT(targetSignal);
			return;
		}

		const Signal* sourceSignal = nullptr;

		auto itSource = sourceSignals.find(hash);
		if (itSource != sourceSignals.end())
		{
			sourceSignal = itSource->second;
		}

		QString appSignalID = targetSignal->appSignalID();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_signalIndex++;
			m_currentObjectName = appSignalID;
		}

		// Only target signal exists
		//
		if (sourceSignal == nullptr)
		{
			addHeaderTableItem(headerTable, appSignalID, tr("Added"), *targetSignal);
			continue;
		}

		// Both signals exist
		//
		if (sourceSignal->changesetID() != targetSignal->changesetID())
		{
			// Compare contents
			//
			if (sourceSignal->deleted() == true)
			{
				Q_ASSERT(false);
				//headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(*sourceSignal)});
			}
			else
			{
				if (targetSignal->deleted() == true)
				{
					Q_ASSERT(false);
					//headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(*targetSignal)});
				}
				else
				{
					bool swap = false;

					// Target changeset should be later or checked-out
					//
					if (sourceSignal->changesetID() == 0 ||  targetSignal->changesetID() == 0)
					{
						// One of files is checked out
						//
						if (sourceSignal->changesetID() == 0 &&  targetSignal->changesetID() != 0)
						{
							swap = true;
						}
					}
					else
					{
						if (sourceSignal->changesetID() >  targetSignal->changesetID())
						{
							swap = true;
						}
					}

					if (swap == true)
					{
						std::swap(sourceSignal, targetSignal);
					}

					compareSignalContents(*sourceSignal, *targetSignal, headerTable, sectionsArray);
				}
			}
		}
	}

	// Process deleted signals

	for (auto it = sourceSignals.begin(); it != sourceSignals.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		Hash hash = it->first;

		const Signal* sourceSignal = it->second;
		if (sourceSignal == nullptr)
		{
			Q_ASSERT(sourceSignal);
			return;
		}

		auto itSource = targetSignals.find(hash);
		if (itSource == targetSignals.end())
		{
			QString appSignalID = sourceSignal->appSignalID();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_signalIndex++;
				m_currentObjectName = appSignalID;
			}

			addHeaderTableItem(headerTable, appSignalID, tr("Deleted"), *sourceSignal);
		}
	}

	return;
}

void ProjectDiffGenerator::compareSignalContents(const Signal& sourceSignal,
											  const Signal& targetSignal,
											  ReportTable* const headerTable,
											  std::vector<std::shared_ptr<ReportSection>>* sectionsArray)
{
	if (headerTable == nullptr || sectionsArray == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(sectionsArray);
		return;
	}

	SignalProperties sourceProperties(sourceSignal);
	SignalProperties targetProperties(targetSignal);

	auto p = sourceProperties.propertyByCaption(SignalProperties::changesetIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(SignalProperties::instanceCreatedCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(SignalProperties::signalInstanceIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(SignalProperties::signalGroupIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}

	p = targetProperties.propertyByCaption(SignalProperties::changesetIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(SignalProperties::instanceCreatedCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(SignalProperties::signalInstanceIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(SignalProperties::signalGroupIDCaption);
	if (p != nullptr)
	{
		p->setExpert(true);
	}

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceProperties, targetProperties, &diffs);

	if (diffs.empty() == false)
	{
		std::shared_ptr<ReportSection> signalDiffSection = std::make_shared<ReportSection>(tr("Signal: ") + targetSignal.appSignalID());
		sectionsArray->push_back(signalDiffSection);

		signalDiffSection->addText(tr("Signal: %1, %2\n\n").arg(targetSignal.appSignalID()).arg(changesetString(targetSignal)), currentCharFormat(), currentBlockFormat());

		addHeaderTableItem(headerTable, targetSignal.appSignalID(), targetSignal.instanceAction().text(), targetSignal);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = signalDiffSection->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")},
																			 {15, 15, 35, 35},
																			 currentCharFormat());
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}
}

void ProjectDiffGenerator::comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	std::vector<std::shared_ptr<Property>> sourceProperties = sourceObject.properties();
	std::vector<std::shared_ptr<Property>> targetProperties = targetObject.properties();

	std::map<Hash, QString> allPropertiesMap;
	std::map<Hash, std::shared_ptr<Property>> sourcePropertyMap;
	std::map<Hash, std::shared_ptr<Property>> targetPropertyMap;

	for (std::shared_ptr<Property> p : sourceProperties)
	{
		// Skip expert properties
		//
		if (p->expert() == true && m_reportParams.expertProperties == false)
		{
			continue;
		}

		Hash hash = ::calcHash(p->caption());

		allPropertiesMap[hash] = p->caption();

		sourcePropertyMap[hash] = p;
	}

	for (std::shared_ptr<Property> p : targetProperties)
	{
		// Skip expert properties
		//
		if (p->expert() == true && m_reportParams.expertProperties == false)
		{
			continue;
		}

		Hash hash = ::calcHash(p->caption());

		allPropertiesMap[hash] = p->caption();

		targetPropertyMap[hash] = p;
	}

	result->reserve(allPropertiesMap.size());

	for (auto it : allPropertiesMap)
	{
		Hash hash = it.first;

		PropertyDiff diff;
		diff.caption =  it.second;	// Name

		auto itSource = sourcePropertyMap.find(hash);
		auto itTarget = targetPropertyMap.find(hash);

		if (itSource != sourcePropertyMap.end() && itTarget == targetPropertyMap.end())
		{
			// Exists only in source
			//
			diff.action = PropertyDiff::Action::Removed;
			result->push_back(diff);
			continue;
		}

		if (itSource == sourcePropertyMap.end() && itTarget != targetPropertyMap.end())
		{
			// Exists only in target
			//
			diff.action = PropertyDiff::Action::Added;

			std::shared_ptr<Property> tp = targetPropertyMap[hash];
			if (tp == nullptr)
			{
				Q_ASSERT(tp);
				continue;
			}

			diff.newValue = tp->value();
			diff.newValueText = ExtWidgets::PropertyTools::propertyValueText(tp.get(), -1);

			result->push_back(diff);
			continue;
		}

		// Exists in both

		Q_ASSERT(itSource != sourcePropertyMap.end() && itTarget != targetPropertyMap.end());

		diff.action = PropertyDiff::Action::Modified;

		std::shared_ptr<Property> sp = sourcePropertyMap[hash];
		std::shared_ptr<Property> tp = targetPropertyMap[hash];

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

void ProjectDiffGenerator::generateTitlePage(QTextCursor* textCursor, const CompareData& compareData, const QString& projectName, const QString& userName, const QString& subreportName)
{
	if (textCursor == nullptr)
	{
		Q_ASSERT(textCursor);
		return;
	}

	QTextBlockFormat blockCenterFormat = textCursor->blockFormat();
	blockCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat blockLeftFormat = textCursor->blockFormat();
	blockLeftFormat.setAlignment(Qt::AlignLeft);

	QTextCharFormat charHeaderFormat = textCursor->charFormat();
	charHeaderFormat.setFontFamily("Times");
	charHeaderFormat.setFontPointSize(72.0);
	//charHeaderFormat.setFontWeight(static_cast<int>(QFont::Bold));

	QTextCharFormat charRegularFormat = textCursor->charFormat();
	charRegularFormat.setFontFamily("Times");
	charRegularFormat.setFontPointSize(36.0);

	// Report Title

	textCursor->setBlockFormat(blockCenterFormat);

	textCursor->setCharFormat(charHeaderFormat);

	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	textCursor->insertText(QObject::tr("Differences Report\n"));

	textCursor->insertText("\n");

	textCursor->insertText(QObject::tr("Project: %1\n").arg(projectName));

	textCursor->insertText("\n");

	if (subreportName.isEmpty() == false)
	{
		textCursor->insertText(QObject::tr("%1\n").arg(subreportName));
	}
	else
	{
		textCursor->insertText("\n");
	}

	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	// User name

	textCursor->setBlockFormat(blockLeftFormat);

	textCursor->setCharFormat(charRegularFormat);

	textCursor->insertText(QObject::tr("User Name: %1\n").arg(userName));

	textCursor->insertText("\n");
	textCursor->insertText("\n");

	// Changeset

	QString changesetStr;

	switch(compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion: changesetStr = tr("Source: Latest Version"); break;
	case CompareVersionType::Changeset: changesetStr = tr("Source Changeset: #%1").arg(compareData.sourceChangeset); break;
	case CompareVersionType::Date: changesetStr = tr("Source Date: %1").arg(compareData.sourceDate.toString("dd/MM/yyyy HH:mm:ss")); break;
	}

	textCursor->insertText(tr("%1\n").arg(changesetStr));

	switch(compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion: changesetStr = tr("Target: Latest Version"); break;
	case CompareVersionType::Changeset: changesetStr = tr("Target Changeset: #%1").arg(compareData.targetChangeset); break;
	case CompareVersionType::Date: changesetStr = tr("Target Date: %1").arg(compareData.targetDate.toString("dd/MM/yyyy HH:mm:ss")); break;
	}
	textCursor->insertText(tr("%1\n").arg(changesetStr));

	textCursor->insertText("\n");
	textCursor->insertText("\n");

	/*
	// Files count

	if (m_filesCount > 0)
	{
		textCursor->insertText(tr("Files compared: %1\n").arg(m_filesCount));
	}
	else
	{
		textCursor->insertText("\n");
	}

	if (m_signalsCount > 0)
	{
		textCursor->insertText(tr("Signals compared: %1\n").arg(m_signalsCount));
	}
	else
	{
		textCursor->insertText("\n");
	}*/
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	// RPCT Version

	textCursor->setBlockFormat(blockCenterFormat);

	textCursor->insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));

	textCursor->insertText("\n");

	textCursor->insertText(tr("%1 version %2\n").arg(qApp->applicationName()).arg(qApp->applicationVersion()));

	return;
}

void ProjectDiffGenerator::generateReportFilesPage(QTextCursor* textCursor, const QStringList& subreportFiles)
{
	if (textCursor == nullptr)
	{
		Q_ASSERT(textCursor);
		return;
	}

	QTextBlockFormat blockCenterFormat = textCursor->blockFormat();
	blockCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat blockLeftFormat = textCursor->blockFormat();
	blockLeftFormat.setAlignment(Qt::AlignLeft);

	QTextCharFormat charRegularFormat = textCursor->charFormat();
	charRegularFormat.setFontFamily("Times");
	charRegularFormat.setFontPointSize(36.0);

	// Report Title

	textCursor->setBlockFormat(blockCenterFormat);

	textCursor->setCharFormat(charRegularFormat);


	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	textCursor->insertText(QObject::tr("Content of this report is stored in the following files:\n"));

	textCursor->insertText("\n");
	textCursor->insertText("\n");
	textCursor->insertText("\n");

	textCursor->setBlockFormat(blockLeftFormat);

	for (QString fileName: subreportFiles)
	{
		fileName = QDir::toNativeSeparators(fileName);

		int pos = fileName.lastIndexOf(QDir::separator());
		if (pos != -1)
		{
			fileName = fileName.right(fileName.length() - pos - 1);
		}

		textCursor->insertText(QObject::tr("%1\n").arg(fileName));

		textCursor->insertText("\n");
	}

	return;
}

void ProjectDiffGenerator::createMarginItems(const CompareData& compareData, const QString& subreportName)
{
	clearMarginItems();

	// Create headers/footers

	QString projectNameStr = tr("Project: ") + m_projectName;

	if (m_reportParams.multipleFiles == true && subreportName.isEmpty() == false)
	{
		projectNameStr += tr("; section: %1").arg(subreportName);
	}

	addMarginItem({projectNameStr, 2, -1, m_marginFont, Qt::AlignLeft | Qt::AlignTop});

	addMarginItem({tr("%OBJECT%"), 2, -1, m_marginFont, Qt::AlignRight | Qt::AlignTop});

	QString changesetStr;

	switch(compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion: changesetStr = tr("Source: Latest Version"); break;
	case CompareVersionType::Changeset: changesetStr = tr("Source Changeset: #%1").arg(compareData.sourceChangeset); break;
	case CompareVersionType::Date: changesetStr = tr("Source: Date %1").arg(compareData.sourceDate.toString("dd/MM/yyyy HH:mm:ss")); break;
	}

	switch(compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion: changesetStr += tr("; Target: Latest Version"); break;
	case CompareVersionType::Changeset: changesetStr += tr("; Target Changeset: #%1").arg(compareData.targetChangeset); break;
	case CompareVersionType::Date: changesetStr += tr("; Target Date: %1").arg(compareData.targetDate.toString("dd/MM/yyyy HH:mm:ss")); break;
	}

	addMarginItem({changesetStr, 2, -1, m_marginFont, Qt::AlignLeft | Qt::AlignBottom});

	addMarginItem({tr("%PAGE%"), 2, -1, m_marginFont, Qt::AlignRight | Qt::AlignBottom});

}

void ProjectDiffGenerator::fillDiffTable(ReportTable* diffTable, const std::vector<PropertyDiff>& diffs)
{
	if (diffTable == nullptr)
	{
		Q_ASSERT(diffTable);
		return;
	}

	for (const PropertyDiff& diff : diffs)
	{
		switch (diff.action)
		{

		case PropertyDiff::Action::Added:
			{
				if (diff.newValueText.length() > 256)
				{
					diffTable->insertRow({diff.caption, tr("Added"), QString(), tr("<Long Data Array>")});
				}
				else
				{
					diffTable->insertRow({diff.caption, tr("Added"), QString(), diff.newValueText});
				}
			}
			break;
		case PropertyDiff::Action::Removed:
			{
				diffTable->insertRow({diff.caption, tr("Removed"), QString(), QString()});
			}
			break;
		case PropertyDiff::Action::Modified:
			{
				if (diff.oldValueText.length() > 256 || diff.newValueText.length() > 256)
				{
					diffTable->insertRow({diff.caption, tr("Modified"), tr("<Long Data Array>"), tr("<Long Data Array>")});
					/*
					QStringList sourceText;
					QStringList targetText;

					// Compare text using diff

					std::vector<FileDiff::FileLine> fileLinesSource;
					std::vector<FileDiff::FileLine> fileLinesTarget;
					std::vector<FileDiff::FileLine> fileLinesCommon;

					FileDiff::loadFileData(diff.oldValueText.toUtf8(), &fileLinesSource);
					FileDiff::loadFileData(diff.newValueText.toUtf8(), &fileLinesTarget);

					fileLinesCommon.reserve(static_cast<int>(fileLinesSource.size() + fileLinesTarget.size()));

					FileDiff::calculateLcs(fileLinesSource, fileLinesTarget, &fileLinesCommon);

					int addedCount = 0;
					int removedCount = 0;
					int alignedCount = 0;

					std::vector<FileDiff::FileLine> fileLinesSourceAligned;
					std::vector<FileDiff::FileLine> fileLinesTargetAligned;
					std::vector<FileDiff::FileDiffAction> fileLinesActions;

					FileDiff::alignResults(fileLinesSource, fileLinesTarget,
										   fileLinesCommon,
										   &fileLinesSourceAligned, &fileLinesTargetAligned,
										   &fileLinesActions,
										   &addedCount,
										   &removedCount,
										   &alignedCount);

					int sourceIndex = 0;
					int targetIndex  = 0;
					//int actionIndex = 0;

					while (sourceIndex < fileLinesSourceAligned.size() || targetIndex < fileLinesTargetAligned.size())
					{
						const FileDiff::FileLine& sourceLine = fileLinesSourceAligned[sourceIndex];
						const FileDiff::FileLine& targetLine = fileLinesTargetAligned[targetIndex];

						if (sourceLine.line == -1)
						{
							sourceText.push_back(QString());
						}
						else
						{
							sourceText.push_back(tr("%1 %2").arg(sourceLine.line).arg(sourceLine.text.trimmed()));
						}

						if (targetLine.line == -1)
						{
							targetText.push_back(QString());
						}
						else
						{
							targetText.push_back(tr("%1 %2").arg(targetLine.line).arg(targetLine.text.trimmed()));
						}

						sourceIndex++;
						targetIndex++;
					}

					diffTable->insertRow({diff.caption, tr("Modified"), sourceText.join('\n'), targetText.join('\n')});*/
				}
				else
				{
					diffTable->insertRow({diff.caption, tr("Modified"), diff.oldValueText, diff.newValueText});
				}
			}
			break;
		}
	}

	diffTable->sortByColumn(0);

	return;
}

void ProjectDiffGenerator::addHeaderTableItem(ReportTable* const headerTable, const QString& caption, const QString& action, std::shared_ptr<DbFile> file)
{
	QString changesetStr = file->changeset() == 0 ? tr("Checked Out") : tr("%1").arg(file->changeset());

	if (headerTable == nullptr || file == nullptr)
	{
		Q_ASSERT(headerTable);
		Q_ASSERT(file);
		return;
	}

	Q_ASSERT(headerTable->columnCount() == 5);

	headerTable->insertRow({caption, action, changesetStr, db()->username(file->userId()), file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")});

	return;
}

void ProjectDiffGenerator::addHeaderTableItem(ReportTable* const headerTable, const QString& caption, const QString& action, const Signal& signal)
{
	QString changesetStr = signal.changesetID() == 0 ? tr("Checked Out") : tr("%1").arg(signal.changesetID());

	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	Q_ASSERT(headerTable->columnCount() == 5);

	headerTable->insertRow({caption, action, changesetStr, db()->username(signal.userID()), signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")});

	return;
}

QString ProjectDiffGenerator::changesetString(const std::shared_ptr<DbFile>& file)
{
	if (file->changeset() == 0)
	{
		return tr("Checked Out at %1 by %2").arg(file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(file->userId()));
	}
	else
	{
		return tr("Changeset #%1 at %2 by %3").arg(file->changeset()).arg(file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(file->userId()));
	}
}

QString ProjectDiffGenerator::changesetString(const Signal& signal)
{
	if (signal.changesetID() == 0)
	{
		return tr("Checked Out %1 by %2").arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(signal.userID()));
	}
	else
	{
		return tr("Changeset #%1 at %2 by %3").arg(signal.changesetID()).arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(signal.userID()));
	}
}

void ProjectDiffGenerator::renderReport(std::map<int, std::vector<std::shared_ptr<ReportSection>>> reportContents)
{
	QStringList generatedReportFiles;

	for (auto it = reportContents.begin(); it != reportContents.end(); it++)
	{
		const std::vector<std::shared_ptr<ReportSection>>& sections = it->second;

		// Render all objects to documents

		int sectionsPagesCount = 0;

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentStatus = WorkerStatus::Rendering;

			m_sectionCount = static_cast<int>(sections.size());
			m_sectionIndex = 0;
		}

		// Init PDF file parameters

		QString subreportName;

		QString pdfFileName = filePath();

		if (m_reportParams.multipleFiles == true)
		{
			int subreportFileType = it->first;

			for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
			{
				if (ft.fileId == subreportFileType)
				{
					subreportName = ft.caption;
					break;
				}
			}

			if (subreportName.isEmpty() == true)
			{
				Q_ASSERT(false);
				subreportName = tr("NoName");
			}

			int pos = pdfFileName.lastIndexOf('.');
			if (pos != -1)
			{
				pdfFileName.insert(pos, tr("_%1").arg(subreportName));
			}
			else
			{
				pdfFileName += tr("_%1.pdf").arg(subreportName);
			}

			pdfFileName.replace(' ', '_');

			QMutexLocker l(&m_statisticsMutex);
			m_currentReportName = subreportName;

			// Set subreport page size
			//
			bool pageSizeFound = false;
			for (const ProjectDiffFileTypeParams& ft : m_reportParams.fileTypeParams)
			{
				if (ft.fileId == subreportFileType)
				{
					setPageSize(ft.pageSize);
					setPageOrientation(ft.orientation);
					setPageMargins(ft.margins);

					pageSizeFound = true;
					break;
				}
			}
			if (pageSizeFound == false)
			{
				Q_ASSERT(false);
			}
		}
		else
		{
			// Set full report page size
			//
			setPageSize(m_reportParams.albumPageSize);
			setPageOrientation(m_reportParams.albumOrientation);
			setPageMargins(m_reportParams.albumMargins);
		}

		generatedReportFiles.push_back(pdfFileName);

		// Create PDF writer

		QPdfWriter pdfWriter(pdfFileName);
		pdfWriter.setTitle(m_projectName);
		pdfWriter.setPageSize(pageSize());
		pdfWriter.setPageOrientation(pageOrientation());
		pdfWriter.setPageMargins(pageMargins(), QPageLayout::Unit::Millimeter);
		pdfWriter.setResolution(resolution());

		QRect pageRectPixels = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		QPainter painter(&pdfWriter);

		// Create title page and margins
		//
		{
			QTextDocument titleTextDocument;

			titleTextDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

			QTextCursor textCursor(&titleTextDocument);

			// Generate title page

			generateTitlePage(&textCursor, m_reportParams.compareData, m_projectName, m_userName, subreportName);

			createMarginItems(m_reportParams.compareData, subreportName);

			printDocument(&pdfWriter, &titleTextDocument, &painter, QString(), nullptr, nullptr, 0);
		}

		// Render sections pages
		//
		for (std::shared_ptr<ReportSection> section : sections)
		{
			if (m_stop == true)
			{
				return;
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_sectionIndex++;
			}

			section->render(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

			sectionsPagesCount += section->pageCount();
		}

		// Print PDF

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentStatus = WorkerStatus::Printing;

			m_pagesCount = 1/*title page*/ + sectionsPagesCount;
			m_pageIndex = 1;
		}

		for (std::shared_ptr<ReportSection> section : sections)
		{
			if (m_stop == true)
			{
				return;
			}

			// Print Text Document

			pdfWriter.newPage();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_pageIndex++;
			}

			printDocument(&pdfWriter, section->textDocument(), &painter, section->caption(), &m_pageIndex, &m_statisticsMutex, m_pagesCount);

			// Print Schemas

			std::shared_ptr<VFrame30::Schema> schema = section->schema();
			if (schema != nullptr)
			{
				printSchema(&pdfWriter, &painter, schema, section->textDocument(), &section->compareItemActions());
			}

			// Clear text document

			section->textDocument()->clear();
		}
	}

	if (m_reportParams.multipleFiles == true)
	{
		clearMarginItems();

		// Generate generic report file with all report files description

		setPageSize(m_reportParams.albumPageSize);
		setPageOrientation(m_reportParams.albumOrientation);
		setPageMargins(m_reportParams.albumMargins);

		QPdfWriter pdfWriter(filePath());

		pdfWriter.setTitle(m_projectName);
		pdfWriter.setPageSize(pageSize());
		pdfWriter.setPageOrientation(pageOrientation());
		pdfWriter.setPageMargins(pageMargins(), QPageLayout::Unit::Millimeter);
		pdfWriter.setResolution(resolution());

		QRect pageRectPixels = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		QPainter painter(&pdfWriter);

		{

			QTextDocument titleTextDocument;

			titleTextDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

			QTextCursor textCursor(&titleTextDocument);

			// Generate title page

			generateTitlePage(&textCursor, m_reportParams.compareData, m_projectName, m_userName, QString());

			printDocument(&pdfWriter, &titleTextDocument, &painter, QString(), nullptr, nullptr, 0);

		}

		pdfWriter.newPage();

		{

			QTextDocument titleTextDocument;

			titleTextDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

			QTextCursor textCursor(&titleTextDocument);

			// Generate title page

			generateReportFilesPage(&textCursor, generatedReportFiles);

			printDocument(&pdfWriter, &titleTextDocument, &painter, QString(), nullptr, nullptr, 0);

		}
	}
}
