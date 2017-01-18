#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "TuningFilter.h"
#include "TuningObject.h"
#include "TuningPage.h"


class TuningFilterEditor : public QDialog
{
	Q_OBJECT

public:

    explicit TuningFilterEditor(TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
                                std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                                QPoint pos,
                                QByteArray geometry,
                                QWidget *parent);

    ~TuningFilterEditor();


signals:

    void editorClosing(std::vector <int>& signalsTableColumnWidth, std::vector <int>& presetsTreeColumnWidth, QPoint pos, QByteArray geometry);

public slots:

    void slot_signalsUpdated();

private slots:

	void on_m_addPreset_clicked();

	void on_m_editPreset_clicked();

	void on_m_removePreset_clicked();

    void on_m_copyPreset_clicked();

    void on_m_pastePreset_clicked();

    void on_m_moveUp_clicked();

	void on_m_moveDown_clicked();

	void on_m_add_clicked();

	void on_m_remove_clicked();

	void on_m_presetsTree_doubleClicked(const QModelIndex &index);

	void on_m_signalTypeCombo_currentIndexChanged(int index);

	void on_m_presetsTree_itemSelectionChanged();

	void on_m_setValue_clicked();

    void on_m_setCurrent_clicked();

    void on_m_applyFilter_clicked();

	void on_m_signalsTable_doubleClicked(const QModelIndex &index);

	void sortIndicatorChanged(int column, Qt::SortOrder order);

    void on_m_filterTypeCombo_currentIndexChanged(int index);

    void on_m_filterText_returnPressed();

    void on_m_presetsTree_contextMenu(const QPoint &pos);

private:

	enum class TreeItemType
	{
		Filter,
		Signal
	};

    enum class FilterType
	{
        All,
		AppSignalID,
		CustomAppSignalID,
        EquipmentID,
        Caption
	};

	enum class SignalType
	{
		All,
		Analog,
		Discrete
	};

    void initUserInterface();

	bool isFilter(QTreeWidgetItem* item);
	bool isSignal(QTreeWidgetItem* item);

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> &filter, QTreeWidgetItem* parent);

	void setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter);
	void setSignalItemText(QTreeWidgetItem* item, const TuningFilterValue& value);

	void fillObjectsList();

	std::shared_ptr<TuningFilter> selectedFilter(QTreeWidgetItem** item);

	void getSelectedCount(int& selectedPresets, int& selectedSignals);


private:

    // User interface
    //

    QTableView* m_signalsTable = nullptr;
    QComboBox* m_signalTypeCombo = nullptr;
    QComboBox* m_filterTypeCombo = nullptr;
    QLineEdit* m_filterText = nullptr;
    QPushButton* m_applyFilter = nullptr;

    //

    QPushButton* m_add = nullptr;
    QPushButton* m_remove = nullptr;

    QTreeWidget* m_presetsTree = nullptr;

    //

    QPushButton* m_addPreset = nullptr;
    QPushButton* m_editPreset = nullptr;
    QPushButton* m_removePreset = nullptr;

    QPushButton* m_copyPreset = nullptr;
    QPushButton* m_pastePreset = nullptr;

    QPushButton* m_moveUp = nullptr;
    QPushButton* m_moveDown = nullptr;

    QPushButton* m_setValue = nullptr;
    QPushButton* m_setCurrent = nullptr;

    QAction* m_addPresetAction = nullptr;
    QAction* m_editPresetAction = nullptr;
    QAction* m_removePresetAction = nullptr;

    QAction* m_copyPresetAction = nullptr;
    QAction* m_pastePresetAction = nullptr;

    QAction* m_moveUpAction = nullptr;
    QAction* m_moveDownAction = nullptr;

    QAction* m_setValueAction = nullptr;
    QAction* m_setCurrentAction = nullptr;

    QMenu* m_presetsTreeContextMenu = nullptr;


    QDialogButtonBox* m_okCancelButtonBox = nullptr;

    // Dialog Data
    //

    TuningItemModel *m_model = nullptr;

	bool m_modified = false;

    bool m_showAutomatic = false;

    TuningFilterStorage* m_filterStorage = nullptr;

    const TuningObjectStorage *m_objects = nullptr;

	int m_sortColumn = 0;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

    std::vector <int> m_signalsTableColumnWidth;
    std::vector <int> m_presetsTreeColumnWidth;
};

#endif // DIALOGPRESETEDITOR_H
