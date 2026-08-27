#include <QTableView>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include "SharedMemoryView.h"
#include <RvduSimulatorLib.h>

SharedMemoryModel::SharedMemoryModel(int dataSize, MemoryType memoryType):
	m_dataSize(dataSize),
	m_memoryType(memoryType)
{
	m_tableHeaders << tr("Offset") << tr("Value") << tr("Hex");
	m_data = new unsigned char[m_dataSize];
	memset(m_data, 0, sizeof(m_dataSize));
}

SharedMemoryModel::~SharedMemoryModel() 
{ 
	delete[] m_data; 
}

void SharedMemoryModel::updateData()
{
	if (m_memoryType == MemoryType::Read)
	{
		rnLibReadMemoryLock();
		rnLibReadReadMemory(0, m_dataSize, m_data);
		rnLibReadMemoryUnlock();
	}

	if (m_memoryType == MemoryType::Write)
	{
		rnLibWriteMemoryLock();
		rnLibReadWriteMemory(0, m_dataSize, m_data);
		rnLibWriteMemoryUnlock();
	}
}

int SharedMemoryModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_dataSize / 2);
}

int SharedMemoryModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m_tableHeaders.size();
}

// Returns cell content based on the view request role
QVariant SharedMemoryModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	unsigned int offset = index.row();

	switch (index.column())
	{
	case 0:
		{
			return QString("%1").arg(offset, 5, 10, QChar('0'));
		}
	case 1:
		{
			uint16_t* wData = (uint16_t*)m_data;
			if (offset >= m_dataSize / 2) 
			{
				Q_ASSERT(false);
				return QVariant();
			}
			return QString::number(wData[offset]);
		}
	case 2:
		{
			uint16_t* wData = (uint16_t*)m_data;
			if (offset >= m_dataSize / 2)
			{
				Q_ASSERT(false);
				return QVariant();
			}
			return QString::number(wData[offset], 16).rightJustified(4, '0');
		}
	}

	return QVariant();
}

// Returns column header names
QVariant SharedMemoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section < m_tableHeaders.size())
			return m_tableHeaders.at(section);
	}
	return QVariant();
}

// SharedMemoryView

SharedMemoryView* SharedMemoryView::create(QWidget* parent)
{
	if (m_instance == nullptr)
	{
		m_instance = new SharedMemoryView(parent);
		m_instance->setAttribute(Qt::WA_DeleteOnClose);
		m_instance->setWindowFlag(Qt::Window);
		m_instance->show();
	}
	return m_instance;
}

SharedMemoryView::SharedMemoryView(QWidget* parent) :
	QWidget(parent),
	m_readModel(TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE, MemoryType::Read),
	m_writeModel(TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE, MemoryType::Write)
{
	setWindowTitle(tr("Memory View"));
	setMinimumSize(400, 720);

	setupUi();

	QPoint p = QSettings().value("SharedMemoryView/pos", QPoint(-1, -1)).toPoint();
	if (p.x() != -1 && p.y() != -1)
	{
		move(p);
	}
	QByteArray ba = QSettings().value("SharedMemoryView/geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &SharedMemoryView::onTimer);
	timer->start(250);

	m_readModel.updateData();
	m_writeModel.updateData();
}

SharedMemoryView::~SharedMemoryView()
{
	QSettings().setValue("SharedMemoryView/pos", pos());
	QSettings().setValue("SharedMemoryView/geometry", saveGeometry());

	m_instance = nullptr;
}

void SharedMemoryView::setupUi()
{
	m_readTableView = new QTableView(this);
	m_readTableView->setModel(&m_readModel);
	m_readTableView->verticalHeader()->setVisible(false);
	m_readTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_readTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_readTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	m_writeTableView = new QTableView(this);
	m_writeTableView->setModel(&m_writeModel);
	m_writeTableView->verticalHeader()->setVisible(false);
	m_writeTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_writeTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_writeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_readTableView);
	layout->addWidget(m_writeTableView);
}

void SharedMemoryView::onTimer()
{ 
	m_readModel.updateData();
	m_writeModel.updateData();

	auto updateTable = [](QTableView* view, QAbstractItemModel* model, int rowCount, int columnFrom, int columnTo)
	{
		int from = view->rowAt(0);

		int to = view->rowAt(view->height() - view->horizontalHeader()->height());

		if (from == -1)
		{
			from = 0;
		}

		if (to == -1)
		{
			to = rowCount - 1;
		}

		// Redraw visible table items
		//
		for (int column = columnFrom; column <= columnTo; column++)
		{
			for (int row = from; row <= to; row++)
			{
				view->update(model->index(row, column));
			}
		}
	};

	updateTable(m_readTableView, &m_readModel, TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE, 1, 2);
	updateTable(m_writeTableView, &m_writeModel, TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE, 1, 2);
}
