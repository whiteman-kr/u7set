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
		virtual ~Table() override;

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
		void remove(const std::vector<int>& removeIndexList);

		void set(const std::vector<Item*>& list_add);
		void clear();

	private:

		int m_measureType = Measure::Type::NoMeasureType;

		ViewHeader m_header;

		mutable QMutex m_measureMutex;
		std::vector<Measure::Item*> m_measureList;
		quint64 m_measureCount = 0;

		int columnCount(const QModelIndex &parent) const override;
		int rowCount(const QModelIndex &parent=QModelIndex()) const override;

		QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
		QVariant data(const QModelIndex &index, int role) const override;

		QString textLinearity(int row, int column, Measure::Item* pMeasurement) const;
		QString textComparator(int row, int column, Measure::Item* pMeasurement) const;
	};

	// ==============================================================================================

	class View : public QTableView
	{
		Q_OBJECT

	public:

		explicit View(Measure::Type measureType, QWidget* parent = nullptr);
		virtual ~View() override;

	public:

		Measure::Type measureType() const { return m_measureType; }
		Table& table() { return m_table; }

		void updateColumn();

	private:

		Measure::Type m_measureType = Measure::Type::NoMeasureType;
		Table m_table;

		QMenu* m_headerContextMenu = nullptr;

		void createContextMenu();

	signals:

		void removeFromBase(Measure::Type measureType, const std::vector<int>& keyList);

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

	bool viewportEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:

	bool m_isTouching = false;
};

// ==============================================================================================

#endif // MEASUREVIEW_H
