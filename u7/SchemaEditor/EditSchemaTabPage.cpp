#include "EditSchemaTabPage.h"
#include "CheckInDialog.h"
#include "EditSchemaWidget.h"

#include <VFrame30/SchemaItemLink.h>
#include <VFrame30/SchemaItemSignal.h>

//
//
// EditSchemaTabPage
//
//
EditSchemaTabPage::EditSchemaTabPage(QTabWidget* tabWidget,
									 std::shared_ptr<VFrame30::Schema> schema,
									 const DbFileInfo& fileInfo,
									 DbController* dbcontroller,
									 AppSignalSetProvider* signalSetProvider) :
	QMainWindow(
		nullptr,
		Qt::WindowType::Widget), // Always created as widget as from start it's attached to TabWidget, later can be switcher to Qt::Window
	HasDbController(dbcontroller),
	m_schemaWidget(nullptr),
	m_tabWidget(tabWidget)
{
	Q_ASSERT(m_tabWidget);
	Q_ASSERT(schema.get() != nullptr);

	setWindowTitle(schema->schemaId());

	// Create controls
	//
	schema->setChangeset(fileInfo.changeset());

	m_schemaWidget = new EditSchemaWidget{schema, fileInfo, dbcontroller, signalSetProvider, this};

	connect(m_schemaWidget, &EditSchemaWidget::detachOrAttachWindow, this, &EditSchemaTabPage::detachOrAttachWindow);
	connect(m_schemaWidget, &EditSchemaWidget::closeTab, this, &EditSchemaTabPage::closeTab);
	connect(m_schemaWidget, &EditSchemaWidget::modifiedChanged, this, &EditSchemaTabPage::modifiedChanged);
	connect(m_schemaWidget, &EditSchemaWidget::saveWorkcopy, this, &EditSchemaTabPage::saveWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::checkInFile, this, &EditSchemaTabPage::checkInFile);
	connect(m_schemaWidget, &EditSchemaWidget::checkOutFile, this, &EditSchemaTabPage::checkOutFile);
	connect(m_schemaWidget, &EditSchemaWidget::undoChangesFile, this, &EditSchemaTabPage::undoChangesFile);
	connect(m_schemaWidget, &EditSchemaWidget::getCurrentWorkcopy, this, &EditSchemaTabPage::getCurrentWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::setCurrentWorkcopy, this, &EditSchemaTabPage::setCurrentWorkcopy);

	// Actions
	//
	m_fileAction = new QAction(tr("File"), this);
	m_fileAction->setEnabled(true);

	m_alignAction = new QAction(tr("Align"), this);
	m_alignAction->setToolTip(tr("Align items' size or position by the first selected item"));
	m_alignAction->setEnabled(true);

	m_orderAction = new QAction(tr("Order"), this);
	m_orderAction->setToolTip(tr("Change selected items' order"));
	m_orderAction->setEnabled(true);

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("Toolbar"), this);
	m_toolBar->setOrientation(Qt::Vertical);
	m_toolBar->setFloatable(false);
	m_toolBar->setMovable(false);
	m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

	// File
	//
	m_toolBar->addAction(m_fileAction);
	m_toolBar->addSeparator();

	if (schema->isLogicSchema() == true)
	{
		m_schemaWidget->fillActionsForLogicSchema(m_toolBar);
	}

	if (schema->isUfbSchema() == true)
	{
		m_schemaWidget->fillActionsForUfbSchema(m_toolBar);
	}

	if (schema->isMonitorSchema() == true)
	{
		m_schemaWidget->fillActionsForMonitorSchema(m_toolBar);
	}

	if (schema->isTuningSchema() == true)
	{
		m_schemaWidget->fillActionsForTuningSchema(m_toolBar);
	}

	if (schema->isDiagSchema() == true)
	{
		m_schemaWidget->fillActionsForDiagSchema(m_toolBar);
	}

	if (schema->isVduSchema() == true)
	{
		m_schemaWidget->fillActionsForVduSchema(m_toolBar);
	}

	if (schema->isActuatorSchema() == true)
	{
		m_schemaWidget->fillActionsForActuatorSchema(m_toolBar);
	}

	// Other menu items
	//
	m_toolBar->addSeparator();
	m_toolBar->addAction(m_orderAction);
	m_toolBar->addAction(m_alignAction);

	m_toolBar->addAction(m_schemaWidget->m_infoModeAction);

	// --
	//
	setCentralWidget(m_schemaWidget);
	addToolBar(Qt::ToolBarArea::LeftToolBarArea, m_toolBar);

	// --
	//
	connect(m_fileAction, &QAction::triggered, this, &EditSchemaTabPage::fileMenuTriggered);
	connect(m_orderAction, &QAction::triggered, this, &EditSchemaTabPage::itemsOrderTriggered);
	connect(m_alignAction, &QAction::triggered, this, &EditSchemaTabPage::sizeAndPosMenuTriggered);

	connect(m_tabWidget, &QTabWidget::currentChanged, m_schemaWidget, &EditSchemaWidget::hideWorkDialogs);

	connect(dbc(), &DbController::projectClosed, this, &EditSchemaTabPage::projectClosed);

	setPageTitle();

	return;
}

