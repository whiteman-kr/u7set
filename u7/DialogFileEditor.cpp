#include "DialogFileEditor.h"
#include "ui_DialogFileEditor.h"
#include "xmlsyntaxhighlighter.h"
#include "Settings.h"
#include "../VFrame30/Afb.h"

DialogFileEditor::DialogFileEditor(const QString& fileName, QByteArray *pData, DbController* pDbController, bool readOnly, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
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

	ui->m_text->blockSignals(true);

    ui->m_text->setPlainText(s);
	ui->m_text->setFont(QFont("Courier", 10));

	ui->m_text->blockSignals(false);

	if (fileName.right(4) == ".afb")
	{
		ui->btnValidate->setVisible(true);
		ui->btnLoadFbl->setVisible(true);
	}
	else
	{
		ui->btnValidate->setVisible(false);
		ui->btnLoadFbl->setVisible(false);
	}

	if (readOnly == true)
    {
		ui->btnOk->setEnabled(false);
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

void DialogFileEditor::on_DialogFileEditor_finished(int result)
{
    Q_UNUSED(result);

    theSettings.m_DialogTextEditorWindowPos = pos();
    theSettings.m_DialogTextEditorWindowGeometry = saveGeometry();

}

bool DialogFileEditor::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, "Subsystem List Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		if (saveChanges() == false)
		{
			return false;
		}
		return true;
	}

	if (result == QMessageBox::No)
	{
		return true;
	}

	return false;
}

bool DialogFileEditor::saveChanges()
{

	if (m_readOnly == false)
	{
		QString s = ui->m_text->toPlainText();
		*m_pData = s.toUtf8();
	}

	m_modified = false;

	return true;
}

void DialogFileEditor::closeEvent(QCloseEvent* e)
{
	if (askForSaveChanged() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
}

void DialogFileEditor::on_btnOk_clicked()
{
	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	accept();
	return;
}

void DialogFileEditor::on_btnCancel_clicked()
{
	if (askForSaveChanged() == true)
	{
		reject();
	}
	return;
}

void DialogFileEditor::on_m_text_textChanged()
{
	m_modified = true;
}

void DialogFileEditor::on_btnValidate_clicked()
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
	QString s = ui->m_text->toPlainText();
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

void DialogFileEditor::on_btnLoadFbl_clicked()
{
	if (m_pData == nullptr)
	{
		Q_ASSERT(m_pData);
		return;
	}

	Afb::AfbElement afb;

	QString s = ui->m_text->toPlainText();
	QByteArray textData = s.toUtf8();

	QXmlStreamReader xmlReader(textData);

	if (afb.loadFromXml(&xmlReader) == true)
	{
        QMessageBox::information(this, "XML Read", "XML debug read finished!");
	}
	else
	{
		QMessageBox::critical(this, "XML Read Error", xmlReader.errorString());
	}

}
