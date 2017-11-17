#ifndef DIALOGCHOOSEPRESET_H
#define DIALOGCHOOSEPRESET_H

#include <QDialog>

#include "../lib/DbController.h"

namespace Ui {
class DialogChoosePreset;
}

class DialogChoosePreset : public QDialog
{
	Q_OBJECT

public:
	DialogChoosePreset(QWidget *parent, DbController* db, Hardware::DeviceType selectedType, bool limitToSelectedType);
	~DialogChoosePreset();

protected:
	virtual void showEvent(QShowEvent* event) override;

public:
	std::shared_ptr<Hardware::DeviceObject> selectedPreset = nullptr;

private slots:
	void on_DialogChoosePreset_accepted();
	void on_DialogChoosePreset_finished(int result);

	void on_m_presetTree_doubleClicked(const QModelIndex &index);

private:
	Ui::DialogChoosePreset *ui;

	DbController* m_db;

	std::vector<std::shared_ptr<Hardware::DeviceObject>> presets;

	static int m_lastSortColumn;
	static QString m_lastSelectedPreset;
	static Qt::SortOrder m_lastSortOrder;
};

#endif // DIALOGCHOOSEPRESET_H
