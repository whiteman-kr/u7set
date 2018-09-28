#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

class TuningTcpClient;

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(TuningTcpClient* tcpClient, bool hasActivationControls, QWidget* parent);
	~DialogTuningSources();

signals:
	void dialogClosed();

protected:
	void timerEvent(QTimerEvent* event);

	virtual bool passwordOk();

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

	enum Columns
	{
		EquipmentId,
		Ip,
		Port,
		Channel,
		SubsystemID,
		LmNumber,

		State,
		IsActive,
		HasUnappliedParams,
		RequestCount,
		ReplyCount,

		ColumnCount
	};

private:

	QTreeWidget* m_treeWidget = nullptr;
	QPushButton* m_btnDetails = nullptr;
	QPushButton* m_btnEnableControl = nullptr;
	QPushButton* m_btnDisableControl = nullptr;
	QLabel* m_labelSingleControlMode = nullptr;

	int m_updateStateTimerId = -1;

	bool m_hasActivationControls = false;

	bool m_singleControlMode = true;

	TuningTcpClient* m_tuningTcpClient = nullptr;

	QWidget* m_parent = nullptr;

	static const QString m_singleLmControlEnabledString;
	static const QString m_singleLmControlDisabledString;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;
};

#endif // DIALOGTUNINGSOURCES_H
