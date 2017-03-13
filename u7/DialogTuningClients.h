#ifndef DIALOGTUNINGCLIENTS_H
#define DIALOGTUNINGCLIENTS_H

#include <QDialog>

namespace Ui {
class DialogTuningClients;
}

class DialogTuningClients : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTuningClients(QWidget *parent = 0);
    ~DialogTuningClients();

private slots:
    void on_m_buttonEdit_clicked();

    void on_m_closeButton_clicked();

    void on_treeWidget_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogTuningClients *ui;
};

extern DialogTuningClients* theDialogTuningClients;

#endif // DIALOGTUNINGCLIENTS_H
