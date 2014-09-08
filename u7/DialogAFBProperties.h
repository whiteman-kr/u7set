#ifndef DIALOGAFBPROPERTIES_H
#define DIALOGAFBPROPERTIES_H

#include <QDialog>
#include "AFBLibrary.h"

namespace Ui {
class DialogAfbProperties;
}

class DialogAfbProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAfbProperties(const QString& caption, QByteArray *pData, DbController* pDbController, QWidget *parent = 0);
    ~DialogAfbProperties();

private slots:
    void on_DialogAfbProperties_accepted();
    void on_validate(QAction* pAction);
    void on_m_validate_clicked();

private:
    Ui::DialogAfbProperties *ui;
    QByteArray* m_pData;

    DbController* m_pDbController;
    std::vector<DbFileInfo> m_validateFiles;

    bool validate(int schemaFileId);
};

#endif // DIALOGAFBPROPERTIES_H
