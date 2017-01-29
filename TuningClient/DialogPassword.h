#ifndef DIALOGPASSWORD_H
#define DIALOGPASSWORD_H

#include <QDialog>
#include <QMessageBox>

#include "UserManager.h"

namespace Ui {
class DialogPassword;
}

class DialogPassword : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPassword(const UserManager *userManager, bool adminNeeded, QWidget *parent);
    ~DialogPassword();

private:

    virtual void accept();

private:
    Ui::DialogPassword *ui;

    static QString m_lastUser;

    const UserManager* m_userManager = nullptr;
};

#endif // DIALOGPASSWORD_H
