#include "ArchiveWidget.h"
#include "Globals.h"
#include "MonitorArchive.h"
#include "MonitorMainWindow.h"
#include "MonitorConfigController.h"
#include "MonitorSignalInfo.h"
#include "DialogChooseArchiveSignals.h"
#include <ReportLib/TableViewReportGenerator.h>
#include <ReportLib/Report.h>
#include <ReportLib/ReportObject.h>

//
// ArchiveExportPrint
//
namespace
{
	class ArchiveReportInfo : public ReportLib::ITableViewReportInfo
	{
	public:
		ArchiveReportInfo(ArchiveSource* source, QString projectName, QString softwareId);
		virtual ~ArchiveReportInfo() = default;

	protected:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const override;

		ArchiveSource* m_source = nullptr;
		QString m_projectName;
		QString m_softwareId;
	};


	ArchiveReportInfo::ArchiveReportInfo(ArchiveSource* source,
										   QString projectName,
										   QString softwareId) :
		m_source(source),
		m_projectName(projectName),
		m_softwareId(softwareId)
	{
	}

	void ArchiveReportInfo::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const
	{
		if (m_source == nullptr)
		{
			Q_ASSERT(m_source);
			return;
		}

		ReportLib::ReportFont marginFont{"Arial", 10};

		report.addMarginItem({QObject::tr("Generated: %1").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")),
							  -1,
							  -1,
							  {marginFont, Qt::AlignLeft | Qt::AlignTop}});

		report.addMarginItem({QObject::tr("Signals Archive"), -1, -1, {marginFont, Qt::AlignCenter | Qt::AlignTop}});

		report.addMarginItem({QObject::tr("Project: %1").arg(m_projectName), -1, -1, {marginFont, Qt::AlignRight | Qt::AlignTop}});

		report.addMarginItem(
			{QObject::tr("%1: %2").arg(qAppName()).arg(m_softwareId), -1, -1, {marginFont, Qt::AlignLeft | Qt::AlignBottom}});

		report.addMarginItem({"%PAGE%", -1, -1, {marginFont, Qt::AlignRight | Qt::AlignBottom}});

		ReportLib::ReportFont textFont{"Arial", 12};

		// Request parameters
		//
		QDateTime from = m_source->requestStartTime.toDateTime();
		QDateTime to = m_source->requestEndTime.toDateTime();


		if (from.date() == to.date())
		{
			mainSection.addText(QObject::tr("Requested interval: %1 - %2 (%3)\n\n")
									.arg(from.toString("dd/MM/yyyy HH:mm:ss"))
									.arg(to.toString("HH:mm:ss"))
									.arg(E::valueToString<E::TimeType>(m_source->timeType)),
								{textFont, Qt::AlignHCenter});
		}


		else
		{
			mainSection.addText(QObject::tr("Requested interval: %1 - %2 (%3)\n\n")
									.arg(from.toString("dd/MM/yyyy HH:mm:ss"))
									.arg(to.toString("dd/MM/yyyy HH:mm:ss"))
									.arg(E::valueToString<E::TimeType>(m_source->timeType)),
								{textFont, Qt::AlignHCenter});
		}

		// Services and signals
		//
		std::map<QString, std::vector<QString>> serviceToSignals;
		for (const ArchiveSignal& s : m_source->acceptedSignals)
		{
			serviceToSignals[s.archiveServiceShortenId].push_back(s.signalParam.customSignalId());
		}

		for (const auto& [service, signalIds] : serviceToSignals)
		{
			mainSection.addText(QObject::tr("Archive Service: %1\n").arg(service), {textFont, Qt::AlignLeft});

			QStringList signalList;
			signalList.reserve(signalIds.size());
			std::copy(signalIds.begin(), signalIds.end(), std::back_inserter(signalList));
			std::sort(signalList.begin(), signalList.end());

			mainSection.addText(QObject::tr("Signal(s): %1\n\n").arg(signalList.join(", ")), {textFont, Qt::AlignLeft});
		}

		return;
	}
}

