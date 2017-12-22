#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	explicit TuningWorkspace(TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, QWidget* parent);
	virtual ~TuningWorkspace();

public:
	void onTimer();

private:

	void fillFiltersTree();

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);

	void updateTreeItemsStatus(QTreeWidgetItem* treeItem = nullptr);

private:

	TuningSignalStorage m_objects;

	TuningFilterStorage* m_filterStorage = nullptr;

	TuningSignalManager* m_tuningSignalManager = nullptr;

	QTreeWidget* m_filterTree = nullptr;
	QLineEdit* m_treeMask = nullptr;
	QPushButton* m_treeMaskApply = nullptr;


	QSplitter* m_hSplitter = nullptr;

	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;

	const int columnName = 0;
	const int columnErrorIndex = 1;
	const int columnSorIndex = 2;

	QTreeWidgetItem* m_treeItemToSelect = nullptr;


private slots:
	void slot_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void slot_maskReturnPressed();
	void slot_maskApply();

	void slot_currentTabChanged(int index);

	void slot_selectPreviousTreeItem();

signals:
	void filterSelectionChanged(std::shared_ptr<TuningFilter> filter);

};

#endif // TUNINGWORKSPACE_H
