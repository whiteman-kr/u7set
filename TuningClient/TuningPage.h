#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include <ClientLib/TuningSignalManager.h>
#include "TuningModel.h"
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
class TuningSignalListSet;

class TuningPageHelper
{
public:
	TuningPageHelper(const ClientLib::TuningUserManager& userManager);
	bool writingIsEnabled(const AppSignalParam& asp, const TuningSignalState& state) const;

private:
	const ClientLib::TuningUserManager& m_userManager;
};

class TuningModelClient : public TuningModel
{
	Q_OBJECT
public:
	TuningModelClient(ClientLib::TuningSignalManager& tuningSignalManager, const ClientLib::TuningUserManager& userManager, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent);

	void blink();

	bool hasPendingChanges();

	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

protected:
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
	QIcon drawCheckBox(int state, bool enabled) const;

private:
	bool m_blink = false;
	const ClientLib::TuningUserManager& m_userManager;
	TuningPageHelper m_helper;
	QWidget* m_parentWidget{nullptr};
};

class TuningTableView : public QTableView
{
	Q_OBJECT

public:
	TuningTableView(const ClientLib::TuningUserManager& userManager);
	bool editorActive();

protected:
	virtual bool edit(const QModelIndex&  index, EditTrigger trigger, QEvent*  event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;

protected slots:
	virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint hint) override;

signals:
	void checkBoxClicked(const QModelIndex& index);

private:
	bool m_editorActive = false;
	TuningPageHelper m_helper;
};


class TuningPageColumnsWidth
{
public:

	TuningPageColumnsWidth();

	bool load(const QString& pageId);
	bool save(const QString& pageId) const;

	int width(TuningModelColumns column) const;
	void setWidth(TuningModelColumns column, int width);

private:
	std::map<TuningModelColumns, int> m_widthMap;
	std::map<TuningModelColumns, int> m_defaultWidthMap;
};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	TuningPage(TuningConfigController& configController,
			   ClientLib::TuningSignalManager& tuningSignalManager,
			   TuningLib::TuningUiStorage& tuningUi,
			   TuningSignalListSet& appSignalLists,
			   ClientLib::TuningUserManager& userManager,
			   ClientLib::TuningConnection& tuningConnection,
			   const QUuid& treeListUuid,             // List selected in list tree
			   const TuningLib::TuningUiItem& pageUi, // Ui item specifies this page
			   const TuningCountersManager& tuningCounters,
			   QWidget* parent = nullptr);
	~TuningPage();

	void fillObjectsList();

	bool hasPendingChanges();

	bool askForSavePendingChanges();

	bool write();

	void apply();

	void undo();

private slots:

	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void slot_setValue();

	void slot_tableDoubleClicked(const QModelIndex& index);

	void slot_ApplyFilter();

	void slot_FilterTypeIndexChanged(int index);

	void slot_FilterValueIndexChanged(int index);

	void slot_listContextMenuRequested(const QPoint& pos);

	void slot_saveSignalsToNewFilter();

	void slot_saveSignalsToExistingFilter();

	void slot_exportContentsToCSV();

	void slot_restoreValuesFromExistingFilter();

	void slot_setAnalogFormat(E::AnalogFormat analogFormat);

	void slot_tableCheckboxClicked(const QModelIndex& index);

public slots:
	void slot_treeFilterChanged(const QUuid& filterUuid);
	void slot_pageFilterChanged(const QUuid& uiItemUuid);

private:

	enum class FilterIDType
	{
		All = 0,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption
	};

	enum class FilterValueType
	{
		All = 0,
		Zero,
		One,
		DefaultNotSet
	};

private:

	bool eventFilter(QObject* object, QEvent* event);

	// Signals processing

	void invertValue(int channel);	// channel is value column number, if it is set to -1 - all columns are inverted
	void addSelectedSignalsToFilter(AppSignalLists::AppSignalList& list);
	void restoreSignalsFromFilter(const AppSignalLists::AppSignalList& list);
	void setToDefaults(const std::vector<Hash>& hashes);

	void setActionButtonsState();
	void updateVisibleItems();

private slots:
	void onTimer();
	void slot_setAll();
	void slot_undo();
	void slot_Write();
	void slot_Apply();

private:
	TuningConfigController& m_configController;
	ClientLib::TuningSignalManager& m_tuningSignalManager;
	TuningLib::TuningUiStorage& m_tuningUi;
	TuningSignalListSet& m_appSignalLists;
	ClientLib::TuningUserManager& m_userManager;
	ClientLib::TuningConnection& m_tuningConnection;

	TuningPageHelper m_helper;

	TuningTableView* m_objectList = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;
	QHBoxLayout* m_bottomLayout = nullptr;

	QPushButton* m_setValueButton = nullptr;
	QPushButton* m_setAllButton = nullptr;
	QPushButton* m_writeButton = nullptr;
	QPushButton* m_undoButton = nullptr;
	QPushButton* m_applyButton = nullptr;

	QPushButton* m_filterButton = nullptr;
	QComboBox* m_filterTextCombo = nullptr;
	QComboBox* m_filterTypeCombo = nullptr;
	QComboBox* m_filterValueCombo = nullptr;

	TuningModelClient* m_model = nullptr;

	QUuid m_treeListUuid;

	const TuningLib::TuningUiItem* m_pageUi = nullptr;

	const TuningCountersManager& m_tuningCounters;

	std::map<QString, std::pair<int, Qt::SortOrder>> m_sortData;

	static int m_instanceCounter;

	int m_instanceNo = -1;

	TuningPageColumnsWidth m_columnWidthStorage;
};

#endif // TUNINGPAGE_H
