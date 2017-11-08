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
	TuningModelClient(TuningSignalManager* tuningSignalManager, QWidget* parent);

	void blink();

protected:
	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
	bool m_blink = false;

};

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, bool check, QWidget* parent = nullptr);

	std::shared_ptr<TuningFilter> filter();

private:
	std::shared_ptr<TuningFilter> m_filter;
	QString m_caption;

private slots:
	void slot_toggled(bool checked);

signals:
	void filterButtonClicked(std::shared_ptr<TuningFilter> filter);
};

class TuningTableView : public QTableView
{

	Q_OBJECT

public:
	bool editorActive();

protected:

	virtual bool edit(const QModelIndex&  index, EditTrigger trigger, QEvent*  event);

protected slots:

	virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint hint);

private:

	bool m_editorActive = false;

};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
	explicit TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> tabFilter, std::shared_ptr<TuningFilter> buttonFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent = 0);
	~TuningPage();

	void fillObjectsList();

private slots:

	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void slot_setValue();

	void slot_tableDoubleClicked(const QModelIndex& index);

	void slot_ApplyFilter();

	void slot_FilterTypeIndexChanged(int index);

public slots:

	void slot_treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);
	void slot_buttonFilterSelectionChanged(std::shared_ptr<TuningFilter> filter);


private:
	enum class FilterType
	{
		All = 0,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption
	};

private:

	virtual void timerEvent(QTimerEvent* event) override;

	bool eventFilter(QObject* object, QEvent* event);

	// Signals processing

	void setValue();

	void invertValue();

private slots:

	void slot_setAll();

	void slot_undo();
	void slot_Write();
	void slot_Apply();



private:

	TuningSignalManager* m_tuningSignalManager = nullptr;

	TuningTcpClient* m_tuningTcpClient = nullptr;

	TuningTableView* m_objectList = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	QHBoxLayout* m_bottomLayout = nullptr;

	QPushButton* m_setValueButton = nullptr;

	QPushButton* m_setAllButton = nullptr;

	QPushButton* m_writeButton = nullptr;

	QPushButton* m_undoButton = nullptr;

	QPushButton* m_applyButton = nullptr;

	QPushButton* m_filterButton = nullptr;

	QLineEdit* m_filterEdit = nullptr;

	QComboBox* m_filterTypeCombo = nullptr;

	TuningModelClient* m_model = nullptr;

	std::shared_ptr<TuningFilter> m_treeFilter = nullptr;

	std::shared_ptr<TuningFilter> m_tabFilter = nullptr;

	std::shared_ptr<TuningFilter> m_buttonFilter = nullptr;

	//int m_tuningPageIndex = 0;

	int m_updateStateTimerId = -1;

	int m_sortColumn = 0;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	static int m_instanceCounter;


};

#endif // TUNINGPAGE_H
