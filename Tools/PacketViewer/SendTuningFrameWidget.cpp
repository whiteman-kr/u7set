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

#include "../OnlineLib/DataProtocols.h"
#include "SendTuningFrameWidget.h"
#include "PacketSourceModel.h"

template <typename TYPE>
inline void writeBigEndian(TYPE& destination, TYPE source)
{
	destination = qToBigEndian<TYPE>(source);
}

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
	  m_randGen(QTime::currentTime().msec())
{
	QSettings settings;

	m_endiansCombo->addItem("Big Endian");
	m_endiansCombo->addItem("Little Endian");
	m_endiansCombo->setCurrentText(settings.value("PacketSourceModel/SendTuningFrameWidget/endians", "").toString());

	m_numerator = static_cast<quint16>(settings.value("PacketSourceModel/SendTuningFrameWidget/numerator", m_numerator).toUInt());

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
	settings.setValue("PacketSourceModel/SendTuningFrameWidget/numerator", m_numerator);

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
	static_assert(sizeof(FotipV2::Header) == 128, "fotip header size check failed");

	if (!checkSocket())
	{
		return;
	}

	static quint16 dataTypes[] =
	{
		static_cast<quint16>(FotipV2::DataType::AnalogSignedInt),
		static_cast<quint16>(FotipV2::DataType::AnalogFloat),
		static_cast<quint16>(FotipV2::DataType::Discrete)
	};

	Rup::Frame frame;
	memset(&frame, 0, sizeof(Rup::Frame));

	if (m_endiansCombo->currentText() == "Big Endian")
	{
		// in big endian format
		//
		Rup::Header& header = frame.header;
		writeBigEndian<quint16>(header.frameSize, Socket::ENTIRE_UDP_SIZE);
		writeBigEndian<quint16>(header.protocolVersion, 4);
		header.flags.tuningData = 1;
		writeBigEndian<quint16>(header.flags.all, header.flags.all);
		writeBigEndian<quint16>(header.moduleType, static_cast<quint16>(m_moduleIdEdit->text().toUInt()));
		writeBigEndian<quint16>(header.numerator, m_numerator++);
		writeBigEndian<quint16>(header.framesQuantity, 1);
		writeBigEndian<quint16>(header.frameNumber, 0);

		QDateTime&& time = QDateTime::currentDateTime();
		Rup::TimeStamp& timeStamp = header.timeStamp;

		writeBigEndian<quint16>(timeStamp.year,  static_cast<quint16>(time.date().year()));
		writeBigEndian<quint16>(timeStamp.month,  static_cast<quint16>(time.date().month()));
		writeBigEndian<quint16>(timeStamp.day,  static_cast<quint16>(time.date().day()));

		writeBigEndian<quint16>(timeStamp.hour,  static_cast<quint16>(time.time().hour()));
		writeBigEndian<quint16>(timeStamp.minute,  static_cast<quint16>(time.time().minute()));
		writeBigEndian<quint16>(timeStamp.second,  static_cast<quint16>(time.time().second()));
		writeBigEndian<quint16>(timeStamp.millisecond,  static_cast<quint16>(time.time().msec()));

		FotipV2::Header& fotip = *reinterpret_cast<FotipV2::Header*>(frame.data);
		writeBigEndian<quint16>(fotip.protocolVersion, 1);
		writeBigEndian<quint64>(fotip.uniqueId, m_uniqueId->text().toUInt());

		fotip.subsystemKey.lmNumber = m_channelNumber->text().toUInt();
		fotip.subsystemKey.subsystemCode = m_subsystemCode->text().toUInt();
		quint16 data = fotip.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fotip.subsystemKey.lmNumber;	// first
		fotip.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1
		writeBigEndian(fotip.subsystemKey.wordVaue, fotip.subsystemKey.wordVaue);

		writeBigEndian<quint16>(fotip.operationCode,
								static_cast<quint16>((m_operationCode->currentIndex() == 0) ?
														FotipV2::OpCode::Read : FotipV2::OpCode::Write));
		writeBigEndian<quint16>(fotip.flags.all,  static_cast<quint16>(m_flags->text().toUInt()));
		writeBigEndian<quint32>(fotip.startAddressW, m_startAddress->text().toUInt());
		writeBigEndian<quint16>(fotip.fotipFrameSizeB,  static_cast<quint16>(m_fotipFrameSize->text().toUInt()));
		writeBigEndian<quint32>(fotip.romSizeB, m_romSize->text().toUInt());
		writeBigEndian<quint16>(fotip.romFrameSizeB,  static_cast<quint16>(m_romFrameSize->text().toUInt()));
		writeBigEndian<quint16>(fotip.dataType, dataTypes[m_dataType->currentIndex()]);

		switch (m_fillFotipDataMethod->currentIndex())
		{
			case 0:
				break;
			case 1:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipV2::Header));
				for (int i = 0; i < FotipV2::TX_RX_DATA_SIZE / 2; i++)
				{
					writeBigEndian(fotipData[i], static_cast<quint16>(m_randGen.generate() & 0xFFFF));
				}
				break;
			}
			case 2:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipV2::Header));
				for (int i = 0; i < FotipV2::TX_RX_DATA_SIZE / 2; i++)
				{
					if (i % 2 == 0)
					{
						fotipData[i] = 0;
					}
					else
					{
						writeBigEndian<quint16>(fotipData[i], static_cast<quint16>(m_startAddress->text().toUInt() + i));
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
		Rup::Header& header = frame.header;
		header.frameSize = Socket::ENTIRE_UDP_SIZE;
		header.protocolVersion = 4;
		header.flags.tuningData = 1;
		header.moduleType = static_cast<quint16>(m_moduleIdEdit->text().toUInt());
		header.numerator = m_numerator++;
		header.framesQuantity = 1;
		header.frameNumber = 0;

		QDateTime&& time = QDateTime::currentDateTime();
		Rup::TimeStamp& timeStamp = header.timeStamp;
		timeStamp.year = static_cast<quint16>(time.date().year());
		timeStamp.month = static_cast<quint16>(time.date().month());
		timeStamp.day = static_cast<quint16>(time.date().day());

		timeStamp.hour = static_cast<quint16>(time.time().hour());
		timeStamp.minute = static_cast<quint16>(time.time().minute());
		timeStamp.second = static_cast<quint16>(time.time().second());
		timeStamp.millisecond = static_cast<quint16>(time.time().msec());

		FotipV2::Header& fotip = *reinterpret_cast<FotipV2::Header*>(frame.data);
		fotip.protocolVersion = 1;
		fotip.uniqueId = m_uniqueId->text().toUInt();

		fotip.subsystemKey.lmNumber = m_channelNumber->text().toUInt();
		fotip.subsystemKey.subsystemCode = m_subsystemCode->text().toUInt();
		quint16 data = fotip.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fotip.subsystemKey.lmNumber;	// first
		fotip.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1

		fotip.operationCode = (static_cast<quint16>((m_operationCode->currentIndex() == 0) ?
								FotipV2::OpCode::Read : FotipV2::OpCode::Write));
		fotip.flags.all =  static_cast<quint16>(m_flags->text().toUInt());
		fotip.startAddressW = m_startAddress->text().toUInt();
		fotip.fotipFrameSizeB = static_cast<quint16>(m_fotipFrameSize->text().toUInt());
		fotip.romSizeB = m_romSize->text().toUInt();
		fotip.romFrameSizeB = static_cast<quint16>(m_romFrameSize->text().toUInt());
		fotip.dataType = dataTypes[m_dataType->currentIndex()];

		switch (m_fillFotipDataMethod->currentIndex())
		{
			case 0:
				break;
			case 1:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipV2::Header));
				for (int i = 0; i < FotipV2::TX_RX_DATA_SIZE / 2; i++)
				{
					fotipData[i] = static_cast<quint16>(m_randGen.generate() & 0xFFFF);
				}
				break;
			}
			case 2:
			{
				quint16* fotipData = reinterpret_cast<quint16*>(frame.data + sizeof(FotipV2::Header));
				for (int i = 0; i < FotipV2::TX_RX_DATA_SIZE / 2; i++)
				{
					if (i % 2 == 0)
					{
						fotipData[i] = 0;
					}
					else
					{
						fotipData[i] = static_cast<quint16>(m_startAddress->text().toUInt() + i);
					}
				}
				break;
			}
			default:
				assert(false);
		}

	}


	m_socket->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(Rup::Frame),
							QHostAddress(m_destinationAddressEdit->text()),
							static_cast<quint16>(m_destinationPortEdit->text().toUInt()));
}

bool SendTuningFrameWidget::checkSocket()
{
	bool ok = false;

	quint16 port = static_cast<quint16>(m_sourcePortEdit->text().toUInt(&ok));

	if (ok == false)
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