EditSchemaTabPage::~EditSchemaTabPage() {}

void EditSchemaTabPage::closeEvent(QCloseEvent* event)
{
	if (windowFlags() == Qt::WindowType::Widget)
	{
		// If windowFlags() == Qt::WindowType::Widget then it is attachet to TabWidget, and close is
		// processed in slot closeTab
		//
		event->accept();
		return;
	}

	// Else (windowFlags() == Qt::WindowType::Window)
	// This is free floating window, ask for saving result
	//
	if (m_schemaWidget->modified() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("The document has been modified."));
		mb.setInformativeText(tr("Do you want to save chages to %1?").arg(fileInfo().fileName()));
		mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Save);

		int result = mb.exec();

		switch (result)
		{
		case QMessageBox::Save:
			saveWorkcopy();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			event->ignore();
			return;
		default:
			Q_ASSERT(false);
		}
	}

	// Find current tab and close it
	//
	emit aboutToClose(this);
	this->deleteLater();

	event->accept();
	return;
}

void EditSchemaTabPage::ensureVisible()
{
	setVisible(true); // Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect = this->screen()->availableGeometry();
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMinimized() == true)
	{
		showNormal();
	}

	if (isMaximized() == false && (intersectRect.width() < size().width() || intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false && (frameGeometry().width() > screenRect.width() || frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7), static_cast<int>(screenRect.height() * 0.7));
	}

	return;
}

void EditSchemaTabPage::setPageTitle()
{
	QString newTitle;

	if (readOnly() == true || fileInfo().userId() != db()->currentUser().userId())
	{
		if (fileInfo().changeset() == -1 || fileInfo().changeset() == 0)
		{
			newTitle = QString("%1: ReadOnly").arg(m_schemaWidget->schema()->schemaId());
		}
		else
		{
			newTitle = QString("%1: %2 ReadOnly").arg(m_schemaWidget->schema()->schemaId()).arg(fileInfo().changeset());
		}

		if (fileInfo().deleted() == true)
		{
			newTitle += QString(", deleted");
		}
	}
	else
	{
		newTitle = m_schemaWidget->schema()->schemaId();
		if (modified() == true)
		{
			newTitle += "*";
		}
	}

	setWindowTitle(newTitle);

	if (parentWidget() != nullptr)
	{
		if (QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget()); tabWidget != nullptr)
		{
			for (int i = 0; i < tabWidget->count(); i++)
			{
				if (tabWidget->widget(i) == this)
				{
					tabWidget->setTabText(i, newTitle);
					return;
				}
			}
		}
	}

	return;
}

void EditSchemaTabPage::updateZoomAndScrolls(bool fitToScreen, bool repaint)
{
	m_schemaWidget->setZoom(fitToScreen ? 0 : m_schemaWidget->zoom(), repaint);
	return;
}

double EditSchemaTabPage::zoom() const
{
	return m_schemaWidget->zoom();
}

void EditSchemaTabPage::setZoom(double zoom, bool repaint)
{
	m_schemaWidget->setZoom(zoom, repaint);
	return;
}

