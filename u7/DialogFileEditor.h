#ifndef DIALOGFILEEDITOR_H
#define DIALOGFILEEDITOR_H

#include <QDialog>
#include "../include/DbController.h"

namespace Ui {
class DialogFileEditor;
}

class DialogFileEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent = 0);
    ~DialogFileEditor();


private slots:
    void on_DialogFileEditor_accepted();

    void on_DialogFileEditor_finished(int result);

private:
    Ui::DialogFileEditor *ui;

    QByteArray* m_pData;

    DbController* m_pDbController;
    bool m_readOnly;
};

#endif // DIALOGFILEEDITOR_H
