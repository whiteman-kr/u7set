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
	const QString& caption();
    void setCaption(const QString& caption);

private:
    QString m_caption;
	int m_opCode = 0;
	int m_inCount = 0;
	int m_outCount = 0;
};

#endif // AFBLIBRARY_H
