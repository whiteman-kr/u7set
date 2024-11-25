#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "AppSignalSetProvider.h"
#include "SignalsModels.h"

class TableDataVisibilityController;
class FindSignalDialog;
class DialogMetrologyConnection;

namespace Hardware
{
	class DeviceAppSignal;
}

struct CreatingSignalOptions
{
	QStringList lmEquipmentIdList;
	QStringList selectedEquipmentIdList;
	QStringList appSignalIdList;
	QStringList customSignalIdList;
	int defaultSignalTypeIndex = -1;
	QString defaultBusTypeId;
	QRect settingsWindowPositionRect;
};

class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	static const int FILTER_ST_ANALOG = TO_INT(E::SignalType::Analog);
	static const int FILTER_ST_DISCRETE = TO_INT(E::SignalType::Discrete);
	static const int FILTER_ST_BUS = TO_INT(E::SignalType::Bus);
	static const int FILTER_ST_ANY = 0xff;

	static const int FILTER_STR_ANY = 0;
	static const int FILTER_STR_APP_SIGNAL_ID = 1;
	static const int FILTER_STR_CUSTOM_APP_SIGNAL_ID = 2;
	static const int FILTER_STR_EQUIPMENT_ID = 3;
	static const int FILTER_STR_CAPTION = 4;
	static const int FILTER_STR_TAGS = 5;

public:
	SignalsTabPage(AppSignalSetProvider* signalSetProvider,
				   AppSignalPropertyManager* propManager,
				   DbController* dbController,
				   QWidget* parent);
	virtual ~SignalsTabPage() override;

	static bool updateSignalsSpecProps(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate,
									   const QStringList& forceUpdateProperties);
	int getMiddleVisibleRow();

	bool editSignals(const std::vector<int> &ids);

protected:
	void createActions(QToolBar* toolBar);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void keyPressEvent(QKeyEvent *e) override;

public slots:
	void projectOpened();
	void projectClosed();

	void onTabPageChanged();

	void loadSignals();
	void createNewSignals();
	void editSignal();
	void cloneSignal();
	void deleteSignal();
	void findAndReplaceSignal();

	void undoSignalChanges();
	void checkIn();
	void viewSignalHistory();

	DialogMetrologyConnection* createMetrologyDialog();
	void deleteMetrologyDialog();
	void openMetrologyConnections();
	void openAppSignalLists();
	void addMetrologyConnection();
	void metrologyDialogClosed();

	void changeSignalsLoadingSequence();

	void setSelection(const std::vector<int>& selectedRowsSignalID, int focusedCellSignalID = AppSignalSet::BAD_ID);
	void saveSelection();
	void restoreSelection(int selectedSignalID = AppSignalSet::BAD_ID);
	void restoreSelections(const std::vector<int>& selectedSignalIDs);
	void onSignalSelectionChanged();

	void changeSignalTypeFilter(int selectedType);
	void changeSignalIdFilter(QStringList strIds, bool refreshSignalList);
	void applySignalIdFilter();
	void resetSignalIdFilter();

	void showError(QString message);

	void compareObject(DbChangesetObject object, CompareData compareData);

private:
	int getMappedSourceRow(const QModelIndex& modelIndex) const;
	void getSelectionSourceIndexes(const QItemSelection& proxySelection, QModelIndexList* selectionSrcIndexes);

private:
	DbController* m_db = nullptr;
	static SignalsTabPage* m_instance;

	SignalsModel* m_signalsModel = nullptr;
	SignalsProxyModel* m_signalsProxyModel = nullptr;

	AppSignalSetProvider* m_signalSetProvider = nullptr;

	QTableView* m_signalsView = nullptr;
	TableDataVisibilityController* m_signalsColumnVisibilityController = nullptr;
	QComboBox* m_signalTypeFilterCombo = nullptr;
	QComboBox* m_signalIdFieldCombo = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_completer = nullptr;
	QStringList m_filterHistory;
	int m_lastVerticalScrollPosition = -1;
	int m_lastHorizontalScrollPosition = -1;
	FindSignalDialog* m_findSignalDialog = nullptr;
	DialogMetrologyConnection* m_metrologyDialog = nullptr;
	QAction* m_addMetrologyConnectionAction = nullptr;

	std::vector<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = AppSignalSet::BAD_ID;
	int m_focusedCellColumn = -1;
};

