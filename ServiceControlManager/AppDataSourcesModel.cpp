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

	if (row < 0 || row >= TO_INT(m_sources.size()) ||
		column < 0 || column >= TO_INT(m_columns.size()))
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::DisplayRole)
	{
		const Network::AppDataSourceState& st = m_sources[row];

		switch (column)
		{
		case 0:	return QString::fromStdString(st.lancontrollerid());
		case 1: return HostAddressPort(st.lancontrollerip(), st.lancontrollerport()).toString();
		case 2: return st.receivesdata() ? QStringLiteral("Yes") : QStringLiteral("No");
		}

		if (st.receivesdata())
		{
			switch (column)
			{
			case 3: return formatUptime(st.uptime());
			case 4: return QString::number(st.datareceivingspeed());
			case 5: return QString::number(st.receivedpacketcount());
			case 6: return QString::number(st.lostpacketcount());
			case 7: return QString::number(	st.errorprotocolversion() +
											st.errorframesquantity() +
											st.errorframeno() +
											st.errorframecrc() +
											st.errordataid() +
											st.errorduplicateplanttime() +
											st.errornonmonotonicplanttime() +
											st.errorplanttimeformat());
			}
		}

		return Separator::EMPTY_STR;
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
	int existSourcesCount = TO_INT(m_sources.size());

	auto copySources = [&]()
	{
		for(int i = 0; i< sourcesCount; i++)
		{
			m_sources[i] = srvInfo.appdatasourcesstates(i);
		}
	};

	if (sourcesCount != existSourcesCount)
	{
		if (sourcesCount > existSourcesCount)
		{
			beginInsertRows(QModelIndex(), existSourcesCount, sourcesCount - 1);
			m_sources.resize(sourcesCount);
			copySources();
			endInsertRows();
		}
		else
		{
			beginRemoveRows(QModelIndex(), sourcesCount, existSourcesCount - 1);
			m_sources.resize(sourcesCount);
			copySources();
			endRemoveRows();
		}

		beginInsertColumns(QModelIndex(), 0, TO_INT(m_columns.size()) - 1);
		endInsertColumns();
	}
	else
	{
		copySources();
	}

	emit dataChanged(QModelIndex(), QModelIndex());
}
