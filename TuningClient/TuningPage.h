#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "../AppSignalLib/TuningSignalManager.h"
#include "../ClientLib/TuningConnection.h"
#include "../ClientLib/TuningUserManager.h"
#include "../lib/Tuning/TuningModel.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningConfigController.h"
#include "TuningClientFilterStorage.h"


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
	TuningModelClient(TuningSignalManager& tuningSignalManager, const ClientLib::TuningUserManager& userManager, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent);

	void blink();

	bool hasPendingChanges();

	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

protected:
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
	bool m_blink = false;
	const ClientLib::TuningUserManager& m_userManager;
	TuningPageHelper m_helper;
};

class TuningTableView : public QTableView
{

	Q_OBJECT

public:
	TuningTableView(const ClientLib::TuningUserManager& userManager);
	bool editorActive();

protected:

	virtual bool edit(const QModelIndex&  index, EditTrigger trigger, QEvent*  event);

protected slots:

	virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint hint);

private:

	bool m_editorActive = false;
	TuningPageHelper m_helper;
};


class TuningPageColumnsWidth
{

public:

	TuningPageColumnsWidth();

	bool load(const QString& pageId);
	bool save() const;

	int width(TuningModelColumns column) const;
	void setWidth(TuningModelColumns column, int width);

private:

	QString m_pageId;

	std::map<TuningModelColumns, int> m_widthMap;
	std::map<TuningModelColumns, int> m_defaultWidthMap;

};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	explicit TuningPage(TuningConfigController& configController,
						TuningSignalManager& tuningSignalManager,
						TuningClientFilterStorage& tuningFilterStorage,
						ClientLib::TuningUserManager& userManager,
						ClientLib::TuningConnection& tuningConnection,
						std::shared_ptr<TuningFilter> treeFilter,
						std::shared_ptr<TuningFilter> pageFilter,
						QWidget* parent = 0);
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

public slots:

	void slot_treeFilterChanged(std::shared_ptr<TuningFilter> filter);

	void slot_pageFilterChanged(std::shared_ptr<TuningFilter> filter);

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

	void invertValue();
	void addSelectedSignalsToFilter(TuningFilter* filter);
	void restoreSignalsFromFilter(TuningFilter* filter);

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
	TuningSignalManager& m_tuningSignalManager;
    TuningClientFilterStorage& m_tuningFilterStorage;
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

	std::shared_ptr<TuningFilter> m_treeFilter = nullptr;

	std::shared_ptr<TuningFilter> m_pageFilter = nullptr;

	std::map<QString, std::pair<int, Qt::SortOrder>> m_sortData;

	static int m_instanceCounter;

	int m_instanceNo = -1;

	TuningPageColumnsWidth m_columnWidthStorage;

	const QString m_autoFilterCaption = tr("Auto-Created Filters");
};

#endif // TUNINGPAGE_H
