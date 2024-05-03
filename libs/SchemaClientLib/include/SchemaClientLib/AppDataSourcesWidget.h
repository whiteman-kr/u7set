#pragma once

#include <QWidget>
#include <map>

namespace ClientLib
{
	class AdsSourceStateConnection;
}

class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QTimerEvent;

namespace SchemaClientLib
{
	class DialogAppDataSourceInfo;

	//
	// DialogAppDataSources
	//
	class AppDataSourcesWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit AppDataSourcesWidget(const ClientLib::AdsSourceStateConnection& connection, QWidget* parent);
		~AppDataSourcesWidget();

		bool treeIsFocused() const;

	public slots:
		void detailsClicked();

	protected:
		void timerEvent(QTimerEvent* event);

	private slots:
		void tuningSourcesArrived();
		void treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column);
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

} // namespace SchemaClientLib
