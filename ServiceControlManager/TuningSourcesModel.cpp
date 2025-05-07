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

	const Network::TuningSourceInfoState& tsi = m_sources[row];
	const Network::DataSourceInfo& dsi = tsi.info();
	const Network::TuningSourceState& tss = tsi.state();

	if (role == Qt::BackgroundRole)
	{
		if (tss.isreply() == false)
		{
			return YELLOW_BRUSH ;
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		switch (column)
		{
		case 0:	return QString::fromStdString(dsi.moduleequipmentid());
		case 1:
			{
				int count = dsi.lancontrollerinfo_size();

				for(int i = 0; i < count; i++)
				{
					const Network::LanControllerInfo& lci = dsi.lancontrollerinfo(i);

					if (lci.tuningenable() == true)
					{
						return QString("%1:%2").arg(QString::fromStdString(lci.tuningip())).arg(lci.tuningport());
					}
				}

				return Separator::EMPTY_STR;
			}
		case 2: return tss.isreply() ? Separator::YES : Separator::NO;
		case 3: return TO_INT64(tss.requestcount());
		case 4: return TO_INT64(tss.errnoreply());
		default: ;
		}

		if (tss.isreply())
		{
			switch(column)
			{
			case 5: return tss.controlisactive() ? Separator::YES : Separator::NO;
			case 6: return tss.writingdisabled() ? Separator::YES : Separator::NO;
			case 7: return tss.hasunappliedparams() ? Separator::YES : Separator::NO;
			case 8: return tss.setsor() ? Separator::YES : Separator::NO;
			case 9: return m_sourcesErrorCount[row];
			default: ;
			}
		}

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
	int sourcesCount = srvInfo.tuningsourcesinfostate_size();
	int existSourcesCount = TO_INT(m_sources.size());

	auto copySources = [&]()
	{
		for(int i = 0; i < sourcesCount; i++)
		{
			const Network::TuningSourceInfoState& st = srvInfo.tuningsourcesinfostate(i);
			m_sources[i] = st;
		}

		std::sort(m_sources.begin(), m_sources.end(), [&](	const Network::TuningSourceInfoState& a,
															const Network::TuningSourceInfoState& b)
					{
						return a.info().moduleequipmentid() < b.info().moduleequipmentid();
					});

		for(int i = 0; i < sourcesCount; i++)
		{
			const Network::TuningSourceState& st = m_sources[i].state();

			m_sourcesErrorCount[i] =
				st.errrupprotocolversion() +
				st.errrupframesize() +
				st.errrupnontuningdata() +
				st.errrupmoduletype() +
				st.errrupframesquantity() +
				st.errrupframenumber() +
				st.errrupcrc() +
				st.errfotipprotocolversion() +
				st.errfotipuniqueid() +
				st.errfotiplmnumber() +
				st.errfotipsubsystemcode() +
				st.errfotipoperationcode() +
				st.errfotipframesize() +
				st.errfotipromsize() +
				st.errfotipromframesize() +
				st.erranaloglowboundcheck() +
				st.erranaloghighboundcheck() +
				st.erruntimelyreplay() +
				st.errsent() +
				st.errpartialsent() +
				st.errreplysize() +
				//st.errnoreply() +
				st.errtuningframeupdate();
		}
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