//
//
//	MonitorArchiveWidget
//
//
ArchiveWidget::ArchiveWidget(ClientLib::AppSignalManager& signalManager,
							 MonitorConfigController* configController,
							 const AppSignalLists::AppSignalListSet& appSignalListSet,
							 QWidget* parent) :
	QMainWindow(parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_signalManager(signalManager),
	m_archiveConnection(*configController, configController->logFile(), this),
	m_appSignalListSet(appSignalListSet)
{
	static int no = 1;
	QString name = tr("Monitor Archive %1").arg(no++);
	MonitorArchive::registerWindow(name, this);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(name);
	setMinimumSize(QSize(750, 400));

	// --
	//
	{
		const auto configuration = configController->configuration();

		m_archiveServices = configuration.archiveServices;
		m_projectName = configuration.configInfo.project;
		m_softwareId = configuration.configInfo.softwareEquipmentId;
	}

	// --
	//
	m_source.timeType = static_cast<E::TimeType>(QSettings{}.value("ArchiveWindow/timeType").toInt());

	QDateTime currentTime = QDateTime::currentDateTime();

	m_source.requestEndTime = TimeStamp{TimeStamp(currentTime).timeStamp / 1_min * 1_min};		// reset seconds and ms
	m_source.requestStartTime = TimeStamp{m_source.requestEndTime.timeStamp - 1_hour};

	m_source.removePeriodicRecords = true;			// By defaut it's true, don't store it in theSettings as users often forget to set this option back!

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("ToolBar"), this);
	m_toolBar->setObjectName("MonitorArchiveToolBar");
	m_toolBar->setMovable(false);

	m_exportButton = new QPushButton(tr("Export..."), this);
	m_printButton = new QPushButton(tr("Print..."), this);
	m_updateButton = new QPushButton(tr("Update"), this);
	m_updateButton->setShortcut(QKeySequence(QKeySequence::StandardKey::Refresh));
	m_signalsButton = new QPushButton(tr("Signals..."), this);

	m_toolBar->addWidget(m_exportButton);
	m_toolBar->addWidget(m_printButton);
	m_toolBar->addSeparator();

	m_startDateTimeEdit = new QDateTimeEdit(m_source.requestStartTime.toDateTime(), this);
	m_startDateTimeEdit->setTimeSpec(Qt::UTC);
	m_startDateTimeEdit->setCalendarPopup(true);
	m_startDateTimeEdit->setDisplayFormat("dd/MM/yyyy  HH:mm:ss");
	m_startDateTimeEdit->setMinimumWidth(QFontMetrics(m_startDateTimeEdit->font()).horizontalAdvance("dd/MM/yyyy  HH:mm:ss") + 20);

	m_endDateTimeEdit = new QDateTimeEdit(m_source.requestEndTime.toDateTime(), this);
	m_endDateTimeEdit->setTimeSpec(Qt::UTC);
	m_endDateTimeEdit->setCalendarPopup(true);
	m_endDateTimeEdit->setDisplayFormat("dd/MM/yyyy  HH:mm:ss");
	m_endDateTimeEdit->setMinimumWidth(QFontMetrics(m_endDateTimeEdit->font()).horizontalAdvance("dd/MM/yyyy  HH:mm:ss") + 20);

	m_toolBar->addWidget(new QLabel(tr(" Start Time: ")));
	m_toolBar->addWidget(m_startDateTimeEdit);

	m_toolBar->addWidget(new QLabel(tr("   End Time: ")));
	m_toolBar->addWidget(m_endDateTimeEdit);

	// TimeType combo
	//
	m_timeType = new QComboBox(this);

	m_timeType->addItem(tr("Server Time"), QVariant::fromValue(E::TimeType::Local));
	m_timeType->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(E::TimeType::System));
	m_timeType->addItem(tr("Plant Time"), QVariant::fromValue(E::TimeType::Plant));

	int currentTimeType = m_timeType->findData(QVariant::fromValue(m_source.timeType));
	Q_ASSERT(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		m_timeType->setCurrentIndex(currentTimeType);
	}

	m_toolBar->addWidget(new QLabel(tr("   Time Type: ")));
	m_toolBar->addWidget(m_timeType);

	m_toolBar->addSeparator();

	// Add stretecher
	//
	QWidget* empty = new QWidget(this);
	empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_toolBar->addWidget(empty);

	m_toolBar->addWidget(m_updateButton);
	m_toolBar->addWidget(m_signalsButton);

	addToolBar(m_toolBar);

	// Status bar
	//
	m_statusBar = new QStatusBar(this);

	m_statusBarTextLabel = new QLabel(m_statusBar);
	m_statusBarStatesReceivedLabel = new QLabel(m_statusBar);
	m_statusBarNetworkRequestsLabel = new QLabel(m_statusBar);

	m_statusBar->addWidget(m_statusBarTextLabel, 1);
	m_statusBar->addWidget(m_statusBarStatesReceivedLabel, 0);
	m_statusBar->addWidget(m_statusBarNetworkRequestsLabel, 0);

	setStatusBar(m_statusBar);

	// --
	//
	connect(m_timeType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ArchiveWidget::timeTypeCurrentIndexChanged);

	connect(m_exportButton, &QPushButton::clicked, this, &ArchiveWidget::exportButton);
	connect(m_printButton, &QPushButton::clicked, this, &ArchiveWidget::printButton);
	connect(m_updateButton, &QPushButton::clicked, this, &ArchiveWidget::updateOrCancelButton);
	connect(m_signalsButton, &QPushButton::clicked, this, &ArchiveWidget::signalsButton);

	// Central widget - model/view
	//
	setContentsMargins(5, 5, 5, 5);

	m_view->setModel(m_model);
	setCentralWidget(m_view);

	m_view->setWordWrap(false);

	// --
	//
	auto archiveHorzHeader = QSettings{}.value("ArchiveWindow/horzHeader").toByteArray();
	auto archiveHorzHeaderCount = QSettings{}.value("ArchiveWindow/horzHeaderCount").toInt();

	if (archiveHorzHeader.isEmpty() == true || archiveHorzHeaderCount != static_cast<int>(ArchiveColumns::ColumnCount))
	{
		// First time? Set what is should be hidden by deafult
		//
		m_view->hideColumn(static_cast<int>(ArchiveColumns::AppSignalId));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Valid));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::StateAvailable));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Simulated));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Blocked));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Mismatch));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::OutOfLimits));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::ArchivingReason));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Duration));
	}

	connect(m_view, &ArchiveView::requestToShowSignalInfo, this, &ArchiveWidget::showSignalInfo);
	connect(m_view, &ArchiveView::requestToRemoveSignal, this, &ArchiveWidget::removeSignal);
	connect(m_view, &ArchiveView::requestToSetSignals, this, &ArchiveWidget::signalsButton);

	connect(&m_archiveConnection, &ArchiveConnection::dataReady, this, &ArchiveWidget::dataReceived);
	connect(&m_archiveConnection, &ArchiveConnection::requestError, this, &ArchiveWidget::requestError);
	connect(&m_archiveConnection, &ArchiveConnection::stats, this, &ArchiveWidget::requestStatus);
	connect(&m_archiveConnection, &ArchiveConnection::done, this, &ArchiveWidget::requestFinished);

	// --
	//
	connect(configController, &MonitorConfigController::configurationArrived, this, &ArchiveWidget::slot_configurationArrived);

	// --
	//
	setAcceptDrops(true);

	restoreWindowState();

	return;
}

