#pragma once

class DataSourceStateModel : public QAbstractTableModel
{
public:
	DataSourceStateModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void updateData(const Network::AppDataSourceState& state);

private:
	Network::AppDataSourceState m_state;

	inline static const std::vector<QString> m_rows =
	{
			QString("Receives data"),						// 0
			QString("Uptime"),								// 1
			QString("LM time"),								// 2
			QString("RUP frame numerator"),					// 3
			QString("Data receiving speed, bytes/s"),		// 4
			QString("Received data size, bytes"),			// 5
			QString("Received frames count"),				// 6
			QString("Received packet count"),				// 7
			QString("Lost packet count"),					// 8
			QString("Received RUP protocol version"),		// 9
			QString("Received DataUID"),					// 10
			QString("Signal states queue current size"),	// 11
			QString("Signal states queue MAX size"),		// 12

			QString("Error RUP protocol version"),			// 13
			QString("Error frames quantity"),				// 14
			QString("Error frame numerator"),				// 15
			QString("Error frame CRC"),						// 16
			QString("Error DataUID"),						// 17
			QString("Error duplicate plant time"),			// 18
			QString("Error non-monotonic plant time"),		// 19
			QString("Error plant time format"),				// 20
	};
};

