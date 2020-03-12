#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "../lib/Tuning/TuningModel.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"

class TuningModelClient : public TuningModel
{
	Q_OBJECT
public:
	TuningModelClient(TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent);

	void blink();

	bool hasPendingChanges();

protected:
	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
	bool m_blink = false;
	TuningClientTcpClient* m_tuningTcpClient = nullptr;

};

class TuningTableView : public QTableView
{

	Q_OBJECT

public:
	TuningTableView(TuningClientTcpClient* tuningTcpClient);
	bool editorActive();

protected:

	virtual bool edit(const QModelIndex&  index, EditTrigger trigger, QEvent*  event);

protected slots:

	virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint hint);

private:

	bool m_editorActive = false;
	TuningClientTcpClient* m_tuningTcpClient = nullptr;

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
	explicit TuningPage(std::shared_ptr<TuningFilter> treeFilter,
						std::shared_ptr<TuningFilter> pageFilter,
						TuningSignalManager* tuningSignalManager,
						TuningClientTcpClient* tuningTcpClient,
						TuningFilterStorage* tuningFilterStorage,
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

	void slot_restoreValuesFromExistingFilter();

public slots:

	void slot_treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);

	void slot_pageFilterChanged(std::shared_ptr<TuningFilter> filter);

private:

	enum class FilterType
	{
		All = 0,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption,
		Zero,
		One
	};

private:

	bool eventFilter(QObject* object, QEvent* event);

	// Signals processing

	void invertValue();

	void addSelectedSignalsToFilter(TuningFilter* filter);

	void restoreSignalsFromFilter(TuningFilter* filter);

private slots:

	void slot_timerTick500();
	void slot_setAll();
	void slot_undo();
	void slot_Write();
	void slot_Apply();

private:

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningClientTcpClient* m_tuningTcpClient = nullptr;

	TuningFilterStorage* m_tuningFilterStorage = nullptr;

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

	int m_sortColumn = 0;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	static int m_instanceCounter;

	int m_instanceNo = -1;

	TuningPageColumnsWidth m_columnWidthStorage;

	const QString m_autoFilterCaption = tr("Auto-Created Filters");

};

#endif // TUNINGPAGE_H