ArchiveWidget::~ArchiveWidget()
{
	MonitorArchive::unregisterWindow(this->windowTitle());
	return;
}

void ArchiveWidget::ensureVisible()
{
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = screen()->availableGeometry();
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMinimized() == true)
	{
		showNormal();
	}

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7),
			   static_cast<int>(screenRect.height() * 0.7));
	}
}

bool ArchiveWidget::setSignals(const std::vector<AppSignalParam>& appSignals)
{
	std::vector<ArchiveSignal> acceptedSignals;
	acceptedSignals.reserve(appSignals.size());

	// Convert AppSignalParam to ArchiveSignals
	// if two or more archive services have this signal, pick up the first one
	//
	for (const AppSignalParam& signalParam : appSignals)
	{
		auto sit = std::find_if(m_archiveServices.begin(), m_archiveServices.end(),
					[&signalParam, &signalManager = m_signalManager](const SoftwareEndpoint::ArchiveService& archiveService)
					{
						return signalManager.dataServiceHasSignal(archiveService.appDataServiceId, signalParam.appSignalId());
					});

		if (sit != m_archiveServices.end())
		{
			acceptedSignals.emplace_back(signalParam, sit->equipmentId, sit->shortenId);
		}
	}

	if (acceptedSignals.size() != appSignals.size())
	{
		// Not all signals have assigned archive service
		//
	}

	return setSignals(std::move(acceptedSignals));
}

bool ArchiveWidget::setSignals(std::vector<ArchiveSignal> archiveSignals)
{
	m_source.acceptedSignals = std::move(archiveSignals);
	return true;
}

bool ArchiveWidget::setTime(QDateTime startTime, QDateTime endTime, E::TimeType timeType)
{
	m_source.requestStartTime = TimeStamp(startTime);
	m_startDateTimeEdit->setDateTime(startTime);

	m_source.requestEndTime = TimeStamp(endTime);
	m_endDateTimeEdit->setDateTime(endTime);

	m_source.timeType = timeType;

	int currentTimeType = m_timeType->findData(QVariant::fromValue(m_source.timeType));
	Q_ASSERT(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		m_timeType->setCurrentIndex(currentTimeType);
	}

	return true;
}

