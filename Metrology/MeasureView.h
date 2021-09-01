#ifndef MEASUREVIEW_H
#define MEASUREVIEW_H

#include <QTableView>
#include <QMenu>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QClipboard>

#include "MeasureViewHeader.h"
#include "MeasureBase.h"
#include "ChartView.h"

namespace Measure
{
	// ==============================================================================================

	class Model : public QAbstractTableModel
	{
		Q_OBJECT

	public:

		explicit Model(QObject* parent = nullptr);
		virtual ~Model() override;

	public:

		Measure::Type measureType() const { return m_measureType; }
		void setMeasureType(Measure::Type measureType) { m_measureType = measureType; }

		int count() const { return TO_INT(m_measureCount); }

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

		Measure::Type m_measureType = Measure::Type::NoMeasureType;

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
		Model& measureModel() { return m_model; }

		void updateColumn();

	private:

		Measure::Type m_measureType = Measure::Type::NoMeasureType;
		Model m_model;

		QMenu* m_headerContextMenu = nullptr;

		void createContextMenu();

		int firstVisibleColumn();

	signals:

		void removeFromBase(Measure::Type measureType, const std::vector<int>& keyList);
		void updateInBase(Measure::Type measureType, const std::vector<Measure::Item*>& list);

	public slots:

		// slots of measure
		//
		void loadMeasurements(const Measure::Base& measureBase);

		void appendMeasure(Measure::Item* pMeasurement);
		void removeMeasure();

		//
		//
		void onCopy();
		void onCopyCell();
		void onProperty();

		void showChart(ChartType chartType);

		// slots for list header, to hide or show columns
		//
		void onHeaderContextMenu(QPoint);
		void onColumnAction(QAction* action);
		void onColumnResized(int index, int, int width);
	};
}

// ==============================================================================================

#endif // MEASUREVIEW_H
