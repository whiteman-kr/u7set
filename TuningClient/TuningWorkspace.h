#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
    TuningWorkspace(const TuningObjectStorage* objects, QWidget* parent);
	~TuningWorkspace();

private:
    void fillFiltersTree();

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent);

private:

    TuningObjectStorage m_objects;

	QTreeWidget* m_filterTree = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;

public slots:
    void slot_runPresetEditor();

private slots:
	void slot_treeSelectionChanged();

signals:
	void filterSelectionChanged(std::shared_ptr<TuningFilter> filter);

};

#endif // TUNINGWORKSPACE_H
