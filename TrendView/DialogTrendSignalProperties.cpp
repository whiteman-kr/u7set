#include "DialogTrendSignalProperties.h"
#include "ui_DialogTrendSignalProperties.h"
#include "DialogTrendSignalPoints.h"
#include <QColorDialog>

//
// DialogTrendSignalProperties
//


DialogTrendSignalProperties::DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal,
														 TrendLib::TrendSignalSet* trendSignalSet,
														 E::TimeType timeType,
														 E::TrendMode trendMode,
														 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTrendSignalProperties),
	m_trendSignal(trendSignal),
	m_trendSignalSet(trendSignalSet),
	m_timeType(timeType),
	m_trendMode(trendMode)

{

	ui->setupUi(this);
	setWindowTitle(tr("Properties - %1").arg(m_trendSignal.signalId()));

	ui->signalIdEdit->setText(m_trendSignal.signalId());
	ui->captionEdit->setText(m_trendSignal.caption());

	ui->typeEdit->setText(E::valueToString<E::SignalType>(m_trendSignal.type()));

	if (m_trendSignal.type() == E::SignalType::Analog)
	{
		ui->unitsEdit->setText(m_trendSignal.unit());
		ui->limitsEdit->setText(
			tr("%1 - %2")
				.arg(QString::number(m_trendSignal.lowLimit(), 'f', trendSignal.precision()))
				.arg(QString::number(m_trendSignal.highLimit(), 'f', trendSignal.precision())));

		ui->viewHighEdit->setText(QString::number(m_trendSignal.viewHighLimit(), 'f', trendSignal.precision()));
		ui->viewLowEdit->setText(QString::number(m_trendSignal.viewLowLimit(), 'f', trendSignal.precision()));
	}

	if (m_trendSignal.type() == E::SignalType::Discrete)
	{
		ui->limitsEdit->setText(tr("0 - 1"));
		ui->viewHighEdit->setText(QString::number(1));
		ui->viewLowEdit->setText(QString::number(0));
		ui->viewHighEdit->setReadOnly(true);
		ui->viewLowEdit->setReadOnly(true);
	}

	ui->colorWidget->setColor(trendSignal.color());

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

	connect(&d, &DialogTrendSignalPoints::signalPointsChanged, this,
			[this]()
			{
				this->emit signalPropertiesChanged();
			});

	d.exec();

	return;
}

void DialogTrendSignalProperties::on_buttonApply_clicked()
{
	applyProperties();
	return;
}

bool DialogTrendSignalProperties::applyProperties()
{
	bool ok = false;

	double viewHighValue = ui->viewHighEdit->text().toDouble(&ok);
	if (ok == false)
	{
		ui->viewHighEdit->setFocus();
		return false;
	}

	double viewLowValue = ui->viewLowEdit->text().toDouble(&ok);
	if (ok == false)
	{
		ui->viewLowEdit->setFocus();
		return false;
	}

	m_trendSignal.setViewHighLimit(qMax(viewHighValue, viewLowValue));
	m_trendSignal.setViewLowLimit(qMin(viewHighValue, viewLowValue));
	m_trendSignal.setColor(ui->colorWidget->color());

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


