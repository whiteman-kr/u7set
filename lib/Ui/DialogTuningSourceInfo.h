#ifndef DIALOGTUNINGSOURCEINFO_H
#define DIALOGTUNINGSOURCEINFO_H

#include <QDialog>
#include "../Hash.h"

class TuningTcpClient;

class DialogTuningSourceInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(TuningTcpClient* tcpClient, QWidget* parent, Hash tuningSourceEquipmentId);
	~DialogTuningSourceInfo();

protected:
	void timerEvent(QTimerEvent* event);

private:
	void updateData();

	void setChildNumberDelta(QTreeWidgetItem* item, int childIndex, quint64 number, quint64 previousNumber);
	void setChildNumber(QTreeWidgetItem* item, int childIndex, quint64 number);
	void setChildText(QTreeWidgetItem* item, int childIndex, const QString& text);

	void updateParentItemState(QTreeWidgetItem* item);


private:
	int m_updateStateTimerId = -1;

	Hash m_tuningSourceEquipmentId = 0;

private:

	TuningTcpClient* m_tcpClient = nullptr;

	QTreeWidget* m_treeWidget = nullptr;
};

#endif // DIALOGTUNINGSOURCEINFO_H
