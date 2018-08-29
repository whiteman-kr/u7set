#ifndef SIMULATORSELECTBUILDDIALOG_H
#define SIMULATORSELECTBUILDDIALOG_H

#include <QDialog>

namespace Ui {
	class SimulatorSelectBuildDialog;
}

class SimSelectBuildDialog : public QDialog
{
	Q_OBJECT

public:
	enum BuildType
	{
		Debug,
		Release
	};

public:
	SimSelectBuildDialog(QString currentProject, BuildType buildType, QString buildPath, QWidget* parent);
	~SimSelectBuildDialog();

protected:
	QString buildsPath();
	void fillBuildList(QString currentBuildPath);

protected slots:
	void buildListSelectionChanged(int currentRow);
	void buildListItemDoubleClicked(QListWidgetItem* item);

	virtual void accept() override;

public:
	BuildType resultBuildType() const;
	QString resultBuildPath() const;

private:
	Ui::SimulatorSelectBuildDialog *ui;

	QString m_projectName;
};

#endif // SIMULATORSELECTBUILDDIALOG_H
