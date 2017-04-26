#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

#include "../lib/Tuning/TuningSignalManager.h"

namespace Ui {
	class DialogTuningSources;
}

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(TuningSignalManager* tuningSignalManager, QWidget* parent = 0);
	~DialogTuningSources();

protected:
	void timerEvent(QTimerEvent* event);

private slots:
	void slot_tuningSourcesArrived();

	void on_treeWidget_doubleClicked(const QModelIndex& index);

	void on_btnClose_clicked();

	void on_btnDetails_clicked();

	void on_treeWidget_itemSelectionChanged();

private:
	void update(bool refreshOnly);


	enum class Columns
	{
		Id,
		EquipmentId,
		Caption,
		Ip,
		Port,
		Channel,
		SubsystemID,
		Subsystem,
		LmNumber,

		IsReply,
		RequestCount,
		ReplyCount,
		CommandQueueSize,

		ColumnCount
	};

private:
	Ui::DialogTuningSources* ui;

	int m_updateStateTimerId = -1;

	TuningSignalManager* m_tuningSignalManager = nullptr;
};

extern DialogTuningSources* theDialogTuningSources;

#endif // DIALOGTUNINGSOURCES_H
