#include "MonitorArchive.h"
#include "Settings.h"
#include <QMessageBox>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextBlock>
#include <QPrinter>
#include <QPageSetupDialog>
#include <QProgressDialog>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include "DialogSignalInfo.h"

std::map<QString, MonitorArchiveWidget*> MonitorArchive::m_archiveList;

std::vector<QString> MonitorArchive::getArchiveList()
{
	std::vector<QString> result;
	result.reserve(m_archiveList.size());

	for (std::pair<QString, MonitorArchiveWidget*> p : m_archiveList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorArchive::activateWindow(QString archiveName)
{
	if (m_archiveList.count(archiveName) != 1)
	{
		assert(m_archiveList.count(archiveName) != 1);
		return false;
	}

	MonitorArchiveWidget* widget = m_archiveList[archiveName];
	assert(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorArchive::startNewWidget(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	MonitorArchiveWidget* window = new MonitorArchiveWidget(configController, parent);

	window->setSignals(appSignals);

	window->show();

	return false;
}

void MonitorArchive::registerWindow(QString name, MonitorArchiveWidget* window)
{
	assert(m_archiveList.count(name) == 0);
	m_archiveList[name] = window;

	return;
}

void MonitorArchive::unregisterWindow(QString name)
{
	assert(m_archiveList.count(name) == 1);
	m_archiveList.erase(name);

	return;
}

MonitorArchiveWidget::MonitorArchiveWidget(MonitorConfigController* configController, QWidget* parent) :
	QMainWindow(parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_archiveService1(configController->configuration().archiveService1),
	m_archiveService2(configController->configuration().archiveService2),
	m_schemasDetais(configController->schemasDetails()),
	m_configuration(configController->configuration())
{
	setAttribute(Qt::WA_DeleteOnClose);

static int no = 1;
	QString name = QString("Monitor Archive %1").arg(no++);
	MonitorArchive::registerWindow(name, this);

	setWindowTitle(name);

	setMinimumSize(QSize(750, 400));

	// --
	//
	m_source.timeType = static_cast<E::TimeType>(theSettings.m_archiveTimeType);

	QDateTime currentTime = QDateTime::currentDateTime();

	m_source.requestEndTime = TimeStamp(currentTime).timeStamp / 1_min * 1_min;		// reset seconds and ms
	m_source.requestStartTime = m_source.requestEndTime.timeStamp - 1_hour;

	m_source.removePeriodicRecords = true;			// By defaut it's true, don't store it in theSettings as users often forget to set this option back

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("ToolBar"));
	m_toolBar->setMovable(false);

	m_exportButton = new QPushButton(tr("Export..."));
	m_printButton = new QPushButton(tr("Print..."));
	m_updateButton = new QPushButton(tr("Update"));
	m_updateButton->setShortcut(QKeySequence(QKeySequence::StandardKey::Refresh));
	m_signalsButton = new QPushButton(tr("Signals..."));

	m_toolBar->addWidget(m_exportButton);
	m_toolBar->addWidget(m_printButton);
	m_toolBar->addSeparator();

	m_startDateTimeEdit = new QDateTimeEdit(m_source.requestStartTime.toDateTime());
	m_startDateTimeEdit->setTimeSpec(Qt::UTC);
	m_startDateTimeEdit->setCalendarPopup(true);
	m_startDateTimeEdit->setDisplayFormat("dd/MM/yyyy  HH:mm:ss");

	m_endDateTimeEdit = new QDateTimeEdit(m_source.requestEndTime.toDateTime());
	m_endDateTimeEdit->setTimeSpec(Qt::UTC);
	m_endDateTimeEdit->setCalendarPopup(true);
	m_endDateTimeEdit->setDisplayFormat("dd/MM/yyyy  HH:mm:ss");

	m_toolBar->addWidget(new QLabel(tr(" Start Time: ")));
	m_toolBar->addWidget(m_startDateTimeEdit);

	m_toolBar->addWidget(new QLabel(tr("   End Time: ")));
	m_toolBar->addWidget(m_endDateTimeEdit);

	// TimeType combo
	//
	m_timeType = new QComboBox;

	m_timeType->addItem(tr("Server Time"), QVariant::fromValue(E::TimeType::Local));
	m_timeType->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(E::TimeType::System));
	m_timeType->addItem(tr("Plant Time"), QVariant::fromValue(E::TimeType::Plant));

	int currentTimeType = m_timeType->findData(QVariant::fromValue(m_source.timeType));
	assert(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		m_timeType->setCurrentIndex(currentTimeType);
	}

	m_toolBar->addWidget(new QLabel(tr("   Time Type: ")));
	m_toolBar->addWidget(m_timeType);

	m_toolBar->addSeparator();

	// Add stretecher
	//
	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_toolBar->addWidget(empty);

	m_toolBar->addWidget(m_updateButton);
	m_toolBar->addWidget(m_signalsButton);

	addToolBar(m_toolBar);

	// Status bar
	//
	m_statusBar = new QStatusBar;

	m_statusBarTextLabel = new QLabel(m_statusBar);
	m_statusBarStatesReceivedLabel = new QLabel(m_statusBar);
	m_statusBarNetworkRequestsLabel = new QLabel(m_statusBar);
	m_statusBarServerLabel = new QLabel(m_statusBar);
	m_statusBarConnectionStateLabel = new QLabel(m_statusBar);

	m_statusBar->addWidget(m_statusBarTextLabel, 1);
	m_statusBar->addWidget(m_statusBarStatesReceivedLabel, 0);
	m_statusBar->addWidget(m_statusBarNetworkRequestsLabel, 0);
	m_statusBar->addWidget(m_statusBarServerLabel, 0);
	m_statusBar->addWidget(m_statusBarConnectionStateLabel, 0);

	setStatusBar(m_statusBar);

	// --
	//
	connect(m_timeType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MonitorArchiveWidget::timeTypeCurrentIndexChanged);

	connect(m_exportButton, &QPushButton::clicked, this, &MonitorArchiveWidget::exportButton);
	connect(m_printButton, &QPushButton::clicked, this, &MonitorArchiveWidget::printButton);
	connect(m_updateButton, &QPushButton::clicked, this, &MonitorArchiveWidget::updateOrCancelButton);
	connect(m_signalsButton, &QPushButton::clicked, this, &MonitorArchiveWidget::signalsButton);

	// Central widget - model/view
	//
	setContentsMargins(2, 2, 2, 2);

	m_view->setModel(m_model);
	setCentralWidget(m_view);

	if (theSettings.m_archiveHorzHeader.isEmpty() == true)
	{
		// First time? Set what is should be hidden by deafult
		//
		m_view->hideColumn(static_cast<int>(ArchiveColumns::AppSignalId));
		m_view->hideColumn(static_cast<int>(ArchiveColumns::Valid));
	}

	connect(m_view, &ArchiveView::requestToShowSignalInfo, this, &MonitorArchiveWidget::showSignalInfo);
	connect(m_view, &ArchiveView::requestToRemoveSignal, this, &MonitorArchiveWidget::removeSignal);
	connect(m_view, &ArchiveView::requestToSetSignals, this, &MonitorArchiveWidget::signalsButton);


	// Communication thread
	//
	m_tcpClient = new ArchiveTcpClient(configController);
	m_tcpClientThread = new SimpleThread(m_tcpClient);

	m_tcpClientThread->start();

	connect(m_tcpClient, &ArchiveTcpClient::dataReady, this, &MonitorArchiveWidget::dataReceived);
	connect(m_tcpClient, &ArchiveTcpClient::requestError, this, &MonitorArchiveWidget::tcpClientError);
	connect(m_tcpClient, &ArchiveTcpClient::statusUpdate, this, &MonitorArchiveWidget::tcpStatus);
	connect(m_tcpClient, &ArchiveTcpClient::requestIsFinished, this, &MonitorArchiveWidget::tcpRequestFinished);

	// --
	//
	connect(configController, &MonitorConfigController::configurationArrived, this, &MonitorArchiveWidget::slot_configurationArrived);

	// --
	//
	setAcceptDrops(true);

	restoreWindowState();

	startTimer(100);

	return;
}

MonitorArchiveWidget::~MonitorArchiveWidget()
{
	MonitorArchive::unregisterWindow(this->windowTitle());

	assert(m_tcpClientThread);
	m_tcpClientThread->quitAndWait(10000);

	return;
}

void MonitorArchiveWidget::ensureVisible()
{
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

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
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
	}
}

bool MonitorArchiveWidget::setSignals(const std::vector<AppSignalParam>& appSignals)
{
	m_source.acceptedSignals = appSignals;
	return true;
}


void MonitorArchiveWidget::requestData()
{
	if (m_tcpClient == nullptr ||
		m_tcpClientThread == nullptr)
	{
		assert(m_tcpClient);
		assert(m_tcpClientThread);
		return;
	}

	if (m_tcpClientThread->isRunning() == false)
	{
		assert(m_tcpClientThread->isRunning() == true);
		return;
	}

	if (m_tcpClient->isRequestInProgress() == true)
	{
		assert(m_tcpClient->isRequestInProgress());
		return;
	}

	m_source.requestStartTime = TimeStamp(m_startDateTimeEdit->dateTime());
	m_source.requestEndTime = TimeStamp(m_endDateTimeEdit->dateTime());

	m_source.timeType = m_timeType->currentData().value<E::TimeType>();
	theSettings.m_archiveTimeType = static_cast<int>(m_source.timeType);

	m_tcpClient->requestData(m_source.requestStartTime,
							 m_source.requestEndTime,
							 m_source.timeType,
							 m_source.removePeriodicRecords,
							 m_source.acceptedSignals);

	m_exportButton->setEnabled(false);
	m_printButton->setEnabled(false);
	m_signalsButton->setEnabled(false);

	m_updateButton->setText(tr("Cancel"));

	return;
}

void MonitorArchiveWidget::cancelRequest()
{
	if (m_tcpClient == nullptr ||
		m_tcpClientThread == nullptr)
	{
		assert(m_tcpClient);
		assert(m_tcpClientThread);
		return;
	}

	if (m_tcpClientThread->isRunning() == false)
	{
		assert(m_tcpClientThread->isRunning() == true);
		return;
	}

	if (m_tcpClient->isRequestInProgress() == false)
	{
		return;
	}

	m_tcpClient->cancelRequest();

	assert(m_tcpClient->isRequestInProgress() == false);

	return;
}

bool MonitorArchiveWidget::exportToTextDocument(QTextDocument* doc, bool onlySelectedRows)
{
	if (doc == nullptr)
	{
		assert(doc);
		return false;
	}

	QTextCursor cursor(doc);

	// Generate Header
	//
	QTextBlockFormat headerCenterFormat = cursor.blockFormat();
	headerCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat regularFormat = cursor.blockFormat();
	regularFormat.setAlignment(Qt::AlignLeft);

	QTextCharFormat headerCharFormat = cursor.charFormat();
	headerCharFormat.setFontWeight(static_cast<int>(QFont::Bold));
	headerCharFormat.setFontPointSize(12.0);

	QTextCharFormat regularCharFormat = cursor.charFormat();
	headerCharFormat.setFontPointSize(10.0);

	cursor.setBlockFormat(headerCenterFormat);
	cursor.setCharFormat(headerCharFormat);
	cursor.insertText(tr("Archive - %1\n").arg(m_configuration.project));
	cursor.insertText("\n");

	cursor.setBlockFormat(regularFormat);
	cursor.setCharFormat(regularCharFormat);
	cursor.insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	cursor.insertText(tr("Monitor: %1\n").arg(m_configuration.softwareEquipmentId));
	cursor.insertText("\n");

	QDateTime from = m_source.requestStartTime.toDateTime();
	QDateTime to = m_source.requestEndTime.toDateTime();

	if (from.date() == to.date())
	{
		cursor.insertText(tr("Requested interval: %1 - %2\n").arg(from.toString("dd/MM/yyyy  HH:mm:ss")).arg(to.toString("HH:mm:ss")));
	}
	else
	{
		cursor.insertText(tr("Requested interval:: %1 - %2\n").arg(from.toString("dd/MM/yyyy  HH:mm:ss")).arg(to.toString("dd/MM/yyyy  HH:mm:ss")));
	}

	cursor.insertText("Signal(s): ");
	for (const AppSignalParam& s : m_source.acceptedSignals)
	{
		cursor.insertText(QString(" %1,").arg(s.customSignalId()));
	}
	cursor.deletePreviousChar();	// Delete last comma
	cursor.insertText("\n");
	cursor.insertText("\n");

	// States
	//
	int rowCount = qBound(0, m_model->rowCount(), m_maxReportStates);		// Limit row count to m_maxReportStates

	// reinit rowCount if printing only selected rows
	//
	QModelIndexList selectedIndexes;
	if (onlySelectedRows == true)
	{
		selectedIndexes = m_view->selectionModel()->selectedRows();
		qSort(selectedIndexes);

		rowCount = selectedIndexes.size();
	}

	int columnCount = m_model->columnCount();

	std::vector<int> shownColums;
	shownColums.reserve(columnCount);

	int shownColumnCount = 0;
	for (int column = 0; column < columnCount; column++)
	{
		if (m_view->isColumnHidden(column) == false)
		{
			shownColumnCount ++;
			shownColums.push_back(column);
		}
	}

	QProgressDialog progressDialog(tr("Generating report..."), tr("Cancel"), 0, rowCount, this);
	progressDialog.setMinimumDuration(100);

	// Add table
	//
	QTextTableFormat tableFormat;

	tableFormat.setHeaderRowCount(1);
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
	tableFormat.setBorder(0);
	tableFormat.setBorderBrush(Qt::NoBrush);
	tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));

	cursor.insertTable(rowCount + 1, shownColumnCount, tableFormat);	// VERY HEAVY OPERATION

	// Fill table header
	//
	for (int column : shownColums)
	{
		QString columnHeader = m_model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();

		cursor.insertText(columnHeader);
		cursor.movePosition(QTextCursor::NextCell);
	}

	// Fill table
	//
	if (onlySelectedRows == false)
	{
		QString cellText;
		for (int row = 0; row < rowCount; row++)
		{
			progressDialog.setValue(row);
			if (progressDialog.wasCanceled() == true)
			{
				break;
			}

			QApplication::processEvents();

			for (int column : shownColums)
			{
				cellText = m_model->data(row, column, Qt::DisplayRole).toString();

				cursor.insertText(cellText);
				cursor.movePosition(QTextCursor::NextCell);
			}
		}
	}
	else
	{
		QString cellText;
		for (const QModelIndex mi : selectedIndexes)
		{
			int row = mi.row();

			progressDialog.setValue(row);
			if (progressDialog.wasCanceled() == true)
			{
				break;
			}

			QApplication::processEvents();

			for (int column : shownColums)
			{
				cellText = m_model->data(row, column, Qt::DisplayRole).toString();

				cursor.insertText(cellText);
				cursor.movePosition(QTextCursor::NextCell);
			}
		}
	}

	cursor.movePosition(QTextCursor::End);

	cursor.insertText("\n\n");

	if (m_model->rowCount() > m_maxReportStates)
	{
		cursor.insertText(tr("Warning: Only first %1 of %2 records present in generated report.\n").arg(rowCount).arg(m_model->rowCount()));
	}

	progressDialog.setValue(rowCount);

	return !progressDialog.wasCanceled();
}

