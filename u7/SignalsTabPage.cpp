#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QLabel>
#include <QCompleter>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QSplitter>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>

#include <UiLib/StandardColors.h>

#include "../lib/ConstStrings.h"
#include "../UtilsLib/Ui/WidgetUtils.h"
#include "../UtilsLib/WUtils.h"
#include "./Forms/ComparePropertyObjectDialog.h"

#include "SignalsTabPage.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"
#include "BusStorage.h"
#include "AppSignalSetProvider.h"
#include "UndoSignalsDialog.h"
#include "SignalHistoryDialog.h"
#include "FindSignalDialog.h"
#include "CheckinSignalsDialog.h"
#include "DlgMetrologyConnection.h"
#include "CreateSignalsDialog.h"

const int DEFAULT_COLUMN_WIDTH = 50;

//
//
// SignalsTabPage
//
//

SignalsTabPage* SignalsTabPage::m_instance = nullptr;


SignalsTabPage::SignalsTabPage(AppSignalSetProvider* signalSetProvider,
							   AppSignalPropertyManager* propManager,
							   DbController* dbController,
							   QWidget* parent) :
	MainTabPage(dbController, parent),
	m_signalSetProvider(signalSetProvider),
	m_db(dbController)
{
	TEST_PTR_RETURN(signalSetProvider);
	TEST_PTR_RETURN(propManager);

	Q_ASSERT(m_instance == nullptr);

	m_instance = this;

	m_signalTypeFilterCombo = new QComboBox(this);
	m_signalTypeFilterCombo->addItem(tr("All signals"), SignalsTabPage::FILTER_ST_ANY);
	m_signalTypeFilterCombo->addItem(tr("Analog signals"), SignalsTabPage::FILTER_ST_ANALOG);
	m_signalTypeFilterCombo->addItem(tr("Discrete signals"), SignalsTabPage::FILTER_ST_DISCRETE);
	m_signalTypeFilterCombo->addItem(tr("Bus signals"), SignalsTabPage::FILTER_ST_BUS);

	m_signalIdFieldCombo = new QComboBox(this);
	m_signalIdFieldCombo->addItem(tr("Any"), FILTER_STR_ANY);
	m_signalIdFieldCombo->addItem(tr("AppSignalID"), FILTER_STR_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("CustomAppSignalID"), FILTER_STR_CUSTOM_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("EquipmentID"), FILTER_STR_EQUIPMENT_ID);
	m_signalIdFieldCombo->addItem(tr("Caption"), FILTER_STR_CAPTION);
	m_signalIdFieldCombo->addItem(tr("Tags"), FILTER_STR_TAGS);

	QToolBar* toolBar = new QToolBar(this);
	toolBar->setStyleSheet("QToolButton { padding-top: 3px; padding-bottom: 3px; padding-left: 3px; padding-right: 3px;}");
	toolBar->setIconSize(toolBar->iconSize() * 0.9);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::showDeviceApplicationSignals, this, &SignalsTabPage::changeSignalIdFilter);

	QToolBar* filterToolBar = new QToolBar(this);
	filterToolBar->setStyleSheet("QToolButton { padding-top: 0px; padding-bottom: 0px; padding-left: 3px; padding-right: 3px;}");

	m_filterEdit = new QLineEdit(this);
	filterToolBar->addWidget(new QLabel("Filter ", this));
	filterToolBar->addWidget(m_signalTypeFilterCombo);
	filterToolBar->addWidget(new QLabel(" by ", this));
	filterToolBar->addWidget(m_signalIdFieldCombo);
	filterToolBar->addWidget(new QLabel(" complies ", this));
	filterToolBar->addWidget(m_filterEdit);

	QSettings settings;
	m_filterHistory = settings.value("SignalsTabPage/filterHistory").toStringList();

	m_completer = new QCompleter(m_filterHistory, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_filterEdit->setCompleter(m_completer);
    connect(m_filterEdit, &QLineEdit::textEdited, [this](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_filterEdit, &QLineEdit::setText);

	QPushButton* applyButton = new QPushButton("Apply", this);
	connect(applyButton, &QPushButton::clicked, this, &SignalsTabPage::applySignalIdFilter);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SignalsTabPage::applySignalIdFilter);
	filterToolBar->addWidget(applyButton);

	QPushButton* resetButton = new QPushButton("Reset", this);
	connect(resetButton, &QPushButton::clicked, this, &SignalsTabPage::resetSignalIdFilter);
	filterToolBar->addWidget(resetButton);

	// Property View
	//
	m_signalsModel = new SignalsModel(signalSetProvider, propManager, this);

	//For testing purposes
	//
	// #ifdef QT_DEBUG
	// #if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
	// new QAbstractItemModelTester(m_signalsModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
	// #endif
	// #endif
	//

	m_signalsProxyModel = new SignalsProxyModel(m_signalsModel, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsProxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_signalsView->verticalHeader()->setFixedWidth(DEFAULT_COLUMN_WIDTH);
	m_signalsView->verticalHeader()->hide();
	SignalsTablePropEditor* editor = m_signalsModel->createDelegate(m_signalsProxyModel);
	m_signalsView->setItemDelegate(editor);

	QHeaderView* horizontalHeader = m_signalsView->horizontalHeader();
	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	horizontalHeader->setHighlightSections(false);

	horizontalHeader->setSortIndicator(-1, Qt::AscendingOrder);
	m_signalsView->setSortingEnabled(true);

	horizontalHeader->setDefaultSectionSize(150);

	int wideColumnWidth = 400;

	m_signalsView->setColumnWidth(propManager->propertyIndex(AppSignalPropNames::APP_SIGNAL_ID), wideColumnWidth);
	m_signalsView->setColumnWidth(propManager->propertyIndex(AppSignalPropNames::CUSTOM_APP_SIGNAL_ID), wideColumnWidth);
	m_signalsView->setColumnWidth(propManager->propertyIndex(AppSignalPropNames::BUS_TYPE_ID), wideColumnWidth);
	m_signalsView->setColumnWidth(propManager->propertyIndex(AppSignalPropNames::CAPTION), wideColumnWidth);
	m_signalsView->setColumnWidth(propManager->propertyIndex(AppSignalPropNames::EQUIPMENT_ID), wideColumnWidth);

	QVector<int> defaultColumnVisibility;

	const std::vector<QString> defaultSignalPropertyVisibility =
	{
		AppSignalPropNames::APP_SIGNAL_ID,
		AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
		AppSignalPropNames::CAPTION,
		AppSignalPropNames::TYPE,
		AppSignalPropNames::IN_OUT_TYPE,
		AppSignalPropNames::EQUIPMENT_ID,
		AppSignalPropNames::LOW_ENGINEERING_UNITS,
		AppSignalPropNames::HIGH_ENGINEERING_UNITS,
	};

	for (const QString& columnName : defaultSignalPropertyVisibility)
	{
		defaultColumnVisibility.push_back(propManager->propertyIndex(columnName));
	}

	m_signalsColumnVisibilityController = new TableDataVisibilityController(m_signalsView, "SignalsTabPage", defaultColumnVisibility);
	connect(propManager, &AppSignalPropertyManager::propertyCountIncreased, m_signalsColumnVisibilityController, &TableDataVisibilityController::checkNewColumns);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));
	m_signalsView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(editor, &SignalsTablePropEditor::itemDoubleClicked, this, &SignalsTabPage::editSignal);
	connect(m_signalTypeFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalsTabPage::changeSignalTypeFilter);

	connect(m_signalsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &SignalsTabPage::changeSignalsLoadingSequence);
	connect(m_signalsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SignalsTabPage::onSignalSelectionChanged);

	connect(signalSetProvider, &AppSignalSetProvider::error, this, &SignalsTabPage::showError);

	// Create Actions
	//
	createActions(toolBar);

	//
	// Layouts
	//

	QVBoxLayout* pMainLayout = new QVBoxLayout();

	QMargins margins = pMainLayout->contentsMargins();
	margins.setTop(0);
	pMainLayout->setContentsMargins(margins);

	pMainLayout->setMenuBar(toolBar);			// Set ToolBar as menu -> no gaps and margins
	pMainLayout->addWidget(filterToolBar);
	pMainLayout->addWidget(m_signalsView);

	setLayout(pMainLayout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SignalsTabPage::projectClosed);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SignalsTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	if (m_findSignalDialog != nullptr)
	{
		m_findSignalDialog->close();
		delete m_findSignalDialog;
	}

	deleteMetrologyDialog();
}

