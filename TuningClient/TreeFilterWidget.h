#pragma once

#include <TuningLib/TuningFilter.h>
#include "TuningConfigController.h"

namespace ClientLib
{
	class TuningUserManager;
	class TuningConnection;
}


class TreeFilterWidget : public QWidget
{
	Q_OBJECT
public:
	TreeFilterWidget(TuningConfigController& configController,
					 TuningFilters::TuningFilterStorage& tuningFilterStorage,
					 ClientLib::TuningUserManager& userManager,
					 ClientLib::TuningConnection& tuningConnection,
					 QWidget* parent);
	~TreeFilterWidget();

	void fillFiltersTree(std::shared_ptr<TuningFilters::TuningFilter> rootFilter);
	void updateFiltersTree();

	bool isEmpty() const;
	QTreeWidget* treeWidget();

private:
	void createFilterTree();
	void addChildTreeObjects(const std::shared_ptr<TuningFilters::TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);
	void updateTreeItemStatus(QTreeWidgetItem* treeItem = nullptr);
	void updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilters::TuningFilter* filter);
	void updateTreeItemCounters(QTreeWidgetItem* treeItem, TuningFilters::TuningFilter* filter);
	void activateControl(const QString& equipmentId, bool enable);
	QTreeWidgetItem* findFilterWidget(const QString& id, QTreeWidgetItem* treeItem);

private slots:
	void slot_treeSelectionChanged();
	void slot_treeContextMenuRequested(const QPoint& pos);
	void slot_treeItemDoubleClicked(QTreeWidgetItem* item, int column);

	void slot_maskReturnPressed();
	void slot_maskApply();

signals:
	void treeFilterSelectionChanged(std::shared_ptr<TuningFilters::TuningFilter> filter);

private:
	TuningConfigController& m_configController;
	TuningFilters::TuningFilterStorage& m_tuningFilterStorage;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;

	QTreeWidget* m_filterTree = nullptr;
	QComboBox* m_treeMaskCombo = nullptr;
	QPushButton* m_treeMaskApply = nullptr;

	const int m_columnNameIndex = 0;
	int m_columnAccessIndex = -1;
	std::vector<int> m_columnDiscreteCountIndexes;
	int m_columnStatusIndex = -1;
	int m_columnSorIndex = -1;
};
