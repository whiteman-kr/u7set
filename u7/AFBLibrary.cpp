#include "AfbLibrary.h"
#include <QXmlStreamWriter>
#include <QString>
#include <QMessageBox>

Afb::Afb()
{
}

const QString& Afb::caption()
{
    return m_caption;
}

void Afb::setCaption(const QString& caption)
{
    m_caption = caption;
}

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
