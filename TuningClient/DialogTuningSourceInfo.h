#ifndef DIALOGTUNINGSOURCEINFO_H
#define DIALOGTUNINGSOURCEINFO_H

#include <QDialog>

#include "TuningClientTcpClient.h"

namespace Ui {
	class DialogTuningSourceInfo;
}

class DialogTuningSourceInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSourceInfo(TuningClientTcpClient* tcpClient, QWidget* parent, Hash tuningSourceEquipmentId);
	~DialogTuningSourceInfo();

protected:
	void timerEvent(QTimerEvent* event);

private:
	void updateData();

	void setChildText(QTreeWidgetItem* item, int childIndex, quint64 number, quint64 previousNumber);
	void setChildText(QTreeWidgetItem* item, int childIndex, quint64 number);
	void setChildText(QTreeWidgetItem* item, int childIndex, const QString& text);

	void updateParentItemState(QTreeWidgetItem* item);


private:
	int m_updateStateTimerId = -1;

	Hash m_tuningSourceEquipmentId = 0;

private:
	Ui::DialogTuningSourceInfo* ui;

	TuningClientTcpClient* m_tcpClient = nullptr;
};

#endif // DIALOGTUNINGSOURCEINFO_H
