#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <QDialog>

namespace Ui {
class DialogTuningSources;
}

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(QWidget *parent = 0);
	~DialogTuningSources();

private:
	Ui::DialogTuningSources *ui;
};

extern DialogTuningSources* theDialogTuningSources;

#endif // DIALOGTUNINGSOURCES_H
