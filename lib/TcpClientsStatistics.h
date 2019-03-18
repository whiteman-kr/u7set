#ifndef TCPCLINENTSSTATISTICS_H
#define TCPCLINENTSSTATISTICS_H

#include "../lib/Tcp.h"

class TcpClientInstance
{
protected:
	TcpClientInstance(Tcp::Client* client);
	virtual ~TcpClientInstance();

private:
	Tcp::Client* m_client = nullptr;
};

class TcpClientInstances
{
public:
	static QMutex m_mutex;
	static void addConnection(Tcp::Client* client);
	static void removeConnection(Tcp::Client* client);
	static std::vector<Tcp::Client*> clients();

private:
	static std::vector<Tcp::Client*> m_clients;
};

class DialogStatistics : public QDialog
{
	Q_OBJECT
public:
	DialogStatistics(QWidget* parent);
	~DialogStatistics();

signals:
	void dialogClosed();

protected:
	virtual void reject() override;

	void timerEvent(QTimerEvent* event);

private slots:
	//void onReconnect();

private:
	void update();

private:
	enum Columns
	{
		Caption,
		IsConnected,
		AddressPort,
		StartTime,
		SentKbytes,
		ReceivedKbytes,
		RequestCount,
		ReplyCount,

		ColumnCount
	};

	QTreeWidget* m_treeWidget = nullptr;
	int m_updateStateTimerId = -1;
};

#endif // DIALOGSTATISTICS_H
