#ifndef DIALOGAFBPROPERTIES_H
#define DIALOGAFBPROPERTIES_H

#include <QDialog>
#include "AFBLibrary.h"

namespace Ui {
class DialogAFBProperties;
}

class DialogAfbProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAfbProperties(const QString& caption, QByteArray *pData, QWidget *parent = 0);
    ~DialogAfbProperties();

private slots:
    void on_DialogAFBProperties_accepted();

private:
    Ui::DialogAFBProperties *ui;
    QByteArray* m_pData;
};

#endif // DIALOGAFBPROPERTIES_H
