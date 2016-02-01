#include "scanoptionswidget.h"
#include "servicetablemodel.h"
#include <QRegExpValidator>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QNetworkInterface>
#include <QDialogButtonBox>
#include <QThread>
#include <QProgressDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>

ScanOptionsWidget::ScanOptionsWidget(ServiceTableModel* serviceModel, QWidget *parent) :
	QDialog(parent),
	m_serviceModel(serviceModel)
{
    setWindowTitle(tr("Scan settings"));
	QRegExp re("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:/(?:[12]?[0-9]|3[0-2]?)?)\\b");
	QRegExpValidator* rev = new QRegExpValidator(re, this);
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setValidator(rev);

    QFormLayout* fl = new QFormLayout;
    fl->addRow(tr("Enter IP or subnet"), m_addressEdit);

    QComboBox* addressCombo = new QComboBox;

    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaceList.count(); i++)
    {
        QList<QNetworkAddressEntry> addressList = interfaceList[i].addressEntries();
        for (int j = 0; j < addressList.count(); j++)
        {
            QHostAddress ip = addressList[j].ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol)
            {
                continue;
            }
			m_addressEntryList.append(addressList[j]);
            QString ipStr = ip.toString();
            if (addressList[j].prefixLength() >= 0)
            {
				ipStr += "/" + QString::number(addressList[j].prefixLength());
            }
            addressCombo->addItem(ipStr);
        }
    }

	QSettings settings;
	m_addressEdit->setText(settings.value("last scan target", addressCombo->currentText()).toString());

	connect(addressCombo, &QComboBox::currentTextChanged, m_addressEdit, &QLineEdit::setText);

    fl->addRow(tr("Or select from following"), addressCombo);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ScanOptionsWidget::startChecking);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ScanOptionsWidget::reject);
    fl->addRow(buttonBox);

    setLayout(fl);
}

QString ScanOptionsWidget::getSelectedAddress()
{
	return m_addressEdit->text();
}

void ScanOptionsWidget::startChecking()
{
	QStringList address = getSelectedAddress().split("/");
	if (address.count() == 1)
	{
		m_serviceModel->addAddress(address[0]);
	}
	else
	{
		QHostAddress ha(address[0]);
		quint32 ip = ha.toIPv4Address();
		int prefixLength = address[1].toUInt();
		if (prefixLength > 32)
		{
			prefixLength = 32;
		}

		//Subnet intersection have same mask by min prefix length and max result prefix length
		//
		int hostCount = 0;
		QList<QPair<quint32,uint>> subnetList;
		foreach (QNetworkAddressEntry entry, m_addressEntryList)
		{
			int entryPrefixLength = entry.prefixLength();
			if (entryPrefixLength == -1)
			{
				entryPrefixLength = 32;
			}
			quint32 entryIp = entry.ip().toIPv4Address();
			uint minPrefixLength = qMin(prefixLength, entryPrefixLength);
			quint32 mask = minPrefixLength == 0 ? 0 : (~0u << (32 - minPrefixLength));
			if ((ip & mask) != (entryIp & mask))
			{
				continue;
			}
			if (prefixLength > entryPrefixLength)
			{
				subnetList.append(QPair<quint32,uint>(ip, prefixLength));
				hostCount += 1u << (32 - prefixLength);
			}
			else
			{
				subnetList.append(QPair<quint32,uint>(entryIp, entryPrefixLength));
				hostCount += 1u << (32 - entryPrefixLength);
			}
		}

		if (hostCount == 0)
		{
			QMessageBox::warning(this, tr("Wrong subnet selected"), tr("Specify contrete ip address or subnet with not null intersection with local subnets"));
			return;
		}

		// Go!
		SubnetChecker* checker = new SubnetChecker(subnetList);

		QThread* thread = new QThread;
		checker->moveToThread(thread);

		QProgressDialog* progress = new QProgressDialog(tr("Scanning network..."), tr("Cancel"), 0, hostCount, parentWidget());
		progress->setMinimumDuration(0);

		connect(checker, &SubnetChecker::setChecked, progress, &QProgressDialog::setValue, Qt::QueuedConnection);
		connect(checker, &SubnetChecker::hostFound, m_serviceModel, &ServiceTableModel::setServiceInformation, Qt::QueuedConnection);
		connect(progress, &QProgressDialog::canceled, checker, &SubnetChecker::stopChecking, Qt::QueuedConnection);
		connect(progress, &QProgressDialog::canceled, progress, &QObject::deleteLater);
		connect(progress, &QProgressDialog::rejected, progress, &QObject::deleteLater);
		connect(thread, &QThread::started, checker, &SubnetChecker::startChecking, Qt::QueuedConnection);
		connect(thread, &QThread::finished, checker, &QObject::deleteLater);
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		thread->start();

		progress->open();
	}

	QSettings settings;
	settings.setValue("last scan target", m_addressEdit->text());

	accept();
}


