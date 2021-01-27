#include "DialogMetrologyConnection.h"

#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QKeyEvent>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// class MetrologyConnectionTable

MetrologyConnectionTable::MetrologyConnectionTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionTable::~MetrologyConnectionTable()
{
	QMutexLocker l(&m_connectionMutex);

	m_connectionList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int MetrologyConnectionTable::columnCount(const QModelIndex&) const
{
	return METROLOGY_CONNECTION_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int MetrologyConnectionTable::rowCount(const QModelIndex&) const
{
	return connectionCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MetrologyConnectionTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < METROLOGY_CONNECTION_COLUMN_COUNT)
		{
			result = qApp->translate("MetrologyConnectionDialog.h", MetrologyConnectionColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MetrologyConnectionTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= connectionCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > METROLOGY_CONNECTION_COLUMN_COUNT)
	{
		return QVariant();
	}

	const Metrology::SignalConnection& connection = at(row);

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == METROLOGY_CONNECTION_COLUMN_IN_ID)
		{
			if (connection.handle().inputID == Metrology::SIGNAL_ID_IS_NOT_FOUND)	// if input signal is not exist
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == METROLOGY_CONNECTION_COLUMN_TYPE)
		{
			if (connection.type() < 0 || connection.type() >= Metrology::CONNECTION_TYPE_COUNT)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == METROLOGY_CONNECTION_COLUMN_OUT_ID)
		{
			if (connection.handle().outputID == Metrology::SIGNAL_ID_IS_NOT_FOUND)	// if output signal is not exist
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, connection);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString MetrologyConnectionTable::text(int row, int column, const Metrology::SignalConnection& connection) const
{
	if (row < 0 || row >= connectionCount())
	{
		return QString();
	}

	if (column < 0 || column > METROLOGY_CONNECTION_COLUMN_COUNT)
	{
		return QString();
	}

	bool visible = true;

	if (	row > 0 &&
			m_connectionList[row - 1].handle().inputID != Metrology::SIGNAL_ID_IS_NOT_FOUND &&
			m_connectionList[row - 1].handle().type == connection.handle().type &&
			m_connectionList[row - 1].handle().inputID == connection.handle().inputID)
	{
		visible = false;
	}

	QString result;

	switch (column)
	{
		case METROLOGY_CONNECTION_COLUMN_IN_ID:		result = visible ? connection.appSignalID( Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT): QString();	break;
		case METROLOGY_CONNECTION_COLUMN_TYPE:		result = visible ? connection.typeStr() : QString("");												break;
		case METROLOGY_CONNECTION_COLUMN_OUT_ID:	result = connection.appSignalID( Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT);						break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int MetrologyConnectionTable::connectionCount() const
{
	QMutexLocker l(&m_connectionMutex);

	return m_connectionList.count();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalConnection MetrologyConnectionTable::at(int index) const
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return Metrology::SignalConnection();
	}

	return m_connectionList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionTable::set(const QVector<Metrology::SignalConnection>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

	m_connectionMutex.lock();

		m_connectionList = list_add;

	m_connectionMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionTable::clear()
{
	int count = connectionCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

	m_connectionMutex.lock();

		m_connectionList.clear();

	m_connectionMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// class DialogMetrologyConnectionItem

DialogMetrologyConnectionItem::DialogMetrologyConnectionItem(SignalSetProvider* signalSetProvider, QWidget *parent) :
	QDialog(parent),
	m_signalSetProvider(signalSetProvider)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnectionItem::~DialogMetrologyConnectionItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.25), static_cast<int>(screen.height() * 0.08));
	move(screen.center() - rect().center());

	// signal connection type
	//
	QHBoxLayout *typeLayout = new QHBoxLayout;

	m_pTypeList = new QComboBox(this);

	typeLayout->addWidget(new QLabel(tr("Connection type"), this));
	typeLayout->addWidget(m_pTypeList);
	typeLayout->addStretch();

	// Signals
	//
	QGroupBox* signalGroup = new QGroupBox(QString());
	QVBoxLayout *signalLayout = new QVBoxLayout;

	// Input signal
	//
	QHBoxLayout *inputSignalLayout = new QHBoxLayout;

	QLabel* pInputSignalLabel = new QLabel(tr("Source AppSignalID"), this);
	m_pInputSignalIDEdit = new QLineEdit(QString(), this);


	inputSignalLayout->addWidget(pInputSignalLabel);
	inputSignalLayout->addWidget(m_pInputSignalIDEdit);

	// Output signal
	//
	QHBoxLayout *outputSignalLayout = new QHBoxLayout;

	QLabel* pOutputSignalLabel = new QLabel(tr("Destination AppSignalID"), this);
	m_pOutputSignalIDEdit = new QLineEdit(QString(), this);


	outputSignalLayout->addWidget(pOutputSignalLabel);
	outputSignalLayout->addWidget(m_pOutputSignalIDEdit);


	signalLayout->addLayout(inputSignalLayout);
	signalLayout->addLayout(outputSignalLayout);

	signalGroup->setLayout(signalLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// fill type list
	//
	for (int type = 0; type < Metrology::CONNECTION_TYPE_COUNT; type++)
	{
		m_pTypeList->addItem(qApp->translate("MetrologyConnectionBase.h", Metrology::ConnectionType[type]), type);
	}
	m_pTypeList->removeItem(Metrology::CONNECTION_TYPE_UNUSED);
	m_pTypeList->setCurrentIndex(0);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogMetrologyConnectionItem::selectedType);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogMetrologyConnectionItem::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogMetrologyConnectionItem::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addLayout(typeLayout);
	mainLayout->addWidget(signalGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::updateSignals()
{
	if (m_pTypeList == nullptr)
	{
		return;
	}

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	int type = m_connection.type() ;
	if ((type < 0 || type >= Metrology::CONNECTION_TYPE_COUNT) || type == Metrology::CONNECTION_TYPE_UNUSED)
	{
		type = Metrology::CONNECTION_TYPE_INPUT_INTERNAL;
		m_connection.setType(type);
	}

	m_pTypeList->setCurrentIndex(type - 1);
	m_pInputSignalIDEdit->setText(m_connection.appSignalID( Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT));
	m_pOutputSignalIDEdit->setText(m_connection.appSignalID( Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT));
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::selectedType(int)
{
	int type = m_pTypeList->currentData().toInt() ;
	if (type < 0 || type >= Metrology::CONNECTION_TYPE_COUNT)
	{
		return;
	}

	m_connection.setType(type);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::setConnection(bool newConnection, const Metrology::SignalConnection& connection)
{
	m_connection = connection;
	m_isNewConnection = newConnection;

	if (newConnection == true)
	{
		setWindowIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
		setWindowTitle(tr("New connection"));
	}
	else
	{
		setWindowIcon(QIcon(":/Images/Images/SchemaOpen.svg"));
		setWindowTitle(tr("Edit connection"));
	}

	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnectionItem::electricLimitIsValid(Signal* pSignal)
{
	if (pSignal == nullptr)
	{
		assert(pSignal);
		return false;
	}

	switch (pSignal->inOutType())
	{
		case E::SignalInOutType::Input:

			if (pSignal->isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || pSignal->isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
			{
				return false;
			}

			if (pSignal->isSpecPropExists(SignalProperties::electricUnitCaption) == false)
			{
				return true;
			}

			if (pSignal->electricLowLimit() == 0.0 && pSignal->electricHighLimit() == 0.0)
			{
				return false;
			}

			if (pSignal->electricUnit() == E::ElectricUnit::NoUnit)
			{
				return false;
			}

			break;

		case E::SignalInOutType::Output:

			if (pSignal->isSpecPropExists(SignalProperties::outputModeCaption) == false)
			{
				return false;
			}

			break;
	}

	return true;
}


// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::onOk()
{
	if (m_signalSetProvider == nullptr)
	{
		return;
	}

	//
	//

	int type = m_connection.type();
	if (type < 0 || type >= Metrology::CONNECTION_TYPE_COUNT)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select connection type!"));
		m_pTypeList->setFocus();
		return;
	}

	//
	//

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	QString inputAppSignalID = m_pInputSignalIDEdit->text().trimmed();
	QString outputAppSignalID = m_pOutputSignalIDEdit->text().trimmed();

	if (inputAppSignalID.isEmpty() == true || outputAppSignalID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input correct AppSignalID!"));
		return;
	}

	//
	//

	Signal* pInSignal = m_signalSetProvider->getSignalByStrID(inputAppSignalID);
	if (pInSignal == nullptr)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not found.\nPlease, input correct AppSignalID!").
								 arg(inputAppSignalID));
		m_pInputSignalIDEdit->setFocus();
		return;
	}

	if (pInSignal->isAnalog() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not analog.\nPlease, select analog signal!").
								 arg(inputAppSignalID));
		m_pInputSignalIDEdit->setFocus();
		return;
	}

	Signal* pOutSignal = m_signalSetProvider->getSignalByStrID(outputAppSignalID);
	if (pOutSignal == nullptr)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not found.\nPlease, input correct AppSignalID!").
								 arg(outputAppSignalID));
		m_pOutputSignalIDEdit->setFocus();
		return;
	}

	if (pOutSignal->isAnalog() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not analog.\nPlease, select analog signal!").
								 arg(outputAppSignalID));
		m_pOutputSignalIDEdit->setFocus();
		return;
	}

	//
	//

	switch(type)
	{
		case Metrology::CONNECTION_TYPE_INPUT_INTERNAL:
		case Metrology::CONNECTION_TYPE_INPUT_OUTPUT:
		case Metrology::CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
		case Metrology::CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
		case Metrology::CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:
		case Metrology::CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:

			if (pInSignal->isInput() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not input signal!").
										 arg(inputAppSignalID));
				m_pInputSignalIDEdit->setFocus();
				return;
			}

			if (electricLimitIsValid(pInSignal) == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 has wrong electric limit!").
										 arg(inputAppSignalID));
				m_pInputSignalIDEdit->setFocus();
				return;
			}

			break;

		case Metrology::CONNECTION_TYPE_TUNING_OUTPUT:

			if (pInSignal->isInternal() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not internal signal!").
										 arg(inputAppSignalID));
				m_pInputSignalIDEdit->setFocus();
				return;
			}

			if (pInSignal->enableTuning() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not tuning signal!").
										 arg(inputAppSignalID));
				m_pInputSignalIDEdit->setFocus();
				return;
			}

			break;
	}

	//
	//

	switch(type)
	{
		case Metrology::CONNECTION_TYPE_INPUT_INTERNAL:
		case Metrology::CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
		case Metrology::CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:

			if (pOutSignal->isInternal() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not internal signal!").
										 arg(outputAppSignalID));
				m_pOutputSignalIDEdit->setFocus();
				return;
			}

			break;

		case Metrology::CONNECTION_TYPE_INPUT_OUTPUT:
		case Metrology::CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
		case Metrology::CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:
		case Metrology::CONNECTION_TYPE_TUNING_OUTPUT:

			if (pOutSignal->isOutput() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not output signal!").
										 arg(outputAppSignalID));
				m_pOutputSignalIDEdit->setFocus();
				return;
			}

			if (electricLimitIsValid(pOutSignal) == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 has wrong electric limit!").
										 arg(outputAppSignalID));
				m_pInputSignalIDEdit->setFocus();
				return;
			}

			break;
	}

	//
	//

	m_connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT, pInSignal);
	m_connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT, pOutSignal);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// class DialogMetrologyConnection

