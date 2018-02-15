#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

#include "TuningClientTcpClient.h"

namespace Ui {
	class DialogTuningSources;
}

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(TuningClientTcpClient* tcpClient, QWidget* parent = 0);
	~DialogTuningSources();

protected:
	void timerEvent(QTimerEvent* event);

private slots:
	void slot_tuningSourcesArrived();

	void on_treeWidget_doubleClicked(const QModelIndex& index);

	void on_btnClose_clicked();

	void on_btnDetails_clicked();

	void on_treeWidget_itemSelectionChanged();

	void on_btnEnableControl_clicked();

	void on_btnDisableControl_clicked();

private:
	void update(bool refreshOnly);

	void activateControl(bool enable);

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

	bool m_singleControlMode = true;

	TuningClientTcpClient* m_tcpClient = nullptr;

	QWidget* m_parent = nullptr;

	static const QString m_singleLmControlEnabledString;
	static const QString m_singleLmControlDisabledString;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;
};

extern DialogTuningSources* theDialogTuningSources;

#endif // DIALOGTUNINGSOURCES_H
