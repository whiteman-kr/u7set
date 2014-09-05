#ifndef AFBLIBRARY_H
#define AFBLIBRARY_H

#include "../include/DbController.h"

class Afb
{
public:
    Afb();


public:
    void storeToXml(QXmlStreamWriter& xmlWriter) const;
    void readFromXml(QXmlStreamReader& xmlReader);

public:
    QString caption();
    void setCaption(const QString& caption);

private:
    QString m_caption;
    int m_opCode;
    int m_inCount;
    int m_outCount;

};

#endif // AFBLIBRARY_H