SubnetChecker::SubnetChecker(QList<QPair<quint32,uint>>& subnetList, QObject* parent) :
	QObject(parent),
	m_subnetList(std::move(subnetList)),
	m_socket(nullptr)
{
	setSubnet(0);
}

void SubnetChecker::startChecking()
{
	m_socket = new QUdpSocket(this);
	connect(m_socket, &QUdpSocket::readyRead, this, &SubnetChecker::readAck);

	m_requestHeader.clientID = 0;
	m_requestHeader.version = 0;
	m_requestHeader.no = 1;
	m_requestHeader.errorCode = RQERROR_OK;
	m_requestHeader.id = RQID_SERVICE_GET_INFO;
	m_requestHeader.dataSize = 0;

	m_sendPacketTimer = new QTimer(this);
	connect(m_sendPacketTimer, SIGNAL(timeout()), this, SLOT(checkNextHost()));
	m_sendPacketTimer->start(5);
}

void SubnetChecker::checkNextHost()
{
	if (m_socket->state() == QUdpSocket::BoundState && m_socket->hasPendingDatagrams())
	{
		readAck();
	}
	for (uint i = 0; i < SERVICE_TYPE_COUNT; i++)
	{
		QHostAddress ip(m_ip);
		qint64 sent = m_socket->writeDatagram((char*)&m_requestHeader, sizeof(m_requestHeader),
											  ip, serviceInfo[i].port);
		if (sent == -1)
	    {
			if (m_socket->error() == QAbstractSocket::NetworkError)
			{
				//Looks like buffer overflow
				thread()->msleep(5);
				return;
			}
			qDebug() << "Socket error " << m_socket->error() << "(ip=" << ip.toString() << "): " << m_socket->errorString();
	    }
    }
	//Select next ip
	//
	m_ip++;
	if (m_ip > m_maxSubnetIp)
	{
		m_subnetIndex++;
		if (m_subnetIndex >= m_subnetList.count())
		{
			m_sendPacketTimer->stop();
			QTimer::singleShot(100, this, SLOT(stopChecking())); //100ms for answers on last packets
			return;
		}
		setSubnet(m_subnetIndex);
	}
	m_checkedHostCount++;
	emit setChecked(m_checkedHostCount);
}

void SubnetChecker::stopChecking()
{
	thread()->quit();
}

void SubnetChecker::readAck()
{
	while (m_socket->hasPendingDatagrams())
	{
		QHostAddress sender;
		quint16 senderPort;
		AckGetServiceInfo ack;

		qint64 size = m_socket->readDatagram((char*)&ack, sizeof(ack), &sender, &senderPort);
		if (size == sizeof(ack))
		{
			emit hostFound(sender.toIPv4Address(), senderPort, ack.serviceInfo);
		}
	}
}

void SubnetChecker::setSubnet(int index)
{
	if (index >= m_subnetList.count())
	{
		assert(false);
		return;
	}
	m_subnetIndex = index;
	quint32 ip = m_subnetList[index].first;
	uint prefixLength = m_subnetList[index].second;
	quint32 mask = ~0u << (32 - prefixLength);
	m_ip = ip & mask;
	m_maxSubnetIp = m_ip + ~mask;
}
