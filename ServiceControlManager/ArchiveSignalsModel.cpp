#include "ArchiveSignalsModel.h"
#include "BaseServiceWidget.h"

ArchiveSignalsModel::ArchiveSignalsModel(QWidget* parent) :
	QAbstractTableModel(parent)
{
}

const Columns& ArchiveSignalsModel::columns() const
{
	return m_columns;
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
		role == Qt::EditRole)
	{
		return QVariant();
	}

	Q_UNUSED(index);
	Q_UNUSED(role);

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= TO_INT(m_archSignals.size()) ||
		column < 0 || column >= TO_INT(m_columns.size()))
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::DisplayRole)
	{
		const Network::ArchSignalInfo& asi = m_archSignals[row];

		switch (column)
		{
		case 0:	return QString::fromStdString(asi.appsignalid());
		case 1: return QString::number(asi.recordspermin());
		case 2: return E::valueToString(static_cast<E::ApertureType>(asi.aperturetype()));
		case 3: return QString::number(asi.coarseaperture());
		case 4: return QString::number(asi.fineaperture());
		case 5: return (asi.apertureoverrided() ? QStringLiteral("Yes") : QStringLiteral("No"));
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

	if (signalsCount != TO_INT(m_archSignals.size()))
	{
		if (signalsCount > m_archSignals.size())
		{
			beginInsertRows(QModelIndex(), TO_INT(m_archSignals.size()), signalsCount - 1);
			m_archSignals.resize(signalsCount);
			endInsertRows();
		}
		else
		{
			beginRemoveRows(QModelIndex(), TO_INT(m_archSignals.size()), signalsCount - 1);
			m_archSignals.resize(signalsCount);
			endRemoveRows();
		}

		beginInsertColumns(QModelIndex(), 0, TO_INT(m_columns.size()) - 1);
		endInsertColumns();
	}

	for(int i = 0; i< signalsCount; i++)
	{
		m_archSignals[i] = srvInfo.archsignalsinfo(i);
	}

	emit dataChanged(QModelIndex(), QModelIndex());
}
