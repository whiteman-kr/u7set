#pragma once

#include "../ClientLib/TuningConnection.h"
#include "TuningClientFilterStorage.h"
#include "TuningConfigController.h"
#include "TuningPage.h"
#include "TreeFilterWidget.h"
#include "SwitchFiltersPage.h"

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(std::shared_ptr<TuningFilter> filter, bool check, QWidget* parent = nullptr);

	std::shared_ptr<TuningFilter> filter();

	int counter() const;
	void update(int discreteCounter);

private:
	std::shared_ptr<TuningFilter> m_filter;
	int m_discreteCounter = 0;

private slots:
	void slot_toggled(bool checked);

signals:
	void filterButtonClicked(std::shared_ptr<TuningFilter> filter);
};


class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	explicit TuningWorkspace(TuningConfigController& configController,
							 TuningSignalManager& tuningSignalManager,
							 TuningClientFilterStorage& tuningFilterStorage,
							 ClientLib::TuningUserManager& userManager,
							 ClientLib::TuningConnection& tuningConnection,
							 std::shared_ptr<TuningFilter> treeFilter,
							 std::shared_ptr<TuningFilter> workspaceFilter,
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

	QWidget* createTuningPageOrWorkspace(std::shared_ptr<TuningFilter> childWorkspaceFilter);

	// Tree items operation

	void updateTabsButtonsCounters();

protected:

	virtual bool eventFilter(QObject *object, QEvent *event) override;

private:
	// Data

	TuningConfigController& m_configController;
	TuningSignalManager& m_tuningSignalManager;
	TuningClientFilterStorage& m_tuningFilterStorage;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;

	std::shared_ptr<TuningFilter> m_workspaceFilter;

	// Interface parts

	TreeFilterWidget* m_treeLayoutWidget = nullptr;
	QVBoxLayout* m_rightLayout = nullptr;
	QHBoxLayout* m_buttonsLayout = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	// Filters containters

	std::vector<FilterButton*> m_filterButtons;
	std::vector<std::shared_ptr<TuningFilter>> m_tabsFilters;

	//

	std::shared_ptr<TuningFilter> m_treeFilter;				// Currently pressed tree filter
	std::shared_ptr<TuningFilter> m_currentbuttonFilter;	// Currently pressed button filter

	// Tuning controls

	TuningPage* m_singleTuningPage = nullptr;
	std::map<QString, TuningPage*> m_tuningPagesMap;
	std::map<QString, TuningWorkspace*> m_tuningWorkspacesMap;
	std::vector<SwitchFiltersPage*> m_switchPresetPages;

	std::map<QString, int> m_activeTabPagesMap;

static int m_instanceCounter;

private slots:
	void slot_parentWorkspaceTreeFilterChanged(std::shared_ptr<TuningFilter> filter);
	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);

signals:
	void treeFilterChanged(std::shared_ptr<TuningFilter> filter);
	void buttonFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);


};
