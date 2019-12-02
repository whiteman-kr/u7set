#ifndef DialogAppDataSources_H
#define DialogAppDataSources_H

#include <QDialog>

#include "DialogSourceInfo.h"

class TcpAppSourcesState;

//
// DialogAppDataSourceInfo
//

class DialogAppDataSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogAppDataSourceInfo(TcpAppSourcesState* tcpClient, QWidget* parent, Hash sourceHash);
	virtual ~DialogAppDataSourceInfo();

private:
	void updateData() override;

private:
	TcpAppSourcesState* m_tcpClient = nullptr;

};

//
// DialogAppDataSources
//

class AppDataSourcesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AppDataSourcesWidget(TcpAppSourcesState* tcpClient,  bool hasCloseButton, QWidget* parent);
	virtual ~AppDataSourcesWidget();

	void showCloseButton(bool show);

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

	void onDetailsDialogClosed(Hash hash);

private:
	void update(bool refreshOnly);

	enum class Columns
	{
		EquipmentId,
		Ip,
		Port,
		Channel,
		SubsystemID,
		LmNumber,

		State,
		Uptime,
		ReceivedPacketCount,
		DataReceivingRate,

		ColumnCount
	};

private:

	QTreeWidget* m_treeWidget = nullptr;
	QPushButton* m_btnDetails = nullptr;
	QLabel* m_labelSingleControlMode = nullptr;
	QPushButton* m_closeButton = nullptr;

	int m_updateStateTimerId = -1;

	bool m_singleControlMode = true;

	TcpAppSourcesState* m_stateTcpClient = nullptr;

	QWidget* m_parent = nullptr;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<Hash, DialogAppDataSourceInfo*> m_sourceInfoDialogsMap;
};

#endif // DialogAppDataSources_H
