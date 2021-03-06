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

namespace Measure
{
	// ==============================================================================================

	class Table : public QAbstractTableModel
	{
		Q_OBJECT

	public:

		explicit Table(QObject* parent = nullptr);
		virtual ~Table();

	public:

		int measureType() const { return m_measureType; }
		void setMeasureType(int measureType) { m_measureType = measureType; }

		int count() const { return m_measureCount; }

		ViewHeader& header() { return m_header; }
		bool columnIsVisible(int column);

		QColor backgroundColor(int row, int column, Measure::Item* pMeasurement) const;
		QString text(int row, int column, Measure::Item* pMeasurement) const;

		bool append(Measure::Item* pMeasurement);
		Measure::Item* at(int index) const;
		void remove(const QVector<int>& removeIndexList);

		void set(const QVector<Measure::Item*>& list_add);
		void clear();

	private:

		int m_measureType = Measure::Type::NoMeasureType;

		ViewHeader m_header;

		mutable QMutex m_measureMutex;
		QVector<Measure::Item*> m_measureList;
		int m_measureCount = 0;

		int columnCount(const QModelIndex &parent) const;
		int rowCount(const QModelIndex &parent=QModelIndex()) const;

		QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		QVariant data(const QModelIndex &index, int role) const;

		QString textLinearity(int row, int column, Measure::Item* pMeasurement) const;
		QString textComparator(int row, int column, Measure::Item* pMeasurement) const;
	};

	// ==============================================================================================

	class View : public QTableView
	{
		Q_OBJECT

	public:

		explicit View(Measure::Type measureType, QWidget* parent = nullptr);
		virtual ~View();

	public:

		Measure::Type measureType() const { return m_measureType; }
		Table& table() { return m_table; }

		void updateColumn();

	private:

		Measure::Type m_measureType = Measure::Type::NoMeasureType;
		Table m_table;

		QMenu* m_headerContextMenu = nullptr;
		QVector<QAction*> m_actionList;

		void createContextMenu();

	signals:

		void removeFromBase(int measureType, const QVector<int>& keyList);

	public slots:

		void onHeaderContextMenu(QPoint);
		void onHeaderContextAction(QAction* action);

		void onColumnResized(int index, int, int width);

		void loadMeasurements(const Measure::Base& measureBase);

		void appendMeasure(Measure::Item* pMeasurement);
		void removeMeasure();

		void copy();

		void showGraph(int graphType);
	};
}

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
