#include "SignalList.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalTable::SignalTable(QObject *)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalTable::~SignalTable()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalTable::columnCount(const QModelIndex&) const
{
	return SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalTable::rowCount(const QModelIndex&) const
{
	return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SIGNAL_LIST_COLUMN_COUNT)
		{
			result = SignalListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		//result = QString("%1").arg(section + 1);
		result = QString();
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= signalCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	PS::Signal* pSignal = signalPtr(row);
	if (pSignal == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		QVariant result = Qt::AlignCenter;

		switch (column)
		{
			case SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_STATE:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_ADB:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_INOUT:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_EN_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_FORMAT:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_STATE_OFFSET:	result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_STATE_BIT:		result = Qt::AlignCenter;	break;
			default:								assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
		{
			return QColor(0xD0, 0xD0, 0xD0);
		}

		if(pSignal->isDiscrete() == true)
		{
			return QColor(0x00, 0x00, 0xFF);
		}
		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalTable::text(int row, int column, PS::Signal* pSignal) const
{
	if (row < 0 || row >= signalCount())
	{
		return QString();
	}

	if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSignal == nullptr)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = pSignal->customAppSignalID();			break;
		case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = pSignal->equipmentID();				break;
		case SIGNAL_LIST_COLUMN_APP_ID:			result = pSignal->appSignalID();				break;
		case SIGNAL_LIST_COLUMN_CAPTION:		result = pSignal->caption();					break;
		case SIGNAL_LIST_COLUMN_STATE:			result = pSignal->stateStr();					break;
		case SIGNAL_LIST_COLUMN_ADB:			result = pSignal->signalTypeStr();				break;
		case SIGNAL_LIST_COLUMN_INOUT:			result = pSignal->signalInOutTypeStr();			break;
		case SIGNAL_LIST_COLUMN_EN_RANGE:		result = pSignal->engineeringRangeStr();		break;
		case SIGNAL_LIST_COLUMN_FORMAT:			result = pSignal->signalFormatStr();			break;
		case SIGNAL_LIST_COLUMN_STATE_OFFSET:	result = pSignal->stateOffsetStr();				break;
		case SIGNAL_LIST_COLUMN_STATE_BIT:		result = pSignal->stateBitStr();				break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalTable::updateColumn(int column)
{
	if (column < 0 || column >= SIGNAL_LIST_COLUMN_COUNT)
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

int SignalTable::signalCount() const
{
	QMutexLocker l(&m_signalMutex);

	return m_signalList.count();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalTable::signalPtr(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_signalList.count())
	{
		return nullptr;
	}

	return m_signalList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalTable::set(const QVector<PS::Signal*> list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList = list_add;

		m_signalMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalTable::clear()
{
	int count = m_signalList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalStateDialog::SignalStateDialog(PS::Signal* pSignal, QWidget *parent) :
	QDialog(parent),
	m_pSignal(pSignal)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

SignalStateDialog::~SignalStateDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalStateDialog::createInterface()
{
	setWindowFlags(Qt::Window  | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/Images/Options.svg"));
	setWindowTitle(tr("Signal state"));

	if (m_pSignal == nullptr || m_pSignal->valueData() == nullptr)
	{
		QMessageBox::critical(this, windowTitle(), tr("It is not possible to change signal state!"));
		return;
	}

	// main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	switch(m_pSignal->signalType())
	{
		case E::SignalType::Analog:
			{
				QLabel* stateLabel = new QLabel(tr("Please, input new state of analog signal:"));
				stateLabel->setAlignment(Qt::AlignHCenter);

				QRegExp rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
				QValidator *validator = new QRegExpValidator(rx, this);

				QString strState, formatStr;
				switch (m_pSignal->analogSignalFormat())
				{
					case E::AnalogAppSignalFormat::SignedInt32:		formatStr = QString::asprintf("%%.%df", 0);								break;
					case E::AnalogAppSignalFormat::Float32:			formatStr = QString::asprintf("%%.%df", m_pSignal->decimalPlaces());	break;
					default:										assert(0);													break;
				}
				strState = QString::asprintf(formatStr.toLocal8Bit(), m_pSignal->state());

				m_stateEdit = new QLineEdit(strState);
				m_stateEdit->setAlignment(Qt::AlignHCenter);
				m_stateEdit->setValidator(validator);

				QLabel* rangeLabel = new QLabel(m_pSignal->engineeringRangeStr());
				rangeLabel->setAlignment(Qt::AlignHCenter);

				// buttons
				//
				QHBoxLayout *buttonLayout = new QHBoxLayout ;

				QPushButton* okButton = new QPushButton(tr("Ok"));
				QPushButton* cancelButton = new QPushButton(tr("Cancel"));

				connect(okButton, &QPushButton::clicked, this, &SignalStateDialog::onOk);
				connect(cancelButton, &QPushButton::clicked, this, &SignalStateDialog::reject);

				buttonLayout->addWidget(okButton);
				buttonLayout->addWidget(cancelButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addWidget(m_stateEdit);
				mainLayout->addWidget(rangeLabel);
				mainLayout->addStretch();
				mainLayout->addLayout(buttonLayout);
			}
			break;

		case E::SignalType::Discrete:
			{
				QLabel* stateLabel = new QLabel(tr("Please, select new state of discrete signal:"));

				// buttons
				//
				QHBoxLayout *buttonLayout = new QHBoxLayout ;

				QPushButton* yesButton = new QPushButton(tr("Yes (1)"));
				QPushButton* noButton = new QPushButton(tr("No (0)"));

				connect(yesButton, &QPushButton::clicked, this, &SignalStateDialog::onYes);
				connect(noButton, &QPushButton::clicked, this, &SignalStateDialog::onNo);

				buttonLayout->addWidget(yesButton);
				buttonLayout->addWidget(noButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addLayout(buttonLayout);
			}
			break;

		default:
			assert(0);
	}

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalStateDialog::onOk()
{
	if (m_pSignal == nullptr)
	{
		return;
	}

	double state = m_stateEdit->text().toDouble();

	if (state < m_pSignal->lowEngineeringUnits() || state > m_pSignal->highEngineeringUnits())
	{
		QString str, formatStr;

		formatStr = QString::asprintf("%%.%df", m_pSignal->decimalPlaces());

		str = QString::asprintf("Failed input value: " + formatStr.toLocal8Bit(), state);
		str += tr("\nRange of signal: %1").arg(m_pSignal->engineeringRangeStr());
		str += tr("\nDo you want to change the signal state?");

		QMessageBox::StandardButton reply = QMessageBox::question(this, windowTitle(), str, QMessageBox::Yes|QMessageBox::No);
		if (reply == QMessageBox::No)
		{
			reject();
			return;
		}
	}

	m_state = state;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalStateDialog::onYes()
{
	m_state = 1;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalStateDialog::onNo()
{
	m_state = 0;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