bool SignalsTabPage::updateSignalsSpecProps(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate,
											const QStringList& forceUpdateProperties)
{
	Q_UNUSED(forceUpdateProperties)

	QString errMsg;

	bool result = AppSignalSetProvider::getInstance()->updateSignalsSpecProps(deviceSignalsToUpdate, &errMsg);

	if (errMsg.isEmpty() == false)
	{
		QMessageBox::critical(m_instance, QApplication::applicationName(), errMsg);
	}

	return result;
}

int SignalsTabPage::getMiddleVisibleRow()
{
	QRect rect = m_signalsView->viewport()->rect();
	return m_signalsView->indexAt(rect.center()).row();
}

void SignalsTabPage::createActions(QToolBar *toolBar)
{
	QAction* action = nullptr;

	action = new QAction(QIcon(":/Images/Images/SchemaOpen.svg"), tr("Edit properties"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::editSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("New signal"), this);
	action->setShortcut(QKeySequence::StandardKey::New);
	connect(action, &QAction::triggered, this, &SignalsTabPage::createNewSignals);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaClone.svg"), tr("Clone signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::cloneSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete signal"), this);
	action->setShortcut(Qt::Key_Delete);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("Check in signal(s)"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::checkIn);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/SchemaHistory.svg"), tr("History"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::viewSignalHistory);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/SchemaRefresh.svg"), tr("Refresh"), this);
	action->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(action, &QAction::triggered, this, &SignalsTabPage::loadSignals);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/Find.svg"), tr("Find"), this);
	action->setShortcuts(QList<QKeySequence>() << QKeySequence::Find << QKeySequence::Replace);
	connect(action, &QAction::triggered, this, &SignalsTabPage::findAndReplaceSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	m_signalsView->addAction(toolBar->addSeparator());

	action = new QAction(QIcon(":/Images/Images/MetrologyConnection.svg"), tr("Metrology connections ..."), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::openMetrologyConnections);
	toolBar->addAction(action);

	m_addMetrologyConnectionAction = new QAction(QIcon(":/Images/Images/MetrologyConnection.svg"), tr("New metrology connection ..."), this);
	connect(m_addMetrologyConnectionAction, &QAction::triggered, this, &SignalsTabPage::addMetrologyConnection);
	m_signalsView->addAction(m_addMetrologyConnectionAction);

}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::keyPressEvent(QKeyEvent* e)
{
	if(e->type() == QKeyEvent::KeyPress && e->matches(QKeySequence::Copy))
	{
		QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

		if (selection.count() == 0)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		}

		QString selectedSignalIds;

		for (const QModelIndex& selIndex : selection)
		{
			int row = getMappedSourceRow(selIndex);

			selectedSignalIds.append(m_signalSetProvider->getLoadedSignalByIndex(row, false)->appSignalID() + "\n");
		}

		QApplication::clipboard()->setText(selectedSignalIds);
	}
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_signalSetProvider->onProjectOpened();

	changeSignalsLoadingSequence();
}

