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

	int					columnCount(const QModelIndex &parent) const;
	int					rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant			headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant			data(const QModelIndex &index, int role) const;

	QString				textLinearity(int row, int column) const;
	QString				textComparator(int row, int column) const;

public:

	MeasureBase			m_measureBase;

	int					measureType() const { return m_measureType; }
	void				setMeasureType(int measureType);

	MeasureViewHeader&	header() { return m_header; }

	int					count() const { return m_measureBase.count(); }

	bool				columnIsVisible(int column);

	bool				append(Measurement* pMeasurement);
	bool				remove(const QVector<int>& removeIndexList);

	QColor				backgroundColor(int row, int column) const;
	QString				text(int row, int column) const;
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

	void				appendMeasure(Measurement* pMeasurement);
	void				removeMeasure();

	void				copy();
};

// ==============================================================================================

#endif // MEASUREVIEW_H
