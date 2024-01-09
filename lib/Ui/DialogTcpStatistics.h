#pragma once


class DialogTcpStatistics : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTcpStatistics(QWidget* parent);
	virtual ~DialogTcpStatistics() = default;

public slots:
	void prepareContextMenu(const QPoint& pos);

signals:
	void dialogClosed();

protected:
	virtual void reject() override;

	void timerEvent(QTimerEvent* event) override;

private slots:
	void reconnectAll();

private:
	void update();

private:
	enum class Columns
	{
		Caption,
		ServerID,
		IsConnected,
		AddressPort,
		StartTime,
		UpTime,
		SentKbytes,
		ReceivedKbytes,
		RequestCount,
		ReplyCount,

		ColumnCount
	};

	enum class ColumnUserData
	{
		Index,
		StatId
	};




	QTreeWidget* m_treeWidget = nullptr;
	int m_updateStateTimerId = -1;
};
