#include "DialogMetrologyConnection.h"

#include <QFileDialog>
#include <QMessageBox>

#include "DialogSignalList.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


QVariant MetrologyConnectionTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
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
			if (ERR_METROLOGY_CONNECTION_TYPE(connection.type()) == true)
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
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnectionItem::DialogMetrologyConnectionItem(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	updateSignals();
}


// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnectionItem::DialogMetrologyConnectionItem(const Metrology::Connection& metrologyConnection, QWidget* parent) :
	QDialog(parent)
{
	m_metrologyConnection = metrologyConnection;

	createInterface();
	updateSignals();
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
	resize(static_cast<int>(screen.width() * 0.2), static_cast<int>(screen.height() * 0.04));
	move(screen.center() - rect().center());

	if (m_metrologyConnection == Metrology::Connection())
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
	m_pTypeList->removeItem(Metrology::ConnectionType::Unused);

	int type = m_metrologyConnection.type() ;
	if (ERR_METROLOGY_CONNECTION_TYPE(type) == true || type == Metrology::ConnectionType::Unused)
	{
		m_metrologyConnection.setType(Metrology::ConnectionType::Input_Internal);
		type = m_metrologyConnection.type();
	}
	m_pTypeList->setCurrentIndex(type - 1);

	// connects
	//
	connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogMetrologyConnectionItem::selectedType);
	connect(m_pInputSignalButton, &QPushButton::clicked, this, &DialogMetrologyConnectionItem::selectInputSignal);
	connect(m_pOutputSignalButton, &QPushButton::clicked, this, &DialogMetrologyConnectionItem::selectOutputSignal);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogMetrologyConnectionItem::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogMetrologyConnectionItem::reject);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addLayout(typeLayout);
	mainLayout->addWidget(signalGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::updateSignals()
{
	if (ERR_METROLOGY_CONNECTION_TYPE(m_metrologyConnection.type()) == true)
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

void DialogMetrologyConnectionItem::selectedType(int)
{
	int type = m_pTypeList->currentData().toInt();

	if (ERR_METROLOGY_CONNECTION_TYPE(type) == true)
	{
		return;
	}

	m_metrologyConnection.setType(type);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::selectInputSignal()
{
	selectSignal(Metrology::ConnectionIoType::Source);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::selectOutputSignal()
{
	selectSignal(Metrology::ConnectionIoType::Destination);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnectionItem::selectSignal(int ioType)
{
	DialogSignalList dialog(true, this);
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

void DialogMetrologyConnectionItem::onOk()
{
	int type = m_metrologyConnection.type();
	if (ERR_METROLOGY_CONNECTION_TYPE(type) == true)
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

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnection::DialogMetrologyConnection(QWidget* parent) :
	DialogList(0.36, 0.4, true, parent)
{
	m_connectionBase = theSignalBase.connections();

	createInterface();
	DialogMetrologyConnection::updateList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnection::DialogMetrologyConnection(Metrology::Signal* pSignal, QWidget* parent) :
	DialogList(0.36, 0.4, true, parent)
{
	m_connectionBase = theSignalBase.connections();

	createInterface();
	DialogMetrologyConnection::updateList();

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return;
	}

	m_pOutputSignal = pSignal;

	createConnectionBySignal(m_pOutputSignal);
}

// -------------------------------------------------------------------------------------------------------------------

DialogMetrologyConnection::~DialogMetrologyConnection()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::createInterface()
{
	setWindowTitle(tr("Metrology connections"));

	// menu
	//
	m_pConnectionMenu = new QMenu(tr("&Connection"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	// action
	//
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

	m_pConnectionMenu->addAction(m_pExportAction);

	m_pImportAction = m_pConnectionMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);

	//
	//
	addMenu(m_pConnectionMenu);
	addMenu(m_pEditMenu);

	//
	//
	connect(m_pCreateAction, &QAction::triggered, this, &DialogMetrologyConnection::OnNew);
	connect(m_pEditAction, &QAction::triggered, this, &DialogMetrologyConnection::onEdit);
	connect(m_pRemoveAction, &QAction::triggered, this, &DialogMetrologyConnection::onRremove);
	connect(m_pMoveUpAction, &QAction::triggered, this, &DialogMetrologyConnection::onMoveUp);
	connect(m_pMoveDownAction, &QAction::triggered, this, &DialogMetrologyConnection::onMoveDown);
	connect(m_pImportAction, &QAction::triggered, this, &DialogMetrologyConnection::onImport);

	//
	//
	m_connectionTable.setColumnCaption(DialogMetrologyConnection::metaObject()->className(), METROLOGY_CONNECTION_COLUMN_COUNT, MetrologyConnectionColumn);
	setModel(&m_connectionTable);

	//
	//
	DialogList::createHeaderContexMenu(METROLOGY_CONNECTION_COLUMN_COUNT, MetrologyConnectionColumn, MetrologyConnectionColumnWidth);
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::createContextMenu()
{
	addContextAction(m_pCreateAction);
	addContextAction(m_pEditAction);
	addContextAction(m_pRemoveAction);
	addContextSeparator();
	addContextAction(m_pMoveUpAction);
	addContextAction(m_pMoveDownAction);
	addContextSeparator();
	addContextAction(m_pCopyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::signalBaseLoaded()
{
	m_connectionBase = theSignalBase.connections();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::updateList()
{
	m_connectionTable.clear();

	std::vector<Metrology::Connection> connectionList;

	int count = m_connectionBase.count();
	for(int i = 0; i < count; i++)
	{
		connectionList.push_back(m_connectionBase.connection(i));
	}

	m_connectionTable.set(connectionList);
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogMetrologyConnection::createConnectionBySignal(Metrology::Signal* pSignal)
{
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return false;
	}

	Metrology::Connection connection;

	//
	//
	Metrology::ConnectionType type = Metrology::ConnectionType::NoConnectionType;

	switch (pSignal->param().inOutType())
	{
		case E::SignalInOutType::Internal:	type = Metrology::ConnectionType::Input_Internal;	break;
		case E::SignalInOutType::Output:	type = Metrology::ConnectionType::Input_Output;		break;
	}

	if (type == Metrology::ConnectionType::NoConnectionType)
	{
		return false;
	}

	//
	//
	connection.setType(type);
	connection.setSignal(Metrology::ConnectionIoType::Destination, pSignal);

	DialogMetrologyConnectionItem dialog(connection, this);
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

	QTableView* pView = view();
	if (pView == nullptr)
	{
		return false;
	}

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return false;
	}

	m_connectionBase.append(connection);
	m_connectionBase.sort();

	DialogMetrologyConnection::updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::OnNew()
{
	DialogMetrologyConnectionItem dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	Metrology::Connection connection = dialog.connection();
	if (connection.isValid() == false)
	{
		return;
	}

	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.append(connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onEdit()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.count())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select сonnection for edit!"));
		return;
	}

	DialogMetrologyConnectionItem dialog(m_connectionTable.at(index), this);
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
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));

		QMessageBox::information(this, windowTitle(), tr("Connection already exist!"));
		return;
	}

	m_connectionBase.setConnection(index, connection);
	m_connectionBase.sort();

	updateList();

	foundIndex = m_connectionBase.findConnectionIndex(connection);
	if (foundIndex != -1)
	{
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onRremove()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int selectedConnectionCount = pView->selectionModel()->selectedRows().count();
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

	int count = m_connectionTable.count();
	for(int index = count - 1; index >= 0; index --)
	{
		if (pView->selectionModel()->isRowSelected(index, QModelIndex()) == true)
		{
			if (index >= 0 && index < m_connectionTable.count())
			{
				m_connectionBase.remove(index);
			}
		}
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onMoveUp()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.count())
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
	if (indexPrev < 0 || indexPrev >= m_connectionTable.count())
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
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onMoveDown()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
	if (index < 0 || index >= m_connectionTable.count())
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
	if (indexNext < 0 || indexNext >= m_connectionTable.count())
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
		pView->setCurrentIndex(m_connectionTable.index(foundIndex, METROLOGY_CONNECTION_COLUMN_TYPE));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onExport()
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

	bool result = m_connectionBase.exportConnectionsToFile(fileName);
	if (result == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to export!"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMetrologyConnection::onImport()
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
				case 0:	connection.setType(line[column].toInt());											break;
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

void DialogMetrologyConnection::onProperties()
{
	onEdit();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
