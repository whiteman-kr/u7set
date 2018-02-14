#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	explicit TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent = 0);
	virtual ~TuningWorkspace();

	bool hasPendingChanges();

	bool askForSavePendingChanges();

	void onTimer();

private:

	// Initialization

	void updateFiltersTree();

	void createButtons();

	void createTabPages();

	QWidget* createTuningPageOrWorkspace(std::shared_ptr<TuningFilter> childWorkspaceFilter);

	// Tree items operation

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);

	void updateCounters();

	void updateTreeItemsStatus(QTreeWidgetItem* treeItem = nullptr);
	void updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilter* filter);

protected:

	bool eventFilter(QObject *object, QEvent *event) override;

private:

	// Data

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	std::shared_ptr<TuningFilter> m_workspaceFilter;

	// Interface parts

	QWidget* m_treeLayoutWidget = nullptr;

	QVBoxLayout* m_rightLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QSplitter* m_hSplitter = nullptr;

	QTreeWidget* m_filterTree = nullptr;

	QLineEdit* m_treeMask = nullptr;

	QTabWidget* m_tab = nullptr;

	const int columnNameIndex = 0;
	int columnDiscreteCountIndex = -1;
	int columnStatusIndex = -1;
	int columnSorIndex = -1;

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

	std::map<QString, int> m_activeTabPagesMap;

static int m_instanceCounter;

private slots:

	void slot_currentTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
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
