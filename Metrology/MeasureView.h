#ifndef MEASUREVIEW_H
#define MEASUREVIEW_H

#include <QTableView>
#include <QMenu>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

#include "MeasureViewHeader.h"
#include "MeasureBase.h"

// ==============================================================================================

class MeasureTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit MeasureTable(QObject* parent = nullptr);
	virtual ~MeasureTable();

public:

	int measureType() const { return m_measureType; }
	void setMeasureType(int measureType) { m_measureType = measureType; }

	int count() const { return m_measureCount; }

	MeasureViewHeader& header() { return m_header; }
	bool columnIsVisible(int column);

	QColor backgroundColor(int row, int column, Measurement* pMeasurement) const;
	QString text(int row, int column, Measurement* pMeasurement) const;

	bool append(Measurement* pMeasurement);
	Measurement* at(int index) const;
	void remove(const QVector<int>& removeIndexList);

	void set(const QVector<Measurement*>& list_add);
	void clear();

private:

	int m_measureType = MeasureType::NoMeasureType;

	MeasureViewHeader m_header;

	mutable QMutex m_measureMutex;
	QVector<Measurement*> m_measureList;
	int m_measureCount = 0;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;

	QString textLinearity(int row, int column, Measurement* pMeasurement) const;
	QString textComparator(int row, int column, Measurement* pMeasurement) const;
};

// ==============================================================================================

class MeasureView : public QTableView
{
	Q_OBJECT

public:

	explicit MeasureView(MeasureType measureType, QWidget* parent = nullptr);
	virtual ~MeasureView();

public:

	MeasureType	measureType() const { return m_measureType; }
	MeasureTable& table() { return m_table; }

	void updateColumn();

private:

	MeasureType	m_measureType = MeasureType::NoMeasureType;
	MeasureTable m_table;

	QMenu* m_headerContextMenu = nullptr;
	QVector<QAction*> m_actionList;

	void createContextMenu();

signals:

	void removeFromBase(int measureType, const QVector<int>& keyList);

public slots:

	void onHeaderContextMenu(QPoint);
	void onHeaderContextAction(QAction* action);

	void onColumnResized(int index, int, int width);

	void loadMeasurements(const MeasureBase& measureBase);

	void appendMeasure(Measurement* pMeasurement);
	void removeMeasure();

	void copy();

	void showGraph(int graphType);
};

// ==============================================================================================

class ChartView : public QtCharts::QChartView
{

public:

	ChartView(QtCharts::QChart* chart, QWidget* parent = nullptr);

protected:

	bool viewportEvent(QEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);

private:

	bool m_isTouching = false;
};

// ==============================================================================================

#endif // MEASUREVIEW_H
