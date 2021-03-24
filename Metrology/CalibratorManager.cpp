#include "CalibratorManager.h"

#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDesktopWidget>

#include "../lib/MetrologySignal.h"

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager::CalibratorManager(std::shared_ptr<Calibrator> pCalibrator, QWidget* parent)
	: QDialog(parent)
	, m_pCalibrator (pCalibrator)
{
	setReadyForManage(true);
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager::~CalibratorManager()
{
	if (m_pErrorDialog != nullptr)
	{
		delete m_pErrorDialog;
	}

	setReadyForManage(false);

	m_pCalibrator.reset();
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManager::calibratorIsConnected() const
{
	if (m_pCalibrator == nullptr)
	{
		return false;
	}

	return m_pCalibrator->isConnected();
}

// -------------------------------------------------------------------------------------------------------------------

int CalibratorManager::calibratorChannel() const
{
	if (m_pCalibrator == nullptr)
	{
		return INVALID_CALIBRATOR_CHANNEL;
	}

	return m_pCalibrator->channel();
}

// -------------------------------------------------------------------------------------------------------------------

QString CalibratorManager::calibratorPort() const
{
	if (m_pCalibrator == nullptr)
	{
		return QString();
	}

	return m_pCalibrator->portName();
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManager::isReadyForManage() const
{
	QMutexLocker l(&m_mutex);

	return m_readyForManage;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::setReadyForManage(bool ready)
{
	QMutexLocker l(&m_mutex);

	m_readyForManage = ready;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::waitReadyForManage()
{
	while(isReadyForManage() != true)
	{
		QThread::msleep(1);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::createInterface()
{
	// create elements of interface
	//

	QGroupBox* valueGroup = new QGroupBox(tr("Display"));
	QVBoxLayout* valueLayout = new QVBoxLayout;

	m_pMeasureLabel = new QLabel(tr("Measure"), this);
	m_pMeasureEdit = new QLineEdit(this);

	m_pSourceLabel = new QLabel(tr("Source"), this);
	m_pSourceEdit = new QLineEdit(this);

	valueLayout->addWidget(m_pMeasureLabel);
	valueLayout->addWidget(m_pMeasureEdit);
	valueLayout->addWidget(m_pSourceLabel);
	valueLayout->addWidget(m_pSourceEdit);

	valueGroup->setLayout(valueLayout);

	m_valueCompleter.create(this);
	m_pValueEdit = new QLineEdit(this);

	QGroupBox* buttonGroup = new QGroupBox(tr("Control"));
	QVBoxLayout* buttonLayout = new QVBoxLayout;

	m_pSetValueButton = new QPushButton(tr("set value"), this);

	QHBoxLayout* stepButtonLayout = new QHBoxLayout;

	m_pStepDownButton = new QPushButton(tr("Step down"), this);
	m_pStepUpButton = new QPushButton(tr("Step up"), this);

	stepButtonLayout->addWidget(m_pStepDownButton);
	stepButtonLayout->addWidget(m_pStepUpButton);

	buttonLayout->addWidget(m_pSetValueButton);
	buttonLayout->addLayout(stepButtonLayout);

	buttonGroup->setLayout(buttonLayout);

	QHBoxLayout* unitLayout = new QHBoxLayout;

	m_pModeList = new QComboBox(this);
	m_pUnitList = new QComboBox(this);

	unitLayout->addWidget(m_pModeList);
	unitLayout->addWidget(m_pUnitList);

	m_pRemoteControlCheck = new QCheckBox(tr("Remote control"), this);

	m_pErrorsButton = new QPushButton(qApp->translate("CalibratorManager.h", ErrorList), this);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(valueGroup);
	mainLayout->addWidget(m_pValueEdit);
	mainLayout->addWidget(buttonGroup);
	mainLayout->addLayout(unitLayout);
	mainLayout->addWidget(m_pErrorsButton);
	mainLayout->addWidget(m_pRemoteControlCheck);
	mainLayout->addStretch();

	m_pErrorDialog = new QDialog(this);

	QVBoxLayout* errorLayout = new QVBoxLayout;

	m_pErrorList = new QTextEdit(this);
	errorLayout->addWidget(m_pErrorList);

	m_pErrorDialog->setLayout(errorLayout);

	setLayout(mainLayout);

	initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::initDialog()
{
	if (m_pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	// init elements of interface
	//
	setWindowFlags(Qt::Drawer);
	setFixedWidth(190);
	setWindowIcon(QIcon(":/icons/Manage.png"));

	QFont* font = new QFont("Arial", 16, 2);

	m_pMeasureLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pMeasureEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pMeasureEdit->setReadOnly(true);
	m_pMeasureEdit->setFont(*font);


	m_pSourceLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pSourceEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pSourceEdit->setReadOnly(true);
	m_pSourceEdit->setFont(*font);


	QRegExp rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
	QValidator* validator = new QRegExpValidator(rx, this);

	m_valueCompleter.load(QString("%1Calibrator%2").arg(CALIBRATOR_OPTIONS_KEY).arg(m_pCalibrator->channel()));

	m_pValueEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pValueEdit->setFont(*font);
	m_pValueEdit->setValidator(validator);
	m_pValueEdit->setCompleter(m_valueCompleter.completer());

	m_pSetValueButton->setDefault(true);
	m_pSetValueButton->setFont(*font);

	delete font;

	m_pRemoteControlCheck->setLayoutDirection(Qt::RightToLeft);
	m_pRemoteControlCheck->setChecked(true);

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	m_pErrorDialog->setMinimumSize(static_cast<int>(screen.width() * 0.4), static_cast<int>(screen.height() * 0.1));
	m_pErrorDialog->resize(static_cast<int>(screen.width() * 0.4), static_cast<int>(screen.height() * 0.15));
	m_pErrorDialog->move(screen.center() - rect().center());

	m_pErrorDialog->setWindowFlags(Qt::Drawer);
	m_pErrorList->setReadOnly(true);

	connect(m_pCalibrator.get(), &Calibrator::connected, this, &CalibratorManager::onCalibratorConnect, Qt::QueuedConnection);
	connect(m_pCalibrator.get(), &Calibrator::disconnected, this, &CalibratorManager::onCalibratorDisconnect, Qt::QueuedConnection);
	connect(m_pCalibrator.get(), &Calibrator::unitIsChanged, this, &CalibratorManager::onUnitChanged, Qt::QueuedConnection);
	connect(m_pCalibrator.get(), &Calibrator::valueIsRequested , this, &CalibratorManager::onValueChanging, Qt::QueuedConnection);
	connect(m_pCalibrator.get(), &Calibrator::valueIsReceived, this, &CalibratorManager::onValueChanged, Qt::QueuedConnection);
	connect(m_pCalibrator.get(), &Calibrator::error, this, &CalibratorManager::onCalibratorError, Qt::QueuedConnection);

	connect(this, &CalibratorManager::calibratorGetValue, m_pCalibrator.get(), &Calibrator::getValue, Qt::QueuedConnection);

	connect(m_pSetValueButton, &QPushButton::clicked, this, &CalibratorManager::onSetValue);
	connect(this, &CalibratorManager::calibratorSetValue, m_pCalibrator.get(), &Calibrator::setValue, Qt::QueuedConnection);
	connect(m_pStepDownButton, &QPushButton::clicked, this, &CalibratorManager::onStepDown);
	connect(this, &CalibratorManager::calibratorStepDown, m_pCalibrator.get(), &Calibrator::stepDown, Qt::QueuedConnection);
	connect(m_pStepUpButton, &QPushButton::clicked, this, &CalibratorManager::onStepUp);
	connect(this, &CalibratorManager::calibratorStepUp, m_pCalibrator.get(), &Calibrator::stepUp, Qt::QueuedConnection);

	connect(m_pModeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CalibratorManager::onSetMode);
	connect(m_pUnitList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CalibratorManager::onSetUnit);
	connect(this, &CalibratorManager::calibratorSetUnit, m_pCalibrator.get(), &Calibrator::setUnit, Qt::QueuedConnection);

	connect(m_pErrorsButton, &QPushButton::clicked, this, &CalibratorManager::onErrorList);

	connect(m_pRemoteControlCheck, &QCheckBox::clicked, this, &CalibratorManager::onRemoveControl);
	connect(this, &CalibratorManager::calibratorRemoveControl, m_pCalibrator.get(), &Calibrator::setRemoteControl, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::setWindowCaption()
{
	if (m_pCalibrator == nullptr)
	{
		setWindowTitle(QString());
		return;
	}

	int channel = m_pCalibrator->channel();
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		setWindowTitle(QString());
		return;
	}

	//
	//

	QString title;

	if (m_pCalibrator->isConnected() == true)
	{
		title = QString("c:%1 ").arg(channel + 1) + m_pCalibrator->typeStr() + " " + m_pCalibrator->serialNo();
	}
	else
	{
		title = tr("c:%1 (%2) - Disconnected").arg(channel + 1).arg(m_pCalibrator->portName()) ;
	}

	setWindowTitle(title);

	//
	//
	if (m_pErrorDialog == nullptr)
	{
		return;
	}

	m_pErrorDialog->setWindowTitle(title + " : " + qApp->translate("CalibratorManager.h", ErrorList));
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::enableInterface(bool enable)
{
	if (calibratorIsConnected() == false)
	{
		enable = false;
	}

	//
	//
	m_pMeasureEdit->setEnabled(enable);
	m_pSourceEdit->setEnabled(enable);
	m_pValueEdit->setEnabled(enable);
	m_pSetValueButton->setEnabled(enable);
	m_pStepDownButton->setEnabled(enable);
	m_pStepUpButton->setEnabled(enable);
	m_pModeList->setEnabled(enable);
	m_pUnitList->setEnabled(enable);
	m_pRemoteControlCheck->setEnabled(enable);

	//
	//
	if (m_pCalibrator == nullptr)
	{
		return;
	}

	if (m_pCalibrator->mode() != CalibratorMode::SourceMode || m_pCalibrator->sourceUnit() == CalibratorUnit::NoCalibratorUnit)
	{
		m_pSetValueButton->setEnabled(false);
		m_pStepDownButton->setEnabled(false);
		m_pStepUpButton->setEnabled(false);
	}

	//
	//
	m_pRemoteControlCheck->setVisible(false);

	switch (m_pCalibrator->type())
	{
		case CalibratorType::Calys75:
			m_pRemoteControlCheck->setText(tr("Remote control"));
			m_pRemoteControlCheck->setVisible(true);
			break;
		case CalibratorType::Ktl6221:
			m_pRemoteControlCheck->setText(tr("Output ON/OFF"));
			m_pRemoteControlCheck->setVisible(true);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::updateModeList()
{
	m_pModeList->clear();
	m_pUnitList->clear();

	if (calibratorIsConnected() == false)
	{
		return;
	}

	m_pModeList->blockSignals(true);

	for (int l = 0; l < CalibratorLimitCount; l++)
	{
		const CalibratorLimit& cl = CalibratorLimits[l];
		if (cl.isValid() == false)
		{
			continue;
		}

		if (cl.type != m_pCalibrator->type())
		{
			continue;
		}

		if (ERR_CALIBRATOR_MODE(cl.mode) == true)
		{
			continue;
		}

		if (m_pModeList->findText(qApp->translate("Calibrator", CalibratorModeCaption(cl.mode).toUtf8())) != -1)
		{
			continue;
		}

		m_pModeList->addItem(qApp->translate("Calibrator", CalibratorModeCaption(cl.mode).toUtf8()), cl.mode);
	}

	m_pModeList->setCurrentIndex(CalibratorMode::NoCalibratorMode);

	m_pModeList->blockSignals(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::updateUnitList()
{
	m_pUnitList->clear();

	if (calibratorIsConnected() == false)
	{
		return;
	}

	int modeIndex = m_pModeList->currentIndex();
	if(modeIndex == -1)
	{
		return;
	}

	int mode = m_pModeList->itemData(modeIndex).toInt();
	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return;
	}

	m_pUnitList->blockSignals(true);

	for (int l = 0; l < CalibratorLimitCount; l++)
	{
		const CalibratorLimit& cl = CalibratorLimits[l];
		if (cl.isValid() == false)
		{
			continue;
		}

		if (cl.type != m_pCalibrator->type())
		{
			continue;
		}

		if (cl.mode != mode)
		{
			continue;
		}

		if (m_pUnitList->findText(qApp->translate("Calibrator", CalibratorUnitCaption(cl.unit).toUtf8())) != -1)
		{
			continue;
		}

		m_pUnitList->addItem(qApp->translate("Calibrator", CalibratorUnitCaption(cl.unit).toUtf8()), cl.unit);
	}

	m_pUnitList->setCurrentIndex(CalibratorUnit::NoCalibratorUnit);

	m_pUnitList->blockSignals(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::updateValue()
{
	if (m_pCalibrator == nullptr)
	{
		return;
	}

	// Measure
	//

	QString measureValue;
	CalibratorLimit measureLimit = m_pCalibrator->currentMeasureLimit();

	if (measureLimit.isValid() == false)
	{
		measureValue = QString::number(m_pCalibrator->measureValue(), 'f', DefaultElectricUnitPrecesion);
	}
	else
	{
		measureValue = QString::number(m_pCalibrator->measureValue(), 'f', measureLimit.precesion) + " " + CalibratorUnitCaption(measureLimit.unit);
	}

	m_pMeasureEdit->setText(measureValue);
	m_pMeasureEdit->setCursorPosition(0);

	// Source
	//

	QString sourceValue;
	CalibratorLimit sourceLimit = m_pCalibrator->currentSourceLimit();

	if (sourceLimit.isValid() == false)
	{
		sourceValue = QString::number(m_pCalibrator->sourceValue(), 'f', DefaultElectricUnitPrecesion);
	}
	else
	{
		sourceValue = QString::number(m_pCalibrator->sourceValue(), 'f', sourceLimit.precesion) + " " + CalibratorUnitCaption(sourceLimit.unit);
	}

	m_pSourceEdit->setText(sourceValue);
	m_pSourceEdit->setCursorPosition(0);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onCalibratorError(QString errorText)
{
	QTime time;
	time.currentTime();

	QString error = QTime::currentTime().toString("hh:mm:ss.zzz - ") + errorText;

	m_pErrorList->append(error);
	m_pErrorsButton->setText(qApp->translate("CalibratorManager.h", ErrorList) + QString("(%1)").arg(m_pErrorList->document()->lineCount()));

	if (isVisible() == false)
	{
		return;
	}

	QMessageBox::critical(this, windowTitle(), errorText);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onCalibratorConnect()
{
	setWindowCaption();

	enableInterface(true);

	updateModeList();
	updateUnitList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onCalibratorDisconnect()
{
	setWindowCaption();

	enableInterface(false);

	m_pModeList->clear();
	m_pUnitList->clear();

	setReadyForManage(false);
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManager::setUnit(int mode, int unit)
{
	if (calibratorIsConnected() == false)
	{
		return false;
	}

	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return false;
	}

	int modeCount = m_pModeList->count();
	for (int modeIndex = 0; modeIndex < modeCount; modeIndex++)
	{
		if (m_pModeList->itemData(modeIndex).toInt() == mode)
		{
			m_pModeList->blockSignals(true);
			m_pModeList->setCurrentIndex(modeIndex);
			m_pModeList->blockSignals(false);

			updateUnitList();

			break;
		}
	}

	if (ERR_CALIBRATOR_UNIT(unit) == true)
	{
		return false;
	}

	int unitCount = m_pUnitList->count();
	for (int unitIndex = 0; unitIndex < unitCount; unitIndex++)
	{
		if (m_pUnitList->itemData(unitIndex).toInt() == unit)
		{
			m_pUnitList->blockSignals(true);
			m_pUnitList->setCurrentIndex(unitIndex);
			m_pUnitList->blockSignals(false);
			break;
		}
	}

	setReadyForManage(true);

	emit calibratorSetUnit(mode, unit);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onSetMode(int modeIndex)
{
	m_pSetValueButton->setEnabled(false);
	m_pStepDownButton->setEnabled(false);
	m_pStepUpButton->setEnabled(false);

	if (modeIndex == -1)
	{
		return;
	}

	int mode = m_pModeList->itemData(modeIndex).toInt();
	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return;
	}

	updateUnitList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onSetUnit(int unitIndex)
{
	m_pSetValueButton->setEnabled(false);
	m_pStepDownButton->setEnabled(false);
	m_pStepUpButton->setEnabled(false);

	int modeIndex = m_pModeList->currentIndex();
	if (modeIndex == -1)
	{
		return;
	}

	int mode = m_pModeList->itemData(modeIndex).toInt();
	if (ERR_CALIBRATOR_MODE(mode) == true)
	{
		return;
	}

	if (mode == CalibratorMode::SourceMode)
	{
		m_pSetValueButton->setEnabled(true);
		m_pStepDownButton->setEnabled(true);
		m_pStepUpButton->setEnabled(true);
	}

	if (unitIndex == -1)
	{
		return;
	}

	int unit = m_pUnitList->itemData(unitIndex).toInt();
	if (ERR_CALIBRATOR_UNIT(unit) == true)
	{
		return;
	}

	if (calibratorIsConnected() == false)
	{
		return;
	}

	setReadyForManage(true);

	emit calibratorSetUnit(mode, unit);

	emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onUnitChanged()
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	updateValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::getValue()
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	setReadyForManage(false);

	emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::setValue(double value)
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	setReadyForManage(false);

	emit calibratorSetValue(value);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onSetValue()
{
	QString value = m_pValueEdit->text();

	if (value.isEmpty() == true)
	{
		return;
	}

	setValue(value.toDouble());

	if (m_pCalibrator == nullptr)
	{
		return;
	}

	m_valueCompleter.appendFilter(value);
	m_valueCompleter.save(QString("%1Calibrator%2").arg(CALIBRATOR_OPTIONS_KEY).arg(m_pCalibrator->channel()));
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::stepDown()
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	setReadyForManage(false);

	emit calibratorStepDown();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onStepDown()
{
	stepDown();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::stepUp()
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	setReadyForManage(false);

	emit calibratorStepUp();
}

// -------------------------------------------------------------------------------------------------------------------


void CalibratorManager::onStepUp()
{
	stepUp();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanging()
{
	enableInterface(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanged()
{
	if (calibratorIsConnected() == false)
	{
		return;
	}

	updateValue();

	enableInterface(true);

	setReadyForManage(true);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onErrorList()
{
	if (m_pErrorDialog == nullptr)
	{
		return;
	}

	m_pErrorDialog->show();
	m_pErrorDialog->move(geometry().center() - m_pErrorDialog->rect().center());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onRemoveControl()
{
	emit calibratorRemoveControl(m_pRemoteControlCheck->isChecked());
}

// -------------------------------------------------------------------------------------------------------------------
