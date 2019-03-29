#include "SwitchFiltersPageOptions.h"
#include "ui_SwitchPresetsPageOptions.h"

SwitchFiltersPageOptions::SwitchFiltersPageOptions(QWidget *parent,
												   int defaultColCount,
												   int defaultRowCount,
												   int defaultWidth,
												   int defaultHeight) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::SwitchPresetsPageOptions)
{
	ui->setupUi(this);

	setWindowTitle("Options");

	ui->m_spinColCount->setValue(defaultColCount);
	ui->m_spinRowCount->setValue(defaultRowCount);
	ui->m_spinButtonWidth->setValue(defaultWidth);
	ui->m_spinButtonHeight->setValue(defaultHeight);


}

SwitchFiltersPageOptions::~SwitchFiltersPageOptions()
{
	delete ui;
}

int SwitchFiltersPageOptions::buttonsColCount() const
{
	return ui->m_spinColCount->value();
}

int SwitchFiltersPageOptions::buttonsRowCount() const
{
	return ui->m_spinRowCount->value();
}

int SwitchFiltersPageOptions::buttonsWidth() const
{
	return ui->m_spinButtonWidth->value();
}

int SwitchFiltersPageOptions::buttonsHeight() const
{
	return ui->m_spinButtonHeight->value();
}
