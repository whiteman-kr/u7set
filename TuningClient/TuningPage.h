#ifndef TUNINGPAGE_H
#define TUNINGPAGE_H

#include "Stable.h"
#include "../lib/TuningModel.h"
#include "../lib/TuningObject.h"
#include "../lib/TuningFilter.h"

class TuningItemModelMain : public TuningItemModel
{
	Q_OBJECT
public:
	TuningItemModelMain(int tuningPageIndex, QWidget *parent);

	void setValue(const std::vector<int>& selectedRows);
	void invertValue(const std::vector<int>& selectedRows);

	void updateStates();

protected:
	virtual QBrush backColor(const QModelIndex& index) const override;
	virtual QBrush foregroundColor(const QModelIndex& index) const override;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;

public slots:

    void slot_setAll();

	void slot_undo();
	void slot_Submit();
    void slot_Conclude();


};

class FilterButton : public QPushButton
{
	Q_OBJECT
public:
	FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, QWidget* parent = nullptr);

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

    virtual bool edit(const QModelIndex & index, EditTrigger trigger, QEvent * event);

protected slots:

    virtual void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint);

private:

    bool m_editorActive = false;

};


class TuningPage : public QWidget
{
	Q_OBJECT
public:
    explicit TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> tabFilter, const TuningObjectStorage* objects, QWidget *parent = 0);
	~TuningPage();

	void fillObjectsList();

    QColor textColor();

    QColor backColor();

private slots:
	void slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter);

	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void slot_setValue();

	void slot_tableDoubleClicked(const QModelIndex &index);

    void slot_ApplyFilter();

    void slot_FilterTypeIndexChanged(int index);

public slots:

    void slot_filterTreeChanged(std::shared_ptr<TuningFilter> filter);


private:

    const TuningObjectStorage* m_objects = nullptr;

    enum class FilterType
    {
        All = 0,
        AppSignalID,
        CustomAppSignalID,
        EquipmentID,
        Caption
    };

    void invertValue();

	virtual void timerEvent(QTimerEvent* event) override;

	bool eventFilter(QObject* object, QEvent* event);

    TuningTableView* m_objectList = nullptr;

	QButtonGroup *m_filterButtonGroup = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	QHBoxLayout* m_buttonsLayout = nullptr;

	QHBoxLayout* m_bottomLayout = nullptr;

	QPushButton* m_setValueButton = nullptr;

    QPushButton* m_setAllButton = nullptr;

	QPushButton* m_submitButton = nullptr;

	QPushButton* m_undoButton = nullptr;

    QPushButton* m_concludeButton = nullptr;

    QPushButton* m_filterButton = nullptr;

    QLineEdit* m_filterEdit = nullptr;

    QComboBox* m_filterTypeCombo = nullptr;

	TuningItemModelMain *m_model = nullptr;

	std::shared_ptr<TuningFilter> m_treeFilter = nullptr;

	std::shared_ptr<TuningFilter> m_tabFilter = nullptr;

	std::shared_ptr<TuningFilter> m_buttonFilter = nullptr;

	int m_tuningPageIndex = 0;

	int m_updateStateTimerId = -1;

	int m_sortColumn = 0;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

};

#endif // TUNINGPAGE_H
