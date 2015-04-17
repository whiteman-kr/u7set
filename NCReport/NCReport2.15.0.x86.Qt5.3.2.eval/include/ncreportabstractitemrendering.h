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
#ifndef NCREPORTABSTRACTITEMRENDERING_H
#define NCREPORTABSTRACTITEMRENDERING_H

#include <QRectF>
#include <QString>

#include "ncreport_global.h"
#include "ncreportgraphrenderer.h"

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

class NCReportOutput;

/*!
This obsolete class is existed for backward compatibility only. Use NCReportGraphRenderer instead.
*/

class NCREPORTSHARED_EXPORT NCReportAbstractItemRendering : public NCReportGraphRenderer
{
public:
    NCReportAbstractItemRendering();
    virtual ~NCReportAbstractItemRendering();

    virtual void paintItem( QPainter* painter, NCReportOutput* output, const QRectF& rect, const QString& itemdata ) =0;
};

#endif
