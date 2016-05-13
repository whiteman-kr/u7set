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
#include <QtEndian>

#include "../include/DataProtocols.h"
#include "SendTuningFrameWidget.h"
#include "PacketSourceModel.h"

template <typename D, typename S>
inline void writeBigEndian(D& destination, S source)
{
	destination = qToBigEndian<D>(source);
}

const int dataTypes[] =
{
	1300,
	1500,
	1700
};

SendTuningFrameWidget::SendTuningFrameWidget(PacketSourceModel* packetSourceModel, QWidget *parent)
	: QDialog(parent),
	  m_endiansCombo(new QComboBox(this)),
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
	  m_fillFotipDataMethod(new QComboBox(this)),
	  m_packetSourceModel(packetSourceModel),
	  numerator(0)
{
	qsrand(static_cast<uint>(QTime::currentTime().msec()));

	QSettings settings;

	m_endiansCombo->addItem("Big Endian");
	m_endiansCombo->addItem("Little Endian");
	m_endiansCombo->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/endians", "").toString());

	numerator = settings.value("PacketSourceModel/SendTuningFrameWidget/numerator", numerator).toInt();

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

	m_sourceAddressCombo->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/sourceAddress", "").toString());

	QIntValidator* portValidator = new QIntValidator(2000, 65535, this);
	m_sourcePortEdit->setValidator(portValidator);
	m_sourcePortEdit->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/sourcePort", "2000").toString());

	QRegExp re("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:/(?:[12]?[0-9]|3[0-2]?)?)\\b");
	QRegExpValidator* ipValidator = new QRegExpValidator(re, this);
	m_destinationAddressEdit->setValidator(ipValidator);
	m_destinationAddressEdit->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/destinationAddress", m_sourceAddressCombo->currentText().split("/")[0]).toString());

	m_destinationPortEdit->setValidator(portValidator);
	m_destinationPortEdit->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/destinationPort", "2000").toString());

	QIntValidator* i16Validator = new QIntValidator(0, 0xffff, this);
	QIntValidator* i32Validator = new QIntValidator(0, 0x7fffffff, this);
	m_moduleIdEdit->setValidator(i16Validator);
	m_moduleIdEdit->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/moduleId", "4352").toString());

	m_operationCode->addItem("Read command");
	m_operationCode->addItem("Write command");

	m_dataType->addItem("Signed integer");
	m_dataType->addItem("Float pointing");
	m_dataType->addItem("Immitation/interlock");

	m_fillFotipDataMethod->addItem("Fill by zero");
	m_fillFotipDataMethod->addItem("Fill by random");
	m_fillFotipDataMethod->addItem("Fill by address progression");

	m_uniqueId->setValidator(i16Validator);
	m_channelNumber->setValidator(i16Validator);
	m_subsystemCode->setValidator(i16Validator);
	m_flags->setValidator(i16Validator);
	m_startAddress->setValidator(i32Validator);
	m_fotipFrameSize->setValidator(i16Validator);
	m_romSize->setValidator(i32Validator);
	m_romFrameSize->setValidator(i16Validator);

	m_uniqueId->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/uniqueId", "0").toString());
	m_channelNumber->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/channelNumber", "0").toString());
	m_subsystemCode->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/subsystemCode", "0").toString());
	m_operationCode->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/operationCode", "Read command").toString());
	m_flags->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/flags", "0").toString());
	m_startAddress->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/startAddress", "0").toString());
	m_fotipFrameSize->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/fotipFrameSize", "0").toString());
	m_romSize->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/romSize", "0").toString());
	m_romFrameSize->setText(settings.value("PacketSourceModel/SendTuningFrameWidget/romFrameSize", "0").toString());
	m_dataType->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/dataType", "Signed integer").toString());
	m_fillFotipDataMethod->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/fillFotipDataMethod", "Fill by zero").toString());

	QPushButton* sendButton = new QPushButton("Send RUP packet", this);
	connect(sendButton, &QPushButton::clicked, this, &SendTuningFrameWidget::sendPacket);

	QGroupBox* gb = new QGroupBox("Format");
	QFormLayout* format = new QFormLayout;

	format->addRow("", m_endiansCombo);

	gb->setLayout(format);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(gb);

	gb = new QGroupBox("IP header");
	QFormLayout* fl = new QFormLayout;

	fl->addRow("Source address", m_sourceAddressCombo);
	fl->addRow("Source port", m_sourcePortEdit);
	fl->addRow("Destination address", m_destinationAddressEdit);
	fl->addRow("Destination port", m_destinationPortEdit);

	gb->setLayout(fl);
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
	fl->addRow("Fill FOTIP Data method", m_fillFotipDataMethod);

	gb->setLayout(fl);
	vl->addWidget(gb);

	vl->addWidget(sendButton);

	setLayout(vl);
}

