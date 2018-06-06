#ifndef TUNINGSERVICEWIDGET_H
#define TUNINGSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"

class QStandardItemModel;
class TcpTuningServiceClient;

static const QStringList tuningSourceStaticFieldsHeaderLabels {
	QStringLiteral("EquipmentId"),
	QStringLiteral("Caption"),
	QStringLiteral("Ip"),
	QStringLiteral("Port"),
	QStringLiteral("Channel"),
	QStringLiteral("SubsystemID"),
	QStringLiteral("Subsystem"),
	QStringLiteral("LmNumber")
};


static const QStringList tuningSourceDynamicFieldsHeaderLabels {
	QStringLiteral("IsReply"),
	QStringLiteral("ControlIsActive"),
	QStringLiteral("SetSOR"),
	QStringLiteral("RequestCount"),
	QStringLiteral("ReplyCount"),
};

static const QStringList tuningSignalsStaticFieldsHeaderLabels {
	QStringLiteral("Custom AppSignal ID"),
	QStringLiteral("Equipment ID"),
	QStringLiteral("App Signal ID"),
	QStringLiteral("Caption"),
	QStringLiteral("Units"),
	QStringLiteral("Type"),
	QStringLiteral("Default"),
};

static const QStringList tuningSignalsDynamicFieldsHeaderLabels {
	QStringLiteral("Value"),
	QStringLiteral("LowLimit"),
	QStringLiteral("HighLimit"),
	QStringLiteral("Valid"),
	QStringLiteral("Underflow"),
	QStringLiteral("Overflow"),
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
	void clearTcpClientSocket();

public slots:
	void updateStateInfo();
	void updateClientsInfo();
	void updateServiceSettings();
	void reloadTuningSourcesList();
	void updateTuningSourcesState();
	void reloadTuningSignalsList();
	void updateTuningSignalsState();

	void clearServiceData();

	void onTuningSourceDoubleClicked(const QModelIndex &index);

	void forgetWidget();

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_tuningSourcesTabModel = nullptr;
	QStandardItemModel* m_tuningSignalsTabModel = nullptr;
	TcpTuningServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
	QList<TuningSourceWidget*> m_tuningSourceWidgetList;
};

#endif // TUNINGSERVICEWIDGET_H
