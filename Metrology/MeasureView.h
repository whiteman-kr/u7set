#ifndef MEASUREVIEW_H
#define MEASUREVIEW_H

#include <QTableView>
#include <QMenu>

#include "MeasureViewHeader.h"
#include "MeasureBase.h"

// ==============================================================================================

class MeasureTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit MeasureTable(QObject* parent = nullptr);
	virtual ~MeasureTable();

private:

	int					m_measureType = MEASURE_TYPE_UNKNOWN;

	MeasureViewHeader	m_header;

	mutable QMutex		m_measureMutex;
	QVector<Measurement*> m_measureList;
	int					m_measureCount = 0;

	int					columnCount(const QModelIndex &parent) const;
	int					rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant			headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant			data(const QModelIndex &index, int role) const;

	QString				textLinearity(int row, int column, Measurement* pMeasurement) const;
	QString				textComparator(int row, int column, Measurement* pMeasurement) const;

public:

	int					measureType() const { return m_measureType; }
	void				setMeasureType(int measureType) { m_measureType = measureType; }

	int					count() const { return m_measureCount; }

	MeasureViewHeader&	header() { return m_header; }
	bool				columnIsVisible(int column);

	QColor				backgroundColor(int row, int column, Measurement* pMeasurement) const;
	QString				text(int row, int column, Measurement* pMeasurement) const;

	bool				append(Measurement* pMeasurement);
	Measurement*		at(int index) const;
	void				remove(const QVector<int>& removeIndexList);

	void				set(const QVector<Measurement*>& list_add);
	void				clear();
};

// ==============================================================================================

class MeasureView : public QTableView
{
	Q_OBJECT

public:

	explicit MeasureView(int measureType, QWidget *parent = nullptr);
	virtual ~MeasureView();

private:

	int					m_measureType = MEASURE_TYPE_UNKNOWN;
	MeasureTable		m_table;

	QMenu*				m_headerContextMenu = nullptr;
	QVector<QAction*>	m_actionList;

	void				createContextMenu();

public:

	int					measureType() const { return m_measureType; }
	MeasureTable&		table() { return m_table; }

	void				updateColumn();

signals:

public slots:

	void				onHeaderContextMenu(QPoint);
	void				onHeaderContextAction(QAction* action);

	void				onColumnResized(int index, int, int width);

	void				loadMeasureList();

	void				appendMeasure(Measurement* pMeasurement);
	void				removeMeasure();

	void				copy();
};

// ==============================================================================================

#endif // MEASUREVIEW_H
