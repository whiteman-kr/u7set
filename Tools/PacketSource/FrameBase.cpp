#include "FrameBase.h"

#include <assert.h>
#include <QMessageBox>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::FrameData()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::FrameData(const FrameData& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::~FrameData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::FrameData::clear()
{
	memset(&m_data[0], 0, Rup::FRAME_DATA_SIZE);
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData& PS::FrameData::operator=(const PS::FrameData& from)
{
	m_frameMutex.lock();

		memcpy(m_data, from.m_data, Rup::FRAME_DATA_SIZE);

	m_frameMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FrameBase::FrameBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

FrameBase::~FrameBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FrameBase::clear()
{
	m_frameMutex.lock();

		m_frameList.clear();

	m_frameMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int FrameBase::count() const
{
	int count = 0;

	m_frameMutex.lock();

		count = m_frameList.count();

	m_frameMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

bool FrameBase::setFrameCount(int count)
{
	bool result = false;

	m_frameMutex.lock();

		m_frameList.clear();

		m_frameList.resize(count);

		result = m_frameList.count() == count;

	m_frameMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData FrameBase::frameData(int index) const
{
	PS::FrameData frameData;

	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			frameData = m_frameList[index];
		}

	m_frameMutex.unlock();

	return frameData;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData* FrameBase::frameDataPtr(int index)
{
	PS::FrameData* pFrameData = nullptr;

	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			pFrameData = &m_frameList[index];
		}

	m_frameMutex.unlock();

	return pFrameData;
}

// -------------------------------------------------------------------------------------------------------------------

void FrameBase::setFrameData(int index, const PS::FrameData &frame)
{
	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			m_frameList[index] = frame;
		}

	m_frameMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

FrameBase& FrameBase::operator=(const FrameBase& from)
{
	m_frameMutex.lock();

		m_frameList = from.m_frameList;

	m_frameMutex.unlock();

	return *this;
}

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

void FrameDataTable::set(const QVector<PS::FrameData*> list_add)
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
