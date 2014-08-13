#include "DialogValueEdit.h"
#include "ui_DialogValueEdit.h"

//-------------------------------------------------------------------------------------------------------
//
DialogValueEdit::DialogValueEdit(ConfigValue* pValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogValueEdit)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags()|Qt::MSWindowsFixedSizeDialogHint);

    if (pValue == nullptr)
    {
        Q_ASSERT(pValue);
        return;
    }

    this->m_pValue = pValue;

    ui->m_TypeDesc->setText(tr("Type: ") + pValue->type());

    QSize formSize = size();

    if (pValue->type() == "bool")
    {
        formSize.setHeight(100);

        ui->m_ValueEdit->setVisible(false);
        ui->m_ValueList->setVisible(false);

        ui->m_BoolCombo->addItem(tr("false"));
        ui->m_BoolCombo->addItem(tr("true"));
        ui->m_BoolCombo->setCurrentIndex(pValue->value() == "true" ? 1 : 0);
    }
    else
    {
        formSize.setHeight(100);

        ui->m_ValueList->setVisible(false);
        ui->m_BoolCombo->setVisible(false);

        ui->m_ValueEdit->setText(pValue->value());
    }

    setFixedSize(formSize);

}

//-------------------------------------------------------------------------------------------------------
//
DialogValueEdit::~DialogValueEdit()
{
    delete ui;
}

//-------------------------------------------------------------------------------------------------------
//
void DialogValueEdit::accept()
{
    if (m_pValue == nullptr)
    {
        Q_ASSERT(m_pValue);
        return;
    }
    if (m_pValue->type() == "bool")
    {
        m_pValue->setValue(ui->m_BoolCombo->currentIndex() == 1 ? "true" : "false");

    }
    else
    {
        QString text = ui->m_ValueEdit->text();
        if (m_pValue->arraySize() > 1)
        {
            // check if number of values is equal to array size
            QStringList valuesList = text.split(QRegExp("\\s+"));
            if (valuesList.count() != m_pValue->arraySize())
            {
                QMessageBox::critical(0, QString("Error"), QString("Wrong number of values, expected: ") + QString::number(m_pValue->arraySize()));
                return;
            }
        }
        m_pValue->setValue(text);
    }

    QDialog::accept();
}

