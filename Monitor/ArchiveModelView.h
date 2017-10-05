#ifndef ARCHIVEMODELVIEW_H
#define ARCHIVEMODELVIEW_H
#include <QTableView>
#include "ArchiveData.h"

enum class ArchiveColumns
{
	Row = 0,
	CustomSignalId,
	Caption,
	State,
	Valid,						// Hidden by default in MonitorArchiveWidget::MonitorArchiveWidget, cannot do it in ArchiveView constructor, don't know why(((
	Time,

	ColumnCount
};

Q_DECLARE_METATYPE(ArchiveColumns);

//
//
//		ArchiveModel
//
//
class ArchiveModel : public QAbstractTableModel
{
public:
	explicit ArchiveModel(QObject* parent = nullptr);

public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
	QString getValueString(const AppSignalState& state, const AppSignalParam& signalParam) const;
	void updateCachedState(int row) const;

	// Data manipultaion
	//
public:
	void setParams(const std::vector<AppSignalParam>& appSignals, E::TimeType timeType);
	void addData(std::shared_ptr<ArchiveChunk> chunk);
	void clear();

	// Data
	//
private:
	std::map<Hash, AppSignalParam> m_appSignals;
	E::TimeType m_timeType = E::TimeType::Local;
	ArchiveData m_archive;

	mutable int m_cachedStateIndex = -1;
	mutable AppSignalState m_cachedSignalState;

	QString nonValidString = "?";
};


//
//
//		ArchiveView
//
//
class ArchiveView : public QTableView
{
public:
	explicit ArchiveView(QWidget* parent = nullptr);
	virtual ~ArchiveView();

protected:

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

	// Data
	//
private:
	QMenu m_columnMenu;
};

#endif // ARCHIVEMODELVIEW_H