void SignalsTabPage::projectClosed()
{
	m_signalsColumnVisibilityController->saveAllHeaderGeomery();

	m_signalsModel->prepareForReset();
	m_signalSetProvider->onProjectClosed();
	m_signalsModel->finishReset();

	this->setEnabled(false);

	resetSignalIdFilter();

	if (m_findSignalDialog != nullptr)
	{
		m_findSignalDialog->close();
		delete m_findSignalDialog;
		m_findSignalDialog = nullptr;
	}

	deleteMetrologyDialog();
}

void SignalsTabPage::onTabPageChanged()
{
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(sender());
	if (tabWidget == nullptr)
	{
		assert(false);
		return;
	}

	if (isEnabled() == false)
	{
		return;
	}

	if (m_findSignalDialog != nullptr)
	{
		if (tabWidget->currentWidget() == this && m_findSignalDialog->shouldReopen() == true)
		{
			findAndReplaceSignal();
		}
		else
		{
			m_findSignalDialog->hide();
		}
	}
}

void SignalsTabPage::loadSignals()
{
	saveSelection();
	m_signalSetProvider->reloadAllSignals();
	restoreSelection();
}

void SignalsTabPage::createNewSignals()
{
	CreateSignalsDialog createSignalsDialog(this);

	if (createSignalsDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	QString equipmentID = createSignalsDialog.getEquipmentID();
	E::SignalType type = createSignalsDialog.getSignalType();
	int channelCount = createSignalsDialog.getChannelCount();
	int signalCount = createSignalsDialog.getSignalCount();

	channelCount = std::clamp(channelCount, MIN_CHANNEL_COUNT, MAX_CHANNEL_COUNT);
	signalCount = std::clamp(signalCount, 1, 1000);

	//

	AppSignal templateSignal;

	templateSignal.setSignalType(type);
	templateSignal.setEquipmentID(equipmentID);

	SignalPropertiesDialog::initNewSignal(templateSignal);

	int signalCounter = db()->nextCounterValue();

	if (signalCounter >= 0)
	{
		QString newId = QString("%1%2").
							arg(E::valueToString<E::SignalType>(type).toUpper()).
							arg(signalCounter, 3, 10, Latin1Char::ZERO);

		templateSignal.setAppSignalID("#" + newId);
		templateSignal.setCustomAppSignalID(newId);
		templateSignal.setCaption("Signal " + newId);
	}

	std::vector<AppSignal*> signalToEdit = { &templateSignal };

	SignalPropertiesDialog dlg(signalToEdit, false, false, this);

	templateSignal.trimTextFields();

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	std::vector<int> addedSignalIDs;

	m_signalSetProvider->createNewSignals(templateSignal, channelCount, signalCount, &addedSignalIDs);

	restoreSelections(addedSignalIDs);
}

void SignalsTabPage::editSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		return;
	}

	int currentRow = getMappedSourceRow(m_signalsView->currentIndex());
	int currentColumn = m_signalsView->currentIndex().column();
	int currentId = m_signalSetProvider->signalID(currentRow);

	std::vector<int> selectedSignalId;

	for (const QModelIndex& selIndex : selection)
	{
		int row = getMappedSourceRow(selIndex);

		selectedSignalId.push_back(m_signalSetProvider->signalID(row));
	}

	editSignals(selectedSignalId);

	m_signalsView->scrollTo(m_signalsProxyModel->mapFromSource(m_signalsModel->index(m_signalSetProvider->signalIndex(currentId), currentColumn)));
}

