#include "DialogTrendSignalPoint.h"
#include "ui_DialogTrendSignalPoint.h"

DialogTrendSignalPoint::DialogTrendSignalPoint(std::vector<TrendLib::TrendStateItem>* stateItems, E::TimeType timeType, E::SignalType signalType, int precision, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTrendSignalPoint),
	m_stateItems(stateItems),
	m_precision(precision),
	m_timeType(timeType),
	m_signalType(signalType)
{
	ui->setupUi(this);
	setWindowTitle(tr("Points"));

	bool firstItem = true;

	for (TrendLib::TrendStateItem stateItem : *m_stateItems )
	{
		if (firstItem == true)
		{
			ui->dateTimeEdit->setDateTime(stateItem.getTime(m_timeType).toDateTime());
			ui->editValue->setText(QString::number(stateItem.value, 'f', m_precision));
			ui->checkBoxValid->setChecked(stateItem.isValid());
			ui->checkBoxRealtime->setChecked(stateItem.isRealtimePoint());
			firstItem = false;
		}
		else
		{
			if (stateItem.getTime(m_timeType) != (*m_stateItems)[0].getTime(m_timeType))
			{
				ui->dateTimeEdit->setEnabled(false);
			}
			if (stateItem.value != (*m_stateItems)[0].value)
			{
				ui->editValue->setText(QString());
			}
			if (stateItem.isValid() != (*m_stateItems)[0].isValid())
			{
				ui->checkBoxValid->setCheckState(Qt::PartiallyChecked);
			}
			if (stateItem.isRealtimePoint() != (*m_stateItems)[0].isRealtimePoint())
			{
				ui->checkBoxRealtime->setCheckState(Qt::PartiallyChecked);
			}
		}
	}
}

DialogTrendSignalPoint::~DialogTrendSignalPoint()
{
	delete ui;
}


void DialogTrendSignalPoint::accept()
{
	bool ok = false;

	bool setTime = ui->dateTimeEdit->isEnabled() == true;

	bool setValid = ui->checkBoxValid->checkState() != Qt::PartiallyChecked;

	bool setRealtime = ui->checkBoxRealtime->checkState() != Qt::PartiallyChecked;

	bool setValue = true;

	if (m_stateItems->size() > 1 && ui->editValue->text().isEmpty() == true)
	{
		setValue = false;
	}

	double value = 0;

	if (setValue == true)
	{
		value = ui->editValue->text().toDouble(&ok);
		if (ok == false)
		{
			ui->editValue->setFocus();
			return;
		}

		if (m_signalType == E::SignalType::Discrete)
		{
			if (value != 0 && value != 1)
			{
				ui->editValue->setFocus();
				return;
			}
		}
	}

	bool valid = ui->checkBoxValid->checkState() == Qt::Checked;

	bool realTime = ui->checkBoxRealtime->checkState() == Qt::Checked;

	TimeStamp time(ui->dateTimeEdit->dateTime());

	for (TrendLib::TrendStateItem& stateItem : *m_stateItems )
	{
		if (setValue == true)
		{
			stateItem.value = value;
		}

		if (setValid == true)
		{
			stateItem.setValid(valid);
		}

		if (setRealtime == true)
		{
			if (realTime)
			{
				stateItem.setRealtimePointFlag();
			}
			else
			{
				stateItem.resetRealtimePointFlag();
			}
		}

		if (setTime == true)
		{
			stateItem.local = time.timeStamp;
			stateItem.system = time.timeStamp;
			stateItem.plant = time.timeStamp;
		}
	}

	QDialog::accept();
}
