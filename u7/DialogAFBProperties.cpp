#include "DialogAfbProperties.h"
#include "ui_DialogAfbProperties.h"
#include "Settings.h"
#include <QtXmlPatterns>
#include <QMenu>
#include <QMessageBox>
#include "xmlsyntaxhighlighter.h"
#include <QtXmlPatterns>
#include "../VFrame30/Fbl.h"

using namespace Afbl;

// XmlSchemaMessageHandler
//

class XmlSchemaMessageHandler : public QAbstractMessageHandler
{
public:
    XmlSchemaMessageHandler()
        : QAbstractMessageHandler(0)
    {
    }

    QString statusMessage() const
    {
        return m_description;
    }

    int line() const
    {
        return m_sourceLocation.line();
    }

    int column() const
    {
        return m_sourceLocation.column();
    }

protected:
    virtual void handleMessage(QtMsgType type, const QString &description,
                               const QUrl &identifier, const QSourceLocation &sourceLocation)
    {
        Q_UNUSED(type);
        Q_UNUSED(identifier);

        m_messageType = type;
        m_description = description;
        m_sourceLocation = sourceLocation;
    }

private:
    QtMsgType m_messageType;
    QString m_description;
    QSourceLocation m_sourceLocation;
};

// DialogAfbProperties
//

DialogAfbProperties::DialogAfbProperties(const QString &caption, QByteArray* pData, DbController *pDbController, bool readOnly, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::DialogAfbProperties),
    m_pData(pData),
    m_pDbController(pDbController),
    m_readOnly(readOnly)
{
    ui->setupUi(this);
    setWindowTitle(caption);
    new XmlSyntaxHighlighter(ui->m_text->document());

	if (pData == nullptr)
	{
		Q_ASSERT(pData);
		return;
	}

    QString s(*pData);

    ui->m_text->setPlainText(s);
    ui->m_text->setFont(QFont("Courier", 10));

    if (caption.right(4) == ".afb")
    {
        ui->m_validate->setVisible(true);
		ui->m_loadFbl->setVisible(true);
	}
    else
    {
        ui->m_validate->setVisible(false);
		ui->m_loadFbl->setVisible(false);
	}

    if (readOnly == true)
    {
        ui->m_okCancel->setEnabled(false);
        setWindowTitle(windowTitle() + tr(" [View Only]"));
        ui->m_text->setReadOnly(true);
    }

    if (theSettings.m_abflPropertiesWindowPos.x() != -1 && theSettings.m_abflPropertiesWindowPos.y() != -1)
    {
        move(theSettings.m_abflPropertiesWindowPos);
        restoreGeometry(theSettings.m_abflPropertiesWindowGeometry);
    }
}

DialogAfbProperties::~DialogAfbProperties()
{
    delete ui;
}

void DialogAfbProperties::on_DialogAfbProperties_accepted()
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

void DialogAfbProperties::on_m_validate_clicked()
{
    if (m_pDbController->getFileList(&m_validateFiles, m_pDbController->afblFileId(), "xsd", this) == false)
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

    connect(group, &QActionGroup::triggered, this, &DialogAfbProperties::on_validate);

    menu->exec(QCursor::pos());
}

void DialogAfbProperties::on_validate(QAction *pAction)
{
    int fileId = pAction->data().toInt();
    validate(fileId);
}

bool DialogAfbProperties::validate(int schemaFileId)
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

void DialogAfbProperties::on_DialogAfbProperties_finished(int result)
{
    Q_UNUSED(result);

    theSettings.m_abflPropertiesWindowPos = pos();
    theSettings.m_abflPropertiesWindowGeometry = saveGeometry();
}

void DialogAfbProperties::on_m_loadFbl_clicked()
{
	if (m_pData == nullptr)
	{
		Q_ASSERT(m_pData);
		return;
	}

	AfbElement afb;

	QString s = ui->m_text->toPlainText();
	QByteArray textData = s.toUtf8();

	QXmlStreamReader xmlReader(textData);

	if (afb.loadFromXml(&xmlReader) == true)
	{
		QMessageBox::information(this, "XML Read", "XML debug read successful!");
	}
	else
	{
		QMessageBox::critical(this, "XML Read Error", xmlReader.errorString());
	}





}