QString EditSchemaTabPage::activeLayer() const
{
	Q_ASSERT(m_schemaWidget);

	auto layer = m_schemaWidget->activeLayer();
	Q_ASSERT(layer);

	QString layerName = (layer != nullptr) ? layer->name() : QString{};
	return layerName;
}

void EditSchemaTabPage::setActiveLayer(QString name)
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->setActiveLayer(name);
	return;
}

void EditSchemaTabPage::layersDialog()
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->layers();
	return;
}

void EditSchemaTabPage::updateAfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateAfbsForSchema();

	return;
}

void EditSchemaTabPage::updateUfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateUfbsForSchema();

	return;
}

void EditSchemaTabPage::updateBussesSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		Q_ASSERT(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateBussesForSchema();

	return;
}

void EditSchemaTabPage::detachOrAttachWindow()
{
	emit pleaseDetachOrAttachWindow(this);
}

void EditSchemaTabPage::closeTab()
{
	if (m_schemaWidget->modified() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("The document has been modified."));
		mb.setInformativeText(tr("Do you want to save chages to %1?").arg(fileInfo().fileName()));
		mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Save);

		int result = mb.exec();

		switch (result)
		{
		case QMessageBox::Save:
			saveWorkcopy();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			return;
		default:
			Q_ASSERT(false);
		}
	}

	// Find current tab and close it
	//
	emit aboutToClose(this);

	this->deleteLater();
	return;
}

void EditSchemaTabPage::projectClosed()
{
	// Save opened schemas so they can be restored on next project open event
	//
	{
		//		static const QUuid currentSession =

		//		QString record = QString{"%1;%2;%3;%4;%5"}
		//						 .arg("1")							// record version
		//						 .arg()
		//						 .arg(schema()->schemaId())			// schema id
		//						 .arg(readOnly() ? "true" : false)	// is read only
		//						 .arg(fileInfo().changeset());		// changeset id, for readonly file
	}

	// Find current tab and close it
	//
	emit aboutToClose(this);

	this->deleteLater();

	return;
}

void EditSchemaTabPage::modifiedChanged(bool /*modified*/)
{
	setPageTitle();
}

void EditSchemaTabPage::checkInFile()
{
	if (readOnly() == true || fileInfo().state() != E::VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdministrator() == false))
	{
		return;
	}

	// Save work-copy and checkin
	//
	if (modified() == true)
	{
		bool saveResult = saveWorkcopy();

		if (saveResult == false)
		{
			return;
		}
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	std::vector<DbFileInfo> updatedFiles;
	bool checkInTree = QSettings{}.value("EditSchemaTabPage::checkInFile/checkInTree", false).toBool();

	bool checkInResult = CheckInDialog::checkIn(files, &updatedFiles, checkInTree, "SchemaID", db(), this, &checkInTree);
	if (checkInResult == false)
	{
		return;
	}

	QSettings{}.setValue("EditSchemaTabPage::checkInFile/checkInTree", checkInTree);

	emit vcsFileStateChanged();

	DbFileInfo fi;
	db()->getFileInfo(fileInfo().fileId(), &fi, this);

	setFileInfo(fi);

	setReadOnly(true);

	setPageTitle();

	return;
}

