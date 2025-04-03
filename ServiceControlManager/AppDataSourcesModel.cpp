#include "AppDataSourcesModel.h"
#include "BaseServiceWidget.h"

AppDataSourcesModel::AppDataSourcesModel(QWidget* parent) :
	QAbstractTableModel(parent)
{
}

const Columns& AppDataSourcesModel::columns() const
{
	return m_columns;
}

int AppDataSourcesModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_sources.size());
}

int AppDataSourcesModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_columns.size());
}

QVariant AppDataSourcesModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::EditRole)
	{
		return QVariant(false);
	}

	Q_UNUSED(index);
	Q_UNUSED(role);

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= TO_INT(m_sources.size()) ||
		column < 0 || column >= TO_INT(m_columns.size()))
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::DisplayRole)
	{
		const Network::AppDataSourceState& st = m_sources[row];

		switch (index.column())
		{
		case 0:	return QString::fromStdString(st.lancontrollerid());
		case 1: return HostAddressPort(st.lancontrollerip(), st.lancontrollerport()).toString();

		default:
			Q_ASSERT(false);
		}
	}

	return QVariant("");
}

QVariant AppDataSourcesModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags AppDataSourcesModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) return Qt::NoItemFlags;
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void AppDataSourcesModel::updateData(const Network::ServiceInfo& srvInfo)
{
	int sourcesCount = srvInfo.appdatasourcesstates_size();

	if (sourcesCount != TO_INT(m_sources.size()))
	{
		if (sourcesCount > m_sources.size())
		{
			beginInsertRows(QModelIndex(), m_sources.size(), sourcesCount - 1);
			m_sources.resize(sourcesCount);
			endInsertRows();
		}
		else
		{
			beginRemoveRows(QModelIndex(), m_sources.size(), sourcesCount - 1);
			m_sources.resize(sourcesCount);
			endRemoveRows();
		}

		beginInsertColumns(QModelIndex(), 0, m_columns.size() - 1);
		endInsertColumns();
	}

	for(int i = 0; i< sourcesCount; i++)
	{
		m_sources[i] = srvInfo.appdatasourcesstates(i);
	}

	emit dataChanged(QModelIndex(), QModelIndex());
}
