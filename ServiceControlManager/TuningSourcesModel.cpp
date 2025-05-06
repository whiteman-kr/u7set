#include "TuningSourcesModel.h"
#include "BaseServiceWidget.h"
#include "Brush.h"

TuningSourcesModel::TuningSourcesModel(QWidget* parent) :
	QAbstractTableModel(parent)
{
}

const Columns& TuningSourcesModel::columns() const
{
	return m_columns;
}

int TuningSourcesModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_sources.size());
}

int TuningSourcesModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_columns.size());
}

QVariant TuningSourcesModel::data(const QModelIndex& index, int role) const
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

	if (row < 0 || row >= TO_INT(m_sources.size()) ||
		column < 0 || column >= TO_INT(m_columns.size()))
	{
		return QVariant(Separator::EMPTY_STR);
	}

	const Network::TuningSourceInfo& tsi = m_sources[row];
	const Network::DataSourceInfo& dsi = tsi.tuningsourceinfo();
	const Network::TuningSourceState& tss = tsi.tuningsourcestate();


	if (role == Qt::BackgroundRole)
	{
		if (m_sourcesErrorCount[row] > 0)
		{
			return YELLOW_BRUSH ;
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
/*		switch (column)
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
			case 7: return QString::number(m_sourcesErrorCount[row]);
			}
		}*/

		return Separator::EMPTY_STR;
	}

	return QVariant("");
}

QVariant TuningSourcesModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags TuningSourcesModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) return Qt::NoItemFlags;
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void TuningSourcesModel::updateData(const Network::ServiceInfo& srvInfo)
{
	int sourcesCount = srvInfo.appdatasourcesstates_size();
	int existSourcesCount = TO_INT(m_sources.size());

	auto copySources = [&]()
	{
		for(int i = 0; i < sourcesCount; i++)
		{
			const Network::TuningSourceInfo& st = srvInfo.tuningsourcesinfo(i);
			m_sources[i] = st;
		}

		std::sort(m_sources.begin(), m_sources.end(), [&](	const Network::TuningSourceInfo& a,
															const Network::TuningSourceInfo& b)
					{
						return a.tuningsourceinfo().moduleequipmentid() < b.tuningsourceinfo().moduleequipmentid();
					});

/*		for(int i = 0; i < sourcesCount; i++)
		{
			const Network::AppDataSourceState& st = m_sources[i];
			m_sourcesErrorCount[i] = st.errorprotocolversion() +
									 st.errorframesquantity() +
									 st.errorframeno() +
									 st.errorframecrc() +
									 st.errordataid() +
									 st.errorduplicateplanttime() +
									 st.errornonmonotonicplanttime() +
									 st.errorplanttimeformat();
		}*/
	};

	if (sourcesCount != existSourcesCount)
	{
		if (sourcesCount > existSourcesCount)
		{
			beginInsertRows(QModelIndex(), existSourcesCount, sourcesCount - 1);
			m_sources.resize(sourcesCount);
			m_sourcesErrorCount.resize(sourcesCount);
			copySources();
			endInsertRows();
		}
		else
		{
			beginRemoveRows(QModelIndex(), sourcesCount, existSourcesCount - 1);
			m_sources.resize(sourcesCount);
			m_sourcesErrorCount.resize(sourcesCount);
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

/*
QString TuningSourcesModel::getSourceLanControllerID(int index)
{
	if (index < 0 || index >= m_sources.size())
	{
		Q_ASSERT(false);
		return Separator::EMPTY_STR;
	}

	return QString::fromStdString(m_sources[index].lancontrollerid());
}*/
