#ifndef ARCHIVEMODELVIEW_H
#define ARCHIVEMODELVIEW_H
#include <QTableView>
#include "ArchiveData.h"

enum class ArchiveColumns
{
	Row = 0,
	AppSignalId,
	CustomSignalId,
	Caption,
	State,
	Valid,					// Hidden by default in MonitorArchiveWidget::MonitorArchiveWidget!!!, cannot do it in ArchiveView constructor, don't know why(((
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
	Q_OBJECT
public:
	explicit ArchiveModel(QObject* parent = nullptr);

public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant data(int row, int column, int role) const;

private:
	QString getValueString(const AppSignalState& state, const AppSignalParam& signalParam) const;
	void updateCachedState(int row) const;

	// Data manipultaion
	//
public:
	void setParams(const std::vector<AppSignalParam>& appSignals, E::TimeType timeType);
	void addData(std::shared_ptr<ArchiveChunk> chunk);
	void clear();

	std::vector<AppSignalParam> appSignals();

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
	Q_OBJECT

public:
	explicit ArchiveView(QWidget* parent = nullptr);
	virtual ~ArchiveView();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event);

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

	void copySelection();

signals:
	void removeAppSignal(QString appSignalId);

	void requestToShowSignalInfo(QString appSignalId);
	void requestToRemoveSignal(QString appSignalId);
	void requestToCopySelection();
	void requestToSetSignals();

	// Data
	//
private:
	QMenu m_columnMenu;

	QAction* copyAction = nullptr;
};

#endif // ARCHIVEMODELVIEW_H
