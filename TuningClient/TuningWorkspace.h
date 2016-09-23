#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	TuningWorkspace(QWidget* parent = nullptr);
	~TuningWorkspace();

	QTreeWidget* m_filterTree = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;

	void addChildTreeObjects(ObjectFilter *filter, QTreeWidgetItem* parent);

private slots:
	void slot_treeSelectionChanged();

signals:
	void filterSelectionChanged(Hash hash);

};

#endif // TUNINGWORKSPACE_H