DialogMetrologyConnection::DialogMetrologyConnection(SignalSetProvider* signalSetProvider, QWidget *parent) :
	QDialog(parent),
	m_signalSetProvider(signalSetProvider)
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnection::~DialogMetrologyConnection()
{
	if (m_dialogConnectionItem != nullptr)
	{
		delete m_dialogConnectionItem;
		m_dialogConnectionItem = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::createInterface()
{
	setWindowTitle(m_windowTitle);
	setWindowIcon(QIcon(":/Images/Images/MetrologyConnection.svg"));
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.40), static_cast<int>(screen.height() * 0.5));
	move(screen.center() - rect().center());

	// menu
	//
	m_pMenuBar = new QMenuBar(this);
	m_pConnectionMenu = new QMenu(tr("&Connection"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	// actions
	//
		// connections
		//
	m_pEditAction = m_pConnectionMenu->addAction(tr("&Edit ..."));
	m_pEditAction->setIcon(QIcon(":/Images/Images/SchemaOpen.svg"));

	m_pCreateAction = m_pConnectionMenu->addAction(tr("&New ..."));
	m_pCreateAction->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_pCreateAction->setShortcut(Qt::Key_Insert);

	m_pRemoveAction = m_pConnectionMenu->addAction(tr("&Remove"));
	m_pRemoveAction->setIcon(QIcon(":/Images/Images/SchemaDelete.svg"));
	m_pRemoveAction->setShortcut(Qt::Key_Delete);

	m_pConnectionMenu->addSeparator();

	m_pCheckInAction = m_pConnectionMenu->addAction(tr("&Check In ..."));
	m_pCheckInAction->setIcon(QIcon(":/Images/Images/SchemaCheckIn.svg"));

	m_pConnectionMenu->addSeparator();

	m_pExportAction = m_pConnectionMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/Images/Images/SchemaUpload.svg"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pImportAction = m_pConnectionMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/Images/Images/SchemaDownload.svg"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

		// edit
		//
	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/Images/Images/Copy.svg"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/Images/Images/SelectAll.svg"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pMenuBar->addMenu(m_pConnectionMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pEditAction, &QAction::triggered, this, &DialogMetrologyConnection::editConnection);
	connect(m_pCreateAction, &QAction::triggered, this, &DialogMetrologyConnection::newConnection);
	connect(m_pRemoveAction, &QAction::triggered, this, &DialogMetrologyConnection::removeConnection);
	connect(m_pCheckInAction, &QAction::triggered, this, &DialogMetrologyConnection::checkinConnection);
	connect(m_pExportAction, &QAction::triggered, this, &DialogMetrologyConnection::exportConnections);
	connect(m_pImportAction, &QAction::triggered, this, &DialogMetrologyConnection::importConnections);

	connect(m_pCopyAction, &QAction::triggered, this, &DialogMetrologyConnection::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &DialogMetrologyConnection::selectAll);

	// toolBar
	//
	QToolBar *toolBar = new QToolBar(this);

	toolBar->setStyleSheet("QToolButton { padding: 6px; }");
	toolBar->setIconSize(toolBar->iconSize() * 0.9);

	toolBar->addAction(m_pEditAction);
	toolBar->addAction(m_pCreateAction);
	toolBar->addAction(m_pRemoveAction);

	toolBar->addSeparator();

	toolBar->addAction(m_pCheckInAction);

	toolBar->addSeparator();

	QLabel* pFindLabel = new QLabel(this);
	pFindLabel->setText(tr(" Filter "));
	pFindLabel->setEnabled(false);
	toolBar->addWidget(pFindLabel);


	m_findTextEdit = new QLineEdit(m_findText, toolBar);
	m_findTextEdit->setPlaceholderText(tr("#SYSTEM_CH*_MD*_IN??"));
	m_findTextEdit->setFixedWidth(300);
	//m_findTextEdit->setClearButtonEnabled(true);

	toolBar->addWidget(m_findTextEdit);
	QAction* action = toolBar->addAction(QIcon(":/Images/Images/Find.svg"), tr("Find text"));
	connect(action, &QAction::triggered, this, &DialogMetrologyConnection::find);

	toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	toolBar->setWindowTitle(tr("Search connections ToolBar"));

	// view
	//
	m_pView = new QTableView(this);
	m_pView->setModel(&m_connectionTable);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	m_pView->verticalHeader()->setHighlightSections(false);
	m_pView->horizontalHeader()->setHighlightSections(false);

	for(int column = 0; column < METROLOGY_CONNECTION_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, ConnectionColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &DialogMetrologyConnection::onListDoubleClicked);

	// buttonBox
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogMetrologyConnection::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogMetrologyConnection::reject);

	// layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(toolBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::updateCheckInStateOnToolBar()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	bool fileIsCheckIn = m_connectionBase.isCheckIn(db);

	// update
	//
	m_pCheckInAction->setDisabled(fileIsCheckIn);

	if (fileIsCheckIn == false)
	{
		setWindowTitle(m_windowTitle + " - Checked Out");
	}
	else
	{
		setWindowTitle(m_windowTitle + " - Checked In");
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::openConnectionBase()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return false;
	}

	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	SignalSet* signalSet = const_cast<SignalSet*>(&m_signalSetProvider->signalSet());
	if (signalSet == nullptr)
	{
		assert(signalSet);
		return false;
	}

	bool result = m_connectionBase.load(db, signalSet);
	if (result == false)
	{
		QMessageBox::critical(this, windowTitle(), tr("Error: File of meterology connection %1 is not open!").arg(Metrology::CONNECTIONS_FILE_NAME));
		return false;
	}

	updateList();

	updateCheckInStateOnToolBar();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::checkOutConnectionBase()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return false;
	}

	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	bool result = m_connectionBase.сheckOut(db);
	if (result == false)
	{
		QMessageBox::critical(this, m_windowTitle, QString("Error: Failed to check out metrology connections file: %1").arg(Metrology::CONNECTIONS_FILE_NAME));
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pEditAction);
	m_pContextMenu->addAction(m_pCreateAction);
	m_pContextMenu->addAction(m_pRemoveAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &DialogMetrologyConnection::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::updateList()
{
	m_connectionTable.clear();

	QVector<Metrology::SignalConnection> connectionList;

	int count = m_connectionBase.count();
	for(int i = 0; i < count; i++)
	{
		connectionList.append(m_connectionBase.connection(i));
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::fillConnection(bool newConnection, const Metrology::SignalConnection& connection)
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	// if dialog was opened early, then delete
	//
	if (m_dialogConnectionItem != nullptr)
	{
		delete m_dialogConnectionItem;
		m_dialogConnectionItem = nullptr;
	}

	// create new dialog
	//
	if (m_dialogConnectionItem == nullptr)
	{
		m_dialogConnectionItem = new DialogMetrologyConnectionItem(m_signalSetProvider, this);
		if (m_dialogConnectionItem == nullptr)
		{
			return;
		}

		m_dialogConnectionItem->setModal(false);

		connect(m_dialogConnectionItem, &QDialog::accepted, this, &DialogMetrologyConnection::connectionChanged, Qt::QueuedConnection);
		connect(m_dialogConnectionItem, &QDialog::rejected, this, &DialogMetrologyConnection::connectionChanged, Qt::QueuedConnection);
	}

	// fill connection
	//
	m_dialogConnectionItem->show();
	m_dialogConnectionItem->setConnection(newConnection, connection);
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::createConnectionBySignal(Signal* pSignal)
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return false;
	}

	if (pSignal == nullptr)
	{
		assert(0);
		return false;
	}

	if (pSignal->isAnalog() == false)
	{
		return false;
	}

	Metrology::SignalConnection connection;

	switch (pSignal->inOutType())
	{
		case E::SignalInOutType::Input:

			connection.setType(Metrology::CONNECTION_TYPE_INPUT_INTERNAL);
			connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT, pSignal);

			break;

		case E::SignalInOutType::Internal:

			if (pSignal->enableTuning() == false)
			{
				connection.setType(Metrology::CONNECTION_TYPE_INPUT_INTERNAL);
				connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT, pSignal);
			}
			else
			{
				connection.setType(Metrology::CONNECTION_TYPE_TUNING_OUTPUT);
				connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT, pSignal);
			}

			break;

		case E::SignalInOutType::Output:

			connection.setType(Metrology::CONNECTION_TYPE_INPUT_OUTPUT);
			connection.setSignal( Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT, pSignal);
			break;

		default:

			assert(0);
			return false;
	}

	fillConnection(true, connection);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::editConnection()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for edit!"));
		return;
	}

	fillConnection(false, m_connectionTable.at(index));
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::newConnection()
{
	fillConnection(true, Metrology::SignalConnection());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::connectionChanged()
{
	if (m_dialogConnectionItem == nullptr)
	{
		return;
	}

	int result = m_dialogConnectionItem->result();
	if (result == QDialog::Accepted)
	{
		if (checkOutConnectionBase() == true)
		{
			Metrology::SignalConnection connection = m_dialogConnectionItem->connection();
			if (connection.isValid() == false)
			{
				QMessageBox::critical(this, windowTitle(), tr("Invalid metrology connection!"));
				return;
			}

			int foundIndex = m_connectionBase.findConnectionIndex(connection);
			if (foundIndex != -1)
			{
				m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

				QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
				return;
			}

			if (m_dialogConnectionItem->isNewConnection() == true)
			{
				m_connectionBase.append(connection);
			}
			else
			{
				int index = m_pView->currentIndex().row();
				if (index < 0 || index >= m_connectionTable.connectionCount())
				{
					return;
				}

				m_connectionBase.setConnection(index, connection);
			}

			m_connectionBase.sort();

			updateList();

			foundIndex = m_connectionBase.findConnectionIndex(connection);
			if (foundIndex != -1)
			{
				m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
			}

			updateCheckInStateOnToolBar();
		}
	}

	delete m_dialogConnectionItem;
	m_dialogConnectionItem = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::removeConnection()
{
	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	if (selectedConnectionCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select connection for remove!"));
		return;
	}

	if (QMessageBox::question(this,
							  windowTitle(), tr("Do you want delete %1 connection(s)?").
							  arg(selectedConnectionCount)) == QMessageBox::No)
	{
		return;
	}

	int count = m_connectionTable.connectionCount();
	for(int index = count - 1; index >= 0; index --)
	{
		if (m_pView->selectionModel()->isRowSelected(index, QModelIndex()) == true)
		{
			if (index >= 0 && index < m_connectionTable.connectionCount())
			{
				if (checkOutConnectionBase() == true)
				{
					m_connectionBase.remove(index);

					updateCheckInStateOnToolBar();
				}
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::checkinConnection()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	// if file is CheckIn, then return
	//
	if (m_connectionBase.isCheckIn(db) == true)
	{
		return;
	}

	// create dialog for comment
	//
	QDialog* pCommendDialog = new QDialog(this);
	if (pCommendDialog == nullptr)
	{
		return;
	}

	pCommendDialog->setWindowTitle(m_windowTitle + tr(" - Check In"));
	pCommendDialog->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	pCommendDialog->resize(static_cast<int>(screen.width() * 0.30), static_cast<int>(screen.height() * 0.15));
	pCommendDialog->move(screen.center() - pCommendDialog->rect().center());

	QPlainTextEdit* pCommentEdit = new QPlainTextEdit(pCommendDialog);

	QHBoxLayout *buttonLayout = new QHBoxLayout;

	QPushButton* pCheckInButton = new QPushButton(tr("Check In"), pCommendDialog);
	QPushButton* pCancelButton = new QPushButton(tr("Cancel"), pCommendDialog);

	buttonLayout->addStretch();
	buttonLayout->addWidget(pCheckInButton);
	buttonLayout->addWidget(pCancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(pCommentEdit);
	mainLayout->addLayout(buttonLayout);

	pCommendDialog->setLayout(mainLayout);
	pCommendDialog->setModal(true);

	connect(pCheckInButton, &QPushButton::clicked, pCommendDialog, &QDialog::accept);
	connect(pCancelButton, &QPushButton::clicked, pCommendDialog, &QDialog::reject);

	int result = pCommendDialog->exec();
	if (result == QDialog::Accepted)
	{
		QString comment = pCommentEdit->toPlainText();

		bool resultSave = m_connectionBase.save(db, true, comment);
		if (resultSave == false)
		{
			QMessageBox::critical(this, m_windowTitle, QString("Error: Failed to save metrology connections file: %1 to database!").arg(Metrology::CONNECTIONS_FILE_NAME));
		}

		updateCheckInStateOnToolBar();
	}

	delete pCommendDialog;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::exportConnections()
{
	QString filter = tr("CSV files (*.csv)");

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save file"),
													Metrology::CONNECTIONS_FILE_NAME,
													filter);
	if (fileName.isEmpty() == true)
	{
		return;
	}

	bool result = m_connectionBase.exportToFile(fileName);
	if (result == false)
	{
		QMessageBox::critical(this, m_windowTitle, tr("Failed to export!"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::importConnections()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Open File"),
													Metrology::CONNECTIONS_FILE_NAME,
													"CSV files (*.csv);;All files (*.*)");
	if (fileName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file: Empty file name"));
		return;
	}

	if (QFile::exists(fileName) == false)
	{
		QMessageBox::critical(this, m_windowTitle, tr("Could not open file: %1\nfile is not found!").arg(fileName));
		return;
	}

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
		return;
	}

	QVector<Metrology::SignalConnection> connectionList;

	// read data
	//
	QTextStream in(&file);
	while (in.atEnd() == false)
	{
		Metrology::SignalConnection connection;

		QStringList line = in.readLine().split(";");
		for(int column = 0; column < line.count(); column++)
		{
			switch (column)
			{
				case 0:	connection.setType(line[column].toInt());												break;
				case 1:	connection.setAppSignalID(Metrology::IO_SIGNAL_CONNECTION_TYPE_INPUT, line[column]);	break;
				case 2:	connection.setAppSignalID(Metrology::IO_SIGNAL_CONNECTION_TYPE_OUTPUT, line[column]);	break;
			}
		}

		// init signals
		//
		for(int type = 0; type < Metrology::IO_SIGNAL_CONNECTION_TYPE_COUNT; type++)
		{
			if (connection.appSignalID(type).isEmpty() == true)
			{
				continue;
			}

			Signal* pSignal = m_signalSetProvider->getSignalByStrID(connection.appSignalID(type));
			if (pSignal == nullptr)
			{
				continue;
			}

			connection.setSignal(type, pSignal);
		}

		// append
		//
		if (m_connectionBase.findConnectionIndex(connection) != -1)
		{
			continue;
		}

		m_connectionBase.append(connection);
	}

	file.close();

	m_connectionBase.sort();

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::find()
{
	m_findText = m_findTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		updateList();
		return;
	}

	if (m_findText.indexOf("*") == -1)
	{
		m_findText.insert(0, "*");
		m_findText.append("*");
	}

	QRegExp rx(m_findText);
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	m_connectionTable.clear();

	QVector<Metrology::SignalConnection> connectionList;

	int count = m_connectionBase.count();
	for(int i = 0; i < count; i++)
	{
		const Metrology::SignalConnection& connection = m_connectionBase.connection(i);

		bool found = false;

		for(int ioType = 0; ioType < Metrology::IO_SIGNAL_CONNECTION_TYPE_COUNT; ioType++)
		{
			if(rx.exactMatch(connection.appSignalID(ioType)) == true)
			{
				found = true;
			}
		}

		if (found == true)
		{
			connectionList.append(m_connectionBase.connection(i));
		}
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::copy()
{
	if (m_pView == nullptr)
	{
		return;
	}

	QString textClipboard;

	// header
	//
	int columnCount = m_pView->model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pView->isColumnHidden(column) == true)
		{
			continue;
		}

		textClipboard.append(m_pView->model()->headerData(column, Qt::Horizontal).toString());
		textClipboard.append("\t");
	}

	textClipboard.append("\n");

	// records
	//
	int connectionCount = m_connectionBase.count();
	for(int i = 0; i < connectionCount; i++)
	{
		const Metrology::SignalConnection& connection = m_connectionBase.connection(i);

		textClipboard.append(connection.typeStr());
		textClipboard.append("\t");

		for(int ioType = 0; ioType < Metrology::IO_SIGNAL_CONNECTION_TYPE_COUNT; ioType++)
		{
			textClipboard.append(connection.appSignalID(ioType));
			textClipboard.append("\t");
		}
		textClipboard.append("\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		if (m_findTextEdit->hasFocus() == true)
		{
			find();
		}

		return;
	}

	QWidget::keyPressEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onOk()
{
	if (m_signalSetProvider == nullptr)
	{
		assert(m_signalSetProvider);
		return;
	}

	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	bool resultSave = m_connectionBase.save(db, false, QString());
	if (resultSave == false)
	{
		QMessageBox::critical(this, m_windowTitle, QString("Error: Failed to save metrology connections file: %1 to database!").arg(Metrology::CONNECTIONS_FILE_NAME));
	}

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

