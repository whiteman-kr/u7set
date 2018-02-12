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
	explicit DialogTuningSourceInfo(TuningClientTcpClient* tcpClient, QWidget* parent, quint64 tuningSourceId);
	~DialogTuningSourceInfo();

protected:
	void timerEvent(QTimerEvent* event);

private:
	void updateData();

private:
	int m_updateStateTimerId = -1;

	quint64 m_tuningSourceId = 0;

private:
	Ui::DialogTuningSourceInfo* ui;

	TuningClientTcpClient* m_tcpClient = nullptr;
};

#endif // DIALOGTUNINGSOURCEINFO_H
