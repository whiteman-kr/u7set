#ifndef TUNINGSOURCESWIDGET_H
#define TUNINGSOURCESWIDGET_H

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

	void setTuningTcpClient(TuningTcpClient* tcpClient);

private:
	void updateData() override;

private:
	TuningTcpClient* m_tcpClient = nullptr;

};

class TuningSourcesWidget : public QWidget
{
	Q_OBJECT
public:

	explicit TuningSourcesWidget(TuningTcpClient* tcpClient, bool hasActivationControls, bool hasCloseButton, QWidget* parent);
	virtual ~TuningSourcesWidget();

	void setTuningTcpClient(TuningTcpClient* tcpClient);

signals:
	void closeButtonPressed();

protected:
	void timerEvent(QTimerEvent* event);

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

	enum class Columns
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

	QString m_singleLmControlEnabledString;
	QString m_singleLmControlDisabledString;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap;
};



#endif // DIALOGTUNINGSOURCES_H