bool SignalsTabPage::editSignals(const std::vector<int>& ids)
{
	m_signalSetProvider->reloadSignals(ids, true);

	bool readOnly = false;
	std::vector<AppSignal*> signalVector;

	for (int id : ids)
	{
		AppSignal* s = m_signalSetProvider->getLoadedSignalByID(id, false);

		TEST_PTR_CONTINUE(s)

		AppSignal* signal = new AppSignal(*s);

		if (!m_signalSetProvider->isEditableSignal(s))
		{
			readOnly = true;
		}

		signalVector.push_back(signal);
	}

	SignalPropertiesDialog dlg(signalVector, readOnly, true, this);

	if (dlg.isValid() == false)
	{
		return false;
	}

	bool hasEditedSignals = false;

	if (dlg.exec() == QDialog::Accepted)
	{
		std::vector<AppSignal*> signalsToSave;

		for (AppSignal* s : signalVector)
		{
			if (dlg.isEditedSignal(s->ID()) == true)
			{
				signalsToSave.push_back(s);
			}
		}

		if (signalsToSave.empty() == false)
		{
			hasEditedSignals = true;

			m_signalSetProvider->saveSignals(signalsToSave, this);
		}
	}

	for (AppSignal* s : signalVector)
	{
		delete s;
	}

	return hasEditedSignals;
}

