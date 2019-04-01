#pragma once
#include <set>
#include <vector>
#include "../lib/Tcp.h"


class TcpClientStatistics
{
protected:
	TcpClientStatistics() = delete;
	TcpClientStatistics(Tcp::Client* client);
	virtual ~TcpClientStatistics();

public:
	struct Statisctics
	{
		Statisctics(size_t _id, QString _objectName, Tcp::ConnectionState _state) :
			id(_id),
			objectName(_objectName),
			state(_state)
		{
		}

		size_t id;		// is a pointer to TcpClientInstance
		QString objectName;
		Tcp::ConnectionState state;
	};

	static std::vector<Statisctics> statistics();
	static void reconnect(uintptr_t id);

private:
	Tcp::Client* m_client = nullptr;

	static QMutex s_mutex;
	static std::set<Tcp::Client*> s_clients;
};


class DialogStatistics : public QDialog
{
	Q_OBJECT

public:
	explicit DialogStatistics(QWidget* parent);
	virtual ~DialogStatistics();

public slots:
	void prepareContextMenu(const QPoint& pos);

signals:
	void dialogClosed();

protected:
	virtual void reject() override;

	void timerEvent(QTimerEvent* event);

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
