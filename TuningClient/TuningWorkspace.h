#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"

class TuningWorkspace : public QWidget
{
public:
	TuningWorkspace(QWidget* parent = nullptr);
	~TuningWorkspace();

	QTreeWidget* m_filterTree = nullptr;
	QSplitter* m_hSplitter = nullptr;
	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;


	void addChildTreeObjects(ObjectFilter *filter, QTreeWidgetItem* parent);

};

#endif // TUNINGWORKSPACE_H
