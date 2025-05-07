#include "ProjectDiffReport.h"

#include "../AppSignalLib/Bus.h"
#include "../Builder/AppSignalProperties.h"
#include "../UtilsLib/Ui/UiTools.h"

#include <DbLib/DbControllerTools.h>
#include <HardwareLib/Connection.h>
#include <HardwareLib/DiagSignalType.h>
#include <ReportLib/ReportPrinter.h>
#include <ReportLib/ReportSchemaView.h>
#include <UiLib/DialogProgress.h>
#include <UiLib/PropertyEditor.h>
#include <VFrame30/Context.h>
#include <VFrame30/Schema.h>
#include <VFrame30/SchemaItem.h>
#include <VFrame30/SchemaLayer.h>

#include "AppSettings.h"

using namespace Builder;
using namespace ReportLib;

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

	qsizetype count = strings.size();
	fileLines->resize(count);

	for (qsizetype i = 0; i < count; i++)
	{
		FileLine& fileLine = (*fileLines)[i];

		fileLine.text = strings[i];
		fileLine.line = i + 1;
		fileLine.hash = ::calcHash(fileLine.text);
	}

	return;
}

template<typename T>
void FileDiff::calculateLcs(const std::vector<T>& source, const std::vector<T>& target, std::vector<T>* result)
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
	result->resize(index /* + 1*/);
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
			(*result)[index - 1] = source[i - 1]; // Put current character in result
			i--;
			j--;
			index--;                              // reduce values of i, j and index
		}

		// If not same, then find the larger of two and
		// go in the direction of larger value
		else
		{
			if (L[i - 1][j] > L[i][j - 1])
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
void FileDiff::alignResults(const std::vector<T>& source,
							const std::vector<T>& target,
							const std::vector<T>& lcs,
							std::vector<T>* sourceAligned,
							std::vector<T>* targetAligned,
							std::vector<FileDiffAction>* actions,
							int* addedCount,
							int* removedCount,
							int* alignedCount)
{
	size_t sourceIndex = 0;
	size_t targetIndex = 0;
	size_t lcsIndex = 0;

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
// ProjectDiffReportParams
//
QPageLayout ProjectDiffReportParams::singleFilePageLayout() const
{
	static QPageLayout lEmpty;

	for (const auto& p : schemaTypesParams)
	{
		if (p.hasFileId() == false)
		{
			const int layoutIndex = 0;
			return p.pageLayoutWithMargins(layoutIndex);
		}
	}

	Q_ASSERT(false);
	return lEmpty;
}

//
// ProjectDiffThread
//

void ProjectDiffGeneratorThread::run(const QString& fileName,
									 const ProjectDiffReportParams& settings,
									 const QString& projectName,
									 const QString& userName,
									 const QString& userPassword,
									 const AppSignalSet* signalSet,
									 QWidget* parent)
{
	// Create schema view

	std::shared_ptr<ReportSchemaView> schemaView = std::make_shared<ReportSchemaView>(false /*infoMode*/);

	schemaView->session().setProject(projectName);
	schemaView->session().setUsername(userName);
	schemaView->session().setHost(QHostInfo::localHostName());

	// Create Worker

	ProjectDiffGenerator* worker = new ProjectDiffGenerator(fileName, settings, schemaView, signalSet, projectName, userName, userPassword);

	// Create Progress Dialog

	UiLib::DialogProgress dialogProgress(QObject::tr("Creating Project Differences Report"), 3, parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	QObject::connect(thread, &QThread::started, worker, &ProjectDiffGenerator::process);
	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater); // Schedule thread deleting

	QObject::connect(&dialogProgress,
					 &UiLib::DialogProgress::getProgress,
					 worker,
					 &ProjectDiffGenerator::progressRequested,
					 Qt::DirectConnection);
	QObject::connect(&dialogProgress, &UiLib::DialogProgress::cancelClicked, worker, &ProjectDiffGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &ProjectDiffGenerator::progressChanged, &dialogProgress, &UiLib::DialogProgress::setProgressMultiple);

	//  Schedule objects deleting

	QObject::connect(worker,
					 &ProjectDiffGenerator::finished,
					 [thread, &dialogProgress, worker, schemaView](const QString& errorMessage)
					 {
						 thread->quit();

						 if (errorMessage.isEmpty() == false)
						 {
							 dialogProgress.setErrorMessage(errorMessage);
						 }

						 dialogProgress.exit();

						 worker->deleteLater();
					 });

	// Start thread

	thread->start();

	dialogProgress.exec();

	if (dialogProgress.hasErrorMessage() == false)
	{
		if (settings.singleFile == true)
		{
			if (QMessageBox::question(parent, qAppName(), QObject::tr("Report generating has been finished.\n\nDo you wish to open it?")) ==
				QMessageBox::Yes)
			{
				UiTools::openPdf(fileName, parent);
			}
		}
		else
		{
			if (QMessageBox::question(parent,
									  qAppName(),
									  QObject::tr("Report generating has been finished.\n\nDo you wish to open the containing folder?")) ==
				QMessageBox::Yes)
			{
				QFileInfo f(fileName);
				QUrl url = QUrl::fromLocalFile(f.absolutePath());
				QDesktopServices::openUrl(url);
			}
		}
	}
	else
	{
		QMessageBox::critical(parent, qAppName(), dialogProgress.errorMessage());
	}

	return;
}

//
// lineFontSize
//

static int lineFontSize(const QPageLayout& pageLayout, const QString& fontName, int desiredLinesPerPage)
{
	// Function calculates approximal font size in points
	// for given page layout, font name and desired number of lines in page

	QRect rect = pageLayout.paintRectPoints();

	int lineHeightPoints = rect.height() / desiredLinesPerPage;

	int fontSize = lineHeightPoints;

	QFontMetrics fm{QFont(fontName, fontSize)};

	double fontCoef = fontSize / static_cast<double>(fm.height());

	return static_cast<int>(fontSize * fontCoef);

	/*qDebug() << "Height = " << fm.height();
	qDebug() << "fontDpi = " << fm.fontDpi();
	qDebug() << "ascent = " << fm.ascent();
	qDebug() << "descent = " << fm.descent();
	qDebug() << "leading = " << fm.leading();


	ReportObjectFormat rf{f, Qt::AlignLeft};
	qDebug() << "lineHeight = " << rf.blockFormat().lineHeight();

	for (int i = 0; i < linesPerPage + 10; i++)
	{
		rs->addText(tr("Line: %1\n").arg(i + 1), {fontName, fontSize * fontScaling, Qt::AlignHCenter});
	}*/
}

//
// ProjectDiffWorker
//

ProjectDiffGenerator::ProjectDiffGenerator(const QString& fileName,
										   const ProjectDiffReportParams& settings,
										   std::shared_ptr<ReportSchemaView> schemaView,
										   const AppSignalSet* signalSet,
										   const QString& projectName,
										   const QString& userName,
										   const QString& userPassword) :
	m_schemaView(schemaView),
	m_reportPrinter(m_schemaView),
	m_diagStateProvider(/*signalSet*/),
	m_diagStateController(m_diagStateProvider, nullptr),
	m_appSignalProvider(signalSet),
	m_appSignalController(m_appSignalProvider, nullptr),
	m_reportParams(settings),
	m_filePath(fileName),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
	m_headerFont{"Arial", 12, QFont::Bold},
	m_normalFont{"Arial", 9, QFont::Normal},
	m_tableFont{"Arial", 9, QFont::Normal},
	m_marginFont{"Arial", 9, QFont::Normal},
	m_headerFormat{m_headerFont, Qt::AlignHCenter},
	m_normalFormat{m_normalFont, Qt::AlignLeft}
{
	return;
}

ProjectDiffGenerator::~ProjectDiffGenerator()
{
	qDebug() << "ProjectDiffGenerator deleted";
}

std::vector<Builder::SchemaTypesParams> ProjectDiffGenerator::defaultFileTypeParams(DbController* db)
{
	std::vector<Builder::SchemaTypesParams> result;

	if (db == nullptr || db->isProjectOpened() == false)
	{
		Q_ASSERT(false);
		return result;
	}

	QMarginsF portatitMargins{30, 20, 15, 20};
	QMarginsF langscapeMargins{20, 30, 20, 15};

	QStringList layoutNames;
	layoutNames << QString();

	// Hardware and signals

	result.push_back(
		{db->systemFileId(DbDir::HardwareConfigurationDir),
		 QObject::tr("Hardware Configuration"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{applicationSignalsTypeId(),
		 QObject::tr("Application Signals"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});


	// Schemas

	result.push_back(
		{db->systemFileId(DbDir::MonitorSchemasDir),
		 QObject::tr("Monitor Schemas"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::TuningSchemasDir),
		 QObject::tr("Tuning Schemas"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::DiagSchemasDir),
		 QObject::tr("Diagnostics Schemas"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::AppLogicDir),
		 QObject::tr("Application Logic"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::UfblDir),
		 QObject::tr("UFBL Descriptions"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::VduSchemasDir),
		 QObject::tr("VDU Schemas"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, langscapeMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});


	// Other objects

	result.push_back(
		{db->systemFileId(DbDir::BusTypesDir),
		 QObject::tr("Busses"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::ConnectionsDir),
		 QObject::tr("Connections"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::DiagSignalTypesDir),
		 QObject::tr("Diagnostics Signal Types"),
		 true,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::SimTestsDir),
		 QObject::tr("Simulator Tests"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::AfblDir),
		 QObject::tr("AFBL Descriptions"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::HardwarePresetsDir),
		 QObject::tr("Hardware Presets"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::ModuleConfigurationDir),
		 QObject::tr("Module Configuration"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{db->systemFileId(DbDir::EtcDir),
		 QObject::tr("Other Files"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	result.push_back(
		{-1,
		 QObject::tr("Single-File Report"),
		 false,
		 QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Portrait, portatitMargins, QPageLayout::Unit::Millimeter),
		 layoutNames});

	return result;
}

void ProjectDiffGenerator::process()
{
	try
	{
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics = {};
			m_statistics.m_state = WorkerStatus::Comparing;
		}

		compareProject();

		// Generate generic report file with all report files description
		//
		if (m_reportParams.singleFile == false)
		{
			generateSummaryReport(m_reportParams.singleFilePageLayout());
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_state = WorkerStatus::Printing;
		}

		// Save reports
		//
		for (auto& report : m_generatedReports)
		{
			// Don't print report if it has only one title page
			//
			if (report->sectionsCount() == 1 && report->section(0)->caption() == titlePageName)
			{
				continue;
			}

			// Print report
			//
			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_printingReportName = report->path();
			}

			m_reportPrinter.save(*report, report->path(), m_stop);
		}

		//

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_state = WorkerStatus::Idle;
		}

		emit finished(QString());
	}

	catch (QString errorMessage)
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

	ProjectDiffGenerator::Statistics stat = statistics();

	switch (stat.m_state)
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
	case ProjectDiffGenerator::WorkerStatus::Comparing:
		{
			progressText.push_back(tr("Comparing: %1").arg(stat.m_currentSectionName));

			progressText.push_back(tr("Signals: %1 / %2, Files: %3 / %4")
									   .arg(stat.m_signalIndex)
									   .arg(stat.m_signalsCount)
									   .arg(stat.m_fileIndex)
									   .arg(stat.m_filesCount));

			QString objectName = stat.m_currentObjectName;
			if (objectName.length() > 48)
			{
				objectName = "..." + objectName.right(48);
			}

			progressText.push_back(tr("Current: %1").arg(objectName));

			progress = stat.m_fileIndex + stat.m_signalIndex;
			progressMax = stat.m_filesCount + stat.m_signalsCount;
		}
		break;
	case ProjectDiffGenerator::WorkerStatus::Printing:
		{
			int unused_progressMin = 0;
			QString text;
			m_reportPrinter.statistics().fill(&progress, &unused_progressMin, &progressMax, &text);
			progressText.push_back(text);
		}
		break;
	default:
		Q_ASSERT(false);
	}

	emit progressChanged(progress, 0, progressMax, progressText);

	return;
}

const QString& ProjectDiffGenerator::filePath() const
{
	return m_filePath;
}

ProjectDiffGenerator::Statistics ProjectDiffGenerator::statistics() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_statistics;
}

DbController* ProjectDiffGenerator::db()
{
	return &m_db;
}

void ProjectDiffGenerator::compareProject()
{
	db()->disableProgress();

	db()->setHost(theAppSettings.serverHost());
	db()->setPort(theAppSettings.serverPort());
	db()->setServerUsername(theAppSettings.serverUsername());
	db()->setServerPassword(theAppSettings.serverPassword());

	bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
	if (ok == false)
	{
		throw(tr("Failed to open project!"));
	}

	// Place signals to front
	//
	for (size_t i = 0; i < m_reportParams.schemaTypesParams.size(); i++)
	{
		Builder::SchemaTypesParams ft = m_reportParams.schemaTypesParams[i];

		if (ft.fileId() == applicationSignalsTypeId() && ft.selected() == true && i != 0)
		{
			m_reportParams.schemaTypesParams.erase(m_reportParams.schemaTypesParams.begin() + i);
			m_reportParams.schemaTypesParams.insert(m_reportParams.schemaTypesParams.begin(), ft);
			break;
		}
	}

	// Get and count files
	//
	m_sourceDeviceObjectMap.clear();
	m_targetDeviceObjectMap.clear();

	int filesCount = 0;

	std::vector<DbFileTree> filesTrees;

	for (const Builder::SchemaTypesParams& ft : m_reportParams.schemaTypesParams)
	{
		if (ft.selected() == false)
		{
			continue;
		}

		if (ft.fileId() == applicationSignalsTypeId())
		{
			// This is not a file
			continue;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentSectionName = ft.caption();
		}

		filesTrees.push_back({});

		DbFileTree* filesTree = &filesTrees[filesTrees.size() - 1];

		ok = db()->getFileListTree(filesTree, ft.fileId(), false /*removeDeleted*/, nullptr);
		if (ok == false)
		{
			throw(tr("DbController::getFileListTree failed on fileId = %1").arg(ft.fileId()));
		}

		filesCount += static_cast<int>(filesTree->files().size());
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_statistics.m_filesCount = filesCount;
	}

	// Process Files

	int fileTreeIndex = 0;

	std::shared_ptr<Report> report;

	for (const Builder::SchemaTypesParams& ft : m_reportParams.schemaTypesParams)
	{
		if (m_stop == true)
		{
			break;
		}

		if (ft.hasFileId() == false)
		{
			continue;
		}

		if (ft.selected() == false)
		{
			continue;
		}

		// Create report object and specify its filename
		//

		QString pdfFileName = filePath();

		// Create filename for multiple-files report
		//
		if (m_reportParams.singleFile == false)
		{
			qsizetype pos = pdfFileName.lastIndexOf('.');
			if (pos != -1)
			{
				pdfFileName.insert(pos, tr("_%1").arg(ft.caption()));
			}
			else
			{
				pdfFileName += tr("_%1.pdf").arg(ft.caption());
			}

			pdfFileName.replace(' ', '_');
		}

		const int layoutIndex = 0;
		QPageLayout pageLayout =
			(m_reportParams.singleFile == true) ? m_reportParams.singleFilePageLayout() : ft.pageLayoutWithMargins(layoutIndex);

		// Create report
		//
		if (report == nullptr || m_reportParams.singleFile == false)
		{
			report = std::make_shared<Report>(ft.caption(), pdfFileName);
			report->setResolution(m_resolution);
			m_generatedReports.push_back(report);

			// Create title page
			//
			auto titlePageSection = generateTitlePage(pageLayout,
													  m_reportParams.compareData,
													  m_projectName,
													  m_userName,
													  m_reportParams.singleFile == false ? ft.caption() : QString());
			report->insertSection(0, titlePageSection);

			// Create margins
			//
			createMarginItems(*report, m_reportParams.compareData, m_reportParams.singleFile == false ? ft.caption() : QString());
		}

		// Specify section name to statistics
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentSectionName = ft.caption();
		}

		std::shared_ptr<ReportSection> fileTypeSummarySection = ReportSection::create(ft.caption(), pageLayout);
		fileTypeSummarySection->addText(tr("%1\n\n").arg(ft.caption()), m_headerFormat);

		std::shared_ptr<ReportSection> fileTypeSection = ReportSection::create(ft.caption(), pageLayout);

		std::shared_ptr<ReportTable> headerTable;

		if (ft.fileId() == ProjectDiffGenerator::applicationSignalsTypeId())
		{
			// This is application signals

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_currentObjectName.clear();
			}

			headerTable = ReportTable::create(
				{m_tableFont, {tr("Signal"), tr("Status"), tr("Changeset"), tr("User"), tr("Date")}, {45, 10, 10, 15, 20}, Qt::AlignLeft});

			fileTypeSummarySection->addTable(headerTable);

			compareSignals(m_reportParams.compareData, fileTypeSection, *headerTable);
		}
		else
		{
			headerTable = ReportTable::create(
				{m_tableFont, {tr("Object"), tr("Status"), tr("Changeset"), tr("User"), tr("Date")}, {45, 10, 10, 15, 20}, Qt::AlignLeft});

			fileTypeSummarySection->addTable(headerTable);

			// Compare files

			const DbFileTree& filesTree = filesTrees[fileTreeIndex++];

			const std::vector<std::shared_ptr<DbFileInfo>>& children = filesTree.children(filesTree.rootFileId());

			for (const auto& child : children)
			{
				compareFilesRecursive(ft.fileId(), filesTree, child, m_reportParams.compareData, fileTypeSection, *headerTable);
			}
		}

		// Remove header if no data
		//
		if (headerTable->rowCount() > 0)
		{
			headerTable->sortByColumn(0);

			report->addSection(fileTypeSummarySection);
		}

		if (fileTypeSection->objectCount() > 0)
		{
			report->addSection(fileTypeSection);
		}
	}

	db()->closeProject(nullptr);

	return;
}

void ProjectDiffGenerator::compareFilesRecursive(int rootFileId,
												 const DbFileTree& filesTree,
												 const std::shared_ptr<DbFileInfo>& fi,
												 const CompareData& compareData,
												 std::shared_ptr<ReportSection> section,
												 ReportTable& headerTable)
{
	if (m_stop == true)
	{
		return;
	}

	if (fi == nullptr)
	{
		Q_ASSERT(fi);
		return;
	}


	compareFile(rootFileId, filesTree, fi, compareData, section, headerTable);

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

		compareFilesRecursive(rootFileId, filesTree, fiChild, compareData, section, headerTable);
	}

	return;
}

void ProjectDiffGenerator::compareFile(int rootFileId,
									   const DbFileTree& filesTree,
									   const std::shared_ptr<DbFileInfo>& fi,
									   const CompareData& compareData,
									   std::shared_ptr<ReportSection> section,
									   ReportTable& headerTable)
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
		m_statistics.m_currentObjectName = fileName;
		m_statistics.m_fileIndex++;
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

	compareFileContents(rootFileId, sourceFile, targetFile, fileName, section, headerTable);

	return;
}

