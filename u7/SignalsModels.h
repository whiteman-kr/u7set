#pragma once

#include "../lib/QDoublevalidatorEx.h"
#include "../lib/StandardColors.h"

#include <QStyledItemDelegate>

class AppSignalSetProvider;
class AppSignalPropertyManager;
class SignalsProxyModel;
class SignalsTablePropEditor;

// -------------------------------------------------------------------------------------------------------
//
// SignalsModel class represents model  of signal set corresponding to AppSignalSet
//
// QModelIndex.row() of SignalsModel equal to signalIndex in AppSignalSet
//
// -------------------------------------------------------------------------------------------------------

class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(AppSignalSetProvider* signalSetProvider,
				 AppSignalPropertyManager* propManager,
				 QWidget* parent = nullptr);
	virtual ~SignalsModel() override;

	AppSignalSetProvider* signalSetProvider();
	AppSignalPropertyManager* propManager();

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

	SignalsTablePropEditor* createDelegate(SignalsProxyModel* signalsProxyModel);

	QWidget* parentWidget();

	void prepareForReset();
	void finishReset();

public slots:
	void slot_signalsUpdated(const std::vector<int>& indexes);
	void slot_signalsCountChanged();

	void beginIncreaseColumnCount(int newColumnCount);
	void beginDecreaseColumnCount(int newColumnCount);
	void endIncreaseColumnCount();
	void endDecreaseColumnCount();

private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;
	QWidget* m_parentWidget;
	QLocale m_defaultLocale;

	int m_rowCount = 0;
	int m_columnCount = 0;

	//

	inline static const QBrush m_checkedInBrush = { StandardColors::VcsCheckedIn };
	inline static const QBrush m_addedBrush = { StandardColors::VcsAdded };
	inline static const QBrush m_modifiedBrush = { StandardColors::VcsModified };
	inline static const QBrush m_deletedBrush = { StandardColors::VcsDeleted };
	inline static const QBrush m_excludedFromBuildBrush = { StandardColors::ExcludedFromBuildForeground };
};

// -------------------------------------------------------------------------------------------------------
//
// SignalsProxyModel class represents sorted and filtered model of AppSignalSet.
//
// This model used by AppSignals tab page table view.
// SignalsModel used as source model.
// QModelIndex.row() of SignalsProxyModel should be mapToSource() to acquire signalIndex in AppSignalSet
//
// -------------------------------------------------------------------------------------------------------

class SignalsProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SignalsProxyModel(SignalsModel* sourceModel, QObject* parent = nullptr);

	bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;
	bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

	void setSignalTypeFilter(int signalType);
	void setSignalIdFilter(QStringList strIds);
	void setIdFilterField(int field);

signals:
	void aboutToSort();	// Before sorting or filtering signals should be fully loaded
	void aboutToFilter();

protected:
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
	void applyNewFilter();

	SignalsModel* m_sourceModel;
	int m_signalType = 0;
	int m_idFilterField = 0;
	QStringList m_strIdMasks;
};

// -------------------------------------------------------------------------------------------------------
//
//	SignalsTablePropEditor class provides display and editing facilities
//	for data from SignalModel and SignalProxyModel on SignalsTabPage.
//
// -------------------------------------------------------------------------------------------------------

class SignalsTablePropEditor : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit SignalsTablePropEditor(SignalsProxyModel* signalsProxyModel,
							 QObject* parent = nullptr);
	~SignalsTablePropEditor();

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

signals:
	void itemDoubleClicked();

public slots:
	void onCloseEditorEvent(QWidget* editor, EndEditHint hint);

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model,
					 const QStyleOptionViewItem& option, const QModelIndex& index);

private:
	SignalsProxyModel* m_proxyModel = nullptr;
	AppSignalSetProvider* m_provider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	QLocale m_defaultLocale;
	QDoubleValidatorEx m_dblValidatorEx;

	mutable int m_editingSignalId = AppSignalSet::BAD_ID;
	mutable bool m_valueChanged = false;
	mutable bool m_signalCheckedOut = false;
};

// -------------------------------------------------------------------------------------------------------
//
// CheckedOutSignalsModel class represents set of checked out signals
//
// This model used by CheckInSignalsDialog and UndoSignalsDialog.
// SignalsModel used as source model.
// QModelIndex.row() of SignalsProxyModel should be mapToSource() to acquire signalIndex in AppSignalSet
//
// -------------------------------------------------------------------------------------------------------

class CheckedOutSignalsModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CheckedOutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent = nullptr);

	virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& proxyIndex, const QVariant & value, int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex & index) const override;
	bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

	void initCheckStates(const QModelIndexList& srcIndexes);
	void setAllCheckStates(bool state);
	void setCheckState(int proxyRow, Qt::CheckState state, int signalIndex);

private:
	SignalsModel* m_sourceModel;
	QTableView* m_view;
	QVector<Qt::CheckState> m_checkStates;			// referred by proxy model indexes
};
