#include "SerialPortDialog.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

// -------------------------------------------------------------------------------------------------------------------

SerialPortDialog::SerialPortDialog(const SerialPortOption& portOption, QWidget* parent) :
	m_portOption(portOption),
	QDialog(parent)
{
	createInterface();
	updatePortOption();
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortDialog::~SerialPortDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Serial port options"));
	setFixedSize(200, 180);

	// Port
	//
	QHBoxLayout *portLayout = new QHBoxLayout;

	m_pPortLabel = new QLabel(tr("Serial port:"), this);
	m_pPortList = new QComboBox(this);
	foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		m_pPortList->addItem(info.portName(), info.portName());
	}
	m_pPortList->addItem(tr("Custom"));

	portLayout->addWidget(m_pPortLabel);
	portLayout->addStretch();
	portLayout->addWidget(m_pPortList);

	connect(m_pPortList,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SerialPortDialog::customPort);

	// Type
	//
	QHBoxLayout *typeLayout = new QHBoxLayout;

	m_pTypeLabel = new QLabel(tr("Type:"), this);
	m_pTypeList = new QComboBox(this);
	for(int i = 0; i < RS_TYPE_COUNT; i++)
	{
		m_pTypeList->addItem(RsType[i]);
	}

	typeLayout->addWidget(m_pTypeLabel);
	typeLayout->addStretch();
	typeLayout->addWidget(m_pTypeList);

	// BaudRate
	//
	QHBoxLayout *baudRateLayout = new QHBoxLayout;

	m_pBaudRateLabel = new QLabel(tr("BaudRate:"), this);
	m_pBaudRateList = new QComboBox(this);
	m_pBaudRateList->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);	// for rs-232
	m_pBaudRateList->addItem(QStringLiteral("914826"), 914826);						// for rs-485
	m_pBaudRateList->addItem(tr("Custom"));

	baudRateLayout->addWidget(m_pBaudRateLabel);
	baudRateLayout->addStretch();
	baudRateLayout->addWidget(m_pBaudRateList);

	connect(m_pBaudRateList,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SerialPortDialog::customBaudRate);

	// Data Size
	//
	QHBoxLayout *dataSizeLayout = new QHBoxLayout;

	QString dataSizeLabel = theOptions.view().showInWord() == true ? tr("Data size (words):") : tr("Data size (bytes):");
	m_pDataSizeLabel = new QLabel(dataSizeLabel, this);
	m_pDataSizeEdit = new QLineEdit(this);
	m_pDataSizeEdit->setValidator(new QIntValidator(0, theOptions.view().showInWord() == true ? MAX_DATA_SIZE/2 : MAX_DATA_SIZE, this));

	dataSizeLayout->addWidget(m_pDataSizeLabel);
	dataSizeLayout->addStretch();
	dataSizeLayout->addWidget(m_pDataSizeEdit);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SerialPortDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SerialPortDialog::reject);

	// Main Layout
	//

	QVBoxLayout *optionLayout = new QVBoxLayout;

	optionLayout->addLayout(portLayout);
	optionLayout->addLayout(typeLayout);
	optionLayout->addLayout(baudRateLayout);
	optionLayout->addLayout(dataSizeLayout);

	QGroupBox* group = new QGroupBox();
	group->setLayout(optionLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(group);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortDialog::customPort(int idx)
{
	bool isCustomPort = !m_pPortList->itemData(idx).isValid();
	m_pPortList->setEditable(isCustomPort);
	if (isCustomPort == true)
	{
		m_pPortList->clearEditText();
		m_pPortList->setEditText(m_portOption.portName());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortDialog::customBaudRate(int idx)
{
	bool isCustomBaudRate = !m_pBaudRateList->itemData(idx).isValid();
	m_pBaudRateList->setEditable(isCustomBaudRate);
	if (isCustomBaudRate == true)
	{
		m_pBaudRateList->clearEditText();
		m_pBaudRateList->lineEdit()->setValidator(new QIntValidator(0, 4000000, this));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortDialog::updatePortOption()
{
	int pos = m_pPortList->findText(m_portOption.portName());
	if (pos == -1)
	{
		m_pPortList->setEditable(true);
		m_pPortList->setEditText(m_portOption.portName());
	}
	else
	{
		m_pPortList->setCurrentText(m_portOption.portName());
	}

	m_pTypeList->setCurrentIndex(m_portOption.type());

	QString baudRate = QString::number(m_portOption.baudRate());
	pos = m_pBaudRateList->findText(baudRate);
	if (pos == -1)
	{
		m_pBaudRateList->setEditable(true);
		m_pBaudRateList->setEditText(baudRate);
	}
	else
	{
		m_pBaudRateList->setCurrentText(baudRate);
	}

	m_pBaudRateList->setCurrentText(QString::number(m_portOption.baudRate()));

	int dataSize = m_portOption.dataSize() - SERIAL_PORT_HEADER_SIZE;

	m_pDataSizeEdit->setText(QString::number(theOptions.view().showInWord() == true ? dataSize/2 : dataSize));
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortDialog::onOk()
{
	QString portName = m_pPortList->currentText();
	if (portName.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select serial port!"));
		return;
	}

	int baudRate = m_pBaudRateList->currentText().toInt();
	if (baudRate == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input BaudRate!"));
		return;
	}

	int dataSize = m_pDataSizeEdit->text().toInt();
	if (dataSize == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input Data size!"));
		return;
	}

	if (theOptions.view().showInWord() == true)
	{
		dataSize *= 2;
	}
	else
	{
		if (dataSize % 2 == 1)
		{
			QMessageBox::information(this, windowTitle(), tr("Please, input Data size. The number of bytes must be even."));
			return;
		}
	}

	dataSize += SERIAL_PORT_HEADER_SIZE;

	m_portOption.setPortName(portName);
	m_portOption.setType(m_pTypeList->currentIndex());
	m_portOption.setBaudRate(baudRate);
	m_portOption.setDataSize(dataSize);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
