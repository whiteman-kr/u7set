#include "SignalBase.h"

#include <assert.h>
#include <QMessageBox>
#include <QFile>
#include <QProgressDialog>

#include "../../lib/XmlHelper.h"
#include "../../Builder/CfgFiles.h"
#include "../../lib/DataProtocols.h"
#include "../../lib/WUtils.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::Signal::Signal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal::Signal(const PS::Signal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal::~Signal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Signal::clear()
{
	m_offset = -1;
	m_frameIndex = -1;
	m_frameOffset = -1;

	m_pValueData = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Signal::calcOffset()
{
	int offset = regValueAddr().offset();
	if (offset == BAD_ADDRESS)
	{
		return;
	}

	// offset - in memory in 16-bit words
	//
	m_offset = offset * 2;

	m_frameIndex = m_offset / Rup::FRAME_DATA_SIZE;

	m_frameOffset = m_offset - Rup::FRAME_DATA_SIZE * m_frameIndex;
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalTypeStr() const
{
	return E::valueToString<E::SignalType>(signalType());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalInOutTypeStr() const
{
	return E::valueToString<E::SignalInOutType>(inOutType());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::engineeringRangeStr() const
{
	if(signalType() != E::SignalType::Analog)
	{
		return QString();
	}

	QString range, formatStr;

	switch (analogSignalFormat())
	{
		case E::AnalogAppSignalFormat::SignedInt32:	formatStr.sprintf("%%.%df", 0);					break;
		case E::AnalogAppSignalFormat::Float32:		formatStr.sprintf("%%.%df", decimalPlaces());	break;
		default:									assert(0);										break;
	}

	range.sprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), lowEngineeringUnits(), highEngineeringUnits());

	if (unit().isEmpty() == false)
	{
		range.append(" " + unit());
	}

	return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalFormatStr() const
{
	if(signalType() != E::SignalType::Analog)
	{
		return QString();
	}

	return E::valueToString<E::AnalogAppSignalFormat>(analogSignalFormat());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateOffsetStr() const
{
	int offset = regValueAddr().offset();
	if (offset == BAD_ADDRESS)
	{
		return QString();
	}

	return QString::number(offset*2);
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateBitStr() const
{
	int bit = regValueAddr().bit();
	if (regValueAddr().offset() == BAD_ADDRESS || bit == BAD_ADDRESS)
	{
		return QString();
	}

	return QString::number(bit);
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateStr() const
{
	QString str, formatStr;

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:		formatStr.sprintf("%%.%df", 0);					break;
				case E::AnalogAppSignalFormat::Float32:			formatStr.sprintf("%%.%df", decimalPlaces());	break;
				default:										assert(0);										break;
			}

			str.sprintf(formatStr.toLocal8Bit(), state());

			if (unit().isEmpty() == false)
			{
				str.append(" " + unit());
			}

			break;

		case E::SignalType::Discrete:

			str = state() == 0.0 ? "No" : "Yes";

			break;
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

double PS::Signal::state() const
{
	if (regBufAddr().offset() == BAD_ADDRESS || regBufAddr().bit() == BAD_ADDRESS)
	{
		return 0;
	}

	if (m_pValueData == nullptr)
	{
		return 0;
	}

	double state = 0;

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						state = reverseUint32(*pDataPtr);
					}

					break;

				case E::AnalogAppSignalFormat::Float32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						float fState = 0;
						memcpy(&fState, &*pDataPtr, sizeof(float));
						state = static_cast<double>(reverseFloat(fState));
					}

					break;

				default: assert(0);
			}

			break;

		case E::SignalType::Discrete:
			{
				quint16* pDataPtr = reinterpret_cast<quint16*>(m_pValueData);
				if (pDataPtr == nullptr)
				{
					break;
				}

				int bitNo = regBufAddr().bit();

				if (bitNo >= 8)
				{
					bitNo -= 8;
				}
				else
				{
					bitNo += 8;
				}

				if ((*pDataPtr & (0x1 << bitNo)) != 0)
				{
					state = 1;
				}
			}
			break;
	}

	return state;
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Signal::setState(double state)
{
	if (regBufAddr().offset() == BAD_ADDRESS || regBufAddr().bit() == BAD_ADDRESS)
	{
		return;
	}

	if (m_pValueData == nullptr)
	{
		return;
	}

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						quint32 iState = reverseUint32(static_cast<quint32>(state));
						*pDataPtr = iState;
					}
					break;

				case E::AnalogAppSignalFormat::Float32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						float fState = reverseFloat(static_cast<float>(state));
						memcpy(pDataPtr, &fState, sizeof(float));
					}

					break;

				default: assert(0); break;
			}

			break;

		case E::SignalType::Discrete:
			{
				quint16* pDataPtr = reinterpret_cast<quint16*>(m_pValueData);
				if (pDataPtr == nullptr)
				{
					break;
				}

				int bitNo = regBufAddr().bit();

				if (bitNo >= 8)
				{
					bitNo -= 8;
				}
				else
				{
					bitNo += 8;
				}

				int iState = static_cast<int>(state);

				switch(iState)
				{
					case 0: *pDataPtr &= ~(0x1 << bitNo);		break;
					case 1: *pDataPtr |= (0x1 << bitNo);		break;
				}
			}
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal& PS::Signal::operator=(const PS::Signal& from)
{
	m_signalMutex.lock();

		m_offset = from.m_offset;
		m_frameIndex = from.m_frameIndex;
		m_frameOffset = from.m_frameOffset;
		m_pValueData = from.m_pValueData;

		::Signal::operator=(from);

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SignalBase::~SignalBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::readFromFile(const QString& path)
{
	clear();

	QString msgTitle = tr("Loading signals");

	if (path.isEmpty() == true)
	{
		QMessageBox::information(nullptr, msgTitle, tr("Please, input path to signals directory!"));
		return 0;
	}

	// read Signals
	//

	QString fileSignals = path + "/" + Builder::FILE_APP_SIGNALS_ASGS;

	QFile fileSignalsAsgs(fileSignals);
	if (fileSignalsAsgs.exists() == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not found!").arg(Builder::FILE_APP_SIGNALS_ASGS));
		return 0;
	}

	if (fileSignalsAsgs.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::information(nullptr, msgTitle, tr("File %1 is not opened!").arg(Builder::FILE_APP_SIGNALS_ASGS));
		return 0;
	}

	QByteArray&& signalsData = fileSignalsAsgs.readAll();
	QByteArray uncompressedData = qUncompress(signalsData);

	::Proto::AppSignalSet protoAppSignalSet;

	bool result = protoAppSignalSet.ParseFromArray(uncompressedData.constData(), uncompressedData.size());
	if (result == false)
	{
		return 0;
	}

	QString strProgressLabel;
	int signalCount = protoAppSignalSet.appsignal_size();

	QProgressDialog* pprd = new QProgressDialog(tr("Loading signals..."), tr("&Cancel"), 0, signalCount);
	pprd->setMinimumDuration(0);
	pprd->setWindowTitle("Please Wait");

	for(int i = 0; i < signalCount; i++)
	{
		pprd->setValue(i) ;
		qApp->processEvents();
		if (pprd->wasCanceled())
		{
			 break;
		}

		const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

		PS::Signal signal;
		signal.serializeFrom(protoAppSignal);

		append(signal);
	}

	delete pprd;

	emit signalsLoaded();

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::append(const PS::Signal& signal)
{
	if (signal.appSignalID().isEmpty() == true || signal.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

	if (m_signalHashMap.contains(signal.hash()) == false)
	{
		m_signalList.append(signal);
		index = m_signalList.count() - 1;

		m_signalHashMap.insert(signal.hash(), index);
	}

	m_signalMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return nullptr;
	}

	PS::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];
			if (index >= 0 && index < m_signalList.count())
			{
				pSignal = (PS::Signal*) &m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalBase& SignalBase::operator=(const SignalBase& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(const Hash& hash) const
{
	PS::Signal signal;

	m_signalMutex.lock();

		int index = m_signalHashMap[hash];
		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(int index) const
{
	PS::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = (PS::Signal*) &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(int index) const
{
	PS::Signal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignal(int index, const PS::Signal &signal)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index] = signal;
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(const QString& appSignalID) const
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return signalPtr(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalTable::SignalTable(QObject *)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalTable::~SignalTable()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
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
		return Qt::AlignCenter;
	}

	if (role == Qt::TextColorRole)
	{
		if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
		{
			return QColor(0xD0, 0xD0, 0xD0);
		}
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
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalTable::signalPtr(int index) const
{
	PS::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			 pSignal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
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
	setWindowIcon(QIcon(":/icons/InOut.png"));
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
					case E::AnalogAppSignalFormat::SignedInt32:		formatStr.sprintf("%%.%df", 0);								break;
					case E::AnalogAppSignalFormat::Float32:			formatStr.sprintf("%%.%df", m_pSignal->decimalPlaces());	break;
					default:										assert(0);													break;
				}
				strState.sprintf(formatStr.toLocal8Bit(), m_pSignal->state());

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

		formatStr.sprintf("%%.%df", m_pSignal->decimalPlaces());

		str.sprintf("Failed input value: " + formatStr.toLocal8Bit(), state);
		str += tr("\nRange of signal: %1").arg(m_pSignal->engineeringRangeStr());

		QMessageBox::critical(this, windowTitle(), str);
		return;
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
