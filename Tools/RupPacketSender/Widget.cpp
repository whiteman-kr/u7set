#include "Widget.h"
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QFormLayout>
#include <QComboBox>
#include <QNetworkInterface>
#include <QSettings>
#include <QUdpSocket>
#include "../../include/DataProtocols.h"

Widget::Widget(QWidget *parent)
	: QWidget(parent),
	  m_sourceAddressCombo(new QComboBox(this)),
	  m_sourcePortEdit(new QLineEdit(this)),
	  m_destinationAddressEdit(new QLineEdit(this)),
	  m_destinationPortEdit(new QLineEdit(this)),
	  m_socket(new QUdpSocket(this)),
	  numerator(0)
{
	QSettings settings;
	numerator = settings.value("numerator", numerator).toInt();

	connect(m_sourceAddressCombo, &QComboBox::currentTextChanged, this, &Widget::rebindSocket);

	QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
	for (int i = 0; i < interfaceList.count(); i++)
	{
		if (interfaceList[i].flags() & QNetworkInterface::IsLoopBack)
		{
			continue;
		}
		QList<QNetworkAddressEntry> addressList = interfaceList[i].addressEntries();
		for (int j = 0; j < addressList.count(); j++)
		{
			QHostAddress ip = addressList[j].ip();
			if (ip.protocol() != QAbstractSocket::IPv4Protocol)
			{
				continue;
			}
			QString ipStr = ip.toString();
			if (addressList[j].prefixLength() >= 0)
			{
				ipStr += "/" + QString::number(addressList[j].prefixLength());
			}
			m_sourceAddressCombo->addItem(ipStr);
		}
	}

	m_sourceAddressCombo->setCurrentText(settings.value("sourceAddress", "").toString());
	rebindSocket();

	QIntValidator* portValidator = new QIntValidator(2000, 65535, this);
	m_sourcePortEdit->setValidator(portValidator);
	m_sourcePortEdit->setText(settings.value("sourcePort", "2000").toString());
	connect(m_sourcePortEdit, &QLineEdit::textChanged, this, &Widget::rebindSocket);

	QRegExp re("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:/(?:[12]?[0-9]|3[0-2]?)?)\\b");
	QRegExpValidator* ipValidator = new QRegExpValidator(re, this);
	m_destinationAddressEdit->setValidator(ipValidator);
	m_destinationAddressEdit->setText(settings.value("destinationAddress", m_sourceAddressCombo->currentText().split("/")[0]).toString());

	m_destinationPortEdit->setValidator(portValidator);
	m_destinationPortEdit->setText(settings.value("destinationPort", "2000").toString());

	QPushButton* sendButton = new QPushButton("Send RUP packet", this);
	connect(sendButton, &QPushButton::clicked, this, &Widget::sendPacket);

	QFormLayout* fl = new QFormLayout;
	fl->addRow("Source address", m_sourceAddressCombo);
	fl->addRow("Source port", m_sourcePortEdit);
	fl->addRow("Destination address", m_destinationAddressEdit);
	fl->addRow("Destination port", m_destinationPortEdit);
	fl->addWidget(sendButton);

	setLayout(fl);
}

Widget::~Widget()
{
	QSettings settings;
	settings.setValue("sourceAddress", m_sourceAddressCombo->currentText());
	settings.setValue("sourcePort", m_sourcePortEdit->text());
	settings.setValue("destinationAddress", m_destinationAddressEdit->text());
	settings.setValue("destinationPort", m_destinationPortEdit->text());
	settings.setValue("numerator", numerator);
}

void Widget::sendPacket()
{
	if (m_socket->state() != QUdpSocket::BoundState)
	{
		rebindSocket();
	}
	RupFrame frame;
	memset(&frame, 0, sizeof(RupFrame));

	RupFrameHeader& header = frame.header;
	header.frameSize = ENTIRE_UDP_SIZE;
	header.protocolVersion = 4;
	header.flags.tuningData = 1;
	header.numerator = numerator++;
	header.framesQuantity = 1;
	header.frameNumber = 0;

	QDateTime&& time = QDateTime::currentDateTime();
	RupTimeStamp& timeStamp = header.TimeStamp;
	timeStamp.year = time.date().year();
	timeStamp.month = time.date().month();
	timeStamp.day = time.date().day();

	timeStamp.hour = time.time().hour();
	timeStamp.minute = time.time().minute();
	timeStamp.second = time.time().second();
	timeStamp.millisecond = time.time().msec();

	m_socket->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(RupFrame), QHostAddress(m_destinationAddressEdit->text()), m_destinationPortEdit->text().toInt());
}

void Widget::rebindSocket()
{
	bool ok;
	int port = m_sourcePortEdit->text().toInt(&ok);
	if (!ok)
	{
		return;
	}
	QString address = m_sourceAddressCombo->currentText().split("/")[0];
	m_socket->bind(QHostAddress(address), port);
}
