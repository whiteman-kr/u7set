#ifndef SIMULATORSELECTBUILDDIALOG_H
#define SIMULATORSELECTBUILDDIALOG_H

#include <QDialog>

namespace Ui {
	class SimulatorSelectBuildDialog;
}

class SimulatorSelectBuildDialog : public QDialog
{
	Q_OBJECT

public:
	enum BuildType
	{
		Debug,
		Release
	};

public:
	SimulatorSelectBuildDialog(BuildType buildType, QString buildPath, QWidget* parent);
	~SimulatorSelectBuildDialog();

private:
	Ui::SimulatorSelectBuildDialog *ui;
};

#endif // SIMULATORSELECTBUILDDIALOG_H
