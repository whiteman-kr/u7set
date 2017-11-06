#ifndef TUNINGWORKSPACE_H
#define TUNINGWORKSPACE_H

#include "TuningPage.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"

class TuningWorkspace : public QWidget
{
	Q_OBJECT
public:
	explicit TuningWorkspace(TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, TuningClientFilterStorage* filterStorage, QWidget* parent);
	virtual ~TuningWorkspace();

private:

	void fillFiltersTree();

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);

private:

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	TuningFilterStorage* m_filterStorage = nullptr;

	QTreeWidget* m_filterTree = nullptr;
	QLineEdit* m_treeMask = nullptr;
	QPushButton* m_treeMaskApply = nullptr;


	QSplitter* m_hSplitter = nullptr;

	QTabWidget* m_tab = nullptr;

	TuningPage* m_tuningPage = nullptr;


private slots:
	void slot_treeSelectionChanged();
	void slot_maskReturnPressed();
	void slot_maskApply();

	void slot_currentTabChanged(int index);

signals:
	void filterSelectionChanged(std::shared_ptr<TuningFilter> filter);

};

#endif // TUNINGWORKSPACE_H
