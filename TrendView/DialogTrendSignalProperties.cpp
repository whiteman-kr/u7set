#include "DialogTrendSignalProperties.h"
#include "ui_DialogTrendSignalProperties.h"
#include <QColorDialog>

DialogTrendSignalProperties::DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogTrendSignalProperties),
	m_trendSignal(trendSignal)
{
	ui->setupUi(this);
	setWindowTitle(tr("Properties %1").arg(m_trendSignal.signalId()));

	ui->signalIdEdit->setText(m_trendSignal.signalId());
	ui->captionEdit->setText(m_trendSignal.caption());

	ui->typeEdit->setText(E::valueToString<E::SignalType>(m_trendSignal.type()));

	ui->unitsEdit->setText(m_trendSignal.unit());
	ui->limitsEdit->setText(
		tr("%1 - %2")
			.arg(QString::number(m_trendSignal.lowLimit()))
			.arg(QString::number(m_trendSignal.highLimit())));

	ui->viewHighEdit->setText(QString::number(m_trendSignal.viewHighLimit()));
	ui->viewLowEdit->setText(QString::number(m_trendSignal.viewLowLimit()));

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
	bool ok = false;

	double viewHighValue = ui->viewHighEdit->text().toDouble(&ok);
	if (ok == false)
	{
		ui->viewHighEdit->setFocus();
	}

	double viewLowValue = ui->viewLowEdit->text().toDouble(&ok);
	if (ok == false)
	{
		ui->viewLowEdit->setFocus();
	}

	m_trendSignal.setViewHighLimit(viewHighValue);
	m_trendSignal.setViewLowLimit(viewLowValue);
	m_trendSignal.setColor(ui->colorWidget->color());

	QDialog::accept();
	return;
}

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

