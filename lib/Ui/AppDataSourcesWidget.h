#ifndef DialogAppDataSources_H
#define DialogAppDataSources_H

#include <QDialog>

#include "DialogSourceInfo.h"
#include "../ClientLib/AdsSourceStateConnection.h"

class TcpAppSourcesState;

//
// DialogAppDataSourceInfo
//

class DialogAppDataSourceInfo : public DialogSourceInfo
{
	Q_OBJECT

public:
	explicit DialogAppDataSourceInfo(const ClientLib::AdsSourceStateConnection& adsSourceStateConnection, QWidget* parent, quint64 id);
	virtual ~DialogAppDataSourceInfo();

private:
	void updateData() override;

private:
	const ClientLib::AdsSourceStateConnection& m_adsSourceStateConnection;
	int m_noStateInfoTimeout = 0;
};

//
// DialogAppDataSources
//

class AppDataSourcesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AppDataSourcesWidget(const ClientLib::AdsSourceStateConnection& connection, QWidget* parent);
	virtual ~AppDataSourcesWidget();

	bool treeIsFocused() const;

public slots:
	void detailsClicked();

protected:
	void timerEvent(QTimerEvent* event);

private slots:
	void tuningSourcesArrived();
	void treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
	void detailsDialogClosed(quint64 id);
	void contextMenuRequested();

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
		LmTime,
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

	const ClientLib::AdsSourceStateConnection& m_adsSourceStateConnection;

	QWidget* m_parent = nullptr;

	static const int columnIndex_Id = 0;
	static const int columnIndex_EquipmentId = 1;

	std::map<quint64, DialogAppDataSourceInfo*> m_sourceInfoDialogsMap; // Used for managing details dialogs. Key is source id (64-bit value)
};

#endif // DialogAppDataSources_H