void ProjectDiffGenerator::compareFileContents(int rootFileId,
											   const std::shared_ptr<DbFile>& sourceFile,
											   const std::shared_ptr<DbFile>& targetFile,
											   const QString& fileName,
											   std::shared_ptr<ReportSection> section,
											   ReportTable& headerTable)
{
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
	if (sourceFile != nullptr && targetFile != nullptr && sourceFile->changeset() == targetFile->changeset())
	{
		return;
	}

	bool fileTypeIsPreset = rootFileId == db()->systemFileId(DbDir::HardwarePresetsDir);

	// File was deleted
	//
	if (sourceFile != nullptr && sourceFile->deleted() == true)
	{
		if (hardwareObject == true)
		{
			if (fileTypeIsPreset == true)
			{
				addHeaderTableItem(headerTable,
								   tr("%1, %2").arg(sourceObject->presetName()).arg(sourceObject->equipmentId()),
								   tr("Deleted"),
								   sourceFile);
			}
			else
			{
				addHeaderTableItem(headerTable, sourceObject->equipmentId(), tr("Deleted"), sourceFile);
			}
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
				if (fileTypeIsPreset == true)
				{
					addHeaderTableItem(headerTable,
									   tr("%1, %2").arg(targetObject->presetName()).arg(targetObject->equipmentId()),
									   tr("Deleted"),
									   targetFile);
				}
				else
				{
					addHeaderTableItem(headerTable, targetObject->equipmentId(), tr("Deleted"), targetFile);
				}
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
		compareDeviceObjects(sourceFile, targetFile, sourceObject, targetObject, section, headerTable, fileTypeIsPreset);
		return;
	}

	if (isConnectionFile(fileName) == true)
	{
		compareConnections(sourceFile, targetFile, section, headerTable);
		return;
	}

	if (isDiagSignalTypeFile(fileName) == true)
	{
		compareDiagSignalTypes(sourceFile, targetFile, section, headerTable);
		return;
	}

	if (isBusTypeFile(fileName) == true)
	{
		compareBusTypes(sourceFile, targetFile, section, headerTable);
		return;
	}

	if (isSchemaFile(fileName) == true)
	{
		compareSchemas(fileName, sourceFile, targetFile, section, headerTable);
		return;
	}

	compareFilesData(sourceFile, targetFile, section, headerTable);
	return;
}

std::shared_ptr<Hardware::DeviceObject> ProjectDiffGenerator::loadDeviceObject(
	const std::shared_ptr<DbFile>& file,
	std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap) const
{
	if (deviceObjectMap == nullptr)
	{
		Q_ASSERT(deviceObjectMap);
		return nullptr;
	}

	std::shared_ptr<Hardware::DeviceObject> object = DbControllerTools::deviceObjectFromDbFile(*file);
	if (object == nullptr)
	{
		return nullptr;
	}

	// Save object to the map
	//
	(*deviceObjectMap)[file->fileId()] = object;

	// Get pointers to parent and expand Equipment ID
	//
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

			// qDebug() << parentObject->fileId() << parentObject->equipmentIdTemplate() << parentObject->equipmentId();

			object->expandEquipmentId();

			// qDebug() << object->fileId() << object->equipmentIdTemplate() << object->equipmentId();
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
												std::shared_ptr<ReportSection> section,
												ReportTable& headerTable,
												bool fileTypeIsPreset)
{
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
		if (fileTypeIsPreset == false)
		{
			addHeaderTableItem(headerTable, tr("%1").arg(sourceObject->equipmentId()), tr("Added"), sourceFile);
		}
		else
		{
			addHeaderTableItem(headerTable,
							   tr("%1, %2").arg(sourceObject->presetName()).arg(sourceObject->equipmentId()),
							   tr("Added"),
							   sourceFile);
		}
		return;
	}
	else
	{
		if (sourceObject == nullptr && targetObject != nullptr)
		{
			if (fileTypeIsPreset == false)
			{
				addHeaderTableItem(headerTable, tr("%1").arg(targetObject->equipmentId()), tr("Added"), targetFile);
			}
			else
			{
				addHeaderTableItem(headerTable,
								   tr("%1, %2").arg(targetObject->presetName()).arg(targetObject->equipmentId()),
								   tr("Added"),
								   targetFile);
			}
			return;
		}
	}

	// Both Objects
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceObject, *targetObject, &diffs);

	if (diffs.empty() == false)
	{
		if (fileTypeIsPreset == false)
		{
			addHeaderTableItem(headerTable,
							   tr("%1").arg(targetObject->equipmentId()),
							   E::valueToString<E::VcsItemAction>(targetFile->action()),
							   targetFile);
		}
		else
		{
			addHeaderTableItem(headerTable,
							   tr("%1, %2").arg(targetObject->presetName()).arg(targetObject->equipmentId()),
							   E::valueToString<E::VcsItemAction>(targetFile->action()),
							   targetFile);
		}

		QString equipmentType = fileTypeIsPreset == true ? tr("Preset") : tr("Equipment");
		QString sectionName;

		if (fileTypeIsPreset == true)
		{
			sectionName = tr("%1: %2, %3").arg(equipmentType).arg(targetObject->presetName()).arg(targetObject->equipmentId());
		}
		else
		{
			sectionName = tr("%1: %2").arg(equipmentType).arg(targetObject->equipmentId());
		}

		section->addText(tr("%1, %2\n\n").arg(sectionName).arg(changesetString(targetFile)), m_normalFormat);

		auto diffTable = section->addTable(
			{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

		fillDiffTable(*diffTable, diffs);
	}

	return;
}

void ProjectDiffGenerator::compareBusTypes(const std::shared_ptr<DbFile>& sourceFile,
										   const std::shared_ptr<DbFile>& targetFile,
										   std::shared_ptr<ReportSection> section,
										   ReportTable& headerTable)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	AppSignalLib::Bus sourceBus;
	AppSignalLib::Bus targetBus;

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
	if ((sourceFile != nullptr && targetFile == nullptr) || (sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleBus = sourceFile != nullptr ? &sourceBus : &targetBus;
		addHeaderTableItem(headerTable, singleBus->busTypeId(), tr("Added"), singleFile);
		return;
	}

	// Both Files
	//
	// Create tables

	std::shared_ptr<ReportTable> busDiffTable = ReportTable::create(
		{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

	std::shared_ptr<ReportTable> busSignalsDiffTable =
		ReportTable::create({m_tableFont, {tr("SignalID"), tr("Caption"), tr("Status")}, {35, 15, 50}, Qt::AlignLeft});

	std::vector<PropertyDiff> busDiffs;

	// Compare bus properties

	comparePropertyObjects(sourceBus, targetBus, &busDiffs);

	if (busDiffs.empty() == false)
	{
		fillDiffTable(*busDiffTable, busDiffs);
	}

	std::map<QString, std::shared_ptr<ReportTable>> busSignalsPropertiesTables;

	// Compare bus signals

	for (const AppSignalLib::BusSignal& targetBusSignal : targetBus.busSignals())
	{
		bool busSignalFound = false;

		for (const AppSignalLib::BusSignal& sourceBusSignal : sourceBus.busSignals())
		{
			if (targetBusSignal.signalId() == sourceBusSignal.signalId())
			{
				std::vector<PropertyDiff> busSignalDiffs;

				comparePropertyObjects(sourceBusSignal, targetBusSignal, &busSignalDiffs);

				if (busSignalDiffs.empty() == false)
				{
					std::shared_ptr<ReportTable> busSignalsPropertiesDiffTable = ReportTable::create(
						{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

					busSignalsPropertiesTables[targetBusSignal.signalId()] = busSignalsPropertiesDiffTable;

					fillDiffTable(*busSignalsPropertiesDiffTable, busSignalDiffs);

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

	for (const AppSignalLib::BusSignal& sourceBusSignal : sourceBus.busSignals())
	{
		bool busSignalFound = false;

		for (const AppSignalLib::BusSignal& targetBusSignal : targetBus.busSignals())
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
		addHeaderTableItem(headerTable, targetBus.busTypeId(), E::valueToString<E::VcsItemAction>(targetFile->action()), targetFile);

		// Add tables to section

		section->addText(tr("Bus: %1, %2\n\n").arg(targetBus.busTypeId()).arg(changesetString(targetFile)), m_normalFormat);

		if (busDiffTable->rowCount() != 0)
		{
			section->addTable(busDiffTable);
			section->addText("\n", m_normalFormat);
		}

		if (busSignalsDiffTable->rowCount() != 0)
		{
			busSignalsDiffTable->sortByColumn(0);

			section->addText(tr("Bus %1 signals:\n\n").arg(targetBus.busTypeId()), m_normalFormat);
			section->addTable(busSignalsDiffTable);
			section->addText("\n", m_normalFormat);
		}

		for (auto it : busSignalsPropertiesTables)
		{
			const QString& signalId = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			section->addText(tr("Bus: %1, signal: %2\n\n").arg(targetBus.busTypeId()).arg(signalId), m_normalFormat);
			section->addTable(itemDiffTable);
		}
	}

	return;
}


void ProjectDiffGenerator::compareSchemas(const QString& fileName,
										  const std::shared_ptr<DbFile>& sourceFile,
										  const std::shared_ptr<DbFile>& targetFile,
										  std::shared_ptr<ReportSection> section,
										  ReportTable& headerTable)
{
	auto context = VFrame30::Context::create(&m_diagStateController, &m_appSignalController, nullptr, nullptr, nullptr);

	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) || (sourceFile == nullptr && targetFile != nullptr))
	{
		// Schema was added - just draw it

		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		addHeaderTableItem(headerTable, fileName, tr("Added"), singleFile);

		std::shared_ptr<VFrame30::Schema> singleSchema = VFrame30::Schema::Create(singleFile->data());
		if (singleSchema == nullptr)
		{
			throw(tr("Failed to load schema from: '%1'").arg(singleFile->fileName()));
		}

		singleSchema->setContext(context);

		// Add schema drawing

		auto reportSchema = ReportSchema::create(tr("Schema: %1, %2\n").arg(singleSchema->schemaId()).arg(changesetString(singleFile)),
												 {},
												 singleSchema,
												 {});

		section->addSchema(reportSchema);
		section->setTag(singleSchema->caption());

		return;
	}

	// Both Files
	//
	std::shared_ptr<VFrame30::Schema> sourceSchema = VFrame30::Schema::Create(sourceFile->data());
	if (sourceSchema == nullptr)
	{
		throw(tr("Failed to load source schema from: '%1'").arg(sourceFile->fileName()));
	}
	sourceSchema->setContext(context);

	std::shared_ptr<VFrame30::Schema> targetSchema = VFrame30::Schema::Create(targetFile->data());
	if (targetSchema == nullptr)
	{
		throw(tr("Failed to load target schema from: '%1'").arg(targetFile->fileName()));
	}
	targetSchema->setContext(context);

	// Create tables

	std::shared_ptr<ReportTable> schemaDiffTable = ReportTable::create(
		{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

	std::shared_ptr<ReportTable> schemaItemsDiffTable =
		ReportTable::create({m_tableFont, {tr("Type"), tr("Label"), tr("Layer"), tr("Status")}, {25, 35, 25, 15}, Qt::AlignLeft});

	// Compare schemas properties

	std::vector<PropertyDiff> schemaDiffs;
	comparePropertyObjects(*sourceSchema, *targetSchema, &schemaDiffs);

	fillDiffTable(*schemaDiffTable, schemaDiffs);

	// Compare schemas items properties

	std::map<QUuid, ReportSchemaCompareAction> itemsActions;

	std::map<std::shared_ptr<VFrame30::SchemaItem>, std::shared_ptr<ReportTable>> itemsTables;

	for (const auto& targetLayer : targetSchema->layers())
	{
		for (const SchemaItemPtr& targetItem : targetLayer->items())
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

					std::shared_ptr<ReportTable> itemDiffTable = ReportTable::create(
						{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

					fillDiffTable(*itemDiffTable, itemDiffs);

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
	std::vector<QByteArray> deletedItems;

	for (const auto& sourceLayer : sourceSchema->layers())
	{
		for (const SchemaItemPtr& sourceItem : sourceLayer->items())
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
				for (const auto& targetLayer : targetSchema->layers())
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						QByteArray ba;
						sourceItem->saveToByteArray(&ba);
						deletedItems.push_back(std::move(ba));
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
		addHeaderTableItem(headerTable, fileName, E::valueToString<E::VcsItemAction>(targetFile->action()), targetFile);

		section->addText(tr("Schema: %1\n").arg(targetSchema->schemaId()), m_normalFormat);

		// Add schema differences drawing

		QString schemaId = targetSchema->schemaId();

		auto reportSchema =
			ReportSchema::create(tr("Schema: %1, %2\n").arg(schemaId).arg(changesetString(targetFile)), {}, targetSchema, itemsActions);

		for (const auto& ba : deletedItems) 
		{
			SchemaItemPtr si = VFrame30::SchemaItem::Create(ba);
			Q_ASSERT(si);
			reportSchema->schema()->activeLayer()->pushBackItem(si);
		}

		section->addSchema(reportSchema);
		section->setTag(targetSchema->caption());

		// Add schema differences tables

		if (schemaDiffTable->rowCount() != 0)
		{
			section->addText(tr("Schema %1 properties:\n\n").arg(schemaId), m_normalFormat);
			section->addTable(schemaDiffTable);
			section->addText("\n", m_normalFormat);
		}

		if (schemaItemsDiffTable->rowCount() != 0)
		{
			schemaItemsDiffTable->sortByColumn(1);

			section->addText(tr("Schema %1 items:\n\n").arg(schemaId), m_normalFormat);
			section->addTable(schemaItemsDiffTable);
			section->addText("\n", m_normalFormat);
		}

		for (const auto& it : itemsTables)
		{
			const std::shared_ptr<VFrame30::SchemaItem>& item = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			QString className(item->metaObject()->className());
			className.remove("VFrame30::");

			if (item->label().isEmpty() == true)
			{
				section->addText(tr("%1 (no label):\n\n").arg(className), m_normalFormat);
			}
			else
			{
				section->addText(tr("%1 %2:\n\n").arg(className).arg(item->label()), m_normalFormat);
			}
			section->addTable(itemDiffTable);
			section->addText(tr("\n"), m_normalFormat);
		}
	}
}

void ProjectDiffGenerator::compareConnections(const std::shared_ptr<DbFile>& sourceFile,
											  const std::shared_ptr<DbFile>& targetFile,
											  std::shared_ptr<ReportSection> section,
											  ReportTable& headerTable)
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
	if ((sourceFile != nullptr && targetFile == nullptr) || (sourceFile == nullptr && targetFile != nullptr))
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
		section->addText(tr("Connection: %1, %2\n\n").arg(targetConnection.connectionID()).arg(changesetString(targetFile)),
						 m_normalFormat);

		addHeaderTableItem(headerTable,
						   targetConnection.connectionID(),
						   E::valueToString<E::VcsItemAction>(targetFile->action()),
						   targetFile);

		std::shared_ptr<ReportTable> diffTable = section->addTable(
			{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

		fillDiffTable(*diffTable, diffs);
	}
}

void ProjectDiffGenerator::compareDiagSignalTypes(const std::shared_ptr<DbFile>& sourceFile,
												  const std::shared_ptr<DbFile>& targetFile,
												  std::shared_ptr<ReportLib::ReportSection> section,
												  ReportLib::ReportTable& headerTable)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	std::shared_ptr<Hardware::DiagSignalTypeObject> sourceType;
	std::shared_ptr<Hardware::DiagSignalTypeObject> targetType;

	if (sourceFile != nullptr)
	{
		sourceType = Hardware::DiagSignalTypeObject::Create(sourceFile->data());
		if (sourceType == nullptr)
		{
			throw(tr("Failed to load source dianostics signal type from: '%1'").arg(sourceFile->fileName()));
		}
	}
	if (targetFile != nullptr)
	{
		targetType = Hardware::DiagSignalTypeObject::Create(targetFile->data());
		if (targetType == nullptr)
		{
			throw(tr("Failed to load target dianostics signal type from: '%1'").arg(targetFile->fileName()));
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) || (sourceFile == nullptr && targetFile != nullptr))
	{
		const auto& singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		const auto& singleType = sourceFile != nullptr ? sourceType : targetType;
		addHeaderTableItem(headerTable, singleType->signalTypeId(), tr("Added"), singleFile);
		return;
	}

	// Both Files
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceType, *targetType, &diffs);

	if (diffs.empty() == false)
	{
		section->addText(tr("Diagnostics signal type: %1, %2\n\n").arg(targetType->signalTypeId()).arg(changesetString(targetFile)),
						 m_normalFormat);

		addHeaderTableItem(headerTable, targetType->signalTypeId(), E::valueToString<E::VcsItemAction>(targetFile->action()), targetFile);

		std::shared_ptr<ReportTable> diffTable = section->addTable(
			{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

		fillDiffTable(*diffTable, diffs);
	}
}

void ProjectDiffGenerator::compareFilesData(const std::shared_ptr<DbFile>& sourceFile,
											const std::shared_ptr<DbFile>& targetFile,
											std::shared_ptr<ReportSection> section,
											ReportTable& headerTable)
{
	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) || (sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		addHeaderTableItem(headerTable, singleFile->fileName(), tr("Added"), singleFile);
		return;
	}

	if (sourceFile->data() == targetFile->data())
	{
		return;
	}

	addHeaderTableItem(headerTable, targetFile->fileName(), E::valueToString<E::VcsItemAction>(targetFile->action()), targetFile);

	// Both Files
	//
	if (isTextFile(targetFile->fileName()) == true)
	{
		section->addText(tr("File: %1, %2\n\n").arg(targetFile->fileName()).arg(changesetString(targetFile)), m_normalFormat);

		std::vector<FileDiff::FileLine> fileLinesSource;
		std::vector<FileDiff::FileLine> fileLinesTarget;
		std::vector<FileDiff::FileLine> fileLinesCommon;

		FileDiff::loadFileData(sourceFile->data(), &fileLinesSource);
		FileDiff::loadFileData(targetFile->data(), &fileLinesTarget);

		fileLinesCommon.reserve(static_cast<int>(fileLinesSource.size() + fileLinesTarget.size()));

		FileDiff::calculateLcs(fileLinesSource, fileLinesTarget, &fileLinesCommon);

		std::shared_ptr<ReportTable> diffTable =
			section->addTable({m_tableFont, {tr("Line"), tr("Source"), tr("Line"), tr("Target")}, {10, 40, 10, 40}, Qt::AlignLeft});

		std::vector<FileDiff::FileLine> fileLinesSourceAligned;
		std::vector<FileDiff::FileLine> fileLinesTargetAligned;
		std::vector<FileDiff::FileDiffAction> fileLinesActions;

		int addedCount = 0;
		int removedCount = 0;
		int alignedCount = 0;

		FileDiff::alignResults(fileLinesSource,
							   fileLinesTarget,
							   fileLinesCommon,
							   &fileLinesSourceAligned,
							   &fileLinesTargetAligned,
							   &fileLinesActions,
							   &addedCount,
							   &removedCount,
							   &alignedCount);

		size_t sourceIndex = 0;
		size_t targetIndex = 0;
		size_t actionIndex = 0;

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
					diffTable->insertRow({tr("%1").arg(sourceLine.line),
										  sourceLine.text.trimmed(),
										  tr("%1").arg(targetLine.line),
										  targetLine.text.trimmed()});
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

		section->addText(str + "\n", m_normalFormat);
	}
}

void ProjectDiffGenerator::compareSignals(const CompareData& compareData, std::shared_ptr<ReportSection> section, ReportTable& headerTable)
{
	bool ok = false;

	{
		QMutexLocker l(&m_statisticsMutex);
		m_statistics.m_state = WorkerStatus::RequestingSignals;
	}

	// Get source signals
	//

	std::vector<AppSignal> sourceSignalsVec;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			ok = db()->getLatestSignalsWithUserID(&sourceSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getLatestSignalsWithUserID failed."));
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

	std::vector<AppSignal> targetSignalsVec;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion:
		{
			ok = db()->getLatestSignalsWithUserID(&targetSignalsVec, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getSignals failed."));
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

	std::map<Hash, AppSignal*> sourceSignals;
	std::map<Hash, AppSignal*> targetSignals;

	std::map<Hash, int> allHashesMap;

	for (AppSignal& s : sourceSignalsVec)
	{
		if (s.deleted() == true)
		{
			continue;
		}

		Hash hash = ::calcHash(s.appSignalID());

		allHashesMap[hash] = 1;

		sourceSignals[::calcHash(s.appSignalID())] = &s;
	}

	for (AppSignal& s : targetSignalsVec)
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
		m_statistics.m_state = WorkerStatus::Comparing;
		m_statistics.m_signalsCount = static_cast<int>(allHashesMap.size());
	}

	for (auto it = targetSignals.begin(); it != targetSignals.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		Hash hash = it->first;

		const AppSignal* targetSignal = it->second;
		if (targetSignal == nullptr)
		{
			Q_ASSERT(targetSignal);
			return;
		}

		const AppSignal* sourceSignal = nullptr;

		auto itSource = sourceSignals.find(hash);
		if (itSource != sourceSignals.end())
		{
			sourceSignal = itSource->second;
		}

		QString appSignalID = targetSignal->appSignalID();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_signalIndex++;
			m_statistics.m_currentObjectName = appSignalID;
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
				// headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(*sourceSignal)});
			}
			else
			{
				if (targetSignal->deleted() == true)
				{
					Q_ASSERT(false);
					// headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(*targetSignal)});
				}
				else
				{
					bool swap = false;

					// Target changeset should be later or checked-out
					//
					if (sourceSignal->changesetID() == 0 || targetSignal->changesetID() == 0)
					{
						// One of files is checked out
						//
						if (sourceSignal->changesetID() == 0 && targetSignal->changesetID() != 0)
						{
							swap = true;
						}
					}
					else
					{
						if (sourceSignal->changesetID() > targetSignal->changesetID())
						{
							swap = true;
						}
					}

					if (swap == true)
					{
						std::swap(sourceSignal, targetSignal);
					}

					compareSignalContents(*sourceSignal, *targetSignal, section, headerTable);
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

		const AppSignal* sourceSignal = it->second;
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
				m_statistics.m_signalIndex++;
				m_statistics.m_currentObjectName = appSignalID;
			}

			addHeaderTableItem(headerTable, appSignalID, tr("Deleted"), *sourceSignal);
		}
	}

	return;
}

void ProjectDiffGenerator::compareSignalContents(const AppSignal& sourceSignal,
												 const AppSignal& targetSignal,
												 std::shared_ptr<ReportSection> section,
												 ReportTable& headerTable)
{
	AppSignalProperties sourceProperties(sourceSignal);
	AppSignalProperties targetProperties(targetSignal);

	auto p = sourceProperties.propertyByCaption(AppSignalPropNames::CHANGESET_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(AppSignalPropNames::INSTANCE_CREATED);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(AppSignalPropNames::SIGNAL_INSTANCE_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = sourceProperties.propertyByCaption(AppSignalPropNames::SIGNAL_GROUP_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}

	p = targetProperties.propertyByCaption(AppSignalPropNames::CHANGESET_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(AppSignalPropNames::INSTANCE_CREATED);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(AppSignalPropNames::SIGNAL_INSTANCE_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}
	p = targetProperties.propertyByCaption(AppSignalPropNames::SIGNAL_GROUP_ID);
	if (p != nullptr)
	{
		p->setExpert(true);
	}

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceProperties, targetProperties, &diffs);

	if (diffs.empty() == false)
	{
		section->addText(tr("Signal: %1, %2\n\n").arg(targetSignal.appSignalID()).arg(changesetString(targetSignal)), m_normalFormat);

		addHeaderTableItem(headerTable,
						   targetSignal.appSignalID(),
						   E::valueToString<E::VcsItemAction>(targetSignal.instanceAction()),
						   targetSignal);

		std::shared_ptr<ReportTable> diffTable = section->addTable(
			{m_tableFont, {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, {15, 15, 35, 35}, Qt::AlignLeft});

		fillDiffTable(*diffTable, diffs);
	}
}

void ProjectDiffGenerator::comparePropertyObjects(const PropertyObject& sourceObject,
												  const PropertyObject& targetObject,
												  std::vector<PropertyDiff>* const result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	const int maxDecimalPlaces = 5;

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
		diff.caption = it.second; // Name

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
			diff.newValueText = ExtWidgets::PropertyTools::propertyValueText(tp.get(), -1, maxDecimalPlaces, true /*noNewLine*/);

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

		diff.oldValueText = ExtWidgets::PropertyTools::propertyValueText(sp.get(), -1, maxDecimalPlaces, true /*noNewLine*/);
		diff.newValueText = ExtWidgets::PropertyTools::propertyValueText(tp.get(), -1, maxDecimalPlaces, true /*noNewLine*/);

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
	if (fileName.endsWith(File::BusFileExtension) == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isConnectionFile(const QString& fileName) const
{
	if (fileName.endsWith(File::OclFileExtension) == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isDiagSignalTypeFile(const QString& fileName) const
{
	if (fileName.endsWith(File::DiagSignalTypeSetFileExtension) == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isTextFile(const QString& fileName) const
{
	const std::array<QString, 5> TextExtensions = {
		".js",  // Script
		".xml", // Xml Document
		".xsd", // Xml Schema
		".csv", // Table
		".txt"  // Text
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
	const std::array<QString, 12> TextExtensions = {File::AlFileExtension,   // Script
													File::AlTemplExtension,  // Xml Document
													File::UfbFileExtension,  // Xml Schema
													File::UfbTemplExtension, // Table
													File::MvsFileExtension,
													File::MvsTemplExtension,
													File::TvsFileExtension,
													File::TvsTemplExtension,
													File::DvsFileExtension,
													File::DvsTemplExtension,
													File::VduFileExtension,
													File::VduTemplExtension};

	for (const QString& ext : TextExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

std::shared_ptr<ReportSection> ProjectDiffGenerator::generateTitlePage(const QPageLayout& pageLayout,
																	   const CompareData& compareData,
																	   const QString& projectName,
																	   const QString& userName,
																	   const QString& subreportName) const
{
	auto rs = ReportSection::create(titlePageName, pageLayout);

	// Initialize font

	QString fontName{"Arial"};

	int titleFontSize = lineFontSize(pageLayout, fontName, 25);
	int regularFontSize = lineFontSize(pageLayout, fontName, 50);

	ReportLib::ReportFont titleFont{fontName, titleFontSize, QFont::Normal};
	ReportLib::ReportFont regularFont{fontName, regularFontSize, QFont::Normal};

	// Report info

	if (pageLayout.orientation() == QPageLayout::Orientation::Portrait)
	{
		rs->addText(tr("\n\n\n"), {titleFont, Qt::AlignHCenter});
	}

	rs->addText(tr("\n\n\n\n\n\n\nProject: %1\n\n\n\n").arg(projectName), {titleFont, Qt::AlignHCenter});

	if (subreportName.isEmpty() == true)
	{
		rs->addText(tr("Differences Report\n\n\n\n\n\n"), {titleFont, Qt::AlignHCenter});
	}
	else
	{
		rs->addText(tr("Differences Report\n\n"), {titleFont, Qt::AlignHCenter});
		rs->addText(QObject::tr("%1\n\n\n\n\n").arg(subreportName), {regularFont, Qt::AlignHCenter});
	}

	// User name

	rs->addText(tr("User Name: %1\n\n\n").arg(userName), {regularFont, Qt::AlignLeft});

	// Changeset

	QString changesetStr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion:
		changesetStr = tr("Source: Latest Version");
		break;
	case CompareVersionType::Changeset:
		changesetStr = tr("Source Changeset: #%1").arg(compareData.sourceChangeset);
		break;
	case CompareVersionType::Date:
		changesetStr = tr("Source Date: %1").arg(compareData.sourceDate.toString("dd/MM/yyyy HH:mm:ss"));
		break;
	}

	rs->addText(tr("%1\n\n").arg(changesetStr), {regularFont, Qt::AlignLeft});

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion:
		changesetStr = tr("Target: Latest Version");
		break;
	case CompareVersionType::Changeset:
		changesetStr = tr("Target Changeset: #%1").arg(compareData.targetChangeset);
		break;
	case CompareVersionType::Date:
		changesetStr = tr("Target Date: %1").arg(compareData.targetDate.toString("dd/MM/yyyy HH:mm:ss"));
		break;
	}

	rs->addText(tr("%1\n\n").arg(changesetStr), {regularFont, Qt::AlignLeft});

	// Generation time

	rs->addText(tr("Generated: %1\n\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")), {regularFont, Qt::AlignRight});

	// RPCT Version

	rs->addText(tr("by %1 version %2\n\n").arg(qApp->applicationName()).arg(qApp->applicationVersion()), {regularFont, Qt::AlignRight});

	return rs;
}

void ProjectDiffGenerator::generateSummaryReport(const QPageLayout& pageLayout)
{
	// Create report
	//
	auto summaryReport = std::make_shared<Report>(m_projectName, filePath());
	summaryReport->setResolution(m_resolution);

	// Create title page
	//
	auto titlePageSection = generateTitlePage(pageLayout, m_reportParams.compareData, m_projectName, m_userName, QString());
	summaryReport->insertSection(0, titlePageSection);


	// Create files list page
	//
	QStringList generatedReportFiles;
	for (const auto& r : m_generatedReports)
	{
		generatedReportFiles.push_back(r->path());
	}

	{
		auto section = generateSummaryReportFilesPage(pageLayout, generatedReportFiles);
		summaryReport->addSection(section);
	}

	m_generatedReports.push_back(summaryReport);
}

std::shared_ptr<ReportSection> ProjectDiffGenerator::generateSummaryReportFilesPage(const QPageLayout& pageLayout,
																					const QStringList& subreportFiles)
{
	auto rs = ReportSection::create("SummaryFiles", pageLayout);

	// Init font

	QString fontName{"Arial"};
	int fontSize = lineFontSize(pageLayout, fontName, 50);

	ReportLib::ReportFont font{fontName, fontSize, QFont::Normal};

	// Create report

	rs->addText(tr("\n\n\n\nContent of this report is stored in the following files:\n\n"), {font, Qt::AlignHCenter});

	for (QString fileName : subreportFiles)
	{
		fileName = QDir::toNativeSeparators(fileName);

		qsizetype pos = fileName.lastIndexOf(QDir::separator());
		if (pos != -1)
		{
			fileName = fileName.right(fileName.length() - pos - 1);
		}

		rs->addText(tr("%1\n").arg(fileName), {font, Qt::AlignLeft});
	}

	return rs;
}

void ProjectDiffGenerator::createMarginItems(Report& report, const CompareData& compareData, const QString& subreportName)
{
	report.clearMarginItems();

	// Create headers/footers

	QString projectNameStr = tr("Project: ") + m_projectName;

	if (m_reportParams.singleFile == false && subreportName.isEmpty() == false)
	{
		projectNameStr += tr("; section: %1").arg(subreportName);
	}

	report.addMarginItem({projectNameStr, 2, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});

	report.addMarginItem({tr("%TAG%"), 2, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});

	QString changesetStr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::LatestVersion:
		changesetStr = tr("Source: Latest Version");
		break;
	case CompareVersionType::Changeset:
		changesetStr = tr("Source Changeset: #%1").arg(compareData.sourceChangeset);
		break;
	case CompareVersionType::Date:
		changesetStr = tr("Source: Date %1").arg(compareData.sourceDate.toString("dd/MM/yyyy HH:mm:ss"));
		break;
	}

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::LatestVersion:
		changesetStr += tr("; Target: Latest Version");
		break;
	case CompareVersionType::Changeset:
		changesetStr += tr("; Target Changeset: #%1").arg(compareData.targetChangeset);
		break;
	case CompareVersionType::Date:
		changesetStr += tr("; Target Date: %1").arg(compareData.targetDate.toString("dd/MM/yyyy HH:mm:ss"));
		break;
	}

	report.addMarginItem({changesetStr, 2, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignBottom}});

	report.addMarginItem({tr("%PAGE%"), 2, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
}

void ProjectDiffGenerator::fillDiffTable(ReportTable& diffTable, const std::vector<PropertyDiff>& diffs)
{
	for (const PropertyDiff& diff : diffs)
	{
		switch (diff.action)
		{
		case PropertyDiff::Action::Added:
			{
				if (diff.newValueText.length() > 256)
				{
					diffTable.insertRow({diff.caption, tr("Added"), QString(), tr("<Long Data Array>")});
				}
				else
				{
					diffTable.insertRow({diff.caption, tr("Added"), QString(), diff.newValueText});
				}
			}
			break;
		case PropertyDiff::Action::Removed:
			{
				diffTable.insertRow({diff.caption, tr("Removed"), QString(), QString()});
			}
			break;
		case PropertyDiff::Action::Modified:
			{
				if (diff.oldValueText.length() > 256 || diff.newValueText.length() > 256)
				{
					diffTable.insertRow({diff.caption, tr("Modified"), tr("<Long Data Array>"), tr("<Long Data Array>")});
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
					diffTable.insertRow({diff.caption, tr("Modified"), diff.oldValueText, diff.newValueText});
				}
			}
			break;
		}
	}

	diffTable.sortByColumn(0);

	return;
}

void ProjectDiffGenerator::addHeaderTableItem(ReportTable& headerTable,
											  const QString& caption,
											  const QString& action,
											  std::shared_ptr<DbFile> file)
{
	QString changesetStr = file->changeset() == 0 ? tr("Checked Out") : tr("%1").arg(file->changeset());

	Q_ASSERT(headerTable.columnCount() == 5);

	headerTable.insertRow(
		{caption, action, changesetStr, db()->username(file->userId()), file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")});

	return;
}

void ProjectDiffGenerator::addHeaderTableItem(ReportTable& headerTable,
											  const QString& caption,
											  const QString& action,
											  const AppSignal& signal)
{
	QString changesetStr = signal.changesetID() == 0 ? tr("Checked Out") : tr("%1").arg(signal.changesetID());

	Q_ASSERT(headerTable.columnCount() == 5);

	headerTable.insertRow(
		{caption, action, changesetStr, db()->username(signal.userID()), signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")});

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
		return tr("Changeset #%1 at %2 by %3")
			.arg(file->changeset())
			.arg(file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss"))
			.arg(db()->username(file->userId()));
	}
}

QString ProjectDiffGenerator::changesetString(const AppSignal& signal)
{
	if (signal.changesetID() == 0)
	{
		return tr("Checked Out %1 by %2")
			.arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss"))
			.arg(db()->username(signal.userID()));
	}
	else
	{
		return tr("Changeset #%1 at %2 by %3")
			.arg(signal.changesetID())
			.arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss"))
			.arg(db()->username(signal.userID()));
	}
}