void SignalsTabPage::cloneSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}

	std::vector<int> signalsToCloneIDs;

	for (const QModelIndex selIndex : selection)
	{
		int row = getMappedSourceRow(selIndex);
		int id = m_signalSetProvider->signalID(row);

		signalsToCloneIDs.push_back(id);
	}

	m_selectedRowsSignalID = m_signalSetProvider->cloneSignals(signalsToCloneIDs);

	if (m_selectedRowsSignalID.empty() == false)
	{
		m_focusedCellSignalID = m_selectedRowsSignalID[0];
	}

	m_signalsView->clearSelection();

	// restoreSelections(signalsToCloneIDs);

	restoreSelections(m_selectedRowsSignalID);
}

void SignalsTabPage::deleteSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}

	std::vector<int> signalsToDeleteIDs;

	for (const QModelIndex& selIndex : selection)
	{
		int row = getMappedSourceRow(selIndex);

		signalsToDeleteIDs.push_back(m_signalSetProvider->signalID(row));
	}

	m_signalSetProvider->deleteSignals(signalsToDeleteIDs);
}

void SignalsTabPage::findAndReplaceSignal()
{
	if (m_findSignalDialog == nullptr)
	{
		m_findSignalDialog = new FindSignalDialog(m_signalsView);
		m_findSignalDialog->setModal(false);

		connect(m_findSignalDialog, &FindSignalDialog::signalSelected, this, &SignalsTabPage::restoreSelection);
	}

	m_findSignalDialog->allowReopen();
	m_findSignalDialog->show();
	m_findSignalDialog->activateWindow();
	m_findSignalDialog->raise();
	m_findSignalDialog->setFocus();
}

