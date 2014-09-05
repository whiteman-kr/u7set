#ifndef DIALOGAFBLEDITOR_H
#define DIALOGAFBLEDITOR_H

#include <QDialog>
#include "../include/DbController.h"
#include "AfbLibrary.h"

namespace Ui {
class DialogAfblEditor;
}

class DialogAfblEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAfblEditor(DbController* pDbController, QWidget *parent = 0);
    ~DialogAfblEditor();

private slots:
    void on_m_add_clicked();
    void on_m_edit_clicked();
    void on_m_checkOut_clicked();
    void on_m_checkIn_clicked();
    void on_m_Undo_clicked();

private:
    Ui::DialogAfblEditor *ui;

    DbController* m_pDbController;
    std::vector<DbFileInfo> files;

private:
    void refreshFiles();
    DbFileInfo* getCurrentFileInfo();
};

#endif // DIALOGAFBLEDITOR_H
