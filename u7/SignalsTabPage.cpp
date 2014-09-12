#include "Stable.h"
#include "SignalsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"


const char* Columns[] =
{
	"StrID",
	"ExtStrID",
	"Name",
	"Channel",
	"DataFormat",
	"InputUnit",
	"OutputUnit",
	"LowADC",
	"HighADC",
	"Adjustment",
	"LowLimit",
	"HighLimit",
	"Unit",
	"Aperture",
	"DropLimit",
	"ExcessLimit",
	"UnbalanceLimit",
	"InputLowLimit",
	"InputHighLimit",
	"OutputLowLimit",
	"OutputHighLimit",
	"Precision",
	"Acquire",
	"Calculated",
	"NormalState",
	"DataSize"
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);


SignalsModel::SignalsModel(QObject *parent) :
	QAbstractTableModel(parent)
{
	emit signalsIdRequest();
}

SignalsModel::~SignalsModel()
{

}

int SignalsModel::rowCount(const QModelIndex &) const
{
	return m_signals.count() + 1;
}

int SignalsModel::columnCount(const QModelIndex &) const
{
	return COLUMNS_COUNT;
}

QVariant SignalsModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (row == m_signals.count())
	{
		return QVariant();
	}
	const Signal& signal = m_signals[row];
	if (role == Qt::DisplayRole)
	{
		switch (col)
		{
			case 0: return signal.strID();
			case 1: return signal.extStrID();
			case 2: return signal.name();
		}
	}

	return QVariant();
}

QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return QString(Columns[section]);
		}
		if (orientation == Qt::Vertical)
		{
			if (section < m_signals.count())
			{
				return m_signals[section].ID();
			}
			return tr("New record");
		}
	}
	return QVariant();
}

bool SignalsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		bool added = false;
		int row = index.row();
		if (row == m_signals.count())
		{
			beginInsertRows(QModelIndex(), row, row);
			Signal signal;
			m_signals.append(signal);
			endInsertRows();
			added = true;
		}

		Signal& signal = m_signals[index.row()];

		switch (index.column())
		{
			case 0: signal.setStrID(value.toString()); break;
			case 1: signal.setExtStrID(value.toString()); break;
			case 2: signal.setName(value.toString()); break;
		}

		if (added)
		{
			emit signalAdded(signal);
		}
		else
		{
			emit signalChanged(signal);
		}
	}

	qDebug() << "setData with role: " << role;

	emit dataChanged(index, index, QVector<int>() << role);

	return true;
}

Qt::ItemFlags SignalsModel::flags(const QModelIndex &index) const
{
	if (index.column() == 3)
	{
		return QAbstractTableModel::flags(index);
	}
	else
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
}

void SignalsModel::signalsIdReceived(QVector<int> signalsId)
{
	for (int i = 0; i < signalsId.count(); i++)
	{
		emit signalDataRequest(signalsId[i]);
	}
}

void SignalsModel::signalDataReceived(Signal signal)
{
	for (int i = 0; i < m_signals.count(); i++)
	{
		if (m_signals[i].ID() == signal.ID())
		{
			m_signals[i] = signal;
			return;
		}
	}
	m_signals.append(signal);
}

//
//
// SignalsTabPage
//
//
SignalsTabPage::SignalsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	/*m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_equipmentView->addAction(m_addSystemAction);
	m_equipmentView->addAction(m_addCaseAction);
	m_equipmentView->addAction(m_addSubblockAction);
	m_equipmentView->addAction(m_addBlockAction);*/

	// Property View
	//
	//m_propertyView = new QTextEdit();
	m_signalsModel = new SignalsModel(this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsModel);
	connect(m_signalsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_signalsView, SLOT(resizeColumnsToContents()));
	connect(m_signalsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_signalsView, SLOT(resizeRowsToContents()));
	m_signalsView->resizeColumnsToContents();
	m_signalsView->resizeRowsToContents();

	// Splitter
	//
	//m_splitter = new QSplitter();

	//m_splitter->addWidget(m_equipmentView);
	//m_splitter->addWidget(m_propertyView);

	//m_splitter->setStretchFactor(0, 2);
	//m_splitter->setStretchFactor(1, 1);

	//m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	//
	// Layouts
	//

	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_signalsView);

	//pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &SignalsTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	//theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
	//theSettings.writeUserScope();
}

void SignalsTabPage::CreateActions()
{
	/*m_addSystemAction = new QAction(tr("Add System"), this);
	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
	//m_addSystemAction->setEnabled(false);
	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);
	*/
	return;
}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	QSet<int> signalsIDs;

	m_signalSet.removeAll();

	dbController()->getSignals(&m_signalSet, this);

	return;
}

void SignalsTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}
