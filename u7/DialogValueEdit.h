#ifndef DIALOGVALUEEDIT_H
#define DIALOGVALUEEDIT_H

#include "../include/ConfigData.h"

namespace Ui {
class DialogValueEdit;
}

class DialogValueEdit : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogValueEdit(ConfigValue* pValue, QWidget *parent = 0);
    ~DialogValueEdit();
    
public slots:
    void accept();

private:
    Ui::DialogValueEdit *ui;

    //Data
    //
private:
    ConfigValue* m_pValue;
};

#endif // DIALOGVALUEEDIT_H
