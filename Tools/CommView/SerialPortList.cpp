#include "SerialPortList.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CommStateTable::CommStateTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

CommStateTable::~CommStateTable()
{
	m_portMutex.lock();

		m_portOptionList.clear();

	m_portMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int CommStateTable::columnCount(const QModelIndex&) const
{
	return COMM_STATE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int CommStateTable::rowCount(const QModelIndex&) const
{
	return portCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommStateTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < COMM_STATE_LIST_COLUMN_COUNT)
		{
			result = CommStateColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommStateTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= portCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > COMM_STATE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	SerialPortOption* option = portOption(row);
	if (option == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (column == COMM_STATE_LIST_COLUMN_PORT)
		{
			if (option->isConnected() == false)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

        if (column == COMM_STATE_LIST_COLUMN_RECEIVED)
        {
			if (option->isNoReply() == true)
            {
                return QColor(0xFF, 0xA0, 0xA0);
            }
        }

		if (column == COMM_STATE_LIST_COLUMN_SKIPPED)
		{
			if ( static_cast<double>(option->skippedBytes()) * 100.0 / static_cast<double>(option->receivedBytes()) > MAX_SKIPPED_BYTES_IN_PERCENTAGES)
			{
				return QColor(0xFF, 0xA0, 0xA0);
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, option);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString CommStateTable::text(int row, int column, SerialPortOption* portOption) const
{
	if (row < 0 || row >= portCount())
	{
		return QString();
	}

	if (column < 0 || column > COMM_STATE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (portOption == nullptr)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
        case COMM_STATE_LIST_COLUMN_PORT:		result = portOption->portName();                        break;
        case COMM_STATE_LIST_COLUMN_TYPE:		result = portOption->typeStr();                         break;
        case COMM_STATE_LIST_COLUMN_BAUDRATE:	result = QString::number(portOption->baudRate());       break;
        case COMM_STATE_LIST_COLUMN_SIZE:		result = portOption->dataSizeStr();                     break;
        case COMM_STATE_LIST_COLUMN_RECEIVED:   result = portOption->receivedBytesStr();                break;
        case COMM_STATE_LIST_COLUMN_SKIPPED:	result = portOption->skippedBytesStr();                 break;
        case COMM_STATE_LIST_COLUMN_QUEUE:		result = portOption->queueBytesStr();                   break;
		case COMM_STATE_LIST_COLUMN_PACKETS:	result = portOption->packetCountStr();                  break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void CommStateTable::updateColumn(int column)
{
	if (column < 0 || column >= COMM_STATE_LIST_COLUMN_COUNT)
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

void CommStateTable::updateColumns()
{
	emit dataChanged(index(0, 0), index(SERIAL_PORT_COUNT, COMM_STATE_LIST_COLUMN_COUNT), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

int CommStateTable::portCount() const
{
	int count = 0;

	m_portMutex.lock();

		count = m_portOptionList.count();

	m_portMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption* CommStateTable::portOption(int index) const
{
	SerialPortOption* portOption = nullptr;

	m_portMutex.lock();

		if (index >= 0 && index < m_portOptionList.count())
		{
			 portOption = m_portOptionList[index];
		}

	m_portMutex.unlock();

	return portOption;
}

// -------------------------------------------------------------------------------------------------------------------

void CommStateTable::set(const QList<SerialPortOption*> list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_portMutex.lock();

			m_portOptionList = list_add;

		m_portMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void CommStateTable::clear()
{
	int count = m_portOptionList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_portMutex.lock();

			m_portOptionList.clear();

		m_portMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CommHeaderTable::CommHeaderTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

CommHeaderTable::~CommHeaderTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int CommHeaderTable::columnCount(const QModelIndex&) const
{
	return SERIAL_PORT_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int CommHeaderTable::rowCount(const QModelIndex&) const
{
	return COMM_PACKET_FIELD_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommHeaderTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SERIAL_PORT_COUNT)
		{
			SerialPortOption* portOption = theOptions.serialPorts().port(section);
			if (portOption != nullptr)
			{
				if (portOption->portName().isEmpty() == true)
				{
					result = tr("Empty");
				}
				else
				{
					result = portOption->portName();
				}
			}
		}
	}

	if (orientation == Qt::Vertical)
	{
		if (section >= 0 && section < COMM_PACKET_FIELD_COUNT)
		{
			result = CommPacketField[section];
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommHeaderTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= COMM_PACKET_FIELD_COUNT)
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > SERIAL_PORT_COUNT)
	{
		return QVariant();
	}

	SerialPortOption* portOption = theOptions.serialPorts().port(column);
	if (portOption == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

    if (role == Qt::BackgroundColorRole)
    {
		if (portOption->isNoReply() == false)
        {
//			if (row == COMM_PACKET_FIELD_DATAID)
//            {

//                if (portOption->isDataUidOk() == false)
//                {
//                    return QColor(0xFF, 0xA0, 0xA0);
//                }
//            }

            if (row == COMM_PACKET_FIELD_HEADER_CRC)
            {

                if (portOption->isHeaderCrcOk() == false)
                {
                    return QColor(0xFF, 0xA0, 0xA0);
                }
            }

            if (row == COMM_PACKET_FIELD_DATA_CRC)
            {
                if (portOption->isDataCrcOk() == false)
                {
                    return QColor(0xFF, 0xA0, 0xA0);
                }
            }
        }

        return QVariant();
    }

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column);
	}

	return QVariant();
}

 // -------------------------------------------------------------------------------------------------------------------

QString CommHeaderTable::text(int row, int column) const
{
	if (row < 0 || row >= COMM_PACKET_FIELD_COUNT)
	{
		return QString();
	}

	if (column < 0 || column > SERIAL_PORT_COUNT)
	{
		return QString();
	}

	SerialPortOption* portOption = theOptions.serialPorts().port(column);
	if (portOption == nullptr)
	{
		return QString();
	}

    if (portOption->isConnected() == false)
    {
        return tr("?");
    }

	SerialPortDataHeader* pHeader = portOption->dataHeader();
	if (pHeader == nullptr)
	{
		return QString();
	}

	QString result;

	if (theOptions.view().showInHex() == true)
	{
		switch(row)
		{
			case COMM_PACKET_FIELD_SIGN:		result.sprintf("0x%08X",  pHeader->Signature);	break;
			case COMM_PACKET_FIELD_VERSION:		result.sprintf("0x%04X",  pHeader->Version);	break;
			case COMM_PACKET_FIELD_TRANSID:		result.sprintf("0x%04X",  pHeader->TransID);	break;
			case COMM_PACKET_FIELD_NUMERATOR:	result.sprintf("0x%04X",  pHeader->Numerator);	break;
			case COMM_PACKET_FIELD_DATASIZE:	result.sprintf("0x%04X",  pHeader->DataSize);	break;
			case COMM_PACKET_FIELD_DATAID:		result.sprintf("0x%08X",  SWAP_4_BYTES(pHeader->DataUID));								break;
			case COMM_PACKET_FIELD_HEADER_CRC:	result = "0x" + QString("%1").arg(SWAP_8_BYTES(pHeader->CRC64), 0, 16).toUpper();			break;
			case COMM_PACKET_FIELD_DATA_CRC:	result = "0x" + QString("%1").arg(SWAP_8_BYTES(portOption->dataCRC()), 0, 16).toUpper();	break;
		}
	}
	else
	{
		switch(row)
		{
			case COMM_PACKET_FIELD_SIGN:		result = QString::number(pHeader->Signature);	break;
			case COMM_PACKET_FIELD_VERSION:		result = QString::number(pHeader->Version);		break;
			case COMM_PACKET_FIELD_TRANSID:		result = QString::number(pHeader->TransID);		break;
			case COMM_PACKET_FIELD_NUMERATOR:	result = QString::number(pHeader->Numerator);	break;
			case COMM_PACKET_FIELD_DATASIZE:	result = QString::number(pHeader->DataSize);	break;
			case COMM_PACKET_FIELD_DATAID:		result = QString::number(SWAP_4_BYTES(pHeader->DataUID));			break;
			case COMM_PACKET_FIELD_HEADER_CRC:	result = QString::number(SWAP_8_BYTES(pHeader->CRC64));			break;
			case COMM_PACKET_FIELD_DATA_CRC:	result = QString::number(SWAP_8_BYTES(portOption->dataCRC()));	break;
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void CommHeaderTable::updateColumns()
{
	emit dataChanged(index(0, 0), index(COMM_PACKET_FIELD_COUNT, SERIAL_PORT_COUNT), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CommDataTable::CommDataTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

CommDataTable::~CommDataTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int CommDataTable::columnCount(const QModelIndex&) const
{
	return SERIAL_PORT_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int CommDataTable::rowCount(const QModelIndex&) const
{
	return dataSize();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommDataTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SERIAL_PORT_COUNT)
		{
			SerialPortOption* portOption = theOptions.serialPorts().port(section);
			if (portOption != nullptr)
			{
				if (portOption->portName().isEmpty() == true)
				{
					result = tr("Empty");
				}
				else
				{
					result = portOption->portName();
				}
			}
		}
	}

	if (orientation == Qt::Vertical)
	{
		if (theOptions.view().showHeader() == false)
		{
			result = QString("%1").arg(section + 1);
		}
		else
		{
			int headerSize = theOptions.view().showInWord() == true ? SERIAL_PORT_HEADER_SIZE/2 : SERIAL_PORT_HEADER_SIZE;

			if (section < headerSize)
			{
				result = QString("H %1").arg(section + 1);
			}
			else
			{
				result = QString("D %1").arg(section + 1 - headerSize);
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant CommDataTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > SERIAL_PORT_COUNT)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString CommDataTable::text(int row, int column) const
{
	if (row < 0 || row >= dataSize())
	{
		return QString();
	}

	if (column < 0 || column > SERIAL_PORT_COUNT)
	{
		return QString();
	}

	SerialPortOption* portOption = theOptions.serialPorts().port(column);
	if (portOption == nullptr)
	{
		return QString();
	}

	int portDataSize = portOption->dataSize();

	if (theOptions.view().showHeader() == false)
	{
		portDataSize -= SERIAL_PORT_HEADER_SIZE;
	}

	if (theOptions.view().showInWord() == true)
	{
		portDataSize = portDataSize/2;
	}

	if (row < 0 || row >= portDataSize)
	{
		return QString();
	}

	if (portOption->isConnected() == false)
	{
		return tr("?");
	}

	QString result;

	if (theOptions.view().showInHex() == true)
	{
		result.sprintf(theOptions.view().showInWord() == true ? "0x%04X" : "0x%02X",  portOption->data(row));
	}
	else
	{
		if (theOptions.view().showInFloat() == false)
		{
			result.sprintf("%u",  portOption->data(row));
		}
		else
		{
			int headerSize = theOptions.view().showInWord() == true ? SERIAL_PORT_HEADER_SIZE/2 : SERIAL_PORT_HEADER_SIZE;
			if (row >= headerSize)
			{
				int floatsize = theOptions.view().showInWord() == true ? 2 : 4;
				if ((row+1) % floatsize == 0)
				{
					quint32 dwFolat1 = 0;
					float aFloat1 = 1;

					memcpy(&dwFolat1, &aFloat1, sizeof(float));



					quint32 dwFolat = 0;
					float aFloat = 0;

					if (theOptions.view().showInWord()  == true)
					{
						dwFolat = MAKEDWORD( portOption->data(row - 0), portOption->data(row - 1));
					}
					else
					{
						dwFolat = MAKEDWORD( MAKEWORD(portOption->data(row - 1), portOption->data(row - 0)), MAKEWORD(portOption->data(row - 3), portOption->data(row - 2)));
					}

					memcpy(&aFloat, &dwFolat, sizeof(float));
					result.sprintf("%.4f",  aFloat);
				}
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void CommDataTable::updateColumns()
{
	emit dataChanged(index(0, 0), index(dataSize(), SERIAL_PORT_COUNT), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

int CommDataTable::dataSize() const
{
	int dataSize = 0;

	m_dataMutex.lock();

		dataSize = theOptions.serialPorts().dataSize();

		if (theOptions.view().showHeader() == false)
		{
			dataSize -= SERIAL_PORT_HEADER_SIZE;
		}

		if (theOptions.view().showInWord() == true)
		{
			dataSize = dataSize/2;
		}

	m_dataMutex.unlock();

	return dataSize;
}

// -------------------------------------------------------------------------------------------------------------------

void CommDataTable::reset()
{
	beginResetModel();
	endResetModel();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
