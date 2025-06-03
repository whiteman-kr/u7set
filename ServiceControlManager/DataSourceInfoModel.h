#pragma once

class DataSourceInfoModel : public QAbstractTableModel
{
public:
	DataSourceInfoModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void updateData(const Network::AppDataSourceState& state);
	void updateData(const Network::TuningSourceInfoState& infoState);

private:
	QString m_moduleEqupmentID;
	QString m_moduleCaption;
	QString m_moduleType;
	QString m_modulePresetName;
	int m_moduleWorkcycleMcs = 0;
	QString m_subsystemID;
	int m_subsystemKey = 0;
	int m_lmNumber = 0;
	QString m_subsystemChannel;
	int m_rupProtocolVersion = 0;
	QString m_expectedDataID = 0;
	int m_dataFramesQuantity = 0;
	int m_dataSizeBytes = 0;
	QString m_lanControllerID;
	QString m_lanControllerIP;

	inline static const std::vector<QString> m_rows =
		{
			QString("Module EquipmentID"),					// 0
			QString("Module caption"),						// 1
			QString("Module type"),							// 2
			QString("Module preset name"),					// 3
			QString("Module workcycle, mcs"),				// 4
			QString("Subsystem ID"),						// 5
			QString("Subsystem key"),						// 6
			QString("LM number"),							// 7
			QString("Subsystem channel"),					// 8
			QString("RUP protocol version"),				// 9
			QString("Expected DataUID"),					// 10
			QString("Data frames quantity"),				// 11
			QString("Data size, bytes"),					// 12
			QString("Lan controller ID"),					// 13
			QString("Lan controller IP"),					// 14
	};
};

