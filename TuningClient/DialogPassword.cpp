#include "DialogPassword.h"
#include "ui_DialogPassword.h"

QString DialogPassword::m_lastUser = "";

DialogPassword::DialogPassword(const UserManager *userManager, bool adminNeeded, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPassword),
    m_userManager(userManager)
{
    ui->setupUi(this);

    int selectedIndex = -1;

    setWindowTitle(adminNeeded ? tr("Enter Administrator Password") : tr("Enter Password"));

    int i = 0;

    for (const User& user : m_userManager->m_users)
    {
        if (adminNeeded && user.admin() == false)
        {
            continue;
        }

        ui->m_userCombo->addItem(user.name(), i);

        if (user.name() == m_lastUser)
        {
            selectedIndex = i;
        }

        i++;
    }

    if (selectedIndex != -1)
    {
        ui->m_userCombo->setCurrentIndex(selectedIndex);
    }
}

DialogPassword::~DialogPassword()
{
    delete ui;
}

void DialogPassword::accept()
{
    QVariant data = ui->m_userCombo->currentData();
    if (data.isValid() == false)
    {
        return;
    }

    int index = data.toInt();

    if (index < 0 || index >= m_userManager->m_users.size())
    {
        assert(false);
        return;
    }

    const User& user = m_userManager->m_users[index];

    QCryptographicHash md5Generator(QCryptographicHash::Md5);
    md5Generator.addData(ui->m_passwordEdit->text().toUtf8());

    if (user.password() != md5Generator.result().toHex())
    {
        QMessageBox::critical(this, tr("Password"), tr("Wrong password!"));
        QDialog::reject();
        return;
    }

    m_lastUser = user.name();

    QDialog::accept();

}
