#pragma once

#include <QWidget>

class QTableView;
class QStandardItemModel;
class TcpAppDataClient;
class QSplitter;

class AppDataSourceWidget : public QWidget
{
	Q_OBJECT
public:
	explicit AppDataSourceWidget(quint64 id, QString equipmentId, QWidget *parent = nullptr);
	~AppDataSourceWidget();

	quint64 id() { return m_id; }
	QString equipmentId() { return m_equipmentId; }
signals:
	void forgetMe();

public slots:
	void updateStateFields();
	void setClientSocket(TcpAppDataClient* tcpClientSocket);
	void unsetClientSocket();

protected:
	void closeEvent(QCloseEvent* event);

private:
	void initTable(QTableView* table, QStandardItemModel* model);

private:
	QTableView* m_infoTable = nullptr;
	QStandardItemModel* m_infoModel = nullptr;

	QTableView* m_stateTable = nullptr;
	QStandardItemModel* m_stateModel = nullptr;

	QSplitter* m_splitter = nullptr;

	TcpAppDataClient* m_tcpClientSocket = nullptr;
	quint64 m_id;
	QString m_equipmentId;
};