bool MonitorArchiveWidget::saveArchiveWithDocWriter(QString fileName, QString format)
{
	if (m_model == nullptr ||
		m_view == nullptr ||
		fileName.isEmpty() == true)
	{
		assert(m_model);
		assert(m_view);
		assert(fileName.isEmpty() == false);
		return false;
	}

	if (m_model->rowCount() > m_maxReportStates)
	{
		QMessageBox::warning(this, qAppName(), tr("Too many archive records, only first %1 states will appear in the generated report.").arg(m_maxReportStates));
	}

	bool isPdf = format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0;
	bool isOdt = format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0;

	if (format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
		return false;
	}

	// Get page size, margins and orientation
	//
	std::unique_ptr<QPageSetupDialog> pageSetupDialog;

	if (isPdf == true || isOdt == true)
	{
		pageSetupDialog = std::make_unique<QPageSetupDialog>(this);

		int pageSetupResult = pageSetupDialog->exec();

		if (pageSetupResult != QDialog::Accepted)
		{
			return false;
		}
	}

	// Set wait cursor
	//
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	// --
	//
	QPrinter printer(QPrinter::PrinterResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);

	if (pageSetupDialog != nullptr)
	{
		printer.setPageLayout(pageSetupDialog->printer()->pageLayout());
	}

	// Generate doc
	//
	QTextDocument doc;

	if (pageSetupDialog != nullptr)
	{
		QSizeF pageSize = pageSetupDialog->printer()->pageRect().size();
		doc.setPageSize(pageSize);
	}

	 exportToTextDocument(&doc, false);

	// --
	//
	if (format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
	{
		printer.setOutputFileName(fileName);
		doc.print(&printer);
	}
	else
	{
		QByteArray docFormat;

		if (format.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
			format.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "html";
		}

		if (format.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "plaintext";
		}

		if (format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "odf";
		}

		QTextDocumentWriter writer(fileName);
		writer.setFormat(docFormat);

		writer.write(&doc);
	}

	// Restore cursor from wait
	//
	QApplication::restoreOverrideCursor();
	QApplication::processEvents();

	return true;
}

bool MonitorArchiveWidget::saveArchiveToCsv(QString fileName)
{
	if (m_model == nullptr ||
		m_view == nullptr ||
		fileName.isEmpty() == true)
	{
		assert(m_model);
		assert(m_view);
		assert(fileName.isEmpty() == false);
		return false;
	}

	if (m_model->rowCount() > m_maxReportStatesForCsv )
	{
		QMessageBox::warning(this, qAppName(), tr("Too many archive records, only first %1 states will appear in the generated report.").arg(m_maxReportStatesForCsv ));
	}

	// --
	//
	QFile file(fileName);

	bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open file %1 for writing.").arg(fileName));
		return false;
	}

	QTextStream out(&file);

	// Set wait cursor
	//
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	// States
	//
	int rowCount = qBound(0, m_model->rowCount(), m_maxReportStatesForCsv);		// Limit row count to m_maxReportStates
	int columnCount = m_model->columnCount();

	std::vector<int> shownColums;
	shownColums.reserve(columnCount);

	int shownColumnCount = 0;
	for (int column = 0; column < columnCount; column++)
	{
		if (m_view->isColumnHidden(column) == false)
		{
			shownColumnCount ++;
			shownColums.push_back(column);
		}
	}

	QProgressDialog progressDialog(tr("Generating report..."), tr("Cancel"), 0, rowCount, this);
	progressDialog.setMinimumDuration(100);

	// Fill header
	//
	for (int column : shownColums)
	{
		QString columnHeader = m_model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
		out << columnHeader << ";";
	}
	out << endl;

	// Fill table
	//
	QString cellText;
	for (int row = 0; row < rowCount; row++)
	{
		progressDialog.setValue(row);
		if (progressDialog.wasCanceled() == true)
		{
			break;
		}

		QApplication::processEvents();

		if (row % 10000 == 0)
		{
			progressDialog.setLabelText(tr("Generating report... %1/%2").arg(row).arg(rowCount));
		}

		for (int column : shownColums)
		{
			cellText = m_model->data(row, column, Qt::DisplayRole).toString();

			if (cellText.contains(';') == true)
			{
				// If cell contains semicolon it must be enclosed in quotes
				//
				cellText.prepend('"');
				cellText.append('"');
			}

			out << cellText << ";";
		}

		out << endl;
	}

	progressDialog.setValue(rowCount);

	// Restore cursor from wait
	//
	QApplication::restoreOverrideCursor();
	QApplication::processEvents();

	return true;
}

