#pragma once

#include <QWidget>
#include <map>

namespace ClientLib
{
	class TuningConnection;
} // namespace ClientLib

class QTreeWidget;
class QTreeWidgetItem;
class QTimerEvent;

namespace SchemaClientLib
{
	class DialogTuningSourceInfo;

	class TuningSourcesWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TuningSourcesWidget(ClientLib::TuningConnection& tuningConnection, bool hasActivationControls, QWidget* parent);
		~TuningSourcesWidget() override;

		bool treeIsFocused() const;

	public slots:
		void detailsClicked();
		void enableControlClicked();
		void disableControlClicked();

	protected:
		void timerEvent(QTimerEvent* event) override;

		virtual bool login(); // Override this function to ask password before activating/deactivating sources

	private slots:
		void treeWidgetItemSelectionChanged();
		void treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column);
		void detailsDialogClosed(Hash lanControllerHash);
		void contextMenuRequested();

	private:
		void updateData();
		void updateTuningSourcesStates();
		void enableActivationControls();
		void activateControl(bool enable);

	signals:
		void activationControlsAccessChanged(bool activateEnabled, bool deactivateEnabled);
		void activateSourceControl(const QString& sourceEquipmentId, bool activate);

	private:
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
			IsActive,
			HasUnappliedParams,
			RequestCount,
			ReplyCount,

			ColumnCount
		};

		QWidget* m_parent = nullptr;
		QTreeWidget* m_treeWidget = nullptr;

		int m_updateStateTimerId = -1;

		bool m_hasActivationControls = false;
		bool m_buttonActivateEnabled = false;
		bool m_buttonDeactivateEnabled = false;

		ClientLib::TuningConnection& m_tuningConnection;

		static const int columnIndex_SourceHash = 0;
		static const int columnIndex_SourceEquipmentId = 1;
		static const int columnIndex_ControllerHash = 2;

		std::map<Hash, DialogTuningSourceInfo*> m_sourceInfoDialogsMap; // Used for managing details dialogs. Key is LAN controller hash
	};
} // namespace SchemaClientLib