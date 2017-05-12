#include "DialogFileEditor.h"
#include "Settings.h"
#include "../VFrame30/Afb.h"

DialogFileEditor::DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
    m_pData(pData),
    m_pDbController(pDbController),
    m_readOnly(readOnly)
{

	setWindowTitle(fileName);

	// Create Text Editor
	//

    QString ext = QFileInfo(fileName).suffix();

	CodeType codeType = CodeType::Unknown;

	if (ext == tr("js"))
	{
		codeType = CodeType::Cpp;
	}
	else
	{
		if (ext == tr("afb") || ext == tr("xml") || ext == tr("xsd"))
		{
			codeType = CodeType::Xml;
		}
	}

	m_editor = new CodeEditor(codeType, this);

	connect(m_editor, &CodeEditor::escapePressed, this, &QDialog::reject);

	if (m_editor == nullptr)
	{
		Q_ASSERT(m_editor);
		return;
	}

	if (pData == nullptr)
    {
        Q_ASSERT(pData);
        return;
    }

    QString s(*pData);

	m_editor->setText(s);

	// Buttons

	m_buttonOK = new QPushButton(tr("OK"));
	m_buttonCancel = new QPushButton(tr("Cancel"));

	connect(m_buttonOK, &QPushButton::clicked, this, &DialogFileEditor::accept);
	connect(m_buttonCancel, &QPushButton::clicked, this, &DialogFileEditor::reject);

	// Layouts

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	buttonLayout->addStretch();
	buttonLayout->addWidget(m_buttonOK);
	buttonLayout->addWidget(m_buttonCancel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(m_editor);
	mainLayout->addLayout(buttonLayout);

	if (readOnly == true)
    {
		m_buttonOK->setEnabled(false);
        setWindowTitle(windowTitle() + tr(" [View Only]"));
		m_editor->setReadOnly(true);
    }

    if (theSettings.m_DialogTextEditorWindowPos.x() != -1 && theSettings.m_DialogTextEditorWindowPos.y() != -1)
    {
        move(theSettings.m_DialogTextEditorWindowPos);
        restoreGeometry(theSettings.m_DialogTextEditorWindowGeometry);
    }
}

DialogFileEditor::~DialogFileEditor()
{
}

void DialogFileEditor::on_DialogFileEditor_finished(int result)
{
    Q_UNUSED(result);

    theSettings.m_DialogTextEditorWindowPos = pos();
    theSettings.m_DialogTextEditorWindowGeometry = saveGeometry();

}

bool DialogFileEditor::saveChanges()
{

	if (m_readOnly == false)
	{
		QString s = m_editor->text();
		*m_pData = s.toUtf8();
	}

	return true;
}

void DialogFileEditor::accept()
{
	if (m_editor->modified() == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	QDialog::accept();
	return;
}

void DialogFileEditor::reject()
{
	if (m_editor->modified() == true)
	{
		QMessageBox::StandardButton result = QMessageBox::warning(this, qAppName(), tr("Do you want to save your changes?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (result == QMessageBox::Yes)
        {
            if (saveChanges() == true)
            {
                accept();
                return;
            }
        }

        if (result == QMessageBox::Cancel)
        {
            return;
        }
    }

    QDialog::reject();
    return;
}