void MonitorArchiveWidget::closeEvent(QCloseEvent*e)
{
	saveWindowState();
	e->accept();
	return;
}

void MonitorArchiveWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void MonitorArchiveWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		assert(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

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

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			auto foundId = std::find_if(m_source.acceptedSignals.begin(), m_source.acceptedSignals.end(),
										[&appSignalParam](const AppSignalParam& sp)
										{
											return sp.appSignalId() == appSignalParam.appSignalId();
										});

			if (foundId == m_source.acceptedSignals.end())
			{
				m_source.acceptedSignals.push_back(appSignalParam);
			}
		}
	}

	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	return;

}

void MonitorArchiveWidget::saveWindowState()
{
	theSettings.m_archiveWindowPos = pos();
	theSettings.m_archiveWindowGeometry = saveGeometry();
	theSettings.m_archiveWindowState = saveState();

	theSettings.writeUserScope();
}

void MonitorArchiveWidget::restoreWindowState()
{
	move(theSettings.m_archiveWindowPos);
	restoreGeometry(theSettings.m_archiveWindowGeometry);
	restoreState(theSettings.m_archiveWindowState);

	ensureVisible();

	return;
}

void MonitorArchiveWidget::timeTypeCurrentIndexChanged(int /*index*/)
{
	assert(m_timeType);

	m_source.timeType = m_timeType->currentData().value<E::TimeType>();
	theSettings.m_archiveTimeType = static_cast<int>(m_source.timeType);

	return;
}

