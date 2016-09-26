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

private:
	void update(bool refreshOnly);


	enum class Columns
	{
		Id,
		EquipmentId,
		Caption,
		DataType,
		Ip,
		Port,
		Channel,
		SubsystemID,
		Subsystem,
		LmNumber,
		LmModuleType,
		LmAdapterID,
		LmDataEnable,
		LmDataID,

		Uptime,
		ReceivedDataSize,
		DataReceivingRate,
		Respond,

		ColumnCount
	};

private:
	Ui::DialogTuningSources *ui;

	int m_updateStateTimerId = -1;
};

extern DialogTuningSources* theDialogTuningSources;

#endif // DIALOGTUNINGSOURCES_H
