#include "DialogFileEditor.h"
#include "ui_DialogFileEditor.h"
#include "xmlsyntaxhighlighter.h"
#include "Settings.h"

DialogFileEditor::DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::DialogFileEditor),
    m_pData(pData),
    m_pDbController(pDbController),
    m_readOnly(readOnly)
{
    ui->setupUi(this);
    setWindowTitle(fileName);

    QString ext = QFileInfo(fileName).suffix();
    if (ext == tr("descr"))
    {
        new CppSyntaxHighlighter(ui->m_text->document());
    }
    if (ext == tr("afb") || ext == tr("xml") || ext == tr("xsd"))
    {
        new XmlSyntaxHighlighter(ui->m_text->document());
    }

    if (pData == nullptr)
    {
        Q_ASSERT(pData);
        return;
    }

    QString s(*pData);

    ui->m_text->setPlainText(s);
    ui->m_text->setFont(QFont("Courier", 14));

    if (readOnly == true)
    {
        ui->m_okCancel->setEnabled(false);
        setWindowTitle(windowTitle() + tr(" [View Only]"));
        ui->m_text->setReadOnly(true);
    }

    if (theSettings.m_DialogTextEditorWindowPos.x() != -1 && theSettings.m_DialogTextEditorWindowPos.y() != -1)
    {
        move(theSettings.m_DialogTextEditorWindowPos);
        restoreGeometry(theSettings.m_DialogTextEditorWindowGeometry);
    }
}

DialogFileEditor::~DialogFileEditor()
{
    delete ui;
}

void DialogFileEditor::on_DialogFileEditor_accepted()
{
    if (m_pData == nullptr)
    {
        Q_ASSERT(m_pData);
        return;
    }

    if (m_readOnly == false)
    {
        QString s = ui->m_text->toPlainText();
        *m_pData = s.toUtf8();
    }

}

void DialogFileEditor::on_DialogFileEditor_finished(int result)
{
    Q_UNUSED(result);

    theSettings.m_DialogTextEditorWindowPos = pos();
    theSettings.m_DialogTextEditorWindowGeometry = saveGeometry();

}