void MonitorArchiveWidget::exportButton()
{
	assert(m_model);
	if (m_model->rowCount() == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("Nothing to export."));
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													"untitled.pdf",
													tr("Portable Documnet Format (*.pdf);;CSV Files, semicolon separated (*.csv);;Plaintext (*.txt);;HTML (*.html)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

	QFileInfo fileInfo(fileName);
	QString extension = fileInfo.completeSuffix();

	if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0)
	{
		saveArchiveToCsv(fileName);
		return;
	}

	if (extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0/* ||
		extension.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0*/)
	{
		saveArchiveWithDocWriter(fileName, extension);
		return;
	}

	QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
	return;
}

void MonitorArchiveWidget::printButton()
{
	if (m_model == nullptr ||
		m_view == nullptr)
	{
		assert(m_model);
		assert(m_view);
		return;
	}

	QPrintDialog dialog(this);

	if (m_view->selectionModel()->hasSelection() == true)
	{
		dialog.setOption(QAbstractPrintDialog::PrintSelection);
		dialog.setOptions(dialog.options() & ~QAbstractPrintDialog::PrintPageRange);

		//dialog.printer()->setPrintRange(QPrinter::PrintRange::Selection);				// Set Selection option by default
	}

	int result = dialog.exec();

	if (result == QDialog::Accepted)
	{
		printRequested(dialog.printer());
	}

//	QPrintPreviewDialog printDialog(this);

//	printDialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
//	printDialog.setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

//	if (m_view->selectionModel()->hasSelection() == true)
//	{
//		qDebug() << "Print selection enabled";
//		printDialog.printer()->setPrintRange(QPrinter::PrintRange::Selection);
//	}

	//connect(&printDialog, &QPrintPreviewDialog::paintRequested, this, &MonitorArchiveWidget::printRequested);

//	printDialog.exec();

	return;
}