void EditSchemaTabPage::checkOutFile()
{
	if (readOnly() == false || fileInfo().state() != E::VcsState::CheckedIn)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	bool result = db()->checkOut(files, this);
	if (result == false)
	{
		return;
	}

	// Read the workcopy and load it to the current document
	//
	std::vector<std::shared_ptr<DbFile>> out;

	result = db()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	m_schemaWidget->schema()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->update();

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPage::undoChangesFile()
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//
	if (readOnly() == true || fileInfo().state() != E::VcsState::CheckedOut || fileInfo().userId() != db()->currentUser().userId())
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() == QMessageBox::Ok)
	{
		DbFileInfo fi = fileInfo();

		bool result = db()->undoChanges(fi, this);

		if (result == true)
		{
			setFileInfo(fi);

			setReadOnly(true);
			setPageTitle();

			m_schemaWidget->resetAction();
			m_schemaWidget->clearSelection();

			m_schemaWidget->update();
		}
	}

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPage::fileMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	m_schemaWidget->updateFileActions();
	QWidget* w = m_toolBar->widgetForAction(m_fileAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_fileSubMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPage::sizeAndPosMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_alignAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_alignSubMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPage::itemsOrderTriggered()
{
	if (m_toolBar == nullptr)
	{
		Q_ASSERT(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_orderAction);

	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_orderSubMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

bool EditSchemaTabPage::saveWorkcopy()
{
	if (readOnly() == true || modified() == false || fileInfo().state() != E::VcsState::CheckedOut ||
		fileInfo().userId() != db()->currentUser().userId())
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return false;
	}

	QByteArray data;
	schema()->saveToByteArray(&data);

	if (data.isEmpty() == true)
	{
		Q_ASSERT(data.isEmpty() == false);
		return false;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo());
	file->swapData(data);

	// Check if schemaId was changed, rename file if so
	//
	bool fileWasRenamed = false;

	if (schema()->schemaId() != m_schemaWidget->m_initialSchemaId)
	{
		QString newFileName = schema()->schemaId() + "." + file->extension();

		if (bool ok = db()->renameFile(*file, newFileName, file.get(), this); ok == false)
		{
			// Don't save file if it was not renamed, as it will lead that filename differs from SchemaID
			// Just return
			//
			return false;
		}

		fileWasRenamed = true;
	}

	file->setDetails(schema()->details(QString{})); // Details must be set here, as file rename will spoils them
													// Ignore path here

	// Save work-copy
	//
	if (bool result = db()->setWorkcopy(file, this); result == false)
	{
		return false;
	}

	resetModified();

	if (fileWasRenamed == true)
	{
		setPageTitle();
		emit vcsFileStateChanged();
	}

	emit fileWasSaved(file->details());

	return true;
}

void EditSchemaTabPage::getCurrentWorkcopy()
{
	// Select destination folder
	//
	QString dir = QFileDialog::getExistingDirectory(this,
													tr("Select Directory"),
													QString(),
													QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}

	if (dir[dir.length() - 1] != '/')
	{
		dir.append("/");
	}

	// Save files to disk
	//
	QString fileName = dir + fileInfo().fileName();

	bool writeResult = m_schemaWidget->schema()->saveToFile(fileName);

	if (writeResult == false)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Write file error."));
		msgBox.setInformativeText(tr("Cannot write file %1.").arg(fileInfo().fileName()));
		msgBox.exec();
	}

	return;
}

void EditSchemaTabPage::setCurrentWorkcopy()
{
	if (readOnly() == true || fileInfo().state() != E::VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdministrator() == false))
	{
		Q_ASSERT(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	// Select file
	//
	static QString path{"."};
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), path);
	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	// Load file
	//
	bool readResult = m_schemaWidget->schema()->Load(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// --
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->resetEditEngine();
	m_schemaWidget->setModified();

	m_schemaWidget->update();

	return;
}

VFrame30::Schema* EditSchemaTabPage::schema()
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->schema();
}

const VFrame30::Schema* EditSchemaTabPage::schema() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->schema();
}

const DbFileInfo& EditSchemaTabPage::fileInfo() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->fileInfo();
}

void EditSchemaTabPage::setFileInfo(const DbFileInfo& fi)
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->setFileInfo(fi);

	m_schemaWidget->schema()->setChangeset(fi.changeset());

	setPageTitle();
}

bool EditSchemaTabPage::readOnly() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->readOnly();
}

void EditSchemaTabPage::setReadOnly(bool value)
{
	Q_ASSERT(m_schemaWidget);
	m_schemaWidget->setReadOnly(value);

	setPageTitle();
}

bool EditSchemaTabPage::modified() const
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->modified();
}

void EditSchemaTabPage::resetModified()
{
	Q_ASSERT(m_schemaWidget);
	return m_schemaWidget->resetModified();
}

bool EditSchemaTabPage::compareWidget() const
{
	return m_schemaWidget->compareWidget();
}

bool EditSchemaTabPage::isCompareWidget() const
{
	return m_schemaWidget->compareWidget();
}

void EditSchemaTabPage::setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target)
{
	return m_schemaWidget->setCompareWidget(value, source, target);
}

void EditSchemaTabPage::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
{
	m_schemaWidget->setCompareItemActions(itemsActions);
}
