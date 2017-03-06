#include "DialogInputValue.h"
#include "ui_DialogInputValue.h"
#include <QMessageBox>

DialogInputValue::DialogInputValue(bool analog, float value, float defaultValue, bool sameValue, float lowLimit, float highLimit, int decimalPlaces, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_defaultValue(defaultValue),
	m_analog(analog),
    m_lowLimit(lowLimit),
    m_highLimit(highLimit),
    m_decimalPlaces(decimalPlaces),
	ui(new Ui::DialogInputValue)
{
	ui->setupUi(this);

	ui->m_checkBox->setVisible(analog == false);
	ui->m_lineEdit->setVisible(analog == true);

    ui->m_buttonDefault->setText(tr("Default: ") + QString::number(m_defaultValue, 'f', m_decimalPlaces));

    if (analog == true)
	{

        QString str = tr("Enter the value (%1 - %2):")
                .arg(QString::number(m_lowLimit, 'f', decimalPlaces))
                .arg(QString::number(m_highLimit, 'f', decimalPlaces));

        setWindowTitle(str);

        if (sameValue == true)
		{
			ui->m_lineEdit->setText(QString::number(value, 'f', decimalPlaces));
            ui->m_lineEdit->selectAll();
		}
	}
	else
	{
		if (sameValue == true)
		{
			ui->m_checkBox->setChecked(value != 0);
            ui->m_checkBox->setText(value != 0 ? tr("1") : tr("0"));
		}
		else
		{
			ui->m_checkBox->setTristate(true);
			ui->m_checkBox->setCheckState(Qt::PartiallyChecked);
			ui->m_checkBox->setText(tr("Unknown"));
		}
	}
}

DialogInputValue::~DialogInputValue()
{
	delete ui;
}

void DialogInputValue::accept()
{
	if (m_analog == true)
	{
		QString text = ui->m_lineEdit->text();
		if (text.isEmpty() == true)
		{
			QMessageBox::critical(this, tr("Error"), tr("Please enter the value."));
			return;
		}

		bool ok = false;
        m_value = text.toFloat(&ok);

        if (ok == false)
		{
			QMessageBox::critical(this, tr("Error"), tr("The value is incorrect."));
			return;
		}

        if (m_value < m_lowLimit || m_value > m_highLimit)
        {
            QMessageBox::critical(this, tr("Error"), tr("The value is out of range."));
            return;
        }
    }
	else
	{
		if (ui->m_checkBox->checkState() == Qt::PartiallyChecked)
		{
			QMessageBox::critical(this, tr("Error"), tr("Please select the value."));
			return;
		}

		if (ui->m_checkBox->checkState() == Qt::Checked)
		{
			m_value = 1;
		}
		else
		{
			m_value = 0;
		}
	}


	QDialog::accept();
}

void DialogInputValue::on_m_checkBox_clicked(bool checked)
{
    ui->m_checkBox->setText(checked ? tr("1") : tr("0"));
}

void DialogInputValue::on_m_buttonDefault_clicked()
{
    if (m_analog == true)
    {
        ui->m_lineEdit->setText(QString::number(m_defaultValue, 'f', m_decimalPlaces));
    }
    else
    {
        bool defaultState = m_defaultValue == 0.0 ? false : true;

        ui->m_checkBox->setChecked(defaultState);

        ui->m_checkBox->setText(defaultState ? tr("1") : tr("0"));
    }

}