void ArchiveWidget::requestData()
{
	if (m_source.acceptedSignals.empty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Select signal(s) to request data from archive."));
		return;
	}

	QSettings{}.setValue("ArchiveWindow/timeType", static_cast<int>(m_source.timeType));

	m_model->clear();
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	m_source.requestStartTime = TimeStamp(m_startDateTimeEdit->dateTime());
	m_source.requestEndTime = TimeStamp(m_endDateTimeEdit->dateTime());
	m_source.timeType = m_timeType->currentData().value<E::TimeType>();

	// Request data from archive
	//
	m_archiveConnection.request(m_source);

	// Update user interface
	//
	updateUiState();

	return;
}

void ArchiveWidget::cancelRequest()
{
	m_archiveConnection.cancelRequest();
	updateUiState();
	return;
}



void ArchiveWidget::closeEvent(QCloseEvent*e)
{
	saveWindowState();
	e->accept();
	return;
}

void ArchiveWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void ArchiveWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		Q_ASSERT(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data
	//
	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam signalParam;
		ok = signalParam.load(appSignalMessage);

		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		// Check if such signal already present in the signal list
		//
		auto foundId = std::find_if(m_source.acceptedSignals.begin(), m_source.acceptedSignals.end(),
			[signalParam](const ArchiveSignal& sp)
			{
				return sp.signalParam.appSignalId() == signalParam.appSignalId();
			});

		if (foundId != m_source.acceptedSignals.end())
		{
			// The signal already in the list
			//
			continue;
		}

		// Find an archive service for the signal and add it to the signal list
		//
		auto sit = std::find_if(m_archiveServices.begin(), m_archiveServices.end(),
						[&signalParam, &signalManager = m_signalManager](const SoftwareEndpoint::ArchiveService& archiveService)
						{
							return signalManager.dataServiceHasSignal(archiveService.appDataServiceId, signalParam.appSignalId());
						});

		if (sit != m_archiveServices.end())
		{
			m_source.acceptedSignals.emplace_back(signalParam, sit->equipmentId, sit->shortenId);
		}
		else
		{
			// Archive service not found for this signal
			//
		}
	}

	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	return;
}

void ArchiveWidget::saveWindowState()
{
	QSettings s{};

	s.setValue("ArchiveWindow/pos", pos());
	s.setValue("ArchiveWindow/geometry", saveGeometry());
	s.setValue("ArchiveWindow/state", saveState());

	return;
}

void ArchiveWidget::restoreWindowState()
{
	QSettings s{};

	auto archiveWindowPos = s.value("ArchiveWindow/pos", QPoint(200, 200)).toPoint();
	auto archiveWindowGeometry = s.value("ArchiveWindow/geometry").toByteArray();
	auto archiveWindowState = s.value("ArchiveWindow/state").toByteArray();

	move(archiveWindowPos);
	restoreGeometry(archiveWindowGeometry);
	restoreState(archiveWindowState);

	ensureVisible();

	return;
}

void ArchiveWidget::updateUiState()
{
	bool disable = m_archiveConnection.requestInProgress();
	bool requestInProgress = disable;

	m_exportButton->setDisabled(disable);
	m_printButton->setDisabled(disable);
	m_signalsButton->setDisabled(disable);

	if (requestInProgress == true)
	{
		m_updateButton->setText(tr("Cancel"));
		m_updateButton->setShortcut(QKeySequence(QKeySequence::StandardKey::Cancel));
	}
	else
	{
		m_updateButton->setText(tr("Update"));
		m_updateButton->setShortcut(QKeySequence(QKeySequence::StandardKey::Refresh));
	}

	return;
}

void ArchiveWidget::timeTypeCurrentIndexChanged(int /*index*/)
{
	Q_ASSERT(m_timeType);

	m_source.timeType = m_timeType->currentData().value<E::TimeType>();
	QSettings{}.setValue("ArchiveWindow/timeType", static_cast<int>(m_source.timeType));

	return;
}

void ArchiveWidget::exportButton()
{
	Q_ASSERT(m_model);
	if (m_model->rowCount() == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("Nothing to export."));
		return;
	}

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													path + QDir::separator() + "untitled.pdf",
													tr("Portable Documnet Format (*.pdf);;CSV Files, semicolon separated (*.csv);;Plaintext (*.txt);;HTML (*.html)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QFileInfo fileInfo(fileName);
	QString extension = fileInfo.completeSuffix();

	if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
	{
		QPageLayout pageLayout(QPageSize(QPageSize::A4),
							   QPageLayout::Orientation::Landscape,
							   QMarginsF(25, 20, 15, 20),
							   QPageLayout::Unit::Millimeter);

		pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("ArchiveExportPageLayout", pageLayout);

		ArchiveReportInfo ri(&m_source, m_projectName, m_softwareId);
		ReportLib::TableViewReportGenerator generator(this, *m_view, ri, pageLayout);
		generator.exportTable(fileName);
		pageLayout = generator.pageLayout();

		ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "ArchiveExportPageLayout");
		return;
	}

	QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
	return;
}