SendTuningFrameWidget::~SendTuningFrameWidget()
{
	QSettings settings;
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/endians", m_endiansCombo->currentText());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/sourceAddress", m_sourceAddressCombo->currentText());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/sourcePort", m_sourcePortEdit->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/destinationAddress", m_destinationAddressEdit->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/destinationPort", m_destinationPortEdit->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/moduleId", m_moduleIdEdit->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/numerator", numerator);

	settings.setValue("PacketSourceModel/SendTuningFrameWidget/uniqueId", m_uniqueId->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/channelNumber", m_channelNumber->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/subsystemCode", m_subsystemCode->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/operationCode", m_operationCode->currentText());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/flags", m_flags->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/startAddress", m_startAddress->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/fotipFrameSize", m_fotipFrameSize->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/romSize", m_romSize->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/romFrameSize", m_romFrameSize->text());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/dataType", m_dataType->currentText());
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/fillFotipDataMethod", m_fillFotipDataMethod->currentText());
}

void SendTuningFrameWidget::sendPacket()
{
	static_assert(sizeof(FotipHeader) == 128, "fotip header size check failed");
	if (!checkSocket())
	{
		return;
	}
	RupFrame frame;
	memset(&frame, 0, sizeof(RupFrame));

	if (m_endiansCombo->currentText() == "Big Endian")
	{
		// in big endian format
		//
		RupFrameHeader& header = frame.header;
		writeBigEndian(header.frameSize, ENTIRE_UDP_SIZE);
		writeBigEndian(header.protocolVersion, 4);
		header.flags.tuningData = 1;
		writeBigEndian(header.flagsWord, header.flagsWord);
		writeBigEndian(header.moduleType, m_moduleIdEdit->text().toUInt());
		writeBigEndian(header.numerator, numerator++);
		writeBigEndian(header.framesQuantity, 1);
		writeBigEndian(header.frameNumber, 0);

		QDateTime&& time = QDateTime::currentDateTime();
		RupTimeStamp& timeStamp = header.timeStamp;
		writeBigEndian(timeStamp.year, time.date().year());
		writeBigEndian(timeStamp.month, time.date().month());
		writeBigEndian(timeStamp.day, time.date().day());

		writeBigEndian(timeStamp.hour, time.time().hour());
		writeBigEndian(timeStamp.minute, time.time().minute());
		writeBigEndian(timeStamp.second, time.time().second());
		writeBigEndian(timeStamp.millisecond, time.time().msec());

		FotipHeader& fotip = *reinterpret_cast<FotipHeader*>(frame.data);
		writeBigEndian(fotip.protocolVersion, 1);
		writeBigEndian(fotip.uniqueId, m_uniqueId->text().toUInt());

		fotip.subsystemKey.channelNumber = m_channelNumber->text().toUInt();
		fotip.subsystemKey.subsystemCode = m_subsystemCode->text().toUInt();
		quint16 data = fotip.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fotip.subsystemKey.channelNumber;	// first
		fotip.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1
		writeBigEndian(fotip.subsystemKeyWord, fotip.subsystemKeyWord);

		writeBigEndian(fotip.operationCode, (m_operationCode->currentIndex() == 0) ? 1200 : 1400);
		writeBigEndian(fotip.flags.all, m_flags->text().toUInt());
		writeBigEndian(fotip.startAddress, m_startAddress->text().toUInt());
		writeBigEndian(fotip.fotipFrameSize, m_fotipFrameSize->text().toUInt());
		writeBigEndian(fotip.romSize, m_romSize->text().toUInt());
		writeBigEndian(fotip.romFrameSize, m_romFrameSize->text().toUInt());
		writeBigEndian(fotip.dataType, dataTypes[m_dataType->currentIndex()]);

		switch (m_fillFotipDataMethod->currentIndex())
		{
			case 0:
				break;
			case 1:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipHeader));
				for (int i = 0; i < FOTIP_TX_RX_DATA_SIZE / 2; i++)
				{
					writeBigEndian(fotipData[i], qrand() % 0x10000);
				}
				break;
			}
			case 2:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipHeader));
				for (int i = 0; i < FOTIP_TX_RX_DATA_SIZE / 2; i++)
				{
					if (i % 2 == 0)
					{
						fotipData[i] = 0;
					}
					else
					{
						writeBigEndian(fotipData[i], m_startAddress->text().toUInt() + i);
					}
				}
				break;
			}
			default:
				assert(false);
		}
	}
	else
	{
		// in little endian format
		//
		RupFrameHeader& header = frame.header;
		header.frameSize = ENTIRE_UDP_SIZE;
		header.protocolVersion = 4;
		header.flags.tuningData = 1;
		header.moduleType = m_moduleIdEdit->text().toUInt();
		header.numerator = numerator++;
		header.framesQuantity = 1;
		header.frameNumber = 0;

		QDateTime&& time = QDateTime::currentDateTime();
		RupTimeStamp& timeStamp = header.timeStamp;
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

		fotip.subsystemKey.channelNumber = m_channelNumber->text().toUInt();
		fotip.subsystemKey.subsystemCode = m_subsystemCode->text().toUInt();
		quint16 data = fotip.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fotip.subsystemKey.channelNumber;	// first
		fotip.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1

		fotip.operationCode = (m_operationCode->currentIndex() == 0) ? 1200 : 1400;
		fotip.flags.all =  m_flags->text().toUInt();
		fotip.startAddress = m_startAddress->text().toUInt();
		fotip.fotipFrameSize = m_fotipFrameSize->text().toUInt();
		fotip.romSize = m_romSize->text().toUInt();
		fotip.romFrameSize = m_romFrameSize->text().toUInt();
		fotip.dataType = dataTypes[m_dataType->currentIndex()];

		switch (m_fillFotipDataMethod->currentIndex())
		{
			case 0:
				break;
			case 1:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipHeader));
				for (int i = 0; i < FOTIP_TX_RX_DATA_SIZE / 2; i++)
				{
					fotipData[i] = qrand() % 0x10000;
				}
				break;
			}
			case 2:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipHeader));
				for (int i = 0; i < FOTIP_TX_RX_DATA_SIZE / 2; i++)
				{
					if (i % 2 == 0)
					{
						fotipData[i] = 0;
					}
					else
					{
						fotipData[i] = m_startAddress->text().toUInt() + i;
					}
				}
				break;
			}
			default:
				assert(false);
		}

	}


	m_socket->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(RupFrame), QHostAddress(m_destinationAddressEdit->text()), m_destinationPortEdit->text().toUInt());
}

bool SendTuningFrameWidget::checkSocket()
{
	bool ok;
	int port = m_sourcePortEdit->text().toUInt(&ok);
	if (!ok)
	{
		return false;
	}

	QString address = m_sourceAddressCombo->currentText();
	if (address.contains('/'))
	{
		address = address.split('/')[0];
	}
	if (address.isEmpty())
	{
		return false;
	}

	if (m_socket != nullptr && m_socket->localAddress().toString() == address && m_socket->localPort() == port)
	{
		return true;
	}
	auto socket = m_packetSourceModel->getSocket(address, port);
	if (!socket)
	{
		// Packet source doesn't listening this address:port, socket must be created and initialized
		if (!m_socket)
		{
			m_socket = std::make_shared<QUdpSocket>(this);
		}
		return m_socket->bind(QHostAddress(address), port);
	}
	else
	{
		m_socket.swap(socket);
	}
	return true;
}
