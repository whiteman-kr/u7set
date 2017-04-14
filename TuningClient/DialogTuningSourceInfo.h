#ifndef DIALOGTUNINGSOURCEINFO_H
#define DIALOGTUNINGSOURCEINFO_H

#include <QDialog>

#include "../lib/Tuning/TuningObjectManager.h"

namespace Ui {
class DialogTuningSourceInfo;
}

class DialogTuningSourceInfo : public QDialog
{
    Q_OBJECT

public:
	explicit DialogTuningSourceInfo(TuningObjectManager *tuningObjectManager, QWidget *parent, quint64 tuningSourceId);
    ~DialogTuningSourceInfo();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void updateData();

private:
    int m_updateStateTimerId = -1;

    quint64 m_tuningSourceId = -1;

private:
    Ui::DialogTuningSourceInfo *ui;

	TuningObjectManager* m_tuningObjectManager = nullptr;
};

#endif // DIALOGTUNINGSOURCEINFO_H
