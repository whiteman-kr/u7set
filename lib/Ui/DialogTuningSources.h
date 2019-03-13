#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

#include "DialogSourceInfo.h"

class TuningTcpClient;

//
// DialogTuningSourceInfo
//

class DialogTuningSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(TuningTcpClient* tcpClient, QWidget* parent, Hash sourceHash);
	virtual ~DialogTuningSourceInfo();

private:
	void updateData() override;

private:
	TuningTcpClient* m_tcpClient = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

};

//
// DialogTuningSources
//

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

	void on_btnClose_clicked();

	void on_btnDetails_clicked();

	void on_treeWidget_itemSelectionChanged();

	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void on_btnEnableControl_clicked();

	void on_btnDisableControl_clicked();

	void onDetailsDialogClosed(Hash hash);

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

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;
};

#endif // DIALOGTUNINGSOURCES_H
