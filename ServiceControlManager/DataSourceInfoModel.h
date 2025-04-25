#pragma once

class DataSourceInfoModel : public QAbstractTableModel
{
public:
	DataSourceInfoModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void updateData(const Network::AppDataSourceState& state);

private:
	Network::AppDataSourceState m_state;

	inline static const std::vector<QString> m_rows =
		{
			QString("Module EquipmentID"),					// 0
			QString("Module caption"),						// 1
			QString("Module type"),							// 2
			QString("Module preset name"),					// 3
			QString("Module workcycle, mcs"),				// 4
			QString("RUP protocol version"),				// 5
			QString("Subsystem ID"),						// 6
			QString("Subsystem key"),						// 7
			QString("LM number"),							// 8
			QString("Subsystem channel"),					// 9
			QString("Lan controller ID"),					// 10
			QString("Lan controller IP"),					// 11
			QString("Lan controller data type"),			// 12
			QString("Expected DataUID"),					// 13
			QString("Data frames quantity"),				// 14
			QString("Data size, bytes"),					// 15
	};
};

