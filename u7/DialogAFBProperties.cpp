#include "DialogAFBProperties.h"
#include "ui_DialogAFBProperties.h"
#include <QtXmlPatterns>
#include "xmlsyntaxhighlighter.h"


DialogAfbProperties::DialogAfbProperties(const QString &caption, QByteArray* pData, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::DialogAFBProperties),
    m_pData(pData)
{
    ui->setupUi(this);
    setWindowTitle(caption);
    new XmlSyntaxHighlighter(ui->m_text->document());

    QString s(*pData);

    ui->m_text->setPlainText(s);
    ui->m_text->setFont(QFont("Courier", 14));
}

DialogAfbProperties::~DialogAfbProperties()
{
    delete ui;
}

void DialogAfbProperties::on_DialogAFBProperties_accepted()
{

    QString s = ui->m_text->toPlainText();

    *m_pData = s.toUtf8();

}
