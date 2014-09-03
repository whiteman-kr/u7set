#include "scanoptionswidget.h"
#include <QRegExpValidator>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QNetworkInterface>
#include <QDialogButtonBox>

ScanOptionsWidget::ScanOptionsWidget(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Scan settings"));
    QRegExp re("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?:/(?:[1-9]|[1-2][0-9]|3[0-2]?)?)\\b");
    QRegExpValidator* rev = new QRegExpValidator(re);
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setValidator(rev);

    QFormLayout* fl = new QFormLayout;
    fl->addRow(tr("Enter IP or subnet"), m_addressEdit);

    QComboBox* addressCombo = new QComboBox;
    connect(addressCombo, SIGNAL(currentTextChanged(QString)), m_addressEdit, SLOT(setText(QString)));

    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaceList.count(); i++)
    {
        QList<QNetworkAddressEntry> addressList = interfaceList[i].addressEntries();
        for (int j = 0; j < addressList.count(); j++)
        {
            QHostAddress ip = addressList[j].ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol)
            {
                continue;
            }
            QString ipStr = ip.toString();
            if (addressList[j].prefixLength() >= 0)
            {
                ipStr += "/" + QString::number(32 - addressList[j].prefixLength());
            }
            addressCombo->addItem(ipStr);
        }
    }

    fl->addRow(tr("Or select from following"), addressCombo);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    fl->addRow(buttonBox);

    setLayout(fl);
}

QString ScanOptionsWidget::getSelectedAddress()
{
    return m_addressEdit->text();
}