void MonitorArchiveWidget::updateOrCancelButton()
{
	if (m_updateButton->text() == "Update")
	{
		if (m_source.acceptedSignals.empty() == true)
		{
			QMessageBox::warning(this, qAppName(), tr("To request data from archive add at least one signal."));
			return;
		}

		m_model->clear();
		m_model->setParams(m_source.acceptedSignals, m_source.timeType);

		requestData();
	}
	else
	{
		cancelRequest();
	}
	return;

}

void MonitorArchiveWidget::signalsButton()
{
	DialogChooseArchiveSignals dialog(m_schemasDetais, m_source, this);

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
	assert(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		m_timeType->setCurrentIndex(currentTimeType);
	}

	m_model->clear();
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	requestData();

	return;
}

void MonitorArchiveWidget::showSignalInfo(QString appSignalId)
{
	DialogSignalInfo::showDialog(appSignalId, this->parentWidget());
}

void MonitorArchiveWidget::removeSignal(QString appSignalId)
{
	std::vector<AppSignalParam> result;
	result.reserve(m_source.acceptedSignals.size());

	for (const AppSignalParam& sp : m_source.acceptedSignals)
	{
		if (sp.appSignalId() != appSignalId)
		{
			result.push_back(sp);
		}
	}

	m_source.acceptedSignals.swap(result);

	m_model->clear();
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	return;
}

