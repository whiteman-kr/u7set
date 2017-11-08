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

private:

	void updateFiltersTree();

	void createButtons();

	void updateTabControl();

	QWidget* createTuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> childWorkspaceFilter);

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask);

private:

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	std::shared_ptr<TuningFilter> m_workspaceFilter;

	// Interface parts

	QWidget* m_treeLayoutWidget = nullptr;

	QVBoxLayout* m_rightLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QSplitter* m_hSplitter = nullptr;

	// Controls

	QTreeWidget* m_filterTree = nullptr;
	QLineEdit* m_treeMask = nullptr;

	TuningPage* m_tuningPage = nullptr;
	QTabWidget* m_tab = nullptr;

	//

	std::shared_ptr<TuningFilter> m_treeFilter;	// Currently pressed button filter
	std::shared_ptr<TuningFilter> m_buttonFilter;	// Currently pressed button filter

	std::map<QString, TuningPage*> m_tuningPagesMap;
	std::map<QString, TuningWorkspace*> m_tuningWorkspacesMap;
	std::map<QString, int> m_activeTabPagesMap;

	static int m_instanceCounter;

private slots:
	void slot_treeSelectionChanged();
	void slot_maskReturnPressed();
	void slot_maskApply();


	void slot_treeFilterChanged(std::shared_ptr<TuningFilter> filter);
	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);


signals:
	void treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);
	void buttonFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);


};

#endif // TUNINGWORKSPACE_H
