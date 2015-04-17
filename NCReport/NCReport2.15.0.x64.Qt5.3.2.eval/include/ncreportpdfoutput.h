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

#ifndef NCREPORTPDFOUTPUT_H
#define NCREPORTPDFOUTPUT_H

#include "ncreportprinteroutput.h"
#include "ncreport_global.h"

class NCREPORTSHARED_EXPORT NCReportPdfOutput : public NCReportPrinterOutput
{
	Q_OBJECT
public:
    NCReportPdfOutput( QPrinter::PrinterMode quality = QPrinter::HighResolution, int dpi=-1, QPrinter::OutputFormat outputFormat = QPrinter::PdfFormat, QObject* parent=0);

	virtual void cleanup();
	virtual void begin();
	virtual void end();
	virtual void setFileName( const QString& );

private:
    QPrinter::OutputFormat m_outputFormat;

};


#endif // NCREPORTPDFOUTPUT_H
