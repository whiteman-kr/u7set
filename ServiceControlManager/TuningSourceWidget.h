#pragma once

#include <QWidget>

#include "DataSourceInfoModel.h"
#include "TuningSourceStateModel.h"

class QTableView;
class QStandardItemModel;
class QSplitter;

class TuningSourceWidget : public QWidget
{
	Q_OBJECT
public:
	TuningSourceWidget(const QString& equipmentID, QWidget* parent);
	~TuningSourceWidget();

	void updateData(const Network::TuningSourceInfoState& state);

signals:
	void forgetMe(QString dataSoureID);

protected:
	void closeEvent(QCloseEvent* event);

private:
	void initTable(QTableView* table, QAbstractTableModel* model);

private:
	QString m_equipmentID;

	QTableView* m_infoTable = nullptr;
	DataSourceInfoModel m_infoModel;

	QTableView* m_stateTable = nullptr;
	TuningSourceStateModel m_stateModel;

	inline static const QString TUNING_SRC_WIDGET_KEY = QString("TuningSourceWidget/");
	inline static const QString INFO_COLUMN_WIDTH_KEY = QString("/infoColumnWidth");
	inline static const QString STATE_COLUMN_WIDTH_KEY = QString("/stateColumnWidth");
};

