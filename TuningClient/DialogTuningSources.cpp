#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"

DialogTuningSources::DialogTuningSources(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogTuningSources)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
}

DialogTuningSources::~DialogTuningSources()
{
	theDialogTuningSources = nullptr;
	delete ui;
}

DialogTuningSources* theDialogTuningSources = nullptr;