void MonitorArchiveWidget::printRequested(QPrinter* printer)
{
	if (printer == nullptr)
	{
		assert(printer);
	}

	QTextDocument doc;

	QSizeF pageSize = printer->pageRect().size();
	doc.setPageSize(pageSize);

	exportToTextDocument(&doc, printer->printRange() == QPrinter::PrintRange::Selection);

	doc.print(printer);

	return;
}

void MonitorArchiveWidget::slot_configurationArrived(ConfigSettings configuration)
{
	m_configuration = configuration;
}

void MonitorArchiveWidget::dataReceived(std::shared_ptr<ArchiveChunk> chunk)
{
	if (chunk == nullptr)
	{
		assert(chunk);
		return;
	}

	m_model->addData(chunk);

	return;
}

void MonitorArchiveWidget::tcpClientError(QString errorMessage)
{
	QMessageBox::critical(this, qAppName(), errorMessage);
	return;
}

void MonitorArchiveWidget::tcpStatus(QString status, int statesReceived, int requestCount, int repliesCount)
{
	assert(m_statusBar);

	m_statusBarTextLabel->setText(status);
	m_statusBarStatesReceivedLabel->setText(QString("States received: %1").arg(statesReceived));

	m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ").arg(requestCount).arg(repliesCount));

	HostAddressPort server = m_tcpClient->currentServerAddressPort();
	m_statusBarServerLabel->setText(QString(" ArchiveServer: %1 ").arg(server.addressPortStr()));

	if (m_tcpClient->isConnected() == true)
	{
		m_statusBarConnectionStateLabel->setText(" Connected ");
	}
	else
	{
		m_statusBarConnectionStateLabel->setText(" NoConnection ");
	}

	return;
}

void MonitorArchiveWidget::tcpRequestFinished()
{
	m_exportButton->setEnabled(true);
	m_printButton->setEnabled(true);
	m_signalsButton->setEnabled(true);

	m_updateButton->setText(tr("Update"));
	m_updateButton->setShortcut(QKeySequence(QKeySequence::StandardKey::Refresh));

	return;
}
