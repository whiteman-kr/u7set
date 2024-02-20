#ifndef DIALOGSOURCEINFO_H
#define DIALOGSOURCEINFO_H

#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;

class DialogSourceInfo : public QDialog
{
	Q_OBJECT

protected:
	DialogSourceInfo(QWidget* parent, quint64 dialogId);
	virtual ~DialogSourceInfo();

protected:
	void createDataItem(QTreeWidgetItem* parentItem, const QString& caption);
	QTreeWidgetItem* dataItem(const QString& caption);

	void setDataItemNumberCompare(QTreeWidgetItem* parentItem, const QString& caption, quint64 number, quint64 previousNumber);
	void setDataItemNumber(const QString& caption, quint64 number);
	void setDataItemText(const QString& caption, const QString& text);

	void updateParentItemState(QTreeWidgetItem* item);

protected:
	void timerEvent(QTimerEvent* event);
	virtual void updateData() = 0;

public slots:
	void prepareContextMenu(const QPoint& pos);

signals:
	void dialogClosed(quint64 dialogId);

public:
	static QColor dataItemErrorColor;

protected:
	quint64 m_dialogId;
	QTreeWidget* m_treeWidget = nullptr;

public:
	virtual void accept() override;
	virtual void reject() override;

private:
	std::map<QString, QTreeWidgetItem*> m_treeWidgetItemsMap;
	int m_updateStateTimerId = -1;
};


#endif // DIALOGSOURCEINFO_H
