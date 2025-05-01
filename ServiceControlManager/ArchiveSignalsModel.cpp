#include "ArchiveSignalsModel.h"
#include "BaseServiceWidget.h"
#include "Brush.h"

#include "../ArchivingService/ArchFileRecord.h"

ArchiveSignalsModel::ArchiveSignalsModel(QWidget* parent) :
	QAbstractTableModel(parent)
{
}

const Columns& ArchiveSignalsModel::columns() const
{
	return m_columns;
}

int ArchiveSignalsModel::size() const
{
	return TO_INT(m_archSignals.size());
}

int ArchiveSignalsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_archSignals.size());
}

int ArchiveSignalsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_columns.size());
}

QVariant ArchiveSignalsModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::CheckStateRole ||
		role == Qt::DecorationRole ||
		role == Qt::EditRole ||
		role == Qt::FontRole)
	{
		return QVariant();
	}

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= TO_INT(m_archSignals.size()) ||
		column < 0 || column >= TO_INT(m_columns.size()))
	{
		return QVariant(Separator::EMPTY_STR);
	}

	const Network::ArchSignalInfo& asi = m_archSignals[row];

	if (role == Qt::BackgroundRole)
	{
		if (asi.apertureoverrided())
		{
			return YELLOW_BRUSH ;
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		E::SignalType st = static_cast<E::SignalType>(asi.signaltype());

		switch(st)
		{
		case E::Discrete:
			switch (column)
			{
			case 0:	return QString::fromStdString(asi.appsignalid());
			case 1: return E::valueToString(st);
			case 2: return QString::number(asi.recordspermin());
			case 3: return fineSize(asi.recordspermin() * sizeof(ArchFileRecord) * 60 *24);
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10: return Separator::EMPTY_STR;
			case 11: return (asi.apertureoverrided() ? QStringLiteral("Yes") : QStringLiteral("No"));
			}

		case E::Analog:
			switch (column)
			{
			case 0:	return QString::fromStdString(asi.appsignalid());
			case 1: return E::valueToString(st);
			case 2: return QString::number(asi.recordspermin());
			case 3: return fineSize(asi.recordspermin() * sizeof(ArchFileRecord) * 60 *24);
			case 4: return E::valueToString(static_cast<E::ApertureType>(asi.aperturetype()));
			case 5: return QString::number(asi.coarseaperture());
			case 6: return QString::number(asi.fineaperture());
			case 7: return QString::number(asi.abscoarseaperture());
			case 8: return QString::number(asi.absfineaperture());
			case 9: return QString::number(asi.lowengineeringunits());
			case 10: return QString::number(asi.highengineeringunits());
			case 11: return (asi.apertureoverrided() ? QStringLiteral("Yes") : QStringLiteral("No"));
			}

		default:
			;
		}

		return Separator::EMPTY_STR;
	}

	return QVariant("");
}

QVariant ArchiveSignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			if (section >= 0 && section < m_columns.size())
			{
				return m_columns[section].caption;
			}

			Q_ASSERT(false);

			return QVariant(QStringLiteral("???"));
		}
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ArchiveSignalsModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) return Qt::NoItemFlags;
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void ArchiveSignalsModel::updateData(const Network::ServiceInfo& srvInfo)
{
	int signalsCount = srvInfo.archsignalsinfo_size();

	int existSignalCount = TO_INT(m_archSignals.size());

	auto copyArchSignals = [&]()
	{
		for(int i = 0; i < signalsCount; i++)
		{
			m_archSignals[i] = srvInfo.archsignalsinfo(i);
		}

		int firstIndex = -1;
		int lastIndex = -1;
		int prevRecordsPerMin = -1;

		auto sortRange = [&]()
		{
			if (firstIndex !=-1 && lastIndex != -1 && firstIndex < lastIndex)
			{
				std::sort(m_archSignals.begin() + firstIndex,
						  m_archSignals.begin() + lastIndex + 1,
						  [](const Network::ArchSignalInfo& a, const Network::ArchSignalInfo& b)
						  {
							  return a.appsignalid() < b.appsignalid();
						  });
			}
		};

		// sorting signals with equal RecordsPerMin by AppSignalID
		//
		for(int i = 0; i < signalsCount; i++)
		{
			if (prevRecordsPerMin == -1)
			{
				prevRecordsPerMin = m_archSignals[i].recordspermin();
				firstIndex = i;
				lastIndex = i;
				continue;
			}

			if (m_archSignals[i].recordspermin() == prevRecordsPerMin)
			{
				lastIndex = i;
				continue;
			}

			sortRange();

			prevRecordsPerMin = m_archSignals[i].recordspermin();
			firstIndex = i;
			lastIndex = i;
		}

		sortRange();
	};

	if (signalsCount != existSignalCount)
	{
		if (signalsCount > existSignalCount)
		{
			beginInsertRows(QModelIndex(), existSignalCount, signalsCount - 1);
			m_archSignals.resize(signalsCount);
			copyArchSignals();
			endInsertRows();
		}
		else
		{
			beginRemoveRows(QModelIndex(), signalsCount, existSignalCount - 1);
			m_archSignals.resize(signalsCount);
			copyArchSignals();
			endRemoveRows();
		}

		beginInsertColumns(QModelIndex(), 0, TO_INT(m_columns.size()) - 1);
		endInsertColumns();
	}
	else
	{
		copyArchSignals();
	}

	emit dataChanged(QModelIndex(), QModelIndex());
}

QString ArchiveSignalsModel::fineSize(qint64 size) const
{
	if (size > 1024 * 1024 * 1024)
	{
		return QString("%1 GBytes").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
	}
	else
	{
		if (size > 1024 * 1024)
		{
			return QString("%1 MBytes").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
		}
		else
		{
			if (size > 1024)
			{
				return QString("%1 KBytes").arg(size / 1024.0, 0, 'f', 1);
			}
		}
	}

	return QString("%1 Bytes").arg(size);
}

const Network::ArchSignalInfo& ArchiveSignalsModel::at(int index)
{
	return m_archSignals[index];
}
