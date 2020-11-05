#include "SignalConnectionList.h"

#include "MainWindow.h"
#include "Options.h"
#include "CopyData.h"
#include "FindData.h"
#include "ExportData.h"
#include "SignalList.h"

#include <QFileDialog>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalConnectionTable::SignalConnectionTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionTable::~SignalConnectionTable()
{
	QMutexLocker l(&m_connectionMutex);

	m_connectionList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionTable::columnCount(const QModelIndex&) const
{
	return SIGNAL_CONNECTION_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionTable::rowCount(const QModelIndex&) const
{
	return connectionCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalConnectionTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SIGNAL_CONNECTION_COLUMN_COUNT)
		{
			result = qApp->translate("SignalConnectionDialog.h", SignalConnectionColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalConnectionTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > SIGNAL_CONNECTION_COLUMN_COUNT)
	{
		return QVariant();
	}

	SignalConnection connection = at(row);

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == SIGNAL_CONNECTION_COLUMN_TYPE)
		{
			if (connection.type() < 0 || connection.type() >= SIGNAL_CONNECTION_TYPE_COUNT)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == SIGNAL_CONNECTION_COLUMN_IN_ID)
		{
			Metrology::Signal* pSignal = connection.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (pSignal == nullptr || pSignal->param().isValid() == false)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		if (column == SIGNAL_CONNECTION_COLUMN_OUT_ID)
		{
			Metrology::Signal* pSignal = connection.signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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

QString SignalConnectionTable::text(int row, int column, const SignalConnection& connection) const
{
	if (row < 0 || row >= connectionCount())
	{
		return QString();
	}

	if (column < 0 || column > SIGNAL_CONNECTION_COLUMN_COUNT)
	{
		return QString();
	}

	bool visible = true;

	if (row > 0 &&
		m_connectionList[row - 1].handle().type == connection.handle().type &&
		m_connectionList[row - 1].handle().inputID == connection.handle().inputID &&
		m_connectionList[row - 1].handle().inputID != 0 )
	{
		visible = false;
	}

	QString result;

	switch (column)
	{
		case SIGNAL_CONNECTION_COLUMN_TYPE_NO:	result = visible ? QString::number(connection.type()) : QString("");				break;
		case SIGNAL_CONNECTION_COLUMN_TYPE:		result = visible ? connection.typeStr() : QString("");								break;
		case SIGNAL_CONNECTION_COLUMN_IN_ID:	result = visible ? connection.appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT): QString();	break;
		case SIGNAL_CONNECTION_COLUMN_OUT_ID:	result = connection.appSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT);						break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionTable::connectionCount() const
{
	QMutexLocker l(&m_connectionMutex);

	return m_connectionList.count();
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection SignalConnectionTable::at(int index) const
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return SignalConnection();
	}

	return m_connectionList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionTable::set(const QVector<SignalConnection>& list_add)
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

void SignalConnectionTable::clear()
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

SignalConnectionItemDialog::SignalConnectionItemDialog(QWidget *parent) :
	QDialog(parent)
{
	createInterface();
	updateSignals();
}


// -------------------------------------------------------------------------------------------------------------------

SignalConnectionItemDialog::SignalConnectionItemDialog(const SignalConnection& signalConnection, QWidget *parent) :
	QDialog(parent)
{
	m_signalConnection = signalConnection;

	createInterface();
	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionItemDialog::~SignalConnectionItemDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::createInterface()
{
	setWindowFlags(Qt::Dialog);
	setWindowIcon(QIcon(":/icons/Connection.png"));

	if (m_signalConnection.handle().uint64 == 0)
	{
		setWindowIcon(QIcon(":/icons/Add.png"));
		setWindowTitle(tr("Create connection"));
	}
	else
	{
		setWindowIcon(QIcon(":/icons/Edit.png"));
		setWindowTitle(tr("Edit connection"));
	}

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

	QLabel* pInputSignalLabel = new QLabel(tr("Input signal"), this);
	m_pInputSignalIDEdit = new QLineEdit(QString(), this);
	m_pInputSignalButton = new QPushButton(tr("Select ..."), this);

	pInputSignalLabel->setFixedWidth(70);
	m_pInputSignalIDEdit->setFixedWidth(200);

	//m_pInputSignalIDEdit->setReadOnly(true);
	m_pInputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	inputSignalLayout->addWidget(pInputSignalLabel);
	inputSignalLayout->addWidget(m_pInputSignalIDEdit);
	inputSignalLayout->addWidget(m_pInputSignalButton);

		// Output signal
		//
	QHBoxLayout *outputSignalLayout = new QHBoxLayout;

	QLabel* pOutputSignalLabel = new QLabel(tr("Output signal"), this);
	m_pOutputSignalIDEdit = new QLineEdit(QString(), this);
	m_pOutputSignalButton = new QPushButton(tr("Select ..."), this);

	pOutputSignalLabel->setFixedWidth(70);
	m_pOutputSignalIDEdit->setFixedWidth(200);

	//m_pOutputSignalIDEdit->setReadOnly(true);
	m_pOutputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	outputSignalLayout->addWidget(pOutputSignalLabel);
	outputSignalLayout->addWidget(m_pOutputSignalIDEdit);
	outputSignalLayout->addWidget(m_pOutputSignalButton);


	signalLayout->addLayout(inputSignalLayout);
	signalLayout->addLayout(outputSignalLayout);

	signalGroup->setLayout(signalLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// fill type list
	//
	for (int type = 0; type < SIGNAL_CONNECTION_TYPE_COUNT; type++)
	{
		m_pTypeList->addItem(qApp->translate("SignalConnectionBase.h", SignalConnectionType[type]), type);
	}
	m_pTypeList->removeItem(SIGNAL_CONNECTION_TYPE_UNUSED);

	int type = m_signalConnection.type() ;
	if ((type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT) || type == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		type = SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL;
		m_signalConnection.setType(type);
	}
	m_pTypeList->setCurrentIndex(type - 1);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalConnectionItemDialog::selectedType);
	connect(m_pInputSignalButton, &QPushButton::clicked, this, &SignalConnectionItemDialog::selectInputSignal);
	connect(m_pOutputSignalButton, &QPushButton::clicked, this, &SignalConnectionItemDialog::selectOutputSignal);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalConnectionItemDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalConnectionItemDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addLayout(typeLayout);
	mainLayout->addWidget(signalGroup);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::updateSignals()
{
	int type = m_signalConnection.type();
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	m_pInputSignalIDEdit->setText(m_signalConnection.appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT));
	m_pOutputSignalIDEdit->setText(m_signalConnection.appSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT));
}
// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::selectedType(int)
{
	int type = m_pTypeList->currentData().toInt() ;
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	m_signalConnection.setType(type);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::selectInputSignal()
{
	selectSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::selectOutputSignal()
{
	selectSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
}


// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::selectSignal(int type)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

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

	switch (type)
	{
		case MEASURE_IO_SIGNAL_TYPE_INPUT:	m_pInputSignalIDEdit->setText(pSignal->param().appSignalID());	break;
		case MEASURE_IO_SIGNAL_TYPE_OUTPUT: m_pOutputSignalIDEdit->setText(pSignal->param().appSignalID());	break;
		default:							assert(0);														break;
	}
}


// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::onOk()
{
	int type = m_signalConnection.type();
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select connection type!"));
		m_pTypeList->setFocus();
		return;
	}

	if (m_pInputSignalIDEdit == nullptr || m_pOutputSignalIDEdit == nullptr)
	{
		return;
	}

	QString inputAppSignalID = m_pInputSignalIDEdit->text();
	if (inputAppSignalID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select input signal!"));
		m_pInputSignalButton->setFocus();
		return;
	}

	Metrology::Signal* pInSignal = theSignalBase.signalPtr(inputAppSignalID);
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Signal %1 is not found.\nPlease, select input signal!").arg(inputAppSignalID));
		m_pInputSignalButton->setFocus();
		return;
	}

	QString outputAppSignalID = m_pOutputSignalIDEdit->text();
	if (outputAppSignalID.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select output signal!"));
		m_pInputSignalButton->setFocus();
		return;
	}

	Metrology::Signal* pOutSignal = theSignalBase.signalPtr(outputAppSignalID);
	if (pOutSignal == nullptr || pOutSignal->param().isValid() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Signal %1 is not found.\nPlease, select output signal!").arg(outputAppSignalID));
		m_pOutputSignalButton->setFocus();
		return;
	}

	m_signalConnection.setSignal(MEASURE_IO_SIGNAL_TYPE_INPUT, pInSignal);
	m_signalConnection.setSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT, pOutSignal);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalConnectionDialog::SignalConnectionDialog(QWidget *parent) :
	QDialog(parent)
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->configSocket() != nullptr)
	{
		connect(pMainWindow->configSocket(), &ConfigSocket::configurationLoaded, this, &SignalConnectionDialog::configurationLoaded, Qt::QueuedConnection);
	}

	m_connectionBase = theSignalBase.signalConnections();

	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionDialog::~SignalConnectionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Connection.png"));
	setWindowTitle(tr("Signal connections"));
	resize(QGuiApplication::primaryScreen()->availableGeometry().width() - 700, 500);
	move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());

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

	m_pImportAction = m_pConnectionMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

	m_pExportAction = m_pConnectionMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

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

	connect(m_pCreateAction, &QAction::triggered, this, &SignalConnectionDialog::createConnection);
	connect(m_pEditAction, &QAction::triggered, this, &SignalConnectionDialog::editConnection);
	connect(m_pRemoveAction, &QAction::triggered, this, &SignalConnectionDialog::removeConnection);
	connect(m_pImportAction, &QAction::triggered, this, &SignalConnectionDialog::importConnections);
	connect(m_pExportAction, &QAction::triggered, this, &SignalConnectionDialog::exportConnections);

	connect(m_pFindAction, &QAction::triggered, this, &SignalConnectionDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &SignalConnectionDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &SignalConnectionDialog::selectAll);


	m_pView = new QTableView(this);
	m_pView->setModel(&m_connectionTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < SIGNAL_CONNECTION_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalConnectionColumnWidth[column]);
	}

	m_pView->hideColumn(SIGNAL_CONNECTION_COLUMN_TYPE_NO);

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &SignalConnectionDialog::onListDoubleClicked);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalConnectionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalConnectionDialog::reject);

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCreateAction);
	m_pContextMenu->addAction(m_pEditAction);
	m_pContextMenu->addAction(m_pRemoveAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalConnectionDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::configurationLoaded()
{
	m_connectionBase.initSignals();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::updateList()
{
	m_connectionTable.clear();

	QVector<SignalConnection> connectionList;

	int count = m_connectionBase.count();
	for(int i = 0; i < count; i++)
	{
		connectionList.append(m_connectionBase.connection(i));
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::createConnection()
{
	SignalConnectionItemDialog dialog;
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	SignalConnection connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return;
	}

	int foundIndex = m_connectionBase.findIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, SIGNAL_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.append(connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, SIGNAL_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::editConnection()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for edit!"));
		return;
	}

	SignalConnection connection = m_connectionTable.at(index);

	SignalConnectionItemDialog dialog(connection);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return;
	}

	int foundIndex = m_connectionBase.findIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, SIGNAL_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.setConnection(index, connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_connectionTable.index(foundIndex, SIGNAL_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::removeConnection()
{
	int selectedConnectionCount = m_pView->selectionModel()->selectedRows().count();
	if (selectedConnectionCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select connection for remove!"));
		return;
	}

	if (QMessageBox::question(this, windowTitle(), tr("Do you want delete %1 connection(s)?").arg(selectedConnectionCount)) == QMessageBox::No)
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

void SignalConnectionDialog::importConnections()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "SignalConnections.csv", "CSV files (*.csv);;All files (*.*)");
	if (fileName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file: Empty file name"));
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

	QVector<SignalConnection> connectionList;

	// read data
	//
	QTextStream in(&file);
	while (in.atEnd() == false)
	{
		if (in.pos() == 0) // first line is header
		{
			QString header = in.readLine();
			qDebug() << header;
			continue;
		}
		else
		{
			SignalConnection connection;

			QStringList line = in.readLine().split(";");
			for(int column = 0; column < line.count(); column++)
			{
				switch (column)
				{
					case SIGNAL_CONNECTION_COLUMN_TYPE_NO:	connection.setType(line[column].toInt());								break;
					case SIGNAL_CONNECTION_COLUMN_IN_ID:	connection.setAppSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT, line[column]);	break;
					case SIGNAL_CONNECTION_COLUMN_OUT_ID:	connection.setAppSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT, line[column]);	break;
				}
			}

			connection.initSignals();

			if (connection.isValid() == false)
			{
				continue;
			}

			m_connectionBase.append(connection);
		}
	}

	file.close();

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::exportConnections()
{
	ExportData* dialog = new ExportData(m_pView, true, "SignalConnections");
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::onOk()
{
	theSignalBase.signalConnections() = m_connectionBase;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