void ArchiveWidget::printButton()
{
	QPageLayout pageLayout(QPageSize(QPageSize::A4),
						   QPageLayout::Orientation::Landscape,
						   QMarginsF(10, 10, 10, 10),
						   QPageLayout::Unit::Millimeter);

	pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("ArchivePrintPageLayout", pageLayout);

	ArchiveReportInfo ri(&m_source, m_projectName, m_softwareId);
	ReportLib::TableViewReportGenerator generator(this, *m_view, ri, pageLayout);
	generator.printTable();
	pageLayout = generator.pageLayout();

	ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "ArchivePrintPageLayout");

	return;
}

void ArchiveWidget::updateOrCancelButton()
{
	if (m_updateButton->text() == tr("Update"))
	{
		if (m_source.acceptedSignals.empty() == true)
		{
			QMessageBox::warning(this, qAppName(), tr("Select at least one signal to request archive data."));
			return;
		}

		requestData();
	}
	else
	{
		cancelRequest();
	}
	return;

}

void ArchiveWidget::signalsButton()
{
	DialogChooseArchiveSignals dialog(m_signalManager, m_archiveServices, m_source, m_appSignalListSet,
									  this);

	int result = dialog.exec();

	if (result == QDialog::Rejected)
	{
		return;
	}

	m_source = dialog.accpetedResult();

	// Request data from archive
	//
	TimeStamp tsStart = qMin(m_source.requestStartTime, m_source.requestEndTime);
	TimeStamp tsEnd = qMax(m_source.requestStartTime, m_source.requestEndTime);

	m_startDateTimeEdit->setDateTime(tsStart.toDateTime());
	m_endDateTimeEdit->setDateTime(tsEnd.toDateTime());

	int currentTimeType = m_timeType->findData(QVariant::fromValue(m_source.timeType));
	Q_ASSERT(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		m_timeType->setCurrentIndex(currentTimeType);
	}

	requestData();

	return;
}

void ArchiveWidget::showSignalInfo(QString appSignalId)
{
	MonitorSignalInfo::showDialog(appSignalId,
								  m_signalManager,
								  theApp.mainWindow()->tuningSignalManager(),
								  theApp.mainWindow()->tuningConnection(),
								  theApp.mainWindow()->tuningAuthorization(),
								  &theApp.mainWindow()->configController(),
								  &theApp.mainWindow()->monitorCentralWidget());
}

void ArchiveWidget::removeSignal(QString appSignalId, QString archiveServiceId)
{
	std::erase_if(m_source.acceptedSignals,
				[&appSignalId, &archiveServiceId](const ArchiveSignal& as)
				{
					return as.signalParam.appSignalId() == appSignalId && as.archiveServiceId == archiveServiceId;
				});

	m_model->removeSignal(appSignalId, archiveServiceId);
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	return;
}


void ArchiveWidget::slot_configurationArrived(MonitorConfigSettings configuration)
{
	m_archiveServices = std::move(configuration.archiveServices);
	m_projectName = configuration.configInfo.project;
	m_softwareId = configuration.configInfo.softwareEquipmentId;

	return;
}

void ArchiveWidget::dataReceived(std::shared_ptr<ArchiveRequestResult> chunk)
{
	if (chunk == nullptr)
	{
		Q_ASSERT(chunk);
		return;
	}

	m_model->addData(std::move(*chunk));

	updateUiState();
	return;
}

void ArchiveWidget::requestError(QString errorMessage)
{
	QMessageBox::critical(this, qAppName(), errorMessage);
	updateUiState();
	return;
}

void ArchiveWidget::requestStatus(QString serverStatus, int requests, int replies, int states)
{
	Q_ASSERT(m_statusBar);

	m_statusBarTextLabel->setText(serverStatus);
	m_statusBarStatesReceivedLabel->setText(tr("States received: %1").arg(states));

	m_statusBarNetworkRequestsLabel->setText(tr(" Network requests/replies: %1 / %2 ").arg(requests).arg(replies));

	updateUiState();
	return;
}

void ArchiveWidget::requestFinished()
{
	updateUiState();
	return;
}
