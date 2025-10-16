#pragma once

#include "../../OnlineLib/SoftwareEndpoint.h"
#include <VFrame30/SchemaDetails.h>
#include <SchemaClientLib/DragDropHelper.h>
#include "SignalSnapshotModel.h"

class IAppSignalManager;

namespace AppSignalLists
{
	class AppSignalListSet;
}

namespace ClientLib
{
	class ISignalDataServer;
}

namespace SchemaClientLib
{
	class ISignalSnapshotWidget;

	struct DialogSignalSnapshotSettings
	{
		QByteArray horzHeader;
		int horzHeaderCount = 0; // Stores SnapshotColumns::ColumnCount constant to restore default settings if columns set changes

		QStringList maskList;

		int sortColumn = 0;
		Qt::SortOrder sortOrder = Qt::AscendingOrder;

		void restore();
		void store();
	};

	//
	// SnapshotTableView
	//
	class SnapshotTableView : public QTableView
	{
	protected:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

	private:
		AppSignalParam m_appSignalParam;
		QPoint m_dragStartPosition;

		SchemaClientLib::DragDropHelper m_dragDropHelper;
	};

	//
	// SignalSnapshotWidget
	//
	class SignalSnapshotWidget : public QWidget
	{
		Q_OBJECT

	public:
		SignalSnapshotWidget(SchemaClientLib::ISignalSnapshotWidget& signalSnapshotVirtFuncDispatcher,
							 IAppSignalManager* appSignalManager,
							 ClientLib::ISignalDataServer* signalDataServer,                       // Can be nullptr, e.g. in Simulator
							 AppSignalLists::AppSignalListSet* appSignalListSet,                   // Can be nullptr, e.g. in Simulator
							 const std::vector<SoftwareEndpoint::AppDataService>& appDataServices, // Can be empty, e.g. in Simulator
							 const QString& projectName,
							 const QString& equipmentId,
							 QWidget* parent);

		SignalSnapshotWidget(SchemaClientLib::ISignalSnapshotWidget& signalSnapshotVirtFuncDispatcher,
							 IAppSignalManager* appSignalManager,
							 const QString& projectName,
							 const QString& equipmentId,
							 QWidget* parent);

		virtual ~SignalSnapshotWidget();

	public:
		QString projectName() const;
		void setProjectName(const QString& projectName);

		const std::vector<AppSignalParam>& specificSignals() const;
		void setSpecificSignals(const std::vector<AppSignalParam>& specificSignals);

		void setLmEquipmentId(const QString& lmEquipmentId);
		void setSignalsMask(const QStringList& masks);
		void setSignalsTags(const QStringList& tags);
		void resetSignalsType();

	protected:
		virtual std::vector<VFrame30::SchemaDetails> schemasDetails();
		virtual std::set<QString> schemaAppSignals(const QString& schemaStrId);

	protected:
		void showEvent(QShowEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void timerEvent(QTimerEvent* event) override;

	signals:
		void signalsUpdated();

	public slots:
		void onSignalsUpdated(); // Should be called when new signals arrived from AppDataService

	private slots:
		void headerColumnContextMenuRequested(const QPoint& pos);
		void headerColumnToggled(bool checked);

		void contextMenuRequested(const QPoint& pos);
		void tableViewdoubleClicked(const QModelIndex& index);
		void sortIndicatorChanged(int column, Qt::SortOrder order);
		void typeComboCurrentIndexChanged(int index);
		void roleComboCurrentIndexChanged(int index);
		void editMaskReturnPressed();
		void editTagsReturnPressed();
		void maskTypeComboCurrentIndexChanged(int index);
		void serverComboIndexChanged(int index);
		void signalListComboIndexChanged(int index);
		void buttonExportClicked();
		void buttonPrintClicked();
		void buttonChooseTagsClicked();
		void buttonClearFilterClicked();

	private:
		void createControls();
		void createMenus();
		void initFiltersView();
		void initSignalsView();

		void fillAppSignalLists();
		void fillSignals();

		void updateTableItems();

		void maskChanged(bool addToCompleter);
		void tagsChanged(bool addToCompleter);

		void exportData(bool exportSelected);
		void printData(bool printSelected);

	signals:
		void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
		void signalInfo(QString appSignalId);

	private:
		ISignalSnapshotWidget& m_signalSnapshotVirtFuncDispatcher;

		IAppSignalManager* m_appSignalManager = nullptr;
		ClientLib::ISignalDataServer* m_signalDataServer = nullptr;
		AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;

		// Ui
		QComboBox* m_typeCombo = nullptr;
		QComboBox* m_roleCombo = nullptr;
		QComboBox* m_maskTypeCombo = nullptr;
		QComboBox* m_serverCombo = nullptr;
		QComboBox* m_signalListCombo = nullptr;

		QLineEdit* m_editMask = nullptr;
		QLineEdit* m_editTags = nullptr;
		QToolButton* m_buttonChooseTags = nullptr;

		QPushButton* m_buttonFixate = nullptr;

		SnapshotTableView* m_tableView = nullptr;
		SignalSnapshotModel m_model;

		QAction* m_formatAutoSelect = nullptr;
		QAction* m_formatDecimal = nullptr;
		QAction* m_formatExponential = nullptr;

		QAction* m_precisionDefault = nullptr;
		QList<QAction*> m_precisionActions;

		QCompleter* m_maskCompleter = nullptr;
		QCompleter* m_tagsCompleter = nullptr;

		QPushButton* m_clearFilterButton = nullptr;

		QMenu m_formatMenu;

		// Project Data
		QString m_projectName;
		QString m_equipmentId;
		std::vector<SoftwareEndpoint::AppDataService> m_appDataServices;

		std::vector<AppSignalParam> m_specificSignals;

		int m_updateStateTimerId = -1;

		bool m_firstShow = true;

		QString m_maskHelp;
		QString m_tagsHelp;

		DialogSignalSnapshotSettings m_settings;

		bool m_storeType = true;
		bool m_storeRole = true;
		bool m_storeMaskData = true;

		static inline SnapshotSignalType m_storedType{SnapshotSignalType::Any};
		static inline SnapshotSignalRole m_storedRole{SnapshotSignalRole::Any};
		static inline SnapshotMaskType m_storedMaskType{SnapshotMaskType::All};
		static inline QStringList m_storedTags;
	};
} // namespace SchemaClientLib