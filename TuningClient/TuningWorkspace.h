#pragma once

#include "TuningConfigController.h"
#include "TuningPage.h"
#include "TreeFilterWidget.h"
#include "SwitchFiltersPage.h"

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(const TuningLib::TuningUiItem& tuningUiItem, bool check, QWidget* parent = nullptr);

	bool hasDiscreteCounter() const; 
	QString filters() const;
	int counter() const;
	void update(int discreteCounter);

private:
	const TuningLib::TuningUiItem& m_tuningUiItem;
	int m_discreteCounter = 0;

private slots:
	void slot_toggled(bool checked);

signals:
	void filterButtonClicked(const QUuid& uiItem);
};

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	TuningWorkspace(TuningConfigController& configController,
					ClientLib::TuningSignalManager& tuningSignalManager,
					TuningLib::TuningUiStorage& tuningUi,
					TuningSignalListSet& appSignalLists,
					ClientLib::TuningUserManager& userManager,
					ClientLib::TuningConnection& tuningConnection,
					const TuningLib::TuningUiItem& workspaceUi, // Ui item specifies this workspace
					TuningCountersManager& tuningCounters,
					const QUuid& treeListUuid,                  // List selected in list tree
					bool hasFilterTree,
					QWidget* parent);

	virtual ~TuningWorkspace();

	bool hasPendingChanges();
	bool askForSavePendingChanges();
	void updateFilters();

private slots:
	void onTimer();

private:
	// Initialization

	void createButtons();
	void createTabPages();

	QWidget* createTuningPageOrWorkspace(const TuningLib::TuningUiItem& childWorkspaceUi);
	
	QWidget* createChildWorkspace(const TuningLib::TuningUiItem& childWorkspaceUi);
	QWidget* createTuningPage(const TuningLib::TuningUiItem& childWorkspaceUi);


	// Tree items operation

	void updateTabsButtonsCounters();

protected:

	virtual bool eventFilter(QObject *object, QEvent *event) override;

private:
	// Data

	TuningConfigController& m_configController;
	ClientLib::TuningSignalManager& m_tuningSignalManager;
	TuningLib::TuningUiStorage& m_tuningUi;
	TuningSignalListSet& m_appSignalLists;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;
	const TuningLib::TuningUiItem& m_workspaceUi;
	TuningCountersManager& m_tuningCounters;
	
	// Interface parts

	TreeFilterWidget* m_treeLayoutWidget = nullptr;
	QVBoxLayout* m_rightLayout = nullptr;
	QHBoxLayout* m_buttonsLayout = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	// Tree filters

	QUuid m_treeListUuid;	// Currently pressed tree filter

	
	// Tabs and filters

	std::vector<const TuningLib::TuningUiItem*> m_tabsUiItems;

	// Buttons and filters

	std::vector<FilterButton*> m_filterButtons;
	const TuningLib::TuningUiItem* m_currentButtonUi = nullptr;

	// Tuning controls

	TuningPage* m_singleTuningPage = nullptr;
	std::map<QUuid, TuningPage*> m_tuningPagesMap;
	std::map<QUuid, TuningWorkspace*> m_tuningWorkspacesMap;
	std::vector<SwitchFiltersPage*> m_switchPresetPages;

	std::map<QUuid, int> m_activeTabPagesMap;

static int m_instanceCounter;

private slots:
	void slot_parentWorkspaceTreeFilterChanged(const QUuid& filterUuid);
	void slot_filterButtonClicked(const QUuid& uiItemUuid);

signals:
	void treeFilterChanged(const QUuid& filterUuid);
	void buttonFilterSelectionChanged(const QUuid& uiItemUuid);


};
