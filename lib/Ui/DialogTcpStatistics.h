#pragma once


class DialogTcpStatistics : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTcpStatistics(QWidget* parent);
	virtual ~DialogTcpStatistics();

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

	QTreeWidget* m_treeWidget = nullptr;
	int m_updateStateTimerId = -1;
};
