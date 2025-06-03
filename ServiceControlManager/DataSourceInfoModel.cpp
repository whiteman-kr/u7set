#include "DataSourceInfoModel.h"
#include <CommonLib/HostAddressPort.h>
#include <HardwareLib/DataProtocols.h>

DataSourceInfoModel::DataSourceInfoModel()
{
}

int DataSourceInfoModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_rows.size());
}

int DataSourceInfoModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant DataSourceInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant DataSourceInfoModel::data(const QModelIndex& index, int role) const
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

	if (row < 0 || row >= TO_INT(m_rows.size()) ||
		column < 0 || column >= 2)
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::DisplayRole)
	{
		if (column == 0)
		{
			return m_rows[row];
		}

		switch (row)
		{
		case 0:	return m_moduleEqupmentID;
		case 1: return m_moduleCaption;
		case 2: return m_moduleType;
		case 3: return m_modulePresetName;
		case 4: return m_moduleWorkcycleMcs;
		case 5: return m_subsystemID;
		case 6: return m_subsystemKey;
		case 7: return m_lmNumber;
		case 8: return m_subsystemChannel;
		case 9: return m_rupProtocolVersion;
		case 10: return m_expectedDataID;
		case 11: return m_dataFramesQuantity;
		case 12: return m_dataSizeBytes;
		case 13: return m_lanControllerID;
		case 14: return m_lanControllerIP;
		}

		return Separator::EMPTY_STR;
	}

	return QVariant(Separator::EMPTY_STR);
}

void DataSourceInfoModel::updateData(const Network::AppDataSourceState& state)
{
	m_moduleEqupmentID = QString::fromStdString(state.lmequipmentid());
	m_moduleCaption = QString::fromStdString(state.lmcaption());
	m_moduleType = QString("0x%1  (%2)").arg(QString("%1").arg(state.moduletype(), 4, 16, Latin1Char::ZERO).toUpper()).
															arg(state.moduletype());
	m_modulePresetName = QString::fromStdString(state.modulepresetname());
	m_moduleWorkcycleMcs = state.moduleworkcyclemcs();
	m_subsystemID = QString::fromStdString(state.subsystemid());
	m_subsystemKey = state.subsystemkey();
	m_lmNumber = state.lmnumber();
	m_subsystemChannel = QString::fromStdString(state.subsystemchannel());
	m_rupProtocolVersion = state.rupprotocolversion();
	m_expectedDataID = QString("0x%1  (%2)").arg(QString("%1").arg(state.expecteddataid(), 8, 16, Latin1Char::ZERO).toUpper()).
																arg(state.expecteddataid());
	m_dataFramesQuantity = state.dataframesquantity();
	m_dataSizeBytes = state.datasizebytes();
	m_lanControllerID = QString::fromStdString(state.lancontrollerid());
	m_lanControllerIP = HostAddressPort(state.lancontrollerip(), state.lancontrollerport()).toString();

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}

void DataSourceInfoModel::updateData(const Network::TuningSourceInfoState& infoState)
{
	const Network::DataSourceInfo& info = infoState.info();

	m_moduleEqupmentID = QString::fromStdString(info.moduleequipmentid());
	m_moduleCaption = QString::fromStdString(info.modulecaption());
	m_moduleType = QString("0x%1  (%2)").arg(QString("%1").arg(info.moduletype(), 4, 16, Latin1Char::ZERO).toUpper()).
			   arg(info.moduletype());
	m_modulePresetName = QString::fromStdString(info.modulepresetname());
	m_moduleWorkcycleMcs = info.workcycle_mcs();
	m_subsystemID = QString::fromStdString(info.subsystemid());
	m_subsystemKey = info.subsystemkey();
	m_lmNumber = info.lmnumber();
	m_subsystemChannel = QString::fromStdString(info.subsystemchannel());

	if (info.lancontrollerinfo_size() > 0)
	{
		const Network::LanControllerInfo& lci = info.lancontrollerinfo(0);

		m_rupProtocolVersion = info.rupprotocolversion();

		m_expectedDataID = QString("0x%1  (%2)").arg(QString("%1").arg(lci.ruptuningdatauid(), 8, 16, Latin1Char::ZERO).toUpper()).
						   arg(lci.ruptuningdatauid());
		m_dataFramesQuantity = 1;
		m_dataSizeBytes = sizeof(Rup::Data);
		m_lanControllerID = QString::fromStdString(lci.equipmentid());
		m_lanControllerIP = QString("%1:%2").arg(QString::fromStdString(lci.tuningip())).arg(lci.tuningport());
	}

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}
