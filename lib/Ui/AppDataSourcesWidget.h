#ifndef DialogAppDataSources_H
#define DialogAppDataSources_H

#include <QDialog>

#include "DialogSourceInfo.h"
#include "AdsConnection.h"

class TcpAppSourcesState;

//
// DialogAppDataSourceInfo
//

class DialogAppDataSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogAppDataSourceInfo(const AdsSourceStateConnection& adsSourceStateConnection, QWidget* parent, quint64 id);
	virtual ~DialogAppDataSourceInfo();

private:
	void updateData() override;

private:
	const AdsSourceStateConnection& m_adsSourceStateConnection;
	int m_noStateInfoTimeout = 0;

};

//
// DialogAppDataSources
//

class AppDataSourcesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AppDataSourcesWidget(const AdsSourceStateConnection& connection, QWidget* parent);
	virtual ~AppDataSourcesWidget();

public slots:
	void detailsClicked();

protected:
	void timerEvent(QTimerEvent* event);

private slots:
	void slot_tuningSourcesArrived();

	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void detailsDialogClosed(quint64 id);

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
	QLabel* m_labelSingleControlMode = nullptr;

	int m_updateStateTimerId = -1;

	bool m_singleControlMode = true;

	const int m_updateIntervalMs = 250;

	const AdsSourceStateConnection& m_adsSourceStateConnection;

	QWidget* m_parent = nullptr;

	static const int columnIndex_Id = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<quint64, DialogAppDataSourceInfo*> m_sourceInfoDialogsMap; // Used for managing details dialogs. Key is source id (64-bit value)
};

#endif // DialogAppDataSources_H
