#include "Widget.h"
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QNetworkInterface>
#include <QSettings>
#include <QUdpSocket>
#include "../../include/DataProtocols.h"

#pragma pack(push, 1)
struct FotipHeader
{
	quint16 protocolVersion;			// frame size including header, bytes
	quint64 uniqueId;
	quint16 subsystemKey;
	quint16 operationCode;
	quint16 flags;
	quint32 startAddress;
	quint16 fotipFrameSize;
	quint32 romSize;
	quint16 romFrameSize;
	quint16 dataType;
};
#pragma pack(pop)

const int dataTypes[] =
{
	1300,
	1500,
	1700
};

Widget::Widget(QWidget *parent)
	: QWidget(parent),
	  m_sourceAddressCombo(new QComboBox(this)),
	  m_sourcePortEdit(new QLineEdit(this)),
	  m_destinationAddressEdit(new QLineEdit(this)),
	  m_destinationPortEdit(new QLineEdit(this)),
	  m_moduleIdEdit(new QLineEdit(this)),
	  m_uniqueId(new QLineEdit(this)),
	  m_channelNumber(new QLineEdit(this)),
	  m_subsystemCode(new QLineEdit(this)),
	  m_operationCode(new QComboBox(this)),
	  m_flags(new QLineEdit(this)),
	  m_startAddress(new QLineEdit(this)),
	  m_fotipFrameSize(new QLineEdit(this)),
	  m_romSize(new QLineEdit(this)),
	  m_romFrameSize(new QLineEdit(this)),
	  m_dataType(new QComboBox(this)),
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

	QIntValidator* i16Validator = new QIntValidator(0, 0xffff, this);
	QIntValidator* i32Validator = new QIntValidator(0, 0x7fffffff, this);
	m_moduleIdEdit->setValidator(i16Validator);
	m_moduleIdEdit->setText(settings.value("moduleId", "4352").toString());

	m_operationCode->addItem("Read command");
	m_operationCode->addItem("Write command");

	m_dataType->addItem("Signed integer");
	m_dataType->addItem("Float pointing");
	m_dataType->addItem("Immitation/interlock");

	m_uniqueId->setValidator(i16Validator);
	m_channelNumber->setValidator(i16Validator);
	m_subsystemCode->setValidator(i16Validator);
	m_flags->setValidator(i16Validator);
	m_startAddress->setValidator(i32Validator);
	m_fotipFrameSize->setValidator(i16Validator);
	m_romSize->setValidator(i32Validator);
	m_romFrameSize->setValidator(i16Validator);

	QPushButton* sendButton = new QPushButton("Send RUP packet", this);
	connect(sendButton, &QPushButton::clicked, this, &Widget::sendPacket);

	QGroupBox* gb = new QGroupBox("IP header");
	QFormLayout* fl = new QFormLayout;
	fl->addRow("Source address", m_sourceAddressCombo);
	fl->addRow("Source port", m_sourcePortEdit);
	fl->addRow("Destination address", m_destinationAddressEdit);
	fl->addRow("Destination port", m_destinationPortEdit);

	gb->setLayout(fl);
	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(gb);

	gb = new QGroupBox("RUP header");
	fl = new QFormLayout;
	fl->addRow("Module ID", m_moduleIdEdit);

	gb->setLayout(fl);
	vl->addWidget(gb);

	gb = new QGroupBox("FOTIP header");
	fl = new QFormLayout;
	fl->addRow("Uniquie ID", m_uniqueId);
	fl->addRow("Channel number", m_channelNumber);
	fl->addRow("Subsystem code", m_subsystemCode);
	fl->addRow("Operation code", m_operationCode);
	fl->addRow("Flags", m_flags);
	fl->addRow("Start address", m_startAddress);
	fl->addRow("Fotip frame size", m_fotipFrameSize);
	fl->addRow("Rom size", m_romSize);
	fl->addRow("Rom frame size", m_romFrameSize);
	fl->addRow("Data type", m_dataType);

	gb->setLayout(fl);
	vl->addWidget(gb);

	vl->addWidget(sendButton);

	setLayout(vl);
}

Widget::~Widget()
{
	QSettings settings;
	settings.setValue("sourceAddress", m_sourceAddressCombo->currentText());
	settings.setValue("sourcePort", m_sourcePortEdit->text());
	settings.setValue("destinationAddress", m_destinationAddressEdit->text());
	settings.setValue("destinationPort", m_destinationPortEdit->text());
	settings.setValue("moduleId", m_moduleIdEdit->text());
	settings.setValue("numerator", numerator);
}

void Widget::sendPacket()
{
	static_assert(sizeof(FotipHeader) == 30, "check size of fotip header");
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
	header.moduleType = m_moduleIdEdit->text().toUInt();
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

	FotipHeader& fotip = *reinterpret_cast<FotipHeader*>(frame.data);
	fotip.protocolVersion = 1;
	fotip.uniqueId = m_uniqueId->text().toUInt();
	fotip.subsystemKey = 0x6141;	//Temporary
	fotip.operationCode = (m_operationCode->currentIndex() == 0) ? 1200 : 1400;
	fotip.flags = m_flags->text().toUInt();
	fotip.startAddress = m_startAddress->text().toUInt();
	fotip.fotipFrameSize = m_fotipFrameSize->text().toUInt();
	fotip.romSize = m_romSize->text().toUInt();
	fotip.romFrameSize = m_romFrameSize->text().toUInt();
	fotip.dataType = dataTypes[m_dataType->currentIndex()];

	m_socket->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(RupFrame), QHostAddress(m_destinationAddressEdit->text()), m_destinationPortEdit->text().toUInt());
}

void Widget::rebindSocket()
{
	bool ok;
	int port = m_sourcePortEdit->text().toUInt(&ok);
	if (!ok)
	{
		return;
	}
	QString address = m_sourceAddressCombo->currentText().split("/")[0];
	m_socket->bind(QHostAddress(address), port);
}
