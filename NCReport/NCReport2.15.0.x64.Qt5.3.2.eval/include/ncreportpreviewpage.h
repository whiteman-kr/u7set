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
#ifndef NCREPORTPREVIEWPAGE_H
#define NCREPORTPREVIEWPAGE_H

#include "ncreport_global.h"
#include <QWidget>
QT_BEGIN_NAMESPACE
class QPicture;
QT_END_NAMESPACE

class NCReportScale;

/*!
Report preview page class. Represents one preview page.
 */
class NCREPORTSHARED_EXPORT NCReportPreviewPage : public QWidget
{
    Q_OBJECT
public:
    NCReportPreviewPage( QWidget * parent = 0 );
    ~NCReportPreviewPage();

    void setContent( const QByteArray& content );
    QByteArray content() const;

    /*! size in mm */
    void setPageSizeMM( const QSizeF& sizeMM );
    QSizeF pageSizeMM() const;
    void setScaleRate(qreal rate);
    qreal scaleRate() const;
    void setDropShadow( bool set );
    bool dropShadow() const;

    static void loadPicture(QPicture& picture, QByteArray &ba);
    static void savePicture(QPicture& picture, QByteArray &ba );

signals:
    void pageDoubleClicked( const QPoint& pos );

protected:
    void paintEvent( QPaintEvent* );
    void keyPressEvent(QKeyEvent *event);
    void drawBackground(QPainter *painter );
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    QByteArray m_content;
    qreal m_scale;
    QSizeF m_sizeMM;	// size in mm
    bool m_safeMode;
    bool m_dropShadow;
};

#endif
