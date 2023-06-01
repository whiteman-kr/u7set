#include "ReportPropertyEditor.h"
#include "../ReportLib/ReportTemplate.h"

ReportPropertyEditor::ReportPropertyEditor(QWidget* parent):
    PropertyTextEditor(parent),
    m_parent(parent)
{
    if (m_parent == nullptr)
    {
        Q_ASSERT(m_parent);
        return;
    }

    // TextEditor
    //
    m_textEdit = new CodeEditor(this);

#if defined(Q_OS_WIN)
        QFont f = QFont("Consolas", 11);
#else
        QFont f = QFont("Courier");
#endif

    m_textEdit->setFont(f);

    XmlHighlighter::createXmlHighlighter(m_textEdit);

    connect(m_textEdit, &CodeEditor::textChanged, this, &ReportPropertyEditor::onTextChanged);

    // TreeWidget
    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderLabels(QStringList() << "Object" << "Type" << "Properties");
    m_treeWidget->header()->setStretchLastSection(true);

    // Top Layout
    //
    m_topSplitter = new QSplitter();
    m_topSplitter->addWidget(m_textEdit);
    m_topSplitter->addWidget(m_treeWidget);
    m_topSplitter->setContentsMargins(0, 0, 0, 0);


    // Buttons
    //
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* validateButton = new QPushButton(tr("Validate"));
    connect(validateButton, &QPushButton::clicked, this, &ReportPropertyEditor::onValidateClicked);
    buttonLayout->addWidget(validateButton);

    buttonLayout->addStretch();

    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &ReportPropertyEditor::onOkClicked);
    buttonLayout->addWidget(m_okButton);

    m_cancelButton = new QPushButton(tr("Cancel"));
    connect(m_cancelButton, &QPushButton::clicked, this, &ReportPropertyEditor::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    // Main Layout
    //
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_topSplitter);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_topSplitter->restoreState(QSettings().value("ReportPropertyEditor/splitterState").toByteArray());

    return;

}

ReportPropertyEditor::~ReportPropertyEditor()
{
    QSettings().setValue("ReportPropertyEditor/splitterState", m_topSplitter->saveState());
}

QString ReportPropertyEditor::text() const
{
    return m_textEdit->text();
}

void ReportPropertyEditor::setText(const QString& text)
{
    m_textEdit->blockSignals(true);
    m_textEdit->setText(text);
    m_textEdit->blockSignals(false);

    fillObjectsTree();

    return;
}

bool ReportPropertyEditor::readOnly() const
{
    return m_textEdit->isReadOnly();
}

void ReportPropertyEditor::setReadOnly(bool value)
{
    m_textEdit->setReadOnly(value);
    m_okButton->setEnabled(value == false);
}

bool ReportPropertyEditor::externalOkCancelButtons() const
{
    return false;
}

bool ReportPropertyEditor::isModified() const
{
    return m_textEdit->isModified();
}

void ReportPropertyEditor::fillObjectsTree()
{
    m_treeWidget->clear();
    ReportLib::ReportTemplateStorage storage;

    QString errorMsg;

    bool ok = storage.load(m_textEdit->text().toUtf8(), &errorMsg);
    if (ok == false)
    {
        return;
    }

    for (const ReportLib::ReportTemplate& templ : storage.templates())
    {
        QTreeWidgetItem* templateItem = new QTreeWidgetItem(QStringList() << templ.caption() << "Report");
        m_treeWidget->addTopLevelItem(templateItem);

        for (const ReportLib::SectionTemplate& section : templ.sections())
        {
            QTreeWidgetItem* sectionItem = new QTreeWidgetItem(QStringList() << section.caption << "Section");
            templateItem->addChild(sectionItem);

            for (const std::shared_ptr<ReportLib::ObjectTemplate>& obj : section.objects)
            {
                QTreeWidgetItem* objItem = new QTreeWidgetItem(QStringList() << "Object" << obj->typeStr() << obj->tag);
                sectionItem->addChild(objItem);
            }

            sectionItem->setExpanded(true);
        }

        templateItem->setExpanded(true);
    }

    for (int i = 0; i < m_treeWidget->topLevelItemCount() - 1; i++)
    {
        m_treeWidget->resizeColumnToContents(i);
    }

}

void ReportPropertyEditor::onTextChanged()
{
    //m_svgWidget.setSvgData(m_textEdit->text());

    return;
}

void ReportPropertyEditor::onValidateClicked()
{
    ReportLib::ReportTemplateStorage storage;

    QString errorMsg;

    bool ok = storage.load(m_textEdit->text().toUtf8(), &errorMsg);
    if (ok == false)
    {
        QMessageBox::warning(m_parent, qAppName(), tr("Report template data is invalid:\n\n%1").arg(errorMsg));
    }
    else
    {
        fillObjectsTree();
        QMessageBox::information(m_parent, qAppName(), tr("Report template data is valid."));
    }
    return;
}

void ReportPropertyEditor::onOkClicked()
{
    ReportLib::ReportTemplateStorage storage;

    QString errorMsg;

    bool ok = storage.load(m_textEdit->text().toUtf8(), &errorMsg);

    if (ok == false)
    {
        int result = QMessageBox::warning(m_parent, qAppName(), tr("Report template data is invalid:\n\n%1\n\n Do you want to save it anyway?").arg(errorMsg),
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (result == QMessageBox::Cancel)
        {
            return;
        }

        if (result == QMessageBox::No)
        {
            onCancelClicked();
            return;
        }
    }

    okButtonPressed();
    return;
}

void ReportPropertyEditor::onCancelClicked()
{
    cancelButtonPressed();
    return;
}

