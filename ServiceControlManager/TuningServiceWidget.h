#ifndef TUNINGSERVICEWIDGET_H
#define TUNINGSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"

class QStandardItemModel;
class TcpTuningServiceClient;

static const QStringList staticFieldsHeaderLabels {
	QStringLiteral("EquipmentId"),
	QStringLiteral("Caption"),
	QStringLiteral("Ip"),
	QStringLiteral("Port"),
	QStringLiteral("Channel"),
	QStringLiteral("SubsystemID"),
	QStringLiteral("Subsystem"),
	QStringLiteral("LmNumber")
};


static const QStringList dynamicFieldsHeaderLabels {
	QStringLiteral("IsReply"),
	QStringLiteral("ControlIsActive"),
	QStringLiteral("SetSOR"),
	QStringLiteral("RequestCount"),
	QStringLiteral("ReplyCount"),
};

class QStandardItemModel;
class TuningSourceWidget;

class TuningServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	TuningServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent = 0);
	~TuningServiceWidget();

signals:
	void newTcpClientSocket(TcpTuningServiceClient* tcpClientSocket);

public slots:
	void updateStateInfo();
	void updateClientsInfo();
	void updateServiceSettings();
	void reloadTuningSourcesList();
	void updateTuningSourcesState();

	void clearServiceData();

	void onTuningSourceDoubleClicked(const QModelIndex &index);

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_tuningSourcesTabModel = nullptr;
	TcpTuningServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
	QList<TuningSourceWidget*> m_tuningSourceWidgetList;
};

#endif // TUNINGSERVICEWIDGET_H
