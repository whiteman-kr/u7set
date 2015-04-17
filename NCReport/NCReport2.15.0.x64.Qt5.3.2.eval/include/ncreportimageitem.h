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
#ifndef NCREPORTIMAGEITEM_H
#define NCREPORTIMAGEITEM_H

#include "ncreportitem.h"

/*!
Image item's data class
 */
class NCReportImageData : public NCReportItemData
{
public:
    NCReportImageData()
     : aspRatMode(Qt::KeepAspectRatio), transformMode(Qt::FastTransformation),
       scaling(true), format(0), alignment(Qt::AlignLeft | Qt::AlignTop)
    {}

    Qt::AspectRatioMode aspRatMode;
    Qt::TransformationMode transformMode;
    bool scaling;
    uint format;
    //short htmlAlign;
    Qt::Alignment alignment;
    QByteArray htmlWidth, htmlHeight;
    QByteArray svgXml;

#ifdef USE_QIMAGE_INSTEAD_OF_QPIXMAP
    QImage image;
#else
    QPixmap image;
#endif

};

/*!
Image item class
 */
class NCReportImageItem : public NCReportItem
{
public:
    NCReportImageItem( NCReportDef* rdef, QGraphicsItem* parent =0);
    ~NCReportImageItem();

    enum ImageFormat { Binary=0, Base64Encoded, Svg };

    inline Qt::AspectRatioMode aspectRatioMode() const
    { return ((NCReportImageData*)d)->aspRatMode; }

    void setAspectRatioMode( Qt::AspectRatioMode am );

    inline bool isScaling() const
    { return ((NCReportImageData*)d)->scaling; }

    inline void setScaling( bool set ) const
    { ((NCReportImageData*)d)->scaling = set; }

    inline Qt::TransformationMode transformMode() const
    { return ((NCReportImageData*)d)->transformMode; }

    inline void setTransformMode( Qt::TransformationMode tm )
    { ((NCReportImageData*)d)->transformMode = tm; }

    //inline QString fileName() const
    //{ return ((NCReportImageData*)d)->filename; }

    //inline void setFileName( const QString& fname )
    //{ ((NCReportImageData*)d)->filename = fname; }

    inline ImageFormat imageFormat() const
    { return (ImageFormat)((NCReportImageData*)d)->format; }

    inline void setImageFormat( ImageFormat f )
    { ((NCReportImageData*)d)->format = f; }

#ifdef USE_QIMAGE_INSTEAD_OF_QPIXMAP
    inline QImage image() const
#else
    inline QPixmap image() const
#endif
    { return ((NCReportImageData*)d)->image; }

#ifdef USE_QIMAGE_INSTEAD_OF_QPIXMAP
    inline void setImage( const QImage& image )
#else
    inline void setImage( const QPixmap& image )
#endif
    { ((NCReportImageData*)d)->image = image; }

    inline QByteArray svg() const
    { return ((NCReportImageData*)d)->svgXml; }

    inline void setSvg( const QByteArray& svg )
    { ((NCReportImageData*)d)->svgXml = svg; }

    inline QByteArray htmlWidth() const
    { return ((NCReportImageData*)d)->htmlWidth; }

    inline void setHtmlWidth( const QByteArray& width )
    { ((NCReportImageData*)d)->htmlWidth = width; }

    inline QByteArray htmlHeight() const
    { return ((NCReportImageData*)d)->htmlHeight; }

    inline void setHtmlHeight( const QByteArray& height )
    { ((NCReportImageData*)d)->htmlHeight = height; }

//	inline short htmlAlign() const
//	{ return ((NCReportImageData*)d)->htmlAlign; }

//	inline void setHtmlAlign( short align )
//	{ ((NCReportImageData*)d)->htmlAlign = align; }

    inline Qt::Alignment alignment() const
    { return ((NCReportImageData*)d)->alignment; }

    inline void setAlignment( Qt::Alignment al )
    { ((NCReportImageData*)d)->alignment = al; }

    void adjustSize();
    bool read( NCReportXMLReader* );
    bool write( NCReportXMLWriter* );
    void setDefaultForEditor();
    void paint( NCReportOutput* output, const QPointF& mPos);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updateValue(NCReportOutput *output, NCReportEvaluator *evaluator);
    void updateContent();
    virtual QList<int> availableSourceTypes() const;

    bool load( NCReportEvaluator* evaluator );

    QByteArray toBase64() const;
    QByteArray toHtml() const;
private:

    QPointF imageTargetPoint( const QRectF& itemRect, const QSizeF &imageSize ) const;

};

#endif
