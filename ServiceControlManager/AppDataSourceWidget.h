#pragma once

#include <QWidget>

#include "DataSourceInfoModel.h"
#include "AppDataSourceStateModel.h"

class QTableView;
class QStandardItemModel;
class QSplitter;

class AppDataSourceWidget : public QWidget
{
	Q_OBJECT
public:
	AppDataSourceWidget(const QString& lanControllerID, QWidget* parent);
	~AppDataSourceWidget();

	void updateData(const Network::AppDataSourceState& state);

signals:
	void forgetMe(QString dataSoureID);

protected:
	void closeEvent(QCloseEvent* event);

private:
	void initTable(QTableView* table, QAbstractTableModel* model);

private:
	QString m_lanControllerID;

	QTableView* m_infoTable = nullptr;
	DataSourceInfoModel m_infoModel;

	QTableView* m_stateTable = nullptr;
	AppDataSourceStateModel m_stateModel;

	inline static const QString APP_DATA_SRC_WIDGET_KEY = QString("AppDataSourceWidget/");
	inline static const QString INFO_COLUMN_WIDTH_KEY = QString("/infoColumnWidth");
	inline static const QString STATE_COLUMN_WIDTH_KEY = QString("/stateColumnWidth");
};

