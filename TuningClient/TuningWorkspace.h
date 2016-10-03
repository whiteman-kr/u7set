#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	TuningWorkspace(QWidget* parent = nullptr);
	~TuningWorkspace();


private:
	void fillFilters(std::vector<QTreeWidgetItem*>& treeItems, TuningFilterStorage& filterStorage);
	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent);

private:

	QTreeWidget* m_filterTree = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;

private slots:
	void slot_treeSelectionChanged();

signals:
	void filterSelectionChanged(std::shared_ptr<TuningFilter> filter);

};

#endif // TUNINGWORKSPACE_H
