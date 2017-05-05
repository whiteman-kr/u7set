#include "DialogFileEditor.h"
#include "../lib/CodeSyntaxHighlighter.h"
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

	if (fileName.right(4) == ".afb")
	{
		m_buttonValidate = new QPushButton("Validate...");
		connect(m_buttonValidate, &QPushButton::clicked, this, &DialogFileEditor::on_validate_clicked);
	}

	m_buttonOK = new QPushButton(tr("OK"));
	m_buttonCancel = new QPushButton(tr("Cancel"));

	connect(m_buttonOK, &QPushButton::clicked, this, &DialogFileEditor::on_ok_clicked);
	connect(m_buttonCancel, &QPushButton::clicked, this, &DialogFileEditor::on_cancel_clicked);

	// Layouts

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	if (m_buttonValidate != nullptr)
	{
		buttonLayout->addWidget(m_buttonValidate);
	}

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

void DialogFileEditor::closeEvent(QCloseEvent* e)
{
	if (m_editor->modified() == true)
    {
        QMessageBox::StandardButton result = QMessageBox::warning(this, "Subsystem List Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (result == QMessageBox::Yes)
        {
            if (saveChanges() == true)
            {
                setResult(QDialog::Accepted);
            }

            e->accept();
            return;
        }

        if (result == QMessageBox::Cancel)
        {
            e->ignore();
            return;
        }
    }

    e->accept();
    return;
}

void DialogFileEditor::reject()
{
	if (m_editor->modified() == true)
	{
        QMessageBox::StandardButton result = QMessageBox::warning(this, "Subsystem List Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

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

void DialogFileEditor::on_ok_clicked()
{
	if (m_editor->modified() == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	accept();
	return;
}

void DialogFileEditor::on_cancel_clicked()
{
    reject();
	return;
}

void DialogFileEditor::on_validate_clicked()
{
	if (m_pDbController->getFileList(&m_validateFiles, m_pDbController->afblFileId(), "xsd", true, this) == false)
	{
		QMessageBox::critical(this, "Error", "Could not get files list!");
		return;
	}

	if (m_validateFiles.empty() == true)
	{
		QMessageBox::information(this, "Validate", "No schema files found!");
		return;
	}

	QMenu *menu = new QMenu(this);
	QActionGroup *group = new QActionGroup(menu);

	for (auto i = m_validateFiles.begin(); i != m_validateFiles.end(); i++)
	{
		DbFileInfo& fi = *i;

		QAction* action = group->addAction(tr("Validate with schema ") + fi.fileName());
		action->setData(fi.fileId());
		menu->addAction(action);
	}

	connect(group, &QActionGroup::triggered, this, &DialogFileEditor::on_validate);

	menu->exec(QCursor::pos());
}

void DialogFileEditor::on_validate(QAction *pAction)
{
	int fileId = pAction->data().toInt();
	validate(fileId);
}

bool DialogFileEditor::validate(int schemaFileId)
{
	DbFileInfo fi;
	fi.setFileId(schemaFileId);

	std::shared_ptr<DbFile> f = std::make_shared<DbFile>();

	if (m_pDbController->getLatestVersion(fi, &f, this) == false)
	{
		QMessageBox::critical(this, "Error", "Get work copy error!");
		return false;
	}

	QByteArray schemaData;
	f->swapData(schemaData);

	// Load text
	QString s = m_editor->text();
	QByteArray textData = s.toUtf8();

	// Validate
	XmlSchemaMessageHandler messageHandler;

	QXmlSchema schema;
	schema.setMessageHandler(&messageHandler);

	if (schema.load(schemaData) == false)
	{
		QMessageBox::critical(0, QString("Error"), QString("Failed to load schema!"));
		return false;
	}

	bool errorOccurred = false;
	if (schema.isValid() == false)
	{
		errorOccurred = true;
	}
	else
	{
		QXmlSchemaValidator validator(schema);
		if (validator.validate(textData) == false)
		{
			errorOccurred = true;
		}
	}

	if (errorOccurred == true)
	{
		QMessageBox::critical(0, QString("Vaildation"), QString("Schema validation failed at line %1, column %2").arg(messageHandler.line()).arg(messageHandler.column()));
		return false;
	}
	else
	{
		QMessageBox::information(0, QString("Vaildation"), QString("Schema validation successful!"));
	}

	return true;
}