void SignalsTabPage::undoSignalChanges()
{
	m_signalSetProvider->enforceAllSignalsLoading();

	QModelIndexList selSrcIndexes;

	getSelectionSourceIndexes(m_signalsView->selectionModel()->selection(), &selSrcIndexes);

	UndoSignalsDialog dlg(selSrcIndexes, m_signalsModel, *m_signalsColumnVisibilityController, this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalSetProvider->reloadSignals(dlg.undoedSignalsIDs(), true);
}

void SignalsTabPage::checkIn()
{
	m_signalSetProvider->enforceAllSignalsLoading();

	QModelIndexList selSrcIndexes;

	getSelectionSourceIndexes(m_signalsView->selectionModel()->selection(), &selSrcIndexes);

	CheckinSignalsDialog dlg(selSrcIndexes, m_signalsModel, *m_signalsColumnVisibilityController, this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}
}

void SignalsTabPage::viewSignalHistory()
{
	int row = getMappedSourceRow(m_signalsView->currentIndex());

	const AppSignal* signal = m_signalSetProvider->getLoadedSignalByIndex(row, false);

	TEST_PTR_RETURN(signal);

	SignalHistoryDialog dlg(db(), *signal, this);

	dlg.exec();
}

DialogMetrologyConnection* SignalsTabPage::createMetrologyDialog()
{
	if (m_signalSetProvider == nullptr)
	{
		Q_ASSERT(m_signalSetProvider);
		return nullptr;
	}

	DialogMetrologyConnection* pMetrologyDialog = new DialogMetrologyConnection(m_signalSetProvider, m_db, this);

	if (pMetrologyDialog == nullptr)
	{
		return nullptr;
	}

	connect(pMetrologyDialog, &QDialog::accepted, this, &SignalsTabPage::metrologyDialogClosed, Qt::QueuedConnection);
	connect(pMetrologyDialog, &QDialog::rejected, this, &SignalsTabPage::metrologyDialogClosed, Qt::QueuedConnection);

	pMetrologyDialog->setModal(false);

	return pMetrologyDialog;
}

void SignalsTabPage::deleteMetrologyDialog()
{
	if (m_metrologyDialog == nullptr)
	{
		return;
	}

	delete m_metrologyDialog;
	m_metrologyDialog = nullptr;
}

void SignalsTabPage::openMetrologyConnections()
{
	if (m_metrologyDialog == nullptr)
	{
		m_metrologyDialog = createMetrologyDialog();
	}

	if (m_metrologyDialog == nullptr)
	{
		return;
	}

	m_metrologyDialog->show();
	m_metrologyDialog->loadConnectionBase();
}

void SignalsTabPage::addMetrologyConnection()
{
	if (m_signalSetProvider == nullptr)
	{
		Q_ASSERT(m_signalSetProvider);
		return;
	}

	if (m_signalsView == nullptr)
	{
		Q_ASSERT(m_signalsView);
		return;
	}

	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);

	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Metrology connections"), tr("No one signal was selected!"));
		return;
	}

	int row = getMappedSourceRow(selection[0]);

	if (row < 0 || row >= m_signalSetProvider->signalCount())
	{
		return;
	}

	AppSignal* s = m_signalSetProvider->getSignalByIndex(row);

	TEST_PTR_RETURN(s);

	AppSignal signal(*s);

	if (signal.isAnalog() == false)
	{
		QMessageBox::warning(this, tr("Metrology connections"), tr("Please, select analog signal!"));
		return;
	}

	if (m_metrologyDialog == nullptr)
	{
		m_metrologyDialog = createMetrologyDialog();
	}

	if (m_metrologyDialog == nullptr)
	{
		return;
	}

	m_metrologyDialog->show();
	m_metrologyDialog->createConnectionBySignal(&signal);
}

void SignalsTabPage::metrologyDialogClosed()
{
	if (m_metrologyDialog == nullptr)
	{
		return;
	}

	deleteMetrologyDialog();
}

void SignalsTabPage::changeSignalsLoadingSequence()
{
	m_signalSetProvider->setMiddleVisibleSignalIndex(getMiddleVisibleRow());
}

void SignalsTabPage::setSelection(const std::vector<int>& selectedRowsSignalID, int focusedCellSignalID)
{
	if (selectedRowsSignalID.empty())
	{
		return;
	}

	if (focusedCellSignalID == AppSignalSet::BAD_ID)
	{
		focusedCellSignalID = selectedRowsSignalID.back();
	}

	m_selectedRowsSignalID = selectedRowsSignalID;

	int focusedRow = m_signalSetProvider->signalIndex(focusedCellSignalID);

	m_lastVerticalScrollPosition = m_signalsView->rowViewportPosition(focusedRow);
	m_lastHorizontalScrollPosition = 0;

	restoreSelection(focusedCellSignalID);
}

void SignalsTabPage::saveSelection()
{
	// Save signal id list of selected rows and signal id with column number of focused cell
	//
	m_selectedRowsSignalID.clear();

	QModelIndexList selectedList = m_signalsView->selectionModel()->selectedRows(0);

	m_selectedRowsSignalID.reserve(selectedList.size());

	foreach (const QModelIndex& index, selectedList)
	{
		int row = getMappedSourceRow(index);

		int id = m_signalSetProvider->signalID(row);

		if (id == AppSignalSet::BAD_ID)
		{
			Q_ASSERT(false);
			continue;
		}

		m_selectedRowsSignalID.push_back(id);
	}

	QModelIndex index = m_signalsView->currentIndex();

	if (index.isValid())
	{
		int row = getMappedSourceRow(index);
		m_focusedCellSignalID = m_signalSetProvider->signalID(row);
		m_focusedCellColumn = index.column();
	}

	m_lastHorizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	m_lastVerticalScrollPosition = m_signalsView->verticalScrollBar()->value();
}

