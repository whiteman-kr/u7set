#pragma once

#include <QDialog>
#include <CommonLib/HashedVector.h>

class QStandardItem;
class QListView;
class QStandardItemModel;
class QAbstractItemModel;
class QHeaderView;

void saveWindowPosition(QWidget* window, QString widgetKey);
void setWindowPosition(QWidget* window, QString widgetKey);

class TableDataVisibilityController : public QObject
{
	Q_OBJECT
public:
	TableDataVisibilityController(QTableView* parent,
								  const QString& settingsBranchName,
								  const QVector<int>& defaultVisibleColumnSet,
								  bool showAllDefaultColumns = false);
	virtual ~TableDataVisibilityController();

	void saveColumnVisibility(int index, bool visible);
	void saveColumnPosition(int index, int position);
	void saveColumnWidth(int index, int width);

	int getColumnPosition(int index) const;
	bool getColumnVisibility(int index) const;
	int getColumnWidth(int index) const;

	void showColumn(int index, bool visible = true);
	void relocateAllColumns();

public slots:
	void onColumnResized(int index, int oldSize, int newSize);
	void onColumnMoved(int index, int oldVisualIndex, int newVisualIndex);

	void editColumnsVisibilityAndOrder();

	void saveAllHeaderGeomery();
	void checkNewColumns();

private:
	class ColumnInfo
	{
	public:
		QString columnName() const;
		void setColumnName(const QString& colName);

		QString settingName() const;

		int position() const;
		void setPosition(int pos);

		bool visible() const;
		void setVisible(bool visible);

		int width() const;
		void setWidth(int width);

		QString saveParamsToString() const;
		void readParamsFromString(const QString& str);

	private:
		QString m_columnName;
		QString m_settingName;		// ==  escape(m_columnName)

		// params

		int m_position = -1;
		bool m_visible = false;
		int m_width = 0;

		static const QString VISIBLE;
		static const QString HIDDEN;
	};

private:
	void saveColumnInfo(const ColumnInfo& ci) const;
	void loadColumnInfo(const QString& columnName, ColumnInfo* ci) const;
	bool isValidColumnIndex(int index) const;

private:
	QTableView* m_tableView = nullptr;
	mutable QSettings m_settings;
	HashedVector<QString, ColumnInfo> m_columnsInfo;			// ColumnName => ColumnInfo
	QString m_settingBranchName;
	QVector<int> m_defaultVisibleColumnSet;
	bool m_showAllDefaultColumns;
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
