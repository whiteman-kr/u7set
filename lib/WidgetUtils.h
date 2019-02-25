#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

class QTableView;
class QStandardItem;
class QListView;
class QHeaderView;
class QStandardItemModel;
class QAbstractItemModel;

#include <QDialog>

void saveWindowPosition(QWidget* window, QString widgetKey);
void setWindowPosition(QWidget* window, QString widgetKey);

class TableDataVisibilityController : public QObject
{
	Q_OBJECT
public:
	TableDataVisibilityController(QTableView* parent, const QString& settingsBranchName, const QVector<int>& defaultVisibleColumnSet);
	virtual ~TableDataVisibilityController();

	void editColumnsVisibilityAndOrder();

	void saveColumnVisibility(int index, bool visible);
	void saveColumnPosition(int index, int position);

public slots:
	void saveColumnWidth(int index);
	void saveAllHeaderGeomery();

private:
	QTableView* m_tableView = nullptr;
	QStringList m_columnNameList;
	QString m_settingBranchName;
};

class EditColumnsVisibilityDialog : public QDialog
{
	Q_OBJECT
public:
	EditColumnsVisibilityDialog(QTableView* tableView, TableDataVisibilityController* controller);
	virtual ~EditColumnsVisibilityDialog() {}

private:
	void updateItems(QList<int> selectedLogicalIndexes = QList<int>(), int currentLogicalIndex = -1);

	bool isHidden(int logicalIndex);
	void updateHidden(int visualIndex, bool hidden);
	void setHidden(int logicalIndex, bool hidden);

private slots:
	void moveUp();
	void moveDown();
	void changeVisibility(QStandardItem* item);

private:
	TableDataVisibilityController* m_controller = nullptr;
	QHeaderView* m_header = nullptr;
	QListView* m_columnList = nullptr;
	QStandardItemModel* m_columnModel = nullptr;
	QAbstractItemModel* m_tableModel = nullptr;

	bool m_changingItems = false;
};

#endif // WIDGETUTILS_H
