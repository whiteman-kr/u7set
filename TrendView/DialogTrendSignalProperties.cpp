#include "DialogTrendSignalProperties.h"
#include "ui_DialogTrendSignalProperties.h"
#include "DialogTrendSignalPoints.h"
#include "TrendScale.h"
#include "../CommonLib/Types.h"

//
// DialogTrendSignalProperties
//


DialogTrendSignalProperties::DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal,
														 TrendLib::TrendSignalSet* trendSignalSet,
														 E::TimeType timeType,
														 E::TrendScaleType scaleType,
														 E::TrendMode trendMode,
														 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTrendSignalProperties),
	m_trendSignal(trendSignal),
	m_trendSignalSet(trendSignalSet),
	m_timeType(timeType),
	m_scaleType(scaleType),
	m_trendMode(trendMode)

{
	ui->setupUi(this);

	setWindowTitle(tr("Properties - %1").arg(m_trendSignal.signalId()));

	ui->viewLineWeightEdit->setValidator(new QIntValidator(0, 10,ui->viewLineWeightEdit));

	// Select analog format

	ui->viewFormatCombo->blockSignals(true);

	ui->viewFormatCombo->addItems(E::enumKeyStrings<E::AnalogFormat>());

	QString analogFormatString = E::valueToString<E::AnalogFormat>(m_trendSignal.analogFormat());
	int index = ui->viewFormatCombo->findText(analogFormatString);
	if (index != -1)
	{
		ui->viewFormatCombo->setCurrentIndex(index);
	}

	ui->viewFormatCombo->blockSignals(false);

	//

	fillProperties();

	return;
}

DialogTrendSignalProperties::~DialogTrendSignalProperties()
{
	delete ui;
}

const TrendLib::TrendSignalParam& DialogTrendSignalProperties::trendSignal() const
{
	return m_trendSignal;
}

void DialogTrendSignalProperties::accept()
{
	if (applyProperties() == false)
	{
		return;
	}

	QDialog::accept();
	return;
}

void DialogTrendSignalProperties::on_buttonPoints_clicked()
{

	DialogTrendSignalPoints d(m_trendSignal,
							  m_trendSignalSet,
							  m_timeType,
							  m_trendMode,
							  this);

	connect(&d, &DialogTrendSignalPoints::signalPointsChanged, this, &DialogTrendSignalProperties::signalPropertiesChanged);

	d.exec();

	return;
}

void DialogTrendSignalProperties::on_buttonApply_clicked()
{
	applyProperties();

	return;
}

