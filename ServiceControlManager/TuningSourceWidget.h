#ifndef TUNINGSOURCEWIDGET_H
#define TUNINGSOURCEWIDGET_H

#include <QWidget>

class QTableView;
class QStandardItemModel;
class TcpTuningServiceClient;

class TuningSourceWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TuningSourceWidget(quint64 id, QString equipmentId, QWidget *parent = nullptr);

	quint64 id() { return m_id; }
	QString equipmentId() { return m_equipmentId; }
signals:

public slots:
	void updateStateFields();
	void setClientSocket(TcpTuningServiceClient* tcpClientSocket);
	void unsetClientSocket();

protected:
	void closeEvent(QCloseEvent* event);

private:
	QTableView* m_stateTable = nullptr;
	QStandardItemModel* m_stateModel = nullptr;
	TcpTuningServiceClient* m_tcpClientSocket = nullptr;
	quint64 m_id;
	QString m_equipmentId;
};

#endif // TUNINGSOURCEWIDGET_H
