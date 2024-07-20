#pragma once

#include "TuningConfigController.h"

namespace ClientLib
{
	class TuningUserManager;
	class TuningConnection;
}

namespace TuningLib
{
	class TuningUiItem;
	class TuningUiStorage;
}

class TuningCountersManager;

class TreeFilterWidget : public QWidget
{
	Q_OBJECT
public:
	TreeFilterWidget(TuningConfigController& configController,
					 TuningLib::TuningUiStorage& tuningUi,
					 AppSignalLists::AppSignalListSet& appSignalLists,
					 ClientLib::TuningUserManager& userManager,
					 ClientLib::TuningConnection& tuningConnection,
					 TuningCountersManager& tuningCounters,
					 QWidget* parent);
	~TreeFilterWidget();

	void fillFiltersTree();
	void updateFiltersTree();

	bool isEmpty() const;
	QTreeWidget* treeWidget();

private:
	void createFilterTree();
	void addTreeObjects(QTreeWidgetItem* item, const QString& mask, const QStringList& includeSystemTags, const QStringList& excludeSystemTags);
	void updateTreeItemStatus(QTreeWidgetItem* treeItem);
	void updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, const AppSignalLists::AppSignalList* treeList);
	void updateTreeItemCounters(QTreeWidgetItem* treeItem, const AppSignalLists::AppSignalList* treeList);
	void activateControl(const QString& equipmentId, bool enable);
	QTreeWidgetItem* findFilterWidget(const QUuid& uuid, QTreeWidgetItem* treeItem);

private slots:
	void slot_treeSelectionChanged();
	void slot_treeContextMenuRequested(const QPoint& pos);
	void slot_treeItemDoubleClicked(QTreeWidgetItem* item, int column);

	void slot_maskReturnPressed();
	void slot_maskApply();

signals:
	void treeFilterSelectionChanged(const QUuid& filterUuid);

private:
	TuningConfigController& m_configController;
	TuningLib::TuningUiStorage& m_tuningUi;
	AppSignalLists::AppSignalListSet& m_appSignalLists;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;
	TuningCountersManager& m_tuningCounters;

	QTreeWidget* m_filterTree = nullptr;
	QComboBox* m_treeMaskCombo = nullptr;
	QPushButton* m_treeMaskApply = nullptr;

	const int m_columnNameIndex = 0;
	int m_columnAccessIndex = -1;
	std::vector<int> m_columnDiscreteCountIndexes;
	int m_columnStatusIndex = -1;
	int m_columnSorIndex = -1;
};
