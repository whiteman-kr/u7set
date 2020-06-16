#ifndef SIMULATORSELECTBUILDDIALOG_H
#define SIMULATORSELECTBUILDDIALOG_H

namespace Ui {
	class SimSelectBuildDialog;
}

class SimSelectBuildDialog : public QDialog
{
	Q_OBJECT

public:
	SimSelectBuildDialog(QString currentProject, QString buildPath, QWidget* parent);
	~SimSelectBuildDialog();

protected:
	QString buildsPath();
	void fillBuildList(QString currentBuildPath);

protected slots:
	void buildListSelectionChanged(int currentRow);
	void buildListItemDoubleClicked(QListWidgetItem* item);

	virtual void accept() override;

public:
	QString resultBuildPath() const;

private:
	Ui::SimSelectBuildDialog *ui;

	QString m_projectName;
};

#endif // SIMULATORSELECTBUILDDIALOG_H
