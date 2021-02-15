#include "DialogMetrologyConnection.h"

#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QKeyEvent>

#include "Settings.h"

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

void MetrologyConnectionTable::setSignalSetProvider(SignalSetProvider* signalSetProvider)
{
	if (signalSetProvider == nullptr)
	{
		Q_ASSERT(signalSetProvider);
		return;
	}

	m_signalSetProvider = signalSetProvider;
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

	if (m_signalSetProvider == nullptr)
	{
		return QVariant();
	}

	const Metrology::Connection& connection = at(row);

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == METROLOGY_CONNECTION_COLUMN_IN_ID)
		{
			::Signal* pSignal = m_signalSetProvider->getSignalByStrID(connection.appSignalID(Metrology::ConnectionIoType::Source));
			if (pSignal == nullptr)	// if input signal is not exist
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == METROLOGY_CONNECTION_COLUMN_TYPE)
		{
			int type = connection.type();
			if (type < 0 || type >= Metrology::ConnectionTypeCount)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == METROLOGY_CONNECTION_COLUMN_OUT_ID)
		{
			::Signal* pSignal = m_signalSetProvider->getSignalByStrID(connection.appSignalID(Metrology::ConnectionIoType::Destination));
			if (pSignal == nullptr)	// if output signal is not exist
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		switch (connection.action().toInt())
		{
			case VcsItemAction::VcsItemActionType::Added :		return QColor(StandardColors::VcsAdded);
			case VcsItemAction::VcsItemActionType::Modified :	return QColor(StandardColors::VcsModified);
			case VcsItemAction::VcsItemActionType::Deleted :	return QColor(StandardColors::VcsDeleted);
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

QString MetrologyConnectionTable::text(int row, int column, const Metrology::Connection& connection) const
{
	if (row < 0 || row >= connectionCount())
	{
		return QString();
	}

	if (column < 0 || column > METROLOGY_CONNECTION_COLUMN_COUNT)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case METROLOGY_CONNECTION_COLUMN_IN_ID:		result = connection.appSignalID(Metrology::ConnectionIoType::Source);		break;
		case METROLOGY_CONNECTION_COLUMN_TYPE:		result = connection.typeStr();												break;
		case METROLOGY_CONNECTION_COLUMN_OUT_ID:	result = connection.appSignalID( Metrology::ConnectionIoType::Destination);	break;
		default:									Q_ASSERT(0);
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

Metrology::Connection MetrologyConnectionTable::at(int index) const
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return Metrology::Connection();
	}

	return m_connectionList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionTable::set(const QVector<Metrology::Connection>& list_add)
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

DialogMetrologyConnectionItem::DialogMetrologyConnectionItem(SignalSetProvider* signalSetProvider, QWidget* parent) :
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
	resize(static_cast<int>(screen.width() * 0.20), static_cast<int>(screen.height() * 0.08));
	move(screen.center() - rect().center());

	QVBoxLayout *labelsLayout = new QVBoxLayout;

	labelsLayout->addWidget(new QLabel(tr("Source AppSignalID"), this) );
	labelsLayout->addWidget(new QLabel(tr("Connection type"), this));
	labelsLayout->addWidget(new QLabel(tr("Destination AppSignalID"), this));

	QVBoxLayout *editLayout = new QVBoxLayout;

	m_pInputSignalIDEdit = new QLineEdit(QString(), this);
	m_pTypeList = new QComboBox(this);
	m_pOutputSignalIDEdit = new QLineEdit(QString(), this);

	editLayout->addWidget(m_pInputSignalIDEdit);
	editLayout->addWidget(m_pTypeList);
	editLayout->addWidget(m_pOutputSignalIDEdit);

	QHBoxLayout *ctrlsLayout = new QHBoxLayout;

	ctrlsLayout->addLayout(labelsLayout);
	ctrlsLayout->addLayout(editLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// fill type list
	//
	for (int type = 0; type < Metrology::ConnectionTypeCount; type++)
	{
		m_pTypeList->addItem(Metrology::ConnectionTypeCaption(static_cast<Metrology::ConnectionType>(type)), type);
	}
	m_pTypeList->removeItem(Metrology::ConnectionType::Unsed);
	m_pTypeList->setCurrentIndex(0);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogMetrologyConnectionItem::selectedType);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogMetrologyConnectionItem::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogMetrologyConnectionItem::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addLayout(ctrlsLayout);
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

	int type = m_connection.type();
	if ((type < 0 || type >= Metrology::ConnectionTypeCount) || type == Metrology::ConnectionType::Unsed)
	{
		m_connection.setType(Metrology::ConnectionType::Input_Internal);
		type = m_connection.type();
	}

	m_pTypeList->setCurrentIndex(type - 1);
	m_pInputSignalIDEdit->setText(m_connection.appSignalID( Metrology::ConnectionIoType::Source));
	m_pOutputSignalIDEdit->setText(m_connection.appSignalID( Metrology::ConnectionIoType::Destination));
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::selectedType(int)
{
	int type = m_pTypeList->currentData().toInt() ;
	if (type < 0 || type >= Metrology::ConnectionTypeCount)
	{
		return;
	}

	m_connection.setType(static_cast<Metrology::ConnectionType>(type));
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::setConnection(bool newConnection, const Metrology::Connection& connection)
{
	m_parentConnection = connection;
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
		Q_ASSERT(pSignal);
		return false;
	}

	switch (pSignal->inOutType())
	{
		case E::SignalInOutType::Input:

			if (pSignal->isSpecPropExists(SignalProperties::electricLowLimitCaption) == false ||
				pSignal->isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
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
	if (type < 0 || type >= Metrology::ConnectionTypeCount)
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
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
		case Metrology::ConnectionType::Input_C_Output_F:

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

		case Metrology::ConnectionType::Tuning_Output:

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
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:

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

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Tuning_Output:

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

	m_connection.setSignal(Metrology::ConnectionIoType::Source, pInSignal);
	m_connection.setSignal(Metrology::ConnectionIoType::Destination, pOutSignal);
	m_connection.createCrc();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// class DialogMetrologyConnection

DialogMetrologyConnection::DialogMetrologyConnection(SignalSetProvider* signalSetProvider, QWidget* parent) :
	QDialog(parent),
	m_signalSetProvider(signalSetProvider)
{
	if (m_signalSetProvider == nullptr)
	{
		Q_ASSERT(m_signalSetProvider);
		return;
	}

	m_connectionTable.setSignalSetProvider(m_signalSetProvider);
	m_connectionBase.setSignalSetProvider(m_signalSetProvider);

	m_isModified = false;

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
	resize(static_cast<int>(screen.width() * 0.5), static_cast<int>(screen.height() * 0.5));
	move(screen.center() - rect().center());

	if (theSettings.m_dialogMetrologyConnectionGeometry.isEmpty() == false)
	{
		restoreGeometry(theSettings.m_dialogMetrologyConnectionGeometry);
	}

	// actions
	//
		// connections
		//
	m_pEditAction = new QAction(tr("&Edit ..."), this);
	m_pEditAction->setIcon(QIcon(":/Images/Images/SchemaOpen.svg"));

	m_pCreateAction = new QAction(tr("&New ..."), this);
	m_pCreateAction->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_pCreateAction->setShortcut(Qt::Key_Insert);

	m_pRemoveAction = new QAction(tr("&Delete"), this);
	m_pRemoveAction->setIcon(QIcon(":/Images/Images/SchemaDelete.svg"));
	m_pRemoveAction->setShortcut(Qt::Key_Delete);

	m_pUnRemoveAction = new QAction(tr("&Undo delete"), this);
	m_pUnRemoveAction->setIcon(QIcon(":/Images/Images/SchemaUndo.svg"));

	m_pCheckInAction = new QAction(tr("&Check In ..."), this);
	m_pCheckInAction->setIcon(QIcon(":/Images/Images/SchemaCheckIn.svg"));

	m_pCopyAction = new QAction(tr("&Copy As Text"), this);
	m_pCopyAction->setIcon(QIcon(":/Images/Images/Copy.svg"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pExportAction = new QAction(tr("&Export ..."), this);
	m_pExportAction->setIcon(QIcon(":/Images/Images/SchemaUpload.svg"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pImportAction = new QAction(tr("&Import ..."), this);
	m_pImportAction->setIcon(QIcon(":/Images/Images/SchemaDownload.svg"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

	m_pSelectAllAction = new QAction(tr("Select &All"), this);
	m_pSelectAllAction->setIcon(QIcon(":/Images/Images/SelectAll.svg"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	connect(m_pEditAction, &QAction::triggered, this, &DialogMetrologyConnection::editConnection);
	connect(m_pCreateAction, &QAction::triggered, this, &DialogMetrologyConnection::newConnection);
	connect(m_pRemoveAction, &QAction::triggered, this, &DialogMetrologyConnection::removeConnection);
	connect(m_pUnRemoveAction, &QAction::triggered, this, &DialogMetrologyConnection::unremoveConnection);
	connect(m_pCheckInAction, &QAction::triggered, this, &DialogMetrologyConnection::checkinConnection);
	connect(m_pCopyAction, &QAction::triggered, this, &DialogMetrologyConnection::copy);
	connect(m_pExportAction, &QAction::triggered, this, &DialogMetrologyConnection::exportConnections);
	connect(m_pImportAction, &QAction::triggered, this, &DialogMetrologyConnection::importConnections);
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
	toolBar->addAction(m_pCopyAction);
	toolBar->addSeparator();
	toolBar->addAction(m_pExportAction);
	toolBar->addAction(m_pImportAction);
	toolBar->addSeparator();

	QLabel* pFindLabel = new QLabel(this);
	pFindLabel->setText(tr(" Filter "));
	pFindLabel->setEnabled(false);
	toolBar->addWidget(pFindLabel);


	m_findTextEdit = new QLineEdit(m_findText, toolBar);
	m_findTextEdit->setPlaceholderText(tr("#SYSTEM_RACK_CH*_MD*_IN??"));
	m_findTextEdit->setFixedWidth(static_cast<int>(screen.width() * 0.25));
	//m_findTextEdit->setClearButtonEnabled(true);

	toolBar->addWidget(m_findTextEdit);
	QAction* action = toolBar->addAction(QIcon(":/Images/Images/Find.svg"), tr("Find text"));
	connect(action, &QAction::triggered, this, &DialogMetrologyConnection::find);

	toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	toolBar->setWindowTitle(tr("Search connections ToolBar"));

	// view
	//
	QSortFilterProxyModel* pSourceProxyModel = new QSortFilterProxyModel(this);
	pSourceProxyModel->setSourceModel(&m_connectionTable);
	m_pView = new QTableView(this);
	m_pView->setModel(pSourceProxyModel);
	m_pView->setSortingEnabled(true);
	pSourceProxyModel->sort(METROLOGY_CONNECTION_COLUMN_IN_ID, Qt::AscendingOrder);

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	m_pView->verticalHeader()->setHighlightSections(false);
	m_pView->verticalHeader()->hide();
	m_pView->horizontalHeader()->setHighlightSections(false);
	m_pView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	if (theSettings.m_dialogMetrologyConnectionColumnsWidth.isEmpty() == false)
	{
		restoreColumnsWidth();
	}
	else
	{
		for(int column = 0; column < METROLOGY_CONNECTION_COLUMN_COUNT; column++)
		{
			m_pView->setColumnWidth(column, ConnectionColumnWidth[column]);
		}
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

	mainLayout->addWidget(toolBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::updateCheckInStateOnToolBar()
{
	bool fileIsCheckIn = m_connectionBase.isCheckIn();

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

bool DialogMetrologyConnection::enableNewConnection(const Signal& signal)
{
	if (signal.isAnalog() == false)
	{
		return false;
	}

	switch (signal.inOutType())
	{
		case E::SignalInOutType::Input:

			if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
			{
				return false;
			}

			if (signal.lowEngineeringUnits() == 0.0 && signal.highEngineeringUnits() == 0.0)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
			{
				return false;
			}

			if (signal.electricLowLimit() == 0.0 && signal.electricHighLimit() == 0.0)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
			{
				return false;
			}

			if (signal.electricUnit() == E::ElectricUnit::NoUnit)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
			{
				return false;
			}

			break;

		case E::SignalInOutType::Internal:

			if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
			{
				return false;
			}

			if (signal.lowEngineeringUnits() == 0.0 && signal.highEngineeringUnits() == 0.0)
			{
				return false;
			}

			break;

		case E::SignalInOutType::Output:

			if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
			{
				return false;
			}

			if (signal.lowEngineeringUnits() == 0.0 && signal.highEngineeringUnits() == 0.0)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
			{
				return false;
			}

			if (signal.isSpecPropExists(SignalProperties::outputModeCaption) == false)
			{
				return false;
			}

			break;

		default:

			Q_ASSERT(0);
			return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::loadConnectionBase()
{
	if (m_signalSetProvider == nullptr)
	{
		Q_ASSERT(m_signalSetProvider);
		return false;
	}


	DbController* db = m_signalSetProvider->dbController();
	if (db == nullptr)
	{
		Q_ASSERT(db);
		return false;
	}

	bool result = m_connectionBase.load(db);
	if (result == false)
	{
		QMessageBox::critical(this, m_windowTitle, tr("Error: File of meterology connection %1 is not open!").arg(Metrology::CONNECTIONS_FILE_NAME));
		return false;
	}

	m_isModified = false;

	m_connectionBase.findSignal_in_signalSet();

	updateList();

	updateCheckInStateOnToolBar();

	if (m_connectionBase.enableEditBase() == false)
	{
		setWindowTitle(m_windowTitle + tr(" - View only (currently file is cheked out by user: \"%1\")").arg(m_connectionBase.userName()));

		m_pEditAction->setDisabled(true);
		m_pCreateAction->setDisabled(true);
		m_pRemoveAction->setDisabled(true);
		m_pUnRemoveAction->setDisabled(true);
		m_pCheckInAction->setDisabled(true);
		m_pImportAction->setDisabled(true);

		m_buttonBox->hide();
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::saveConnectionBase(bool checkIn, const QString& comment)
{
	if (m_connectionBase.enableEditBase() == false)
	{
		return;
	}

	bool resultSave = m_connectionBase.save(checkIn, comment);
	if (resultSave == false)
	{
		QMessageBox::critical(this, m_windowTitle, QString("Error: Failed to save metrology connections file: %1 to database!").arg(Metrology::CONNECTIONS_FILE_NAME));
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::checkOutConnectionBase()
{
	bool result = m_connectionBase.checkOut();
	if (result == false)
	{
		QMessageBox::critical(this, m_windowTitle, QString("Error: Failed to check out metrology connections file: %1").arg(Metrology::CONNECTIONS_FILE_NAME));
	}

	updateCheckInStateOnToolBar();

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
	m_pContextMenu->addAction(m_pUnRemoveAction);
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
	QVector<Metrology::Connection> connectionList;

	m_connectionTable.clear();

	m_findText = m_findTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		int count = m_connectionBase.count();
		for(int i = 0; i < count; i++)
		{
			connectionList.append(m_connectionBase.connection(i));
		}
	}
	else
	{
		if (m_findText.indexOf("*") == -1)
		{
			m_findText.insert(0, "*");
			m_findText.append("*");
		}

		QRegExp rx(m_findText);
		rx.setPatternSyntax(QRegExp::Wildcard);
		rx.setCaseSensitivity(Qt::CaseInsensitive);

		int count = m_connectionBase.count();
		for(int i = 0; i < count; i++)
		{
			const Metrology::Connection& connection = m_connectionBase.connection(i);

			bool found = false;

			for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
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
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::selectConnectionInList(const Metrology::Connection& connection)
{
	if (m_pView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int count = pSourceProxyModel->rowCount();
	for (int i = 0; i < count; i ++)
	{
		QModelIndex index = m_pView->model()->index(i, 0);

		int conncetionIndex = pSourceProxyModel->mapToSource(index).row();
		if (conncetionIndex < 0 || conncetionIndex >= m_connectionTable.connectionCount())
		{
			continue;
		}

		if (m_connectionTable.at(conncetionIndex).crc().result() == connection.crc().result())
		{
			m_pView->setCurrentIndex(index);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::fillConnection(bool newConnection, const Metrology::Connection& connection)
{
	if (m_signalSetProvider == nullptr)
	{
		Q_ASSERT(m_signalSetProvider);
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
	if (pSignal == nullptr)
	{
		Q_ASSERT(0);
		return false;
	}

	if (pSignal->isAnalog() == false)
	{
		return false;
	}

	if(m_connectionBase.count() == 0)
	{
		loadConnectionBase();
	}

	Metrology::Connection connection;

	switch (pSignal->inOutType())
	{
		case E::SignalInOutType::Input:

			connection.setType(Metrology::ConnectionType::Input_Internal);
			connection.setSignal(Metrology::ConnectionIoType::Source, pSignal);

			break;

		case E::SignalInOutType::Internal:

			if (pSignal->enableTuning() == false)
			{
				connection.setType(Metrology::ConnectionType::Input_Internal);
				connection.setSignal(Metrology::ConnectionIoType::Destination, pSignal);
			}
			else
			{
				connection.setType(Metrology::ConnectionType::Tuning_Output);
				connection.setSignal(Metrology::ConnectionIoType::Source, pSignal);
			}

			break;

		case E::SignalInOutType::Output:

			connection.setType(Metrology::ConnectionType::Input_Output);
			connection.setSignal(Metrology::ConnectionIoType::Destination, pSignal);
			break;

		default:

			Q_ASSERT(0);
			return false;
	}

	fillConnection(true, connection);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::editConnection()
{
	if (m_pView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	QModelIndex currInndex = m_pView->currentIndex();

	int conncetionIndex = pSourceProxyModel->mapToSource(currInndex).row();
	if (conncetionIndex < 0 || conncetionIndex >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, m_windowTitle, tr("Please, select сonnection for edit!"));
		return;
	}

	const Metrology::Connection& connection = m_connectionTable.at(conncetionIndex);

	if (connection.action() == VcsItemAction::VcsItemActionType::Deleted)
	{
		QMessageBox::information(this, m_windowTitle, tr("This connection is deleted!"));
		return;
	}

	fillConnection(false, connection);

	m_pView->selectionModel()->clear();
	m_pView->setCurrentIndex(currInndex);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::newConnection()
{
	fillConnection(true, Metrology::Connection());
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
			Metrology::Connection connection = m_dialogConnectionItem->connection();
			if (connection.isValid() == false)
			{
				QMessageBox::critical(this, m_windowTitle, tr("Invalid metrology connection!"));
				return;
			}

			int foundIndex = m_connectionBase.findConnectionIndex(connection);
			if (foundIndex != -1)
			{
				selectConnectionInList(connection);
				QMessageBox::information(this, m_windowTitle, tr("Connection already exist!"));
				return;
			}

			if (m_dialogConnectionItem->isNewConnection() == true)
			{
				connection.setAction(VcsItemAction::VcsItemActionType::Added);

				m_connectionBase.append(connection);
			}
			else
			{
				int connectionIndex = m_connectionBase.findConnectionIndex(m_dialogConnectionItem->parentConnection());
				if (connectionIndex < 0 || connectionIndex >= m_connectionBase.count())
				{
					return;
				}

				connection.setAction(VcsItemAction::VcsItemActionType::Modified);

				m_connectionBase.setConnection(connectionIndex, connection);
			}

			updateList();
			selectConnectionInList(connection);

			m_isModified = true;
		}
	}

	delete m_dialogConnectionItem;
	m_dialogConnectionItem = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::removeConnection()
{
	if (m_dialogConnectionItem != nullptr)
	{
		m_dialogConnectionItem->activateWindow();
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	if (selectedConnectionCount == 0)
	{
		QMessageBox::information(this, m_windowTitle, tr("Please, select connection for delete!"));
		return;
	}

	int result = QMessageBox::question(this,
									   m_windowTitle,
									   tr("Do you want delete %1 connection(s)?").
									   arg(selectedConnectionCount));
	if (result == QMessageBox::No)
	{
		return;
	}

	for( int i = 0; i < selectedConnectionCount; i++)
	{
		int tableIndex = pSourceProxyModel->mapToSource(m_pView->selectionModel()->selectedRows().at(i)).row();
		if (tableIndex >= 0 && tableIndex < m_connectionTable.connectionCount())
		{
			const Metrology::Connection& connection = m_connectionTable.at(tableIndex);

			int connectionIndex = m_connectionBase.findConnectionIndex(connection);
			if (connectionIndex >= 0 && connectionIndex < m_connectionBase.count())
			{
				if (checkOutConnectionBase() == true)
				{
					m_connectionBase.setAction(connectionIndex, VcsItemAction::VcsItemActionType::Deleted);

					m_isModified = true;
				}
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::unremoveConnection()
{
	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	if (selectedConnectionCount == 0)
	{
		QMessageBox::information(this, m_windowTitle, tr("Please, select connection for undo delete!"));
		return;
	}

	for( int i = 0; i < selectedConnectionCount; i++)
	{
		int conncetionIndex = pSourceProxyModel->mapToSource(m_pView->selectionModel()->selectedRows().at(i)).row();

		if (conncetionIndex >= 0 && conncetionIndex < m_connectionTable.connectionCount())
		{
			if (m_connectionBase.connection(conncetionIndex).action() == VcsItemAction::VcsItemActionType::Deleted)
			{
				m_connectionBase.setAction(conncetionIndex, VcsItemAction::VcsItemActionType::Modified);

				m_isModified = true;
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::checkinConnection()
{
	// if file is CheckIn, then return
	//
	if (m_connectionBase.isCheckIn() == true)
	{
		return;
	}

	// create dialog for comment
	//
	DialogComment commentDialog(this);

	int result = commentDialog.exec();
	if (result != QDialog::Accepted)
	{
		return;
	}

	QString comment = commentDialog.comment();
	if (comment.isEmpty() == true)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Checkin comment is empty!"));
		return;
	}

	saveConnectionBase(true, comment);

	updateCheckInStateOnToolBar();

	updateList();

	m_isModified = false;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::exportConnections()
{
	QString filter = tr("CSV files (*.csv)");

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Export to file"),
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
		Q_ASSERT(m_signalSetProvider);
		return;
	}

	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Import from file"),
													Metrology::CONNECTIONS_FILE_NAME,
													"CSV files (*.csv);;All files (*.*)");
	if (fileName.isEmpty() == true)
	{
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
		QMessageBox::critical(this, m_windowTitle, tr("Could not open file"));
		return;
	}

	if (checkOutConnectionBase() == false)
	{
		file.close();
		return;
	}

	// read data
	//
	QTextStream in(&file);
	while (in.atEnd() == false)
	{
		Metrology::Connection connection;

		QStringList line = in.readLine().split(";");
		for(int column = 0; column < line.count(); column++)
		{
			switch (column)
			{
				case 0:	connection.setType(static_cast<Metrology::ConnectionType>(line[column].toInt()));	break;
				case 1:	connection.setAppSignalID(Metrology::ConnectionIoType::Source, line[column]);		break;
				case 2:	connection.setAppSignalID(Metrology::ConnectionIoType::Destination, line[column]);	break;
			}
		}

		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
		{
			if (connection.appSignalID(ioType).isEmpty() == true)
			{
				continue;
			}
		}

		// append
		//
		connection.createCrc();
		connection.setAction(VcsItemAction::Added);

		if (m_connectionBase.findConnectionIndex(connection) != -1)
		{
			continue;
		}

		m_connectionBase.append(connection);
	}

	file.close();

	// init signals
	//
	int connectionCount = m_connectionBase.count();
	for (int i = 0; i < connectionCount; i++)
	{
		Metrology::Connection* pConnection = m_connectionBase.connectionPtr(i);
		if (pConnection == nullptr)
		{
			continue;
		}

		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
		{
			Signal* pSignal = m_signalSetProvider->getSignalByStrID(pConnection->appSignalID(ioType));
			if (pSignal == nullptr)
			{
				continue;
			}

			pConnection->setSignal(ioType, pSignal);
		}
	}

	updateList();

	m_isModified = true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::copy()
{
	if (m_pView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	if (selectedConnectionCount == 0)
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
	for( int i = 0; i < selectedConnectionCount; i++)
	{
		int conncetionIndex = pSourceProxyModel->mapToSource(m_pView->selectionModel()->selectedRows().at(i)).row();
		if (conncetionIndex >= 0 && conncetionIndex < m_connectionTable.connectionCount())
		{
			const Metrology::Connection& connection = m_connectionTable.at(conncetionIndex);

			textClipboard.append(connection.appSignalID(Metrology::ConnectionIoType::Source));
			textClipboard.append("\t");

			textClipboard.append(connection.typeStr());
			textClipboard.append("\t");

			textClipboard.append(connection.appSignalID(Metrology::ConnectionIoType::Destination));
			textClipboard.append("\t");

			textClipboard.append("\n");
		}
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onContextMenu(QPoint)
{
	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	//
	//
	bool enableUnremove = false;

	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	for( int i = 0; i < selectedConnectionCount; i++)
	{
		int conncetionIndex = pSourceProxyModel->mapToSource(m_pView->selectionModel()->selectedRows().at(i)).row();

		if (conncetionIndex >= 0 && conncetionIndex < m_connectionTable.connectionCount())
		{
			if (m_connectionBase.connection(conncetionIndex).action() == VcsItemAction::VcsItemActionType::Deleted)
			{
				enableUnremove = true;
			}
		}
	}

	m_pUnRemoveAction->setEnabled(enableUnremove);

	//
	//
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		if (m_findTextEdit->hasFocus() == true)
		{
			updateList();
		}

		return;
	}

	QWidget::keyPressEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::saveChanges()
{
	if (m_isModified == false)
	{
		return;
	}

	int result = QMessageBox::question(this,
									   m_windowTitle,
									   tr("List of metrology connections has been changed\nDo you want to save changes?"));
	if (result == QMessageBox::No)
	{
		return;
	}

	saveConnectionBase(false, QString());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::saveColumnsWidth()
{
	if (m_pView == nullptr)
	{
		Q_ASSERT(m_pView);
		return;
	}

	int columnCount = m_pView->model()->columnCount();
	for (int i = 0; i < columnCount; i++)
	{
		QString columnName = m_pView->model()->headerData(i, Qt::Horizontal).toString();

		int pos = columnName.indexOf(QChar::LineFeed);
		if (pos != -1)
		{
			columnName = columnName.left(pos);
		}

		theSettings.m_dialogMetrologyConnectionColumnsWidth[columnName] = m_pView->columnWidth(i);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::restoreColumnsWidth()
{
	if (m_pView == nullptr)
	{
		Q_ASSERT(m_pView);
		return;
	}
	for (int i = 0; i < m_pView->model()->columnCount(); i++)
	{
		QString columnName = m_pView->model()->headerData(i, Qt::Horizontal).toString();

		int pos = columnName.indexOf(QChar::LineFeed);
		if (pos != -1)
		{
			columnName = columnName.left(pos);
		}

		auto it = theSettings.m_dialogMetrologyConnectionColumnsWidth.find(columnName);
		if (it != theSettings.m_dialogMetrologyConnectionColumnsWidth.end())
		{
			m_pView->setColumnWidth(i, it.value());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::saveSettings()
{
	theSettings.m_dialogMetrologyConnectionGeometry = saveGeometry();
	saveColumnsWidth();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::closeEvent(QCloseEvent*)
{
	saveChanges();

	saveSettings();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::done(int r)
{
	if (r == QDialog::Rejected)
	{
		saveChanges();
	}

	saveSettings();

	QDialog::done(r);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onOk()
{
	saveConnectionBase(false, QString());

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// class DialogComment

DialogComment::DialogComment(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComment::createInterface()
{
	setWindowTitle(tr("Check In"));
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.15));
	move(screen.center() - rect().center());

	QLabel* pCommentLabel = new QLabel(tr("Check In Comment:"), this);
	m_pCommentEdit = new QPlainTextEdit(this);
	QHBoxLayout *buttonLayout = new QHBoxLayout;

	QPushButton* pCheckInButton = new QPushButton(tr("Check In"), this);
	QPushButton* pCancelButton = new QPushButton(tr("Cancel"), this);

	buttonLayout->addStretch();
	buttonLayout->addWidget(pCheckInButton);
	buttonLayout->addWidget(pCancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(pCommentLabel);
	mainLayout->addWidget(m_pCommentEdit);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	connect(pCheckInButton, &QPushButton::clicked, this, &DialogComment::onOk);
	connect(pCancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComment::onOk()
{
	if (m_pCommentEdit == nullptr)
	{
		Q_ASSERT(m_pCommentEdit);
		return;
	}

	m_comment = m_pCommentEdit->toPlainText();
	if (m_comment.isEmpty() == true)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Checkin comment is empty!"));
		return;
	}

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

