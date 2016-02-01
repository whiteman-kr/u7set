#ifndef SCANOPTIONSWIDGET_H
#define SCANOPTIONSWIDGET_H

#include <QDialog>
#include <QSet>
#include <QNetworkAddressEntry>
#include "../include/SocketIO.h"
#include "../include/Service.h"

class QLineEdit;
class ServiceTableModel;
class QUdpSocket;
//class QTimer;

class SubnetChecker : public QObject
{
	Q_OBJECT
public:
    explicit SubnetChecker(QList<QPair<quint32,uint>>& subnetList, QObject *parent = 0);

signals:
	void setChecked(int count);
	void hostFound(quint32 ip, quint16 port, ServiceInformation serviceInfo);

public slots:
	void startChecking();
	void checkNextHost();
	void stopChecking();

	void readAck();

private:
	QList<QPair<quint32,uint>> m_subnetList;
	int m_subnetIndex = 0;
	quint32 m_ip;
	quint32 m_maxSubnetIp;
	int m_checkedHostCount = 1;
	RequestHeader m_requestHeader;
	QUdpSocket* m_socket;
	QTimer* m_sendPacketTimer;

	void setSubnet(int index);
};

class ScanOptionsWidget : public QDialog
{
    Q_OBJECT
public:
	explicit ScanOptionsWidget(ServiceTableModel* serviceModel, QWidget *parent = 0);

    QString getSelectedAddress();

signals:

public slots:
	void startChecking();

private:
    QLineEdit* m_addressEdit;
	QList<QNetworkAddressEntry> m_addressEntryList;
	ServiceTableModel* m_serviceModel;
};

#endif // SCANOPTIONSWIDGET_H
