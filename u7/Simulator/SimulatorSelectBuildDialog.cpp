#include "SimulatorSelectBuildDialog.h"
#include "ui_SimulatorSelectBuildDialog.h"

SimulatorSelectBuildDialog::SimulatorSelectBuildDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SimulatorSelectBuildDialog)
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	return;
}

SimulatorSelectBuildDialog::~SimulatorSelectBuildDialog()
{
	delete ui;
}