void DialogTrendSignalProperties::on_viewFormatCombo_currentIndexChanged(const QString &text)
{
	// Update analog format

	bool ok = false;

	E::AnalogFormat analogFormat = E::stringToValue<E::AnalogFormat>(text, &ok);
	if (ok == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_trendSignal.setAnalogFormat(analogFormat);

	//

	fillProperties();

	return;
}

void DialogTrendSignalProperties::fillProperties()
{
	ui->signalIdEdit->setText(m_trendSignal.signalId());
	ui->captionEdit->setText(m_trendSignal.caption());

	ui->typeEdit->setText(E::valueToString<E::SignalType>(m_trendSignal.type()));

	ui->viewLineWeightEdit->setText(QString::number(static_cast<int>(m_trendSignal.lineWeight())));

	if (m_trendSignal.type() == E::SignalType::Analog)
	{
		double viewHighLimit = m_trendSignal.viewHighLimit(m_scaleType);
		double viewLowLimit = m_trendSignal.viewLowLimit(m_scaleType);

		if (m_scaleType == E::TrendScaleType::Period)
		{
			// Limit values are reversed in periodic scale
			//
			if (std::fabs(viewHighLimit) < 1)
			{
				viewHighLimit = 1;
			}
			if (std::fabs(viewLowLimit) < 1)
			{
				viewLowLimit = -1;
			}

			viewHighLimit = TrendLib::TrendScale::periodScaleInfinity / viewHighLimit;
			viewLowLimit = TrendLib::TrendScale::periodScaleInfinity / viewLowLimit;
		}

		ui->unitsEdit->setText(m_trendSignal.unit());

		ui->limitsEdit->setText(
			tr("%1 - %2")
					.arg(TrendLib::TrendScale::scaleValueText(m_trendSignal.lowLimit(), m_scaleType, m_trendSignal))
					.arg(TrendLib::TrendScale::scaleValueText(m_trendSignal.highLimit(), m_scaleType, m_trendSignal)));

		ui->viewHighEdit->setText(TrendLib::TrendScale::scaleValueText(viewHighLimit, m_scaleType, m_trendSignal));
		ui->viewLowEdit->setText(TrendLib::TrendScale::scaleValueText(viewLowLimit, m_scaleType, m_trendSignal));

		ui->viewPrecisionEdit->setText(QString::number(m_trendSignal.precision()));
	}

	if (m_trendSignal.type() == E::SignalType::Discrete)
	{
		ui->limitsEdit->setText(tr("0 - 1"));
		ui->viewHighEdit->setText(QString::number(1));
		ui->viewLowEdit->setText(QString::number(0));
		ui->viewHighEdit->setReadOnly(true);
		ui->viewLowEdit->setReadOnly(true);
		ui->viewPrecisionEdit->setReadOnly(true);
		ui->viewFormatCombo->setEnabled(false);
	}

	ui->colorWidget->setColor(m_trendSignal.color());

	return;
}

bool DialogTrendSignalProperties::applyProperties()
{
	bool ok = false;

	int lineWeight = ui->viewLineWeightEdit->text().toInt(&ok);
	if (ok == false)
	{
		ui->viewLineWeightEdit->setFocus();
		return false;
	}

	m_trendSignal.setLineWeight(lineWeight);
	m_trendSignal.setColor(ui->colorWidget->color().rgb());

	if (m_trendSignal.type() == E::SignalType::Analog)
	{
		// Analog signal only

		double viewHighLimit = ui->viewHighEdit->text().toDouble(&ok);
		if (ok == false)
		{
			ui->viewHighEdit->setFocus();
			return false;
		}

		double viewLowLimit = ui->viewLowEdit->text().toDouble(&ok);
		if (ok == false)
		{
			ui->viewLowEdit->setFocus();
			return false;
		}

		if (m_scaleType == E::TrendScaleType::Period)
		{
			if (std::fabs(viewHighLimit) < 1 ||
				std::fabs(viewLowLimit) < 1 ||
				std::fabs(viewHighLimit) > TrendLib::TrendScale::periodScaleInfinity ||
				std::fabs(viewLowLimit) > TrendLib::TrendScale::periodScaleInfinity)
			{
				QMessageBox::critical(this, qAppName(), tr("Absolute value of view limits should be in range [1..999] for period scale!"));
				return false;
			}

			viewHighLimit = TrendLib::TrendScale::periodScaleInfinity / viewHighLimit;
			viewLowLimit = TrendLib::TrendScale::periodScaleInfinity / viewLowLimit;
		}

		int precision = ui->viewPrecisionEdit->text().toInt(&ok);
		if (ok == false)
		{
			ui->viewPrecisionEdit->setFocus();
			return false;
		}

		m_trendSignal.setViewHighLimit(m_scaleType, qMax(viewHighLimit, viewLowLimit));
		m_trendSignal.setViewLowLimit(m_scaleType, qMin(viewHighLimit, viewLowLimit));
		m_trendSignal.setPrecision(precision);
	}

	emit signalPropertiesChanged();

	return true;
}

//
// ChooseColorWidget
//

ChooseColorWidget::ChooseColorWidget(QWidget* parent) :
	QLabel(parent)
{
}

void ChooseColorWidget::paintEvent(QPaintEvent* event)
{
	 QPainter p(this);
	 p.fillRect(event->rect(), m_color);
}

void ChooseColorWidget::mousePressEvent(QMouseEvent* /*event*/)
{
	QColorDialog d(m_color, this);
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		m_color = d.selectedColor();
		update();
	}

	return;
}

QColor ChooseColorWidget::color() const
{
	return m_color;
}

void ChooseColorWidget::setColor(QColor value)
{
	m_color = value;
}



