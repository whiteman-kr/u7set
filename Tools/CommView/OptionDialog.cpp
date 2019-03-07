#include "OptionDialog.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

// -------------------------------------------------------------------------------------------------------------------

OptionDialog::OptionDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionDialog::~OptionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Application options"));
	setFixedSize(300, 120);

	// Result File Name
	//
	QHBoxLayout *fileNameLayout = new QHBoxLayout;

	m_pFileNameLabel = new QLabel(tr("Result file name"), this);
	m_pFileNameEdit = new QLineEdit(theOptions.testOption().fileName(), this);

	fileNameLayout->addWidget(m_pFileNameLabel);
	fileNameLayout->addStretch();
	fileNameLayout->addWidget(m_pFileNameEdit);

	// Max packet count
	//
	QHBoxLayout *packetCountLayout = new QHBoxLayout;

	m_pPacketCountLabel = new QLabel(tr("Max packet amount for test"), this);
	m_pPacketCountEdit = new QLineEdit(QString::number(theOptions.testOption().maxPacketCount()), this);
	m_pPacketCountEdit->setValidator(new QIntValidator(this));

	packetCountLayout->addWidget(m_pPacketCountLabel);
	packetCountLayout->addStretch();
	packetCountLayout->addWidget(m_pPacketCountEdit);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OptionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OptionDialog::reject);

	// Main Layout
	//

	QVBoxLayout *optionLayout = new QVBoxLayout;

	optionLayout->addLayout(fileNameLayout);
	optionLayout->addLayout(packetCountLayout);

	QGroupBox* group = new QGroupBox();
	group->setLayout(optionLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(group);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionDialog::onOk()
{
	QString fileName = m_pFileNameEdit->text();
	if (fileName.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input result file name!"));
		return;
	}

	int packetCount = m_pPacketCountEdit->text().toInt();
	if (packetCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input max packet amount for test!"));
		return;
	}

	theOptions.testOption().setFileName(fileName);
	theOptions.testOption().setMaxPacketCount(packetCount);
	theOptions.testOption().save();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
