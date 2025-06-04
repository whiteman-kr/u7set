#pragma once

class TuningSourceStateModel : public QAbstractTableModel
{
public:
	TuningSourceStateModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void updateData(const Network::TuningSourceInfoState& state);

private:
	void updateValueTime(int row, qint64 value);
	bool valueChanged(int row, bool receivesData) const;

	void onTimer1s();

private:
	Network::TuningSourceInfoState m_state;

	std::vector<std::pair<qint64, qint64>> m_valueTime;

	QTimer m_timer1s;
	qint64 m_curTime = 0;

	inline static const QVariant m_cleanVariant;

	inline static const std::vector<QString> m_rows =
	{
			QString("Source reply"),							// 0
			QString("LM time"),									// 1
			QString("Received DataUID"),						// 2
			QString("Request count"),							// 3
			QString("Reply count"),								// 4
			QString("Control is active"),						// 5
			QString("Set SOR"),									// 6
			QString("Writing disabled"),						// 7
			QString("Has unapplied params"),					// 8
			QString("FOTIP bounds check success"),				// 9
			QString("FOTIP write success"),						// 10
			QString("FOTIP apply success"),						// 11
			QString("Error untimely reply"),					// 12
			QString("Error sent"),								// 13
			QString("Error partial sent"),						// 14
			QString("Error reply size"),						// 15
			QString("Error no reply"),							// 16
			QString("Error tuning frame update"),				// 17
			QString("Error RUP protocol version"),				// 18
			QString("Error RUP frame size"),					// 19
			QString("Error RUP no tuning data"),				// 20
			QString("Error RUP module type"),					// 21
			QString("Error RUP frames quantity"),				// 22
			QString("Error RUP frame No"),						// 23
			QString("Error RUP frame CRC"),						// 24
			QString("Error RUP DataUID"),						// 25
			QString("Error RUP duplicate plant time"),			// 26
			QString("Error RUP non-monotonic plant time"),		// 27
			QString("Error RUP plant time format"),				// 28
			QString("Error FOTIP protocol version"),			// 29
			QString("Error FOTIP data UID"),					// 30
			QString("Error FOTIP LM number"),					// 31
			QString("Error FOTIP subsystem code"),				// 32
			QString("Error FOTIP operation code"),				// 33
			QString("Error FOTIP tuning frame size"),			// 34
			QString("Error FOTIP tuning ROM size"),				// 35
			QString("Error FOTIP tuning ROM frame size"),		// 36
			QString("Error FOTIP analog low bound check"),		// 37
			QString("Error FOTIP analog high bound check"),		// 38
	};
};
