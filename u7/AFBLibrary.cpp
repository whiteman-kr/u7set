#include "AfbLibrary.h"
#include <QXmlStreamWriter>
#include <QString>
#include <QtXmlPatterns>
#include <QMessageBox>

Afb::Afb() :
    m_opCode(0),
    m_inCount(0),
    m_outCount(0)
{
}

QString Afb::caption()
{
    return m_caption;
}

void Afb::setCaption(const QString& caption)
{
    m_caption = caption;
}

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

/*
bool AfbLibrary::restore(const QString& fileName, Afb &afb)
{
    // Load Schema
    QFile schemaFile(":/afb_schema.xsd");
    bool result = schemaFile.open(QIODevice::ReadOnly);
    if (result == false)
    {
        QMessageBox::critical(0, QString("Error"), QString("Failed to open schema file!"));
        return false;
    }

    XmlSchemaMessageHandler messageHandler;

    QXmlSchema schema;
    schema.setMessageHandler(&messageHandler);

    result = schema.load(&schemaFile);
    if (result == false)
    {
        QMessageBox::critical(0, QString("Error"), QString("Failed to load schema!"));
        return false;
    }

    // Load Data
    QFile dataFile(fileName);
    result = dataFile.open(QIODevice::ReadOnly);
    if (result == false)
    {
        QMessageBox::critical(0, QString("Error"), QString("Failed to open data file!"));
        return false;
    }

    bool errorOccurred = false;
    if (!schema.isValid())
    {
        errorOccurred = true;
    } else
    {
        QXmlSchemaValidator validator(schema);
        if (!validator.validate(&dataFile))
        {
            errorOccurred = true;
        }
    }

    if (errorOccurred == true)
    {
        QMessageBox::critical(0, QString("Error"), QString("Schema validation failed!"));
        return false;
    }

    return true;
}
*/
void Afb::storeToXml(QXmlStreamWriter& xmlWriter) const
{
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("ApplicationFunctionalBlocks");

    xmlWriter.writeStartElement("AFB");
    xmlWriter.writeTextElement("Caption", m_caption);
    xmlWriter.writeTextElement("OpCode", QString::number(m_opCode));
    xmlWriter.writeTextElement("InCount", QString::number(m_inCount));
    xmlWriter.writeTextElement("OutCount", QString::number(m_outCount));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
}
