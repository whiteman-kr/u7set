/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport Report Generator System
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  norbert@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORTPRINTEROUTPUT_H
#define NCREPORTPRINTEROUTPUT_H

#include "ncreportoutput.h"
#include "ncreport_global.h"

QT_BEGIN_NAMESPACE
class QPrinter;
QT_END_NAMESPACE

/*!
Printer output class.
*/
class NCREPORTSHARED_EXPORT NCReportPrinterOutput : public NCReportOutput
{
    Q_OBJECT
public:
    NCReportPrinterOutput( QObject* parent=0 );
    NCReportPrinterOutput( QPrinter::PrinterMode quality, int dpi=-1, QObject* parent=0 );
    NCReportPrinterOutput( const QPrinterInfo & printerInfo, QPrinter::PrinterMode quality, int dpi=-1, QObject* parent=0 );

    virtual ~NCReportPrinterOutput();

    virtual bool setup();
    virtual void cleanup();
    virtual void begin();
    virtual void end();
    virtual void newPage();
    QPrinter* printer() const;
    virtual void setCopies( int num );
    virtual int resolution() const;
    virtual void setResolution( int dpi );
    virtual QPaintDevice* device() const;

protected:
    virtual QPrinter* createPrinter( QPrinter::PrinterMode quality = QPrinter::HighResolution, int dpi=-1 );
    virtual QPrinter* createPrinter(const QPrinterInfo & printerInfo, QPrinter::PrinterMode quality = QPrinter::HighResolution, int dpi=-1 );
    /*!
    Printer initialization
    */
    virtual	void initPrinter();
    /*!
    Custom Printer initialization. Called from initPrinter()
    */
    virtual void customizePrinter( QPrinter* printer );
    virtual void rotatePrinter( QPrinter* printer );

    void setPrinter( QPrinter* printer );
    void setDialogDone( bool set );
    bool dialogDone() const;

private:
    QPrinter *m_printer;
    bool m_initDone;
    bool m_dialogDone;
};


#endif // NCREPORTPRINTEROUTPUT_H
