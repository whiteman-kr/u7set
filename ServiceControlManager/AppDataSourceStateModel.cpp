#include "AppDataSourceStateModel.h"
#include <CommonLib/HostAddressPort.h>
#include "../UtilsLib/WUtils.h"
#include "Brush.h"

AppDataSourceStateModel::AppDataSourceStateModel()
{
	m_valueTime.resize(m_rows.size(), std::make_pair(0, 0));
	m_curTime = QDateTime::currentMSecsSinceEpoch();

	connect(&m_timer1s, &QTimer::timeout, this, &AppDataSourceStateModel::onTimer1s);
	m_timer1s.start(1000);
}

int AppDataSourceStateModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_rows.size());
}

int AppDataSourceStateModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant AppDataSourceStateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section) {
		case 0: return "Property";
		case 1: return "Value";
		}
	}
	return QVariant();
}

QVariant AppDataSourceStateModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::CheckStateRole ||
		role == Qt::DecorationRole ||
		role == Qt::EditRole ||
		role == Qt::FontRole)
	{
		return m_cleanVariant;
	}

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= TO_INT(m_rows.size()) ||
		column < 0 || column >= 2)
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == 0)
		{
			return QVariant();
		}

		switch (row)
		{
		case 0:	return (m_state.receivesdata() ? m_cleanVariant : YELLOW_BRUSH);
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 9:
			return m_cleanVariant;

		case ROW_SS_QUEUE_CUR_SIZE: return m_state.signalstatesqueuecursize() > 10 ? YELLOW_BRUSH : m_cleanVariant ;

		case ROW_RECEIVED_DATA_UID: return m_state.receivesdata() && m_state.receiveddataid() != m_state.expecteddataid() ? YELLOW_BRUSH : m_cleanVariant;

		case ROW_LOST_PACKET_COUINT:
		case ROW_ERR_PROTOCOL_VERSION:
		case ROW_ERR_FRAMES_QUANTITY:
		case ROW_ERR_FRAME_NO:
		case ROW_ERR_FRAME_CRC:
		case ROW_ERR_DATA_UID:
		case ROW_ERR_DUP_PLANT_TIME:
		case ROW_ERR_NONMONO_PLANT_TIME:
		case ROW_ERR_PLANT_TIME_FORMAT:
			return valueChanged(row, m_state.receivesdata()) ? YELLOW_BRUSH : m_cleanVariant;
		}

		return m_cleanVariant;
	}

	if (role == Qt::DisplayRole)
	{
		if (column == 0)
		{
			return m_rows[row];
		}

		if (m_state.receivesdata() == false)
		{
			if (row == 0)
			{
				return "No";
			}

			return m_cleanVariant;
		}

		switch (row)
		{
		case 0:	return "Yes";
		case 1: return formatUptime(m_state.uptime());
		case 2: return formatTime_DD_MM_YYYY(m_state.lmtime());
		case 3: return m_state.rupframenumerator();
		case 4: return m_state.datareceivingspeed();
		case 5: return TO_QVARIANT_QINT64(m_state.receivedframescount());
		case 6: return TO_QVARIANT_QINT64(m_state.receivedpacketcount());
		case 7: return TO_QVARIANT_QINT64(m_state.lostpacketcount());
		case 8: return m_state.signalstatesqueuecursize();
		case 9: return m_state.signalstatesqueuecurmaxsize();
		case 10: return QString("0x%1  (%2)").arg(QString("%1").arg(m_state.receiveddataid(), 8, 16, Latin1Char::ZERO).toUpper()).
												arg(m_state.receiveddataid());
		case 11: return TO_QVARIANT_QINT64(m_state.errorprotocolversion());
		case 12: return TO_QVARIANT_QINT64(m_state.errorframesquantity());
		case 13: return TO_QVARIANT_QINT64(m_state.errorframeno());
		case 14: return TO_QVARIANT_QINT64(m_state.errorframecrc());
		case 15: return TO_QVARIANT_QINT64(m_state.errordataid());
		case 16: return TO_QVARIANT_QINT64(m_state.errorduplicateplanttime());
		case 17: return TO_QVARIANT_QINT64(m_state.errornonmonotonicplanttime());
		case 18: return TO_QVARIANT_QINT64(m_state.errorplanttimeformat());
		}
	}

	return m_cleanVariant;
}

void AppDataSourceStateModel::updateData(const Network::AppDataSourceState& state)
{
	m_state = state;

	m_curTime = QDateTime::currentMSecsSinceEpoch();

	updateValueTime(ROW_LOST_PACKET_COUINT, m_state.lostpacketcount());
	updateValueTime(ROW_ERR_PROTOCOL_VERSION, m_state.errorprotocolversion());
	updateValueTime(ROW_ERR_FRAMES_QUANTITY, m_state.errorframesquantity());
	updateValueTime(ROW_ERR_FRAME_NO, m_state.errorframeno());
	updateValueTime(ROW_ERR_FRAME_CRC, m_state.errorframecrc());
	updateValueTime(ROW_ERR_DATA_UID, m_state.errordataid());
	updateValueTime(ROW_ERR_DUP_PLANT_TIME, m_state.errorduplicateplanttime());
	updateValueTime(ROW_ERR_NONMONO_PLANT_TIME, m_state.errornonmonotonicplanttime());
	updateValueTime(ROW_ERR_PLANT_TIME_FORMAT, m_state.errorplanttimeformat());

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}

void AppDataSourceStateModel::updateValueTime(int row, qint64 value)
{
	if (row < 0 || row >= m_valueTime.size())
	{
		Q_ASSERT(false);
		return;
	}

	if (value > 0 && m_valueTime[row].first != value)
	{
		m_valueTime[row].first = value;
		m_valueTime[row].second = m_curTime;
	}
}

bool AppDataSourceStateModel::valueChanged(int row, bool receivesData) const
{
	if (row < 0 || row >= m_valueTime.size())
	{
		Q_ASSERT(false);
		return false;
	}

	if (!receivesData)
	{
		return false;
	}

	qint64 dt = m_curTime - m_valueTime[row].second;

	return dt < (30 * 1000);	// 30 sec
}

void AppDataSourceStateModel::onTimer1s()
{
	m_curTime = QDateTime::currentMSecsSinceEpoch();
}
