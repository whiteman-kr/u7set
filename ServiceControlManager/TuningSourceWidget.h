#pragma once

#include <QWidget>

class QTableView;
class QStandardItemModel;
class TcpTuningServiceClient;
class QSplitter;

class TuningSourceWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TuningSourceWidget(quint64 id, QString equipmentId, QString controllerEquipmentId, int controllerIndex, QWidget *parent = nullptr);
	~TuningSourceWidget();

	quint64 id() const { return m_id; }
	const QString& equipmentId() const { return m_equipmentId; }
	const QString& controllerEquipmentId() const { return m_controllerEquipmentId; }
signals:
	void forgetMe();

public slots:
	void updateStateFields();
	void setClientSocket(TcpTuningServiceClient* tcpClientSocket);
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

	TcpTuningServiceClient* m_tcpClientSocket = nullptr;
	quint64 m_id;

	QString m_equipmentId;
	QString m_controllerEquipmentId;
	int m_controllerIndex = 0;
};

