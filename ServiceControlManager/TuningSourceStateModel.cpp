#include "TuningSourceStateModel.h"
#include <CommonLib/HostAddressPort.h>
#include "../UtilsLib/WUtils.h"
#include "Brush.h"

TuningSourceStateModel::TuningSourceStateModel()
{
	m_valueTime.resize(m_rows.size(), std::make_pair(0, 0));
	m_curTime = QDateTime::currentMSecsSinceEpoch();

	connect(&m_timer1s, &QTimer::timeout, this, &TuningSourceStateModel::onTimer1s);
	m_timer1s.start(1000);
}

int TuningSourceStateModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_rows.size());
}

int TuningSourceStateModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant TuningSourceStateModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant TuningSourceStateModel::data(const QModelIndex& index, int role) const
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

	const Network::TuningSourceState& st = m_state.state();

	if (role == Qt::BackgroundRole)
	{
		if (column == 0)
		{
			return QVariant();
		}

		switch (row)
		{
		case 0:	return (st.isreply() ? m_cleanVariant : YELLOW_BRUSH);
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			return m_cleanVariant;
		}

		if (row >= 9 && row <= 38)
		{
			return (valueChanged(row, true) ? YELLOW_BRUSH : m_cleanVariant);
		}

		return m_cleanVariant;
	}

	if (role == Qt::DisplayRole)
	{
		if (column == 0)
		{
			return m_rows[row];
		}

		switch (row)
		{
		case 0:	return (st.isreply() ? Separator::YES : Separator::NO);
		case 1: return formatTime_DD_MM_YYYY(st.lmtime());
		case 2:	return Separator::QUESTIONS;
		case 3: return TO_QVARIANT_QINT64(st.requestcount());
		case 4: return TO_QINT64(st.replycount());
		case 5: return (st.controlisactive() ?  Separator::YES : Separator::NO);
		case 6: return (st.setsor() ?  Separator::YES : Separator::NO);
		case 7: return (st.writingdisabled() ?  Separator::YES : Separator::NO);
		case 8: return (st.hasunappliedparams() ?  Separator::YES : Separator::NO);
		case 9: return TO_QVARIANT_QINT64(st.fotipflagboundschecksuccess());
		case 10: return TO_QVARIANT_QINT64(st.fotipflagwritesuccess());
		case 11: return TO_QVARIANT_QINT64(st.fotipflagapplysuccess());
		case 12: return TO_QVARIANT_QINT64(st.erruntimelyreplay());
		case 13: return TO_QVARIANT_QINT64(st.errsent());
		case 14: return TO_QVARIANT_QINT64(st.errpartialsent());
		case 15: return TO_QVARIANT_QINT64(st.errreplysize());
		case 16: return TO_QVARIANT_QINT64(st.errnoreply());
		case 17: return TO_QVARIANT_QINT64(st.errtuningframeupdate());
		case 18: return TO_QVARIANT_QINT64(st.errrupprotocolversion());
		case 19: return TO_QVARIANT_QINT64(st.errrupframesize());
		case 20: return TO_QVARIANT_QINT64(st.errrupnontuningdata());
		case 21: return TO_QVARIANT_QINT64(st.errrupmoduletype());
		case 22: return TO_QVARIANT_QINT64(st.errrupframesquantity());
		case 23: return TO_QVARIANT_QINT64(st.errrupframenumber());
		case 24: return TO_QVARIANT_QINT64(st.errrupcrc());
		case 25: return Separator::QUESTIONS;		// err data uid
		case 26: return Separator::QUESTIONS;		// err duplicate plant time
		case 27: return Separator::QUESTIONS;		// err non-monotonic plant time
		case 28: return Separator::QUESTIONS;		// err plant time format
		case 29: return TO_QVARIANT_QINT64(st.errfotipprotocolversion());
		case 30: return TO_QVARIANT_QINT64(st.errfotipuniqueid());
		case 31: return TO_QVARIANT_QINT64(st.errfotiplmnumber());
		case 32: return TO_QVARIANT_QINT64(st.errfotipsubsystemcode());
		case 33: return TO_QVARIANT_QINT64(st.errfotipoperationcode());
		case 34: return TO_QVARIANT_QINT64(st.errfotipframesize());
		case 35: return TO_QVARIANT_QINT64(st.errfotipromsize());
		case 36: return TO_QVARIANT_QINT64(st.errfotipromframesize());
		case 37: return TO_QVARIANT_QINT64(st.erranaloglowboundcheck());
		case 38: return TO_QVARIANT_QINT64(st.erranaloghighboundcheck());
		}
	}

	return m_cleanVariant;
}

void TuningSourceStateModel::updateData(const Network::TuningSourceInfoState& state)
{
	m_state = state;

	m_curTime = QDateTime::currentMSecsSinceEpoch();

	const Network::TuningSourceState& st = m_state.state();

	updateValueTime(9, st.fotipflagboundschecksuccess());
	updateValueTime(10, st.fotipflagwritesuccess());
	updateValueTime(11, st.fotipflagapplysuccess());
	updateValueTime(12, st.erruntimelyreplay());
	updateValueTime(13, st.errsent());
	updateValueTime(14, st.errpartialsent());
	updateValueTime(15, st.errreplysize());
	updateValueTime(16, st.errnoreply());
	updateValueTime(17, st.errtuningframeupdate());
	updateValueTime(18, st.errrupprotocolversion());
	updateValueTime(19, st.errrupframesize());
	updateValueTime(20, st.errrupnontuningdata());
	updateValueTime(21, st.errrupmoduletype());
	updateValueTime(22, st.errrupframesquantity());
	updateValueTime(23, st.errrupframenumber());
	updateValueTime(24, st.errrupcrc());
//	updateValueTime(25: return Separator::QUESTIONS;		// err data uid
//	updateValueTime(26: return Separator::QUESTIONS;		// err duplicate plant time
//	updateValueTime(27: return Separator::QUESTIONS;		// err non-monotonic plant time
//	updateValueTime(28: return Separator::QUESTIONS;		// err plant time format
	updateValueTime(29, st.errfotipprotocolversion());
	updateValueTime(30, st.errfotipuniqueid());
	updateValueTime(31, st.errfotiplmnumber());
	updateValueTime(32, st.errfotipsubsystemcode());
	updateValueTime(33, st.errfotipoperationcode());
	updateValueTime(34, st.errfotipframesize());
	updateValueTime(35, st.errfotipromsize());
	updateValueTime(36, st.errfotipromframesize());
	updateValueTime(37, st.erranaloglowboundcheck());
	updateValueTime(38, st.erranaloghighboundcheck());

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}

void TuningSourceStateModel::updateValueTime(int row, qint64 value)
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

bool TuningSourceStateModel::valueChanged(int row, bool receivesData) const
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

void TuningSourceStateModel::onTimer1s()
{
	m_curTime = QDateTime::currentMSecsSinceEpoch();
}
