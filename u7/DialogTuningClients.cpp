#include "DialogTuningClients.h"
#include "ui_DialogTuningClients.h"

DialogTuningClients* theDialogTuningClients = nullptr;

DialogTuningClients::DialogTuningClients(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::DialogTuningClients)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
}

DialogTuningClients::~DialogTuningClients()
{
    delete ui;
}

void DialogTuningClients::on_m_buttonEdit_clicked()
{

}

void DialogTuningClients::on_m_closeButton_clicked()
{

}

void DialogTuningClients::on_treeWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);

}
