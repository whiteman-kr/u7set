#include "MetrologyConnectionList.h"

#include <QFileDialog>
#include <QMessageBox>

#include "ProcessData.h"
#include "SignalList.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

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

	Metrology::Connection connection = at(row);

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == METROLOGY_CONNECTION_COLUMN_IN_ID)
		{
			Metrology::Signal* pSignal = theSignalBase.signalPtr(connection.appSignalID(Metrology::ConnectionIoType::Source));
			if (pSignal == nullptr || pSignal->param().isValid() == false)
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
			Metrology::Signal* pSignal = theSignalBase.signalPtr(connection.appSignalID(Metrology::ConnectionIoType::Destination));
			if (pSignal == nullptr || pSignal->param().isValid() == false)
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

	bool visible = true;

	QString result;

	switch (column)
	{
		case METROLOGY_CONNECTION_COLUMN_IN_ID:		result = connection.appSignalID(Metrology::ConnectionIoType::Source);			break;
		case METROLOGY_CONNECTION_COLUMN_TYPE:		result = qApp->translate("MetrologyConnection", connection.typeStr().toUtf8());	break;
		case METROLOGY_CONNECTION_COLUMN_OUT_ID:	result = connection.appSignalID(Metrology::ConnectionIoType::Destination);		break;
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

MetrologyConnectionItemDialog::MetrologyConnectionItemDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	updateSignals();
}


// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionItemDialog::MetrologyConnectionItemDialog(const Metrology::Connection& metrologyConnection, QWidget* parent) :
	QDialog(parent)
{
	m_metrologyConnection = metrologyConnection;

	createInterface();
	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionItemDialog::~MetrologyConnectionItemDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

	QRect screen = QDesktopWidget().availableGeometry(this);
	resize(static_cast<int>(screen.width() * 0.2), static_cast<int>(screen.height() * 0.04));
	move(screen.center() - rect().center());

	if (m_metrologyConnection.crc().result() == 0xFFFFFFFFFFFFFFFF)
	{
		setWindowIcon(QIcon(":/icons/Add.png"));
		setWindowTitle(tr("Create connection"));
	}
	else
	{
		setWindowIcon(QIcon(":/icons/Edit.png"));
		setWindowTitle(tr("Edit connection"));
	}

	// metrology connection type
	//
	QHBoxLayout* typeLayout = new QHBoxLayout;

	m_pTypeList = new QComboBox(this);

	typeLayout->addWidget(new QLabel(tr("Connection type"), this));
	typeLayout->addWidget(m_pTypeList);
	typeLayout->addStretch();

	// Signals
	//
	QGroupBox* signalGroup = new QGroupBox(QString());
	QVBoxLayout* signalLayout = new QVBoxLayout;

		// Input signal
		//
	QHBoxLayout* inputSignalLayout = new QHBoxLayout;

	QLabel* pInputSignalLabel = new QLabel(tr("AppSignalID (source)"), this);
	m_pInputSignalIDEdit = new QLineEdit(QString(), this);
	m_pInputSignalButton = new QPushButton(tr("Select ..."), this);

	pInputSignalLabel->setFixedWidth(150);
	m_pInputSignalIDEdit->setFixedWidth(200);

	//m_pInputSignalIDEdit->setReadOnly(true);
	m_pInputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	inputSignalLayout->addWidget(pInputSignalLabel);
	inputSignalLayout->addWidget(m_pInputSignalIDEdit);
	inputSignalLayout->addWidget(m_pInputSignalButton);
	inputSignalLayout->addStretch();

		// Output signal
		//
	QHBoxLayout* outputSignalLayout = new QHBoxLayout;

	QLabel* pOutputSignalLabel = new QLabel(tr("AppSignalID (destination)"), this);
	m_pOutputSignalIDEdit = new QLineEdit(QString(), this);
	m_pOutputSignalButton = new QPushButton(tr("Select ..."), this);

	pOutputSignalLabel->setFixedWidth(150);
	m_pOutputSignalIDEdit->setFixedWidth(200);

	//m_pOutputSignalIDEdit->setReadOnly(true);
	m_pOutputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	outputSignalLayout->addWidget(pOutputSignalLabel);
	outputSignalLayout->addWidget(m_pOutputSignalIDEdit);
	outputSignalLayout->addWidget(m_pOutputSignalButton);
	outputSignalLayout->addStretch();


	signalLayout->addLayout(inputSignalLayout);
	signalLayout->addLayout(outputSignalLayout);

	signalGroup->setLayout(signalLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// fill type list
	//
	for (int type = 0; type < Metrology::ConnectionTypeCount; type++)
	{
		m_pTypeList->addItem(qApp->translate("MetrologyConnection", Metrology::ConnectionTypeCaption(static_cast<Metrology::ConnectionType>(type)).toUtf8()), type);
	}
	m_pTypeList->removeItem(Metrology::ConnectionType::Unsed);

	int type = m_metrologyConnection.type() ;
	if ((type < 0 || type >= Metrology::ConnectionTypeCount) || type == Metrology::ConnectionType::Unsed)
	{
		m_metrologyConnection.setType(Metrology::ConnectionType::Input_Internal);
		type = m_metrologyConnection.type();
	}
	m_pTypeList->setCurrentIndex(type - 1);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MetrologyConnectionItemDialog::selectedType);
	connect(m_pInputSignalButton, &QPushButton::clicked, this, &MetrologyConnectionItemDialog::selectInputSignal);
	connect(m_pOutputSignalButton, &QPushButton::clicked, this, &MetrologyConnectionItemDialog::selectOutputSignal);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &MetrologyConnectionItemDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &MetrologyConnectionItemDialog::reject);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addLayout(typeLayout);
	mainLayout->addWidget(signalGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::updateSignals()
{
	int type = m_metrologyConnection.type();
	if (type < 0 || type >= Metrology::ConnectionTypeCount)
	{
		return;
	}

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	m_pInputSignalIDEdit->setText(m_metrologyConnection.appSignalID(Metrology::ConnectionIoType::Source));
	m_pOutputSignalIDEdit->setText(m_metrologyConnection.appSignalID(Metrology::ConnectionIoType::Destination));
}
// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::selectedType(int)
{
	int type = m_pTypeList->currentData().toInt() ;
	if (type < 0 || type >= Metrology::ConnectionTypeCount)
	{
		return;
	}

	m_metrologyConnection.setType(static_cast<Metrology::ConnectionType>(type));
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::selectInputSignal()
{
	selectSignal(Metrology::ConnectionIoType::Source);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::selectOutputSignal()
{
	selectSignal(Metrology::ConnectionIoType::Destination);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::selectSignal(int ioType)
{
	SignalListDialog dialog(true, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	Hash selectedSignalHash = dialog.selectedSignalHash();
	if (selectedSignalHash == UNDEFINED_HASH)
	{
		assert(selectedSignalHash != UNDEFINED_HASH);
		return;
	}

	Metrology::Signal* pSignal = theSignalBase.signalPtr(selectedSignalHash);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return;
	}

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return;
	}

	switch (ioType)
	{
		case Metrology::ConnectionIoType::Source:		m_pInputSignalIDEdit->setText(pSignal->param().appSignalID());	break;
		case Metrology::ConnectionIoType::Destination:	m_pOutputSignalIDEdit->setText(pSignal->param().appSignalID());	break;
		default:										assert(0);														break;
	}
}


// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionItemDialog::onOk()
{
	int type = m_metrologyConnection.type();
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

	//
	//

	QString inputAppSignalID = m_pInputSignalIDEdit->text().trimmed();
	if (inputAppSignalID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select input signal!"));
		m_pInputSignalButton->setFocus();
		return;
	}

	Metrology::Signal* pInSignal = theSignalBase.signalPtr(inputAppSignalID);
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not found.\nPlease, select input signal!").
								 arg(inputAppSignalID));
		m_pInputSignalButton->setFocus();
		return;
	}

	if (pInSignal->param().isAnalog() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not analog.\nPlease, select analog signal!").
								 arg(inputAppSignalID));
		m_pInputSignalButton->setFocus();
		return;
	}

	//
	//

	QString outputAppSignalID = m_pOutputSignalIDEdit->text().trimmed();
	if (outputAppSignalID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select output signal!"));
		m_pInputSignalButton->setFocus();
		return;
	}

	Metrology::Signal* pOutSignal = theSignalBase.signalPtr(outputAppSignalID);
	if (pOutSignal == nullptr || pOutSignal->param().isValid() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not found.\nPlease, select output signal!").
								 arg(outputAppSignalID));
		m_pOutputSignalButton->setFocus();
		return;
	}

	if (pOutSignal->param().isAnalog() == false)
	{
		QMessageBox::information(this,
								 windowTitle(),
								 tr("Signal %1 is not analog.\nPlease, select analog signal!").
								 arg(inputAppSignalID));
		m_pOutputSignalButton->setFocus();
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

			if (pInSignal->param().isInput() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not input signal!").
										 arg(inputAppSignalID));
				m_pInputSignalButton->setFocus();
				return;
			}

			if (pInSignal->param().electricRangeIsValid() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 has wrong electric limit!").
										 arg(inputAppSignalID));
				m_pInputSignalButton->setFocus();
				return;
			}

			break;

		case Metrology::ConnectionType::Tuning_Output:

			if (pInSignal->param().isInternal() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not internal signal!").
										 arg(inputAppSignalID));
				m_pInputSignalButton->setFocus();
				return;
			}

			if (pInSignal->param().enableTuning() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not tuning signal!").
										 arg(inputAppSignalID));
				m_pInputSignalButton->setFocus();
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

			if (pOutSignal->param().isInternal() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not internal signal!").
										 arg(outputAppSignalID));
				m_pOutputSignalButton->setFocus();
				return;
			}

			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Tuning_Output:

			if (pOutSignal->param().isOutput() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 is not output signal!").
										 arg(outputAppSignalID));
				m_pOutputSignalButton->setFocus();
				return;
			}

			if (pOutSignal->param().electricRangeIsValid() == false)
			{
				QMessageBox::information(this,
										 windowTitle(),
										 tr("Signal %1 has wrong electric limit!").
										 arg(outputAppSignalID));
				m_pInputSignalButton->setFocus();
				return;
			}

			break;
	}

	//
	//

	m_metrologyConnection.setSignal(Metrology::ConnectionIoType::Source, pInSignal);
	m_metrologyConnection.setSignal(Metrology::ConnectionIoType::Destination, pOutSignal);
	m_metrologyConnection.createCrc();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionDialog::MetrologyConnectionDialog(QWidget* parent) :
	QDialog(parent)
{
	m_connectionBase = theSignalBase.connections();

	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionDialog::MetrologyConnectionDialog(Metrology::Signal* pSignal, QWidget* parent) :
	QDialog(parent)
{
	m_connectionBase = theSignalBase.connections();

	createInterface();
	updateList();

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return;
	}

	m_pOutputSignal = pSignal;

	createConnectionBySignal(m_pOutputSignal);
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyConnectionDialog::~MetrologyConnectionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Connection.png"));
	setWindowTitle(tr("Metrology connections"));

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.36), static_cast<int>(screen.height() * 0.4));
	move(screen.center() - rect().center());

	m_pMenuBar = new QMenuBar(this);
	m_pConnectionMenu = new QMenu(tr("&Connection"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	m_pCreateAction = m_pConnectionMenu->addAction(tr("&Create ..."));
	m_pCreateAction->setIcon(QIcon(":/icons/Add.png"));
	m_pCreateAction->setShortcut(Qt::Key_Insert);

	m_pEditAction = m_pConnectionMenu->addAction(tr("&Edit ..."));
	m_pEditAction->setIcon(QIcon(":/icons/Edit.png"));

	m_pRemoveAction = m_pConnectionMenu->addAction(tr("&Remove"));
	m_pRemoveAction->setIcon(QIcon(":/icons/Remove.png"));
	m_pRemoveAction->setShortcut(Qt::Key_Delete);

	m_pConnectionMenu->addSeparator();

	m_pMoveUpAction = m_pConnectionMenu->addAction(tr("Move &Up"));
	m_pMoveUpAction->setShortcut(Qt::CTRL + Qt::Key_Up);

	m_pMoveDownAction = m_pConnectionMenu->addAction(tr("Move &Down"));
	m_pMoveDownAction->setShortcut(Qt::CTRL + Qt::Key_Down);

	m_pConnectionMenu->addSeparator();

	m_pExportAction = m_pConnectionMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pImportAction = m_pConnectionMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

	m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
	m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pMenuBar->addMenu(m_pConnectionMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pCreateAction, &QAction::triggered, this, &MetrologyConnectionDialog::createConnection);
	connect(m_pEditAction, &QAction::triggered, this, &MetrologyConnectionDialog::editConnection);
	connect(m_pRemoveAction, &QAction::triggered, this, &MetrologyConnectionDialog::removeConnection);
	connect(m_pMoveUpAction, &QAction::triggered, this, &MetrologyConnectionDialog::moveUpConnection);
	connect(m_pMoveDownAction, &QAction::triggered, this, &MetrologyConnectionDialog::moveDownConnection);
	connect(m_pExportAction, &QAction::triggered, this, &MetrologyConnectionDialog::exportConnections);
	connect(m_pImportAction, &QAction::triggered, this, &MetrologyConnectionDialog::importConnections);

	connect(m_pFindAction, &QAction::triggered, this, &MetrologyConnectionDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &MetrologyConnectionDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &MetrologyConnectionDialog::selectAll);


	m_pView = new QTableView(this);
	m_pView->setModel(&m_connectionTable);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < METROLOGY_CONNECTION_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, MetrologyConnectionColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &MetrologyConnectionDialog::onListDoubleClicked);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &MetrologyConnectionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &MetrologyConnectionDialog::reject);

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCreateAction);
	m_pContextMenu->addAction(m_pEditAction);
	m_pContextMenu->addAction(m_pRemoveAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pMoveUpAction);
	m_pContextMenu->addAction(m_pMoveDownAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &MetrologyConnectionDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::signalBaseLoaded()
{
	m_connectionBase = theSignalBase.connections();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::updateList()
{
	m_connectionTable.clear();

	QVector<Metrology::Connection> connectionList;

	int count = m_connectionBase.count();
	for(int i = 0; i < count; i++)
	{
		connectionList.append(m_connectionBase.connection(i));
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyConnectionDialog::createConnectionBySignal(Metrology::Signal* pSignal)
{
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return false;
	}

	Metrology::Connection connection;

	//
	//
	Metrology::ConnectionType type = Metrology::ConnectionType::Unknown;

	switch (pSignal->param().inOutType())
	{
		case E::SignalInOutType::Internal:	type = Metrology::ConnectionType::Input_Internal;	break;
		case E::SignalInOutType::Output:	type = Metrology::ConnectionType::Input_Output;		break;
	}

	if (type == Metrology::ConnectionType::Unknown)
	{
		return false;
	}

	//
	//
	connection.setType(type);
	connection.setSignal(Metrology::ConnectionIoType::Destination, pSignal);

	MetrologyConnectionItemDialog dialog(connection);
	if (dialog.exec() != QDialog::Accepted)
	{
		reject();
		return false;
	}

	connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return false;
	}

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return false;
	}

	m_connectionBase.append(connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::createConnection()
{
	MetrologyConnectionItemDialog dialog;
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	Metrology::Connection connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return;
	}

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.append(connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::editConnection()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for edit!"));
		return;
	}

	MetrologyConnectionItemDialog dialog(m_connectionTable.at(index));
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	Metrology::Connection connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return;
	}

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.setConnection(index, connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::removeConnection()
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
				m_connectionBase.remove(index);
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::moveUpConnection()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for move!"));
		return;
	}

	Metrology::Connection connection = m_connectionTable.at(index);
	if (connection.isValid() == false)
	{
		return;
	}

	int indexPrev = index - 1;
	if (indexPrev < 0 || indexPrev >= m_connectionTable.connectionCount())
	{
		return;
	}

	Metrology::Connection connectionPrev = m_connectionTable.at(indexPrev);
	if (connectionPrev.isValid() == false)
	{
		return;
	}

	if (connection.metrologySignal(Metrology::ConnectionIoType::Source) != connectionPrev.metrologySignal(Metrology::ConnectionIoType::Source))
	{
		return;
	}

	m_connectionBase.setConnection(index, connectionPrev);
	m_connectionBase.setConnection(indexPrev, connection);

	updateList();

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::moveDownConnection()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for move!"));
		return;
	}

	Metrology::Connection connection = m_connectionTable.at(index);
	if (connection.isValid() == false)
	{
		return;
	}

	int indexNext = index + 1;
	if (indexNext < 0 || indexNext >= m_connectionTable.connectionCount())
	{
		return;
	}

	Metrology::Connection connectionNext = m_connectionTable.at(indexNext);
	if (connectionNext.isValid() == false)
	{
		return;
	}

	if (connection.metrologySignal(Metrology::ConnectionIoType::Source) != connectionNext.metrologySignal(Metrology::ConnectionIoType::Source))
	{
		return;
	}

	m_connectionBase.setConnection(index, connectionNext);
	m_connectionBase.setConnection(indexNext, connection);

	updateList();

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::exportConnections()
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
		QMessageBox::critical(this, tr("Error"), tr("Failed to export!"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::importConnections()
{
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
		QMessageBox::critical(this, tr("Error"), tr("Could not open file: %1\nfile is not found!").arg(fileName));
		return;
	}

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
		return;
	}

	QVector<Metrology::Connection> connectionList;

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

		if (m_connectionBase.findConnectionIndex(connection) != -1)
		{
			continue;
		}

		m_connectionBase.append(connection);
	}

	file.close();

	// init metrology signals
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
			Metrology::Signal* pSignal = theSignalBase.signalPtr(pConnection->appSignalID(ioType));
			if (pSignal == nullptr)
			{
				continue;
			}

			pConnection->setSignal(ioType, pSignal);
		}
	}

	m_connectionBase.sort();

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyConnectionDialog::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
