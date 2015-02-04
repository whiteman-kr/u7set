/****************************************************************************
*
*  Copyright (C) 2002-2008 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: nszabo@helta.hu, info@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  nszabo@helta.hu if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTPREVIEWOUTPUT_H
#define NCREPORTPREVIEWOUTPUT_H

#include "ncreportoutput.h"
#include <QList>

QT_BEGIN_NAMESPACE
class QPicture;
QT_END_NAMESPACE

/*!
Print Preview Output class. NCReportPreviewOutput is used by NCReportPreviewWindow.
For previewing the contents of NCReportPreviewOutput, this object must be added to
NCReportPreviewWindow by NCReportPreviewWindow::setOutput( NCReportPreviewOutput* ) function.
*/
class NCREPORTSHARED_EXPORT NCReportPreviewOutput : public NCReportOutput
{
    Q_OBJECT
public:
    NCReportPreviewOutput( QObject* parent=0 );
    ~NCReportPreviewOutput();

    QByteArray page( int pageNo ) const;
    QSizeF pageSizeMM( int pageNo ) const;
    const QList<QByteArray>& pages();
    const QList<int>& startPagesOfReports();

    bool setup();
    void cleanup();
    void begin();
    void end();
    void newPage();

    void deletePages();
private:
    int m_last_reportno;
    QPicture *m_currentPage;

    QList<QByteArray> m_pages;
    QList<QSizeF> m_pageSizesMM;
    QList<int> m_startPagesOfReports;   //map of reportno and page

private:
    void endPage();
    void beginPage();
};

#endif
