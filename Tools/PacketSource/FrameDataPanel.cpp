#include "FrameDataPanel.h"

#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QClipboard>

#include "MainWindow.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FrameDataTable::FrameDataTable(QObject *)
{
}

// -------------------------------------------------------------------------------------------------------------------

FrameDataTable::~FrameDataTable()
{
	m_frameMutex.lock();

		m_frameList.clear();

	m_frameMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int FrameDataTable::columnCount(const QModelIndex&) const
{
	return FRAME_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int FrameDataTable::rowCount(const QModelIndex&) const
{
	return dataSize();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FrameDataTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < FRAME_LIST_COLUMN_COUNT)
		{
			result = FrameDataListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("+%1").arg(section);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FrameDataTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= dataSize())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > FRAME_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	int frameIndex = row / Rup::FRAME_DATA_SIZE;

	PS::FrameData* pFrameData = frame(frameIndex);
	if (pFrameData == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row - Rup::FRAME_DATA_SIZE * frameIndex, column, pFrameData);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString FrameDataTable::text(int row, int column, PS::FrameData* pFrameData) const
{
	if (row < 0 || row >= Rup::FRAME_DATA_SIZE)
	{
		return QString();
	}

	if (column < 0 || column > FRAME_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pFrameData == nullptr)
	{
		return QString();
	}

	quint8 byte = pFrameData->data()[row];

	QString result;

	switch (column)
	{
		case FRAME_LIST_COLUMN_DEC:		result = QString::number(byte);	break;
		case FRAME_LIST_COLUMN_HEX:		result = "0x" + QString::number(byte, 16).rightJustified(2, '0').toUpper();	break;
		default:						assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataTable::updateColumn(int column)
{
	if (column < 0 || column >= FRAME_LIST_COLUMN_COUNT)
	{
		return;
	}

	int count = rowCount();

	for (int row = 0; row < count; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int FrameDataTable::dataSize() const
{
	int count = 0;

	m_frameMutex.lock();

		count = m_frameList.count() * Rup::FRAME_DATA_SIZE;

	m_frameMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData* FrameDataTable::frame(int index) const
{
	PS::FrameData* pData = nullptr;

	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			 pData = m_frameList[index];
		}

	m_frameMutex.unlock();

	return pData;
}

// -------------------------------------------------------------------------------------------------------------------

quint8 FrameDataTable::byte(int index) const
{
	if (index < 0 || index >= dataSize())
	{
		return 0;
	}

	int frameIndex = index / Rup::FRAME_DATA_SIZE;

	PS::FrameData* pFrameData = frame(frameIndex);
	if (pFrameData == nullptr)
	{
		return 0;
	}

	int frameOffset = index - Rup::FRAME_DATA_SIZE * frameIndex;
	if (frameOffset < 0 || frameOffset >= Rup::FRAME_DATA_SIZE)
	{
		return 0;
	}

	return pFrameData->data()[frameOffset];
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataTable::setByte(int index, quint8 byte)
{
	if (index < 0 || index >= dataSize())
	{
		return;
	}

	int frameIndex = index / Rup::FRAME_DATA_SIZE;

	PS::FrameData* pFrameData = frame(frameIndex);
	if (pFrameData == nullptr)
	{
		return;
	}

	int frameOffset = index - Rup::FRAME_DATA_SIZE * frameIndex;
	if (frameOffset < 0 || frameOffset >= Rup::FRAME_DATA_SIZE)
	{
		return;
	}

	pFrameData->data()[frameOffset] = byte;
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataTable::set(const QVector<PS::FrameData*>& list_add)
{
	int count = list_add.count() * Rup::FRAME_DATA_SIZE;
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_frameMutex.lock();

			m_frameList = list_add;

		m_frameMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataTable::clear()
{
	int count = dataSize();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_frameMutex.lock();

			m_frameList.clear();

		m_frameMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FrameDataStateDialog::FrameDataStateDialog(quint8 byte, QWidget *parent) :
	QDialog(parent),
	m_byte(byte)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

FrameDataStateDialog::~FrameDataStateDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataStateDialog::createInterface()
{
	setWindowFlags(Qt::Window  | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/InOut.png"));
	setWindowTitle(tr("State"));

	// main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

		QLabel* stateLabel = new QLabel(tr("Please, input new state:"));
		stateLabel->setAlignment(Qt::AlignHCenter);

		m_stateEdit = new QLineEdit(QString::number(m_byte));
		m_stateEdit->setAlignment(Qt::AlignHCenter);
		m_stateEdit->setValidator(new QIntValidator(0, 255, this));

		QLabel* rangeLabel = new QLabel("Range: 0 .. 255");
		rangeLabel->setAlignment(Qt::AlignHCenter);

		// buttons
		//
		QHBoxLayout *buttonLayout = new QHBoxLayout ;

		QPushButton* okButton = new QPushButton(tr("Ok"));
		QPushButton* cancelButton = new QPushButton(tr("Cancel"));

		connect(okButton, &QPushButton::clicked, this, &FrameDataStateDialog::onOk);
		connect(cancelButton, &QPushButton::clicked, this, &FrameDataStateDialog::reject);

		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		// main Layout
		//
		mainLayout->addWidget(stateLabel);
		mainLayout->addWidget(m_stateEdit);
		mainLayout->addWidget(rangeLabel);
		mainLayout->addStretch();
		mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataStateDialog::onOk()
{
	m_byte = static_cast<quint8>(m_stateEdit->text().toUInt());
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FrameDataPanel::FrameDataPanel(QWidget* parent) :
	QDockWidget(parent)
{
	m_pMainWindow = dynamic_cast<QMainWindow*> (parent);
	if (m_pMainWindow == nullptr)
	{
		return;
	}

	setWindowTitle("Frame of data panel");
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

FrameDataPanel::~FrameDataPanel()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::createInterface()
{
	m_pFrameWindow = new QMainWindow;

	m_pView = new QTableView(m_pFrameWindow);
	m_pView->setModel(&m_frameDataTable);
	m_pView->verticalHeader()->setDefaultSectionSize(22);

	m_pFrameWindow->setCentralWidget(m_pView);

	for(int column = 0; column < FRAME_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, FRAME_LIST_COLUMN_ROW_WIDTH);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(m_pView, &QTableView::doubleClicked , this, &FrameDataPanel::onListDoubleClicked);
	m_pView->installEventFilter(this);

	setWidget(m_pFrameWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::createContextMenu()
{
	if (m_pFrameWindow == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Signal state"), m_pFrameWindow);

	m_pSetStateAction = m_pContextMenu->addAction(tr("Set state ..."));
	connect(m_pSetStateAction, &QAction::triggered, this, &FrameDataPanel::onSetStateAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &FrameDataPanel::onContextMenu);
}


// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::clear()
{
	m_frameDataTable.clear();

}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::set(const QVector<PS::FrameData*>& list_add)
{
	m_frameDataTable.set(list_add);
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::setState()
{
	if (m_pView == nullptr)
	{
		return;
	}

	QModelIndexList rows = m_pView->selectionModel()->selectedRows();

	int rowCount = rows.count();
	if (rowCount <= 0)
	{
		return;
	}

	int byteIndex = rows.first().row();
	if (byteIndex < 0 || byteIndex >= m_frameDataTable.dataSize())
	{
		return;
	}

	quint8 byte = m_frameDataTable.byte(byteIndex);

	FrameDataStateDialog dialog(byte);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	for (int r = 0; r < rowCount; r++)
	{
		int byteIndex = rows[r].row();
		if (byteIndex < 0 || byteIndex >= m_frameDataTable.dataSize())
		{
			continue;
		}

		m_frameDataTable.setByte(byteIndex, dialog.byte());
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool FrameDataPanel::event(QEvent* e)
{
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

		if (keyEvent->key() == Qt::Key_Return)
		{
			setState();
		}
	}

	return QDockWidget::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::onSetStateAction()
{
	setState();
}

// -------------------------------------------------------------------------------------------------------------------

void FrameDataPanel::onListDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);

	setState();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
