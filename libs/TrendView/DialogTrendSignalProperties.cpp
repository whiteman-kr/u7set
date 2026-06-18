#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QColorDialog>
#include "DialogTrendSignalProperties.h"
#include "ui_DialogTrendSignalProperties.h"
#include "DialogTrendSignalPoints.h"
#include "TrendScale.h"

namespace TrendLib
{
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
		initUi();
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
		if (applyProperties(m_trendSignal) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("An error has occurred while setting the propreties!"));
			return;
		}
		
		emit signalPropertiesChanged();

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
		if (applyProperties(m_trendSignal) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("An error has occurred while setting the propreties!"));
		}
		else
		{
			// Reset modification flags
			//
			m_modifiedFields.value = 0;
			updateModifiedLabels();

			emit signalPropertiesChanged();
		}

		return;
	}

	void DialogTrendSignalProperties::on_buttonApplyToAll_clicked() 
	{ 
		auto params = m_trendSignalSet->trendSignalsMutable();

		for (TrendLib::TrendSignalParam* p : params)
		{
			if (applyProperties(*p) == false)
			{
				QMessageBox::critical(this, qAppName(), tr("An error has occurred while setting the propreties!"));
				return;
			}
		}

		if (applyProperties(m_trendSignal) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("An error has occurred while setting the propreties!"));
		}

		// Reset modification flags
		//
		m_modifiedFields.value = 0;
		updateModifiedLabels();

		emit signalPropertiesChanged();
		return;
	}

	void DialogTrendSignalProperties::on_resetHigh_clicked()
	{
		if (m_trendSignal.type() == E::SignalType::Analog)
		{
			double highLimit = m_trendSignal.highLimit();
			m_trendSignal.setViewHighLimit(m_scaleType, highLimit);
			ui->viewHighEdit->setText(TrendLib::TrendScale::scaleValueText(highLimit, m_scaleType, m_trendSignal));

			// Update modification flags
			//
			m_modifiedFields.bits.viewHighLimit = true;
			updateModifiedLabels();
		}
	}

	void DialogTrendSignalProperties::on_resetLow_clicked()
	{
		if (m_trendSignal.type() == E::SignalType::Analog)
		{
			double lowLimit = m_trendSignal.lowLimit();
			m_trendSignal.setViewLowLimit(m_scaleType, lowLimit);
			ui->viewLowEdit->setText(TrendLib::TrendScale::scaleValueText(lowLimit, m_scaleType, m_trendSignal));

			// Update modification flags
			//
			m_modifiedFields.bits.viewLowLimit = true;
			updateModifiedLabels();
		}
	}

	void DialogTrendSignalProperties::on_viewFormatCombo_currentIndexChanged(const QString& text)
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

		fillProperties();

		// Update modification flags
		//
		m_modifiedFields.bits.format = true;
		updateModifiedLabels();

		return;
	}

	
	void DialogTrendSignalProperties::initUi()
	{
		ui->viewLineWeightEdit->setValidator(new QIntValidator(0, 10, ui->viewLineWeightEdit));

		// Select analog format
		//
		ui->viewFormatCombo->blockSignals(true);

		ui->viewFormatCombo->addItems(E::enumKeyStrings<E::AnalogFormat>());

		QString analogFormatString = E::valueToString<E::AnalogFormat>(m_trendSignal.analogFormat());
		int index = ui->viewFormatCombo->findText(analogFormatString);
		if (index != -1)
		{
			ui->viewFormatCombo->setCurrentIndex(index);
		}

		ui->viewFormatCombo->blockSignals(false);

		// Update modification flags events
		//
		connect(ui->viewFormatCombo,
				&QComboBox::currentTextChanged,
				this,
				&DialogTrendSignalProperties::on_viewFormatCombo_currentIndexChanged);

		connect(ui->viewPrecisionEdit,
				&QLineEdit::textEdited,
				this,
				[this](const QString&)
				{
					m_modifiedFields.bits.precision = true;
					updateModifiedLabels();
				});

		connect(ui->viewHighEdit,
				&QLineEdit::textEdited,
				this,
				[this](const QString&)
				{
					m_modifiedFields.bits.viewHighLimit = true;
					updateModifiedLabels();
				});

		connect(ui->viewLowEdit,
				&QLineEdit::textEdited,
				this,
				[this](const QString&)
				{
					m_modifiedFields.bits.viewLowLimit = true;
					updateModifiedLabels();
				});

		connect(ui->colorWidget,
				&ChooseColorWidget::colorChanged,
				this,
				[this]()
				{
					m_modifiedFields.bits.color = true;
					updateModifiedLabels();
				});

		connect(ui->viewLineWeightEdit,
				&QLineEdit::textEdited,
				this,
				[this](const QString&)
				{
					m_modifiedFields.bits.lineWeight = true;
					updateModifiedLabels();
				});
	}
	
	void DialogTrendSignalProperties::fillProperties()
	{
		ui->labelAppSignalID->setText(m_trendSignal.signalId());
		ui->labelCaption->setText(m_trendSignal.caption());

		ui->viewLineWeightEdit->setText(QString::number(static_cast<int>(m_trendSignal.lineWeight())));

		if (m_trendSignal.type() == E::SignalType::Analog)
		{
			ui->labelType->setText(tr("Analog"));

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

			ui->labelUnits->setText(m_trendSignal.unit());

			ui->labelHigh->setText(TrendLib::TrendScale::scaleValueText(m_trendSignal.highLimit(), m_scaleType, m_trendSignal));
			ui->labelLow->setText(TrendLib::TrendScale::scaleValueText(m_trendSignal.lowLimit(), m_scaleType, m_trendSignal));

			ui->viewHighEdit->setText(TrendLib::TrendScale::scaleValueText(viewHighLimit, m_scaleType, m_trendSignal));
			ui->viewLowEdit->setText(TrendLib::TrendScale::scaleValueText(viewLowLimit, m_scaleType, m_trendSignal));

			ui->viewPrecisionEdit->setText(QString::number(m_trendSignal.precision()));
		}

		if (m_trendSignal.type() == E::SignalType::Discrete)
		{
			ui->labelType->setText(tr("Discrete"));

			ui->labelHigh->setText(tr("1"));
			ui->labelLow->setText(tr("0"));
			ui->labelUnits->setText("");
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

	void DialogTrendSignalProperties::updateModifiedLabels()
	{
		auto updateLabel = [](QLabel* l, bool flag)
		{
			QString text = l->text();
			if (flag == false) {
				text.remove("<b>");
				text.remove("</b>");
				l->setText(text);
			}
			else
			{
				if (text.startsWith("<b>") == false)
				{
					l->setText("<b>" + text + "</b>");
				}
			}
		};

		updateLabel(ui->viewPrecisionLabel, m_modifiedFields.bits.precision);
		updateLabel(ui->viewFormatLabel, m_modifiedFields.bits.format);
		
		updateLabel(ui->viewHighLabel, m_modifiedFields.bits.viewHighLimit);
		updateLabel(ui->viewLowLabel, m_modifiedFields.bits.viewLowLimit);
		
		updateLabel(ui->colorLabel, m_modifiedFields.bits.color);
		updateLabel(ui->viewLineWeightLabel, m_modifiedFields.bits.lineWeight);
	}

	bool DialogTrendSignalProperties::applyProperties(TrendLib::TrendSignalParam& trendSignal)
	{
		bool ok = false;

		int lineWeight = ui->viewLineWeightEdit->text().toInt(&ok);
		if (ok == false)
		{
			ui->viewLineWeightEdit->setFocus();
			return false;
		}

		if (trendSignal.type() == E::SignalType::Analog)
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

			QString analogFormatString = ui->viewFormatCombo->currentText();
			E::AnalogFormat analogFormat = E::stringToValue<E::AnalogFormat>(analogFormatString, &ok);
			if (ok == true)
			{
				if (m_modifiedFields.bits.format == true)
				{
					trendSignal.setAnalogFormat(analogFormat);
				}
			}
			else
			{
				Q_ASSERT(ok);
			}

			if (m_modifiedFields.bits.viewHighLimit == true)
			{
				trendSignal.setViewHighLimit(m_scaleType, qMax(viewHighLimit, viewLowLimit));
			}

			if (m_modifiedFields.bits.viewLowLimit == true)
			{
				trendSignal.setViewLowLimit(m_scaleType, qMin(viewLowLimit, viewLowLimit));
			}

			if (m_modifiedFields.bits.precision == true)
			{
				trendSignal.setPrecision(precision);
			}
		}

		if (m_modifiedFields.bits.color == true)
		{
			trendSignal.setColor(ui->colorWidget->color().rgb());
		}

		if (m_modifiedFields.bits.lineWeight == true)
		{
			trendSignal.setLineWeight(lineWeight);
		}

		return true;
	}
} // namespace TrendLib

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

		m_modified = true;
		emit colorChanged();
	}

	return;
}

QColor ChooseColorWidget::color() const
{
	return m_color;
}

void ChooseColorWidget::setColor(QColor value)
{
	m_modified = false;
	m_color = value;
}

bool ChooseColorWidget::modified() const 
{ 
	return m_modified; 
}
