#pragma once

namespace SchemaClientLib
{
	class TcpStatisticsWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TcpStatisticsWidget(bool showClose, QWidget* parent);

	protected:
		void timerEvent(QTimerEvent* event) override;

	private slots:
		void prepareContextMenu(const QPoint& pos);
		void reconnectAll();
		void update();

	signals:
		void closeClicked();

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
} // namespace SchemaClientLib