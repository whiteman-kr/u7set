#ifndef DialogAppDataSources_H
#define DialogAppDataSources_H

#include <QDialog>

#include "DialogSourceInfo.h"

class TcpAppDataSourcesStateClient;

//
// DialogAppDataSourceInfo
//

class DialogAppDataSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogAppDataSourceInfo(TcpAppDataSourcesStateClient* tcpClient, QWidget* parent, Hash sourceHash);
	virtual ~DialogAppDataSourceInfo();

private:
	void updateData() override;

private:
	TcpAppDataSourcesStateClient* m_tcpClient = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

};

//
// DialogAppDataSources
//

class DialogAppDataSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogAppDataSources(TcpAppDataSourcesStateClient* tcpClient, QWidget* parent);
	~DialogAppDataSources();

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

	void onDetailsDialogClosed(Hash hash);

private:
	void update(bool refreshOnly);

	enum Columns
	{
		EquipmentId,
		Ip,
		Port,
		Channel,
		SubsystemID,
		LmNumber,

		State,
		Uptime,
		DataReceivingRate,
		ReceivedPacketCount,
		ProcessedPacketCount,

		ColumnCount
	};

private:

	QTreeWidget* m_treeWidget = nullptr;
	QPushButton* m_btnDetails = nullptr;
	QLabel* m_labelSingleControlMode = nullptr;

	int m_updateStateTimerId = -1;

	bool m_singleControlMode = true;

	TcpAppDataSourcesStateClient* m_stateTcpClient = nullptr;

	QWidget* m_parent = nullptr;

	static const int columnIndex_Hash = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<Hash, DialogAppDataSourceInfo*> m_sourceInfoDialogsMap;
};

#endif // DialogAppDataSources_H