void SignalsTabPage::restoreSelection(int selectedSignalID)
{
	restoreSelections(std::vector<int>{selectedSignalID});
}

void SignalsTabPage::restoreSelections(const std::vector<int>& selectedSignalIDs)
{
	if (selectedSignalIDs.empty())
	{
		return;
	}

	m_focusedCellColumn = 0;
	m_focusedCellSignalID = selectedSignalIDs.back();

	QModelIndex lastProxyIndex;

	for(int selectedSignalID : selectedSignalIDs)
	{
		if (selectedSignalID == AppSignalSet::BAD_ID)
		{
			continue;
		}

		int signalIndex = m_signalSetProvider->signalIndex(selectedSignalID);

		if (signalIndex == AppSignalSet::BAD_INDEX)
		{
			continue;
		}

		QModelIndex currentSourceIndex = m_signalsModel->index(signalIndex, m_focusedCellColumn);
		QModelIndex currentProxyIndex = m_signalsProxyModel->mapFromSource(currentSourceIndex);

		m_signalsView->selectionModel()->setCurrentIndex(currentProxyIndex, QItemSelectionModel::Select);
		m_signalsView->selectionModel()->select(currentProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

		lastProxyIndex = currentProxyIndex;
	}

	m_signalsView->horizontalScrollBar()->setValue(m_lastHorizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(m_lastVerticalScrollPosition);

	if (lastProxyIndex.isValid())
	{
		m_signalsView->scrollTo(lastProxyIndex);
	}
}

// Checks only first selected signal, because Metrology editor reads only first signal
//
void SignalsTabPage::onSignalSelectionChanged()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
	if (selection.count() == 0)
	{
		m_addMetrologyConnectionAction->setEnabled(false);
		return;
	}
	int row = getMappedSourceRow(selection[0]);

	if (m_signalSetProvider->getLoadedSignalByIndex(row, true)->isAnalog() == true)
	{
		m_addMetrologyConnectionAction->setEnabled(true);
	}
	else
	{
		m_addMetrologyConnectionAction->setEnabled(false);
	}
}

void SignalsTabPage::changeSignalTypeFilter(int selectedType)
{
	saveSelection();
	int signalType = m_signalTypeFilterCombo->itemData(selectedType, Qt::UserRole).toInt();
	m_signalsProxyModel->setSignalTypeFilter(signalType);
	restoreSelection();

	for (int i = 0; i < m_signalsModel->columnCount(); i++)
	{
		if (signalType == SignalsTabPage::FILTER_ST_ANY ||
			m_signalSetProvider->signalPropertyManager().isHiddenFor(static_cast<E::SignalType>(signalType), i, theSettings.isExpertMode()) == false)
		{
			bool hidden = m_signalsColumnVisibilityController->getColumnVisibility(i) == false;
			m_signalsView->setColumnHidden(i, hidden);
		}
		else
		{
			m_signalsView->setColumnHidden(i, true);
		}
	}
}

void SignalsTabPage::changeSignalIdFilter(QStringList strIds, bool refreshSignalList)
{
	// Update signals
	//
	if (refreshSignalList == true)
	{
		m_signalSetProvider->reloadAllSignals();
	}

	// Reset signal type filter
	//
	QObject* changeCustomer = sender();
	if (changeCustomer != nullptr && typeid(*changeCustomer) == typeid(GlobalMessanger))
	{
		for (int i = 0; i < m_signalTypeFilterCombo->count(); i++)
		{
			if (m_signalTypeFilterCombo->itemData(i) == SignalsTabPage::FILTER_ST_ANY)
			{
				m_signalTypeFilterCombo->setCurrentIndex(i);
			}
		}

		m_signalsProxyModel->setSignalTypeFilter(SignalsTabPage::FILTER_ST_ANY);
		m_signalIdFieldCombo->setCurrentIndex(SignalsTabPage::FILTER_STR_EQUIPMENT_ID);
		m_signalsProxyModel->setIdFilterField(SignalsTabPage::FILTER_STR_EQUIPMENT_ID);
	}

	// Set signal id filter
	//
	m_signalsProxyModel->setSignalIdFilter(strIds);

	// Set signal id filter editor text and save filter history
	//
	QString newFilter = strIds.join(" | ");
	while (newFilter.indexOf("  ") != -1)
	{
		newFilter.replace("  ", " ");
	}

	if (!newFilter.isEmpty() && !m_filterHistory.contains(newFilter))
	{
		m_filterHistory.append(newFilter);

		QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
		assert(model != nullptr);
		if (model != nullptr)
		{
			model->setStringList(m_filterHistory);
		}

		QSettings settings;
		settings.setValue("SignalsTabPage/filterHistory", m_filterHistory);
	}

	m_filterEdit->setText(newFilter);

	GlobalMessanger::instance().fireChangeCurrentTab(this);
}

void SignalsTabPage::applySignalIdFilter()
{
	m_signalsProxyModel->setIdFilterField(m_signalIdFieldCombo->currentIndex());
	changeSignalIdFilter(m_filterEdit->text().trimmed().split("|", Qt::SkipEmptyParts), false);
}

void SignalsTabPage::resetSignalIdFilter()
{
	m_signalsProxyModel->setSignalIdFilter(QStringList());
	m_signalsProxyModel->setSignalTypeFilter(SignalsTabPage::FILTER_ST_ANY);
	m_filterEdit->setText("");
	m_signalTypeFilterCombo->setCurrentIndex(0);
}

void SignalsTabPage::showError(QString message)
{
	if (!message.isEmpty())
	{
		QMessageBox::warning(this, "Error", message);
	}
}

void SignalsTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	if (isVisible() == false)
	{
		return;
	}

	// Can compare only files which are EquipmentObjects
	//
	if (object.isSignal() == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<AppSignalProperties> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<AppSignal> outSignals;

			bool ok = db()->getSpecificSignals(signalIds, compareData.sourceChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				source = std::make_shared<AppSignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			AppSignal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				source = std::make_shared<AppSignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<AppSignalProperties> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<AppSignal> outSignals;

			bool ok = db()->getSpecificSignals(signalIds, compareData.targetChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				target = std::make_shared<AppSignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			AppSignal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				target = std::make_shared<AppSignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}

int SignalsTabPage::getMappedSourceRow(const QModelIndex& proxyIndex) const
{
	return m_signalsProxyModel->mapToSource(proxyIndex).row();
}

void SignalsTabPage::getSelectionSourceIndexes(const QItemSelection& proxySelection,
											   QModelIndexList* selectionSrcIndexes)
{
	TEST_PTR_RETURN(selectionSrcIndexes);

	selectionSrcIndexes->clear();

	int rowCount = 0;

	for(const QItemSelectionRange& proxySelRange : proxySelection)
	{
		rowCount += proxySelRange.height();
	}

	selectionSrcIndexes->reserve(rowCount);

	for(const QItemSelectionRange& proxySelRange : proxySelection)
	{
		int startRow = proxySelRange.top();
		int endRow = proxySelRange.bottom();

		for(int proxyRow = startRow; proxyRow <= endRow; proxyRow++)
		{
			QModelIndex srcIndex = m_signalsProxyModel->mapToSource(
										m_signalsProxyModel->index(proxyRow, 0));
			selectionSrcIndexes->append(srcIndex);
		}
	}
}

