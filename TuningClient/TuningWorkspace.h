#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"
#include "SwitchFiltersPage.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"

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
	explicit TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter,
							 std::shared_ptr<TuningFilter> workspaceFilter,
							 TuningSignalManager* tuningSignalManager,
							 TuningClientTcpClient* tuningTcpClient,
							 TuningClientFilterStorage* tuningFilterStorage,
							 QWidget* parent);

	virtual ~TuningWorkspace();

	bool hasPendingChanges();

	bool askForSavePendingChanges();

	void onTimer();

	void updateFilters(std::shared_ptr<TuningFilter> rootFilter);

private:

	// Tree update

	void updateFiltersTree(std::shared_ptr<TuningFilter> rootFilter);

	// Initialization

	void createButtons();

	void createTabPages();

	QWidget* createTuningPageOrWorkspace(std::shared_ptr<TuningFilter> childWorkspaceFilter);

	// Tree items operation

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);

	void updateTabsButtonsCounters();

	void updateTreeItemsStatus(QTreeWidgetItem* treeItem = nullptr);

	void updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilter* filter);

	void updateTreeItemCounters(QTreeWidgetItem* treeItem, TuningFilter* filter);

	void activateControl(const QString& equipmentId, bool enable);

	QTreeWidgetItem* findFilterWidget(const QString& id, QTreeWidgetItem* treeItem);

protected:

	virtual bool eventFilter(QObject *object, QEvent *event) override;

private:

	// Data

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	TuningClientFilterStorage* m_tuningFilterStorage = nullptr;

	std::shared_ptr<TuningFilter> m_workspaceFilter;

	// Interface parts

	QWidget* m_treeLayoutWidget = nullptr;

	QVBoxLayout* m_rightLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QSplitter* m_hSplitter = nullptr;

	QTreeWidget* m_filterTree = nullptr;

	QComboBox* m_treeMaskCombo = nullptr;

	QPushButton* m_treeMaskApply = nullptr;

	QTabWidget* m_tab = nullptr;

	const int m_columnNameIndex = 0;
	int m_columnAccessIndex = -1;
	std::vector<int> m_columnDiscreteCountIndexes;
	int m_columnStatusIndex = -1;
	int m_columnSorIndex = -1;

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

	void slot_treeSelectionChanged();
	void slot_treeContextMenuRequested(const QPoint& pos);


	void slot_maskReturnPressed();
	void slot_maskApply();

	void slot_parentTreeFilterChanged(std::shared_ptr<TuningFilter> filter);
	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);

signals:
	void treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);
	void buttonFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);


};

#endif // TUNINGWORKSPACE_H
