#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

namespace Ui {
class DialogTuningSources;
}

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(QWidget *parent = 0);
	~DialogTuningSources();

protected:
	void timerEvent(QTimerEvent* event);

private slots:
	void slot_tuningSourcesArrived();

    void on_treeWidget_doubleClicked(const QModelIndex &index);

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
	Ui::DialogTuningSources *ui;

	int m_updateStateTimerId = -1;
};

extern DialogTuningSources* theDialogTuningSources;

#endif // DIALOGTUNINGSOURCES_H
