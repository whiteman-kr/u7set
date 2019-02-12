#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

class QTableView;

#include <QDesktopWidget>

void saveWindowPosition(QWidget* window, QString widgetKey);
void setWindowPosition(QWidget* window, QString widgetKey);

class TableDataVisibilityController : public QObject
{
	Q_OBJECT
public:
	TableDataVisibilityController(QTableView* parent, const QString& settingsBranchName, const QVector<int>& defaultVisibleColumnSet);
	virtual ~TableDataVisibilityController();

	void editColumnsVisibilityAndOrder();

private:
	void saveColumnVisibility(int index, bool visible);
	void saveColumnPosition(int index, int position);

private slots:
	void saveColumnWidth(int index);

private:
	QTableView* m_signalsView = nullptr;
	QStringList m_columnNameList;
	QString m_settingBranchName;
};

#endif // WIDGETUTILS_H
