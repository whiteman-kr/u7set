#include "SignalConnectionList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "SignalList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalConnectionTable::m_showCustomID = true;

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionTable::SignalConnectionTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionTable::~SignalConnectionTable()
{
	m_connectionMutex.lock();

		m_connectionList.clear();

	m_connectionMutex.unlock();
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
			result = SignalConnectionColumn[section];
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
	if (connection.isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::TextColorRole)
	{
		if ((column == SIGNAL_CONNECTION_COLUMN_IN_RACK || column == SIGNAL_CONNECTION_COLUMN_IN_CAPTION))
		{
			Metrology::Signal* pSignal = connection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (pSignal == nullptr || pSignal->param().isValid() == false)
			{
				return QColor(Qt::red);
			}
		}

		if ((column == SIGNAL_CONNECTION_COLUMN_OUT_RACK || column == SIGNAL_CONNECTION_COLUMN_OUT_CAPTION))
		{
			Metrology::Signal* pSignal = connection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
			if (pSignal == nullptr || pSignal->param().isValid() == false)
			{
				return QColor(Qt::red);
			}
		}

		return QVariant();
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (column == SIGNAL_CONNECTION_COLUMN_SEPARATOR1 || column == SIGNAL_CONNECTION_COLUMN_SEPARATOR2 || column == SIGNAL_CONNECTION_COLUMN_SEPARATOR3)
		{
			return QColor(Qt::lightGray);
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

QString SignalConnectionTable::text(int row, int column, const SignalConnection& signal) const
{
	if (row < 0 || row >= connectionCount())
	{
		return QString();
	}

	if (column < 0 || column > SIGNAL_CONNECTION_COLUMN_COUNT)
	{
		return QString();
	}

	if (signal.isValid() == false)
	{
		return QString();
	}

	Metrology::SignalParam inParam;
	Metrology::SignalParam outParam;

	Metrology::Signal* pInSignal = signal.metrologySignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	Metrology::Signal* pOutSignal = signal.metrologySignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

	if (pInSignal != nullptr)
	{
		inParam = pInSignal->param();
	}

	if (pOutSignal != nullptr)
	{
		outParam = pOutSignal->param();
	}

	QString result;

	switch (column)
	{
		case SIGNAL_CONNECTION_COLUMN_TYPE:			result = signal.typeStr();																						break;
		case SIGNAL_CONNECTION_COLUMN_SEPARATOR1:	result.clear();																									break;
		case SIGNAL_CONNECTION_COLUMN_IN_RACK:		inParam.isValid() == false ?	result = QString("???")	 : result = inParam.location().rack().caption();		break;
		case SIGNAL_CONNECTION_COLUMN_IN_ID:		inParam.isValid() == false ?	result = signal.appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT) : result = m_showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID();		break;
		case SIGNAL_CONNECTION_COLUMN_IN_CAPTION:	inParam.isValid() == false ?	result = QString("???")	 : result = inParam.caption();							break;
		case SIGNAL_CONNECTION_COLUMN_SEPARATOR2:	result.clear();																									break;
		case SIGNAL_CONNECTION_COLUMN_OUT_RACK:		outParam.isValid() == false ?	result = QString("???")	 : result = outParam.location().rack().caption();		break;
		case SIGNAL_CONNECTION_COLUMN_OUT_ID:		outParam.isValid() == false ?	result = signal.appSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT) : result = m_showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID();	break;
		case SIGNAL_CONNECTION_COLUMN_OUT_CAPTION:	outParam.isValid() == false ?	result = QString("???")	 : result = outParam.caption();							break;
		case SIGNAL_CONNECTION_COLUMN_SEPARATOR3:	result.clear();																									break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionTable::connectionCount() const
{
	int count = 0;

	m_connectionMutex.lock();

		count = m_connectionList.count();

	m_connectionMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection SignalConnectionTable::at(int index) const
{
	if (index < 0 || index >= connectionCount())
	{
		return SignalConnection();
	}

	SignalConnection signal;

	m_connectionMutex.lock();

		signal = m_connectionList[index];

	m_connectionMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionTable::set(const QList<SignalConnection> list_add)
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

bool SignalConnectionItemDialog::m_showCustomID = true;

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
	setWindowIcon(QIcon(":/icons/InOut.png"));

	if (m_signalConnection.isValid() == false)
	{
		setWindowIcon(QIcon(":/icons/Add.png"));
		setWindowTitle(tr("Append connection"));
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

	// Show Custom ID
	//
	m_pShowCustomIDCheck = new QCheckBox(tr("Show Custom ID"), this);
	m_pShowCustomIDCheck->setChecked(m_showCustomID);
	m_pShowCustomIDCheck->setLayoutDirection(Qt::RightToLeft);

	typeLayout->addWidget(new QLabel(tr("Connection type"), this));
	typeLayout->addWidget(m_pTypeList);
	typeLayout->addStretch();
	typeLayout->addWidget(m_pShowCustomIDCheck);

	// Signals
	//
	QGroupBox* signalGroup = new QGroupBox(QString());
	QVBoxLayout *signalLayout = new QVBoxLayout;

		// Input signal
		//
	QHBoxLayout *inputSignalLayout = new QHBoxLayout;

	QLabel* pInputSignalLabel = new QLabel(tr("Input signal"), this);
	m_pInputSignalIDEdit = new QLineEdit(QString(), this);
	m_pInputSignalCaptionEdit = new QLineEdit(QString(), this);
	m_pInputSignalButton = new QPushButton(tr("Select ..."), this);

	pInputSignalLabel->setFixedWidth(70);
	m_pInputSignalIDEdit->setFixedWidth(200);
	m_pInputSignalCaptionEdit->setFixedWidth(200);

	m_pInputSignalIDEdit->setReadOnly(true);
	m_pInputSignalCaptionEdit->setReadOnly(true);
	m_pInputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	inputSignalLayout->addWidget(pInputSignalLabel);
	inputSignalLayout->addWidget(m_pInputSignalIDEdit);
	inputSignalLayout->addWidget(new QLabel(tr(" - "), this));
	inputSignalLayout->addWidget(m_pInputSignalCaptionEdit);
	inputSignalLayout->addWidget(m_pInputSignalButton);

		// Output signal
		//
	QHBoxLayout *outputSignalLayout = new QHBoxLayout;

	QLabel* pOutputSignalLabel = new QLabel(tr("Output signal"), this);
	m_pOutputSignalIDEdit = new QLineEdit(QString(), this);
	m_pOutputSignalCaptionEdit = new QLineEdit(QString(), this);
	m_pOutputSignalButton = new QPushButton(tr("Select ..."), this);

	pOutputSignalLabel->setFixedWidth(70);
	m_pOutputSignalIDEdit->setFixedWidth(200);
	m_pOutputSignalCaptionEdit->setFixedWidth(200);

	m_pOutputSignalIDEdit->setReadOnly(true);
	m_pOutputSignalCaptionEdit->setReadOnly(true);
	m_pOutputSignalButton->setEnabled(theSignalBase.signalCount() != 0);

	outputSignalLayout->addWidget(pOutputSignalLabel);
	outputSignalLayout->addWidget(m_pOutputSignalIDEdit);
	outputSignalLayout->addWidget(new QLabel(tr(" - "), this));
	outputSignalLayout->addWidget(m_pOutputSignalCaptionEdit);
	outputSignalLayout->addWidget(m_pOutputSignalButton);


	signalLayout->addLayout(inputSignalLayout);
	signalLayout->addLayout(outputSignalLayout);

	signalGroup->setLayout(signalLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// fill type list
	//
	for (int type = 0; type < SIGNAL_CONNECTION_TYPE_COUNT; type++)
	{
		m_pTypeList->addItem(SignalConnectionType[type], type);
	}
	m_pTypeList->removeItem(SIGNAL_CONNECTION_TYPE_UNUSED);

	int type = m_signalConnection.type() ;
	if ((type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT) || type == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		type = SIGNAL_CONNECTION_TYPE_FROM_INPUT;
		m_signalConnection.setType(type);
	}
	m_pTypeList->setCurrentIndex(type - 1);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalConnectionItemDialog::selectedType);
	connect(m_pInputSignalButton, &QPushButton::clicked, this, &SignalConnectionItemDialog::selectInputSignal);
	connect(m_pOutputSignalButton, &QPushButton::clicked, this, &SignalConnectionItemDialog::selectOutputSignal);
	connect(m_pShowCustomIDCheck, &QPushButton::clicked, this, &SignalConnectionItemDialog::showCustomID);
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

	Metrology::Signal* pInSignal = m_signalConnection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (pInSignal != nullptr && pInSignal->param().isValid() == true)
	{
		m_pInputSignalIDEdit->setText(m_showCustomID == true ? pInSignal->param().customAppSignalID() : pInSignal->param().appSignalID());
		m_pInputSignalCaptionEdit->setText(pInSignal->param().caption());
	}
	else
	{
		m_pInputSignalIDEdit->setText(m_signalConnection.appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT));
		m_pInputSignalCaptionEdit->setText(QString());
	}

	Metrology::Signal* pOutSignal = m_signalConnection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
	if (pOutSignal != nullptr &&  pOutSignal->param().isValid() == true)
	{
		m_pOutputSignalIDEdit->setText(m_showCustomID == true ? pOutSignal->param().customAppSignalID() : pOutSignal->param().appSignalID());
		m_pOutputSignalCaptionEdit->setText(pOutSignal->param().caption());
	}
	else
	{
		m_pOutputSignalIDEdit->setText(m_signalConnection.appSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT));
		m_pOutputSignalCaptionEdit->setText(QString());
	}


	m_pInputSignalIDEdit->setSelection(0,0);
	m_pInputSignalCaptionEdit->setSelection(0,0);
	m_pOutputSignalIDEdit->setSelection(0,0);
	m_pOutputSignalCaptionEdit->setSelection(0,0);
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
	if (theSignalBase.signalCount() == 0)
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

	m_signalConnection.setMetrologySignal(MEASURE_IO_SIGNAL_TYPE_INPUT, pSignal);

	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::selectOutputSignal()
{
	if (theSignalBase.signalCount() == 0)
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

	m_signalConnection.setMetrologySignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT, pSignal);

	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::showCustomID()
{
	m_showCustomID = !m_showCustomID;

	updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionItemDialog::onOk()
{
	int type = m_signalConnection.type();
	if ((type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT) || type == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select output signal type!"));
		m_pTypeList->setFocus();
		return;
	}

	Metrology::Signal* pInSignal = m_signalConnection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select input signal!"));
		m_pInputSignalButton->setFocus();
		return;
	}

	Metrology::Signal* pOutSignal = m_signalConnection.metrologySignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
	if (pOutSignal == nullptr || pOutSignal->param().isValid() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select output signal!"));
		m_pOutputSignalButton->setFocus();
		return;
	}

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
		connect(pMainWindow->configSocket(), &ConfigSocket::configurationLoaded, this, &SignalConnectionDialog::updateList, Qt::QueuedConnection);
	}

	m_сonnectionBase = theSignalBase.signalConnections();

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
	setWindowIcon(QIcon(":/icons/InOut.png"));
	setWindowTitle(tr("Signal connections"));
	resize(QApplication::desktop()->availableGeometry().width() - 700, 500);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	m_pMenuBar = new QMenuBar(this);
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);

	m_pAddAction = m_pSignalMenu->addAction(tr("&Add ..."));
	m_pAddAction->setIcon(QIcon(":/icons/Add.png"));
	m_pAddAction->setShortcut(Qt::Key_Insert);

	m_pEditAction = m_pSignalMenu->addAction(tr("&Edit ..."));
	m_pEditAction->setIcon(QIcon(":/icons/Edit.png"));

	m_pRemoveAction = m_pSignalMenu->addAction(tr("&Remove"));
	m_pRemoveAction->setIcon(QIcon(":/icons/Remove.png"));
	m_pRemoveAction->setShortcut(Qt::Key_Delete);

	m_pSignalMenu->addSeparator();

	m_pImportAction = m_pSignalMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

	m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
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

	m_pShowCustomIDAction = m_pViewMenu->addAction(tr("Show Custom ID"));
	m_pShowCustomIDAction->setCheckable(true);
	m_pShowCustomIDAction->setChecked(m_сonnectionTable.showCustomID());
	m_pShowCustomIDAction->setShortcut(Qt::CTRL + Qt::Key_Tab);

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pAddAction, &QAction::triggered, this, &SignalConnectionDialog::addConnection);
	connect(m_pEditAction, &QAction::triggered, this, &SignalConnectionDialog::editConnection);
	connect(m_pRemoveAction, &QAction::triggered, this, &SignalConnectionDialog::removeConnection);
	connect(m_pImportAction, &QAction::triggered, this, &SignalConnectionDialog::importConnections);
	connect(m_pExportAction, &QAction::triggered, this, &SignalConnectionDialog::exportConnections);

	connect(m_pFindAction, &QAction::triggered, this, &SignalConnectionDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &SignalConnectionDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &SignalConnectionDialog::selectAll);

	connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalConnectionDialog::showCustomID);


	m_pView = new QTableView(this);
	m_pView->setModel(&m_сonnectionTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < SIGNAL_CONNECTION_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalConnectionColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

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

	m_pContextMenu->addAction(m_pAddAction);
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

void SignalConnectionDialog::updateList()
{
	m_сonnectionTable.clear();

	m_сonnectionBase.sort();

	QList<SignalConnection> connectionList;

	int count = m_сonnectionBase.count();
	for(int i = 0; i < count; i++)
	{
		SignalConnection signalConnection = m_сonnectionBase.connection(i);
		if (signalConnection.isValid() == false)
		{
			continue;
		}

		connectionList.append(signalConnection);
	}

	m_сonnectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::addConnection()
{
	SignalConnectionItemDialog dialog;
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	SignalConnection connection = dialog.сonnection();
	if (connection.isValid() == false)
	{
		return;
	}

	int foundIndex = m_сonnectionBase.findIndex(connection);
	if (foundIndex != -1)
	{
		m_pView->setCurrentIndex(m_сonnectionTable.index(foundIndex, SIGNAL_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Signal already exist!"));
		return;
	}

	m_сonnectionBase.append(connection);

	updateList();

	m_pView->setCurrentIndex(m_сonnectionTable.index(m_сonnectionBase.count() - 1, SIGNAL_CONNECTION_COLUMN_TYPE));
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::editConnection()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_сonnectionTable.connectionCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select signal for edit!"));
		return;
	}

	SignalConnection connection = m_сonnectionTable.at(index);

	SignalConnectionItemDialog dialog(connection);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	connection = dialog.сonnection();
	if (connection.isValid() == false)
	{
		return;
	}

	m_сonnectionBase.setSignal(index, connection);

	updateList();

	m_pView->setCurrentIndex(m_сonnectionTable.index(index, SIGNAL_CONNECTION_COLUMN_TYPE));
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

	int count = m_сonnectionTable.connectionCount();
	for(int index = count - 1; index >= 0; index --)
	{
		if (m_pView->selectionModel()->isRowSelected(index, QModelIndex()) == true)
		{
			if (index >= 0 && index < m_сonnectionTable.connectionCount())
			{
				m_сonnectionBase.remove(index);
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::importConnections()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::exportConnections()
{
	ExportData* dialog = new ExportData(m_pView, tr("SignalConnections"));
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
	QString textClipboard;

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::showCustomID()
{
	m_сonnectionTable.setShowCustomID(m_pShowCustomIDAction->isChecked());

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionDialog::onOk()
{
	theSignalBase.signalConnections() = m_сonnectionBase;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
