#ifndef TUNINGSERVICEWIDGET_H
#define TUNINGSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"

class QStandardItemModel;
class TcpTuningServiceClient;

static const QStringList staticFieldsHeaderLabels {
	"Id",
	"EquipmentId",
	"Caption",
	"Ip",
	"Port",
	"Channel",
	"SubsystemID",
	"Subsystem",
	"LmNumber"
};


static const QStringList dynamicFieldsHeaderLabels {
	"IsReply",
	"ControlIsActive",
	"RequestCount",
	"ReplyCount",
};

class QStandardItemModel;

class TuningServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	TuningServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent = 0);

public slots:
	void updateStateInfo();
	void updateClientsInfo();
	void updateServiceSettings();
	void reloadTuningSourcesList();
	void updateTuningSourcesState();

	void clearServiceData();

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_tuningSourcesTabModel = nullptr;
	TcpTuningServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};

#endif // TUNINGSERVICEWIDGET_H
