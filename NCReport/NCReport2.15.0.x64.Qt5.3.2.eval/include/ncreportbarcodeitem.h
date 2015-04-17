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
#ifndef NCREPORTBARCODEITEM_H
#define NCREPORTBARCODEITEM_H

#include <QHash>
//#include <QImage>
#include "ncreportitem.h"

#define BARCODE_DEFAULT_TYPE	13
#define BARCODE_DEFAULT_SCALE	0.00
#define BARCODE_DEFAULT_HEIGHT	0
#define BARCODE_DEFAULT_WSPACE	0
#define BARCODE_DEFAULT_ROTATE	0
#define BARCODE_DEFAULT_BORDER	0
#define BARCODE_DEFAULT_FC		Qt::black
#define BARCODE_DEFAULT_BC		Qt::white
#define BARCODE_DEFAULT_COLS	0
#define BARCODE_DEFAULT_VERS	0
#define BARCODE_DEFAULT_SECURE	0
#define BARCODE_DEFAULT_MODE	0

/*!
Barcode item data class
*/
class NCReportBarcodeData : public NCReportItemData
{
public:
    NCReportBarcodeData()
     :	barcodeType(BARCODE_DEFAULT_TYPE), showCode(true),
        borderType(BARCODE_DEFAULT_BORDER), borderWidth(0), whiteSpace(BARCODE_DEFAULT_WSPACE), scale(BARCODE_DEFAULT_SCALE), heightRatio(BARCODE_DEFAULT_HEIGHT),
        columns(BARCODE_DEFAULT_COLS), version(BARCODE_DEFAULT_VERS), secure(BARCODE_DEFAULT_SECURE), mode(BARCODE_DEFAULT_MODE),
        foreColor(BARCODE_DEFAULT_FC), backColor(BARCODE_DEFAULT_BC),
        rotation(BARCODE_DEFAULT_ROTATE)
    {}

    uint barcodeType;
    bool showCode;
    int borderType;
    int borderWidth;
    int whiteSpace;
    qreal scale;
    int heightRatio;
    int columns;
    int version;
    int secure;
    int mode;
    QColor foreColor, backColor;
    int rotation;

    QString primaryText;
    /*!xml (svg) format of barcode */
    QByteArray data;
};

/*!
Barcode item class
 */
class NCReportBarcodeItem : public NCReportItem
{
public:
    NCReportBarcodeItem( NCReportDef* rdef, QGraphicsItem* parent =0);
    ~NCReportBarcodeItem();

    /*
    class BarcodeType
    {
    public:
        BarcodeType(): typeCode(0), typeName( QString() ) {}
        BarcodeType( int code, const QString& name = QString() ): typeCode(code), typeName( name ) {}
        int typeCode;
        QString typeName;
    };
    */
    //static void typesToVector( QVector<BarcodeType>& );
    bool read( NCReportXMLReader* );
    bool write( NCReportXMLWriter* );
    void setDefaultForEditor();
    void paint( NCReportOutput*, const QPointF& );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPixmap toPixmap(const QSize & size);
    QImage toImage(const QSize & size);

    QString barcodeName( uint type ) const;

    inline uint barcodeType() const
    { return ((NCReportBarcodeData*)d)->barcodeType; }

    inline void setBarcodeType(uint type)
    { ((NCReportBarcodeData*)d)->barcodeType = type; }

    int maxBarcodeID() const
    { return 142; }

    inline bool isShowCode() const
    { return ((NCReportBarcodeData*)d)->showCode; }

    inline void setShowCode(bool set)
    { ((NCReportBarcodeData*)d)->showCode = set; }

    bool isComposite() const;
    //{ return ((NCReportBarcodeData*)d)->composite; }

    inline int borderWidth() const
    { return ((NCReportBarcodeData*)d)->borderWidth; }

    inline void setBorderWidth(int w)
    { ((NCReportBarcodeData*)d)->borderWidth = w; }

    inline int borderType() const
    { return ((NCReportBarcodeData*)d)->borderType; }

    inline void setBorderType(int t)
    { ((NCReportBarcodeData*)d)->borderType = t; }

    inline int whiteSpace() const
    { return ((NCReportBarcodeData*)d)->whiteSpace; }

    inline void setWhiteSpace(int ws)
    { ((NCReportBarcodeData*)d)->whiteSpace = ws; }

    inline qreal scale() const
    { return ((NCReportBarcodeData*)d)->scale; }

    inline void setScale(qreal scale)
    { ((NCReportBarcodeData*)d)->scale = scale; }

    inline int heightRatio() const
    { return ((NCReportBarcodeData*)d)->heightRatio; }

    inline void setHeightRatio(int h)
    { ((NCReportBarcodeData*)d)->heightRatio = h; }

    inline QColor foreColor() const
    { return ((NCReportBarcodeData*)d)->foreColor; }

    inline void setForeColor( const QColor& color )
    { ((NCReportBarcodeData*)d)->foreColor = color; }

    inline QColor backColor() const
    { return ((NCReportBarcodeData*)d)->backColor; }

    inline void setBackColor( const QColor& color )
    { ((NCReportBarcodeData*)d)->backColor = color; }

    inline QString primaryText() const
    { return ((NCReportBarcodeData*)d)->primaryText; }

    inline void setPrimaryText( const QString& ctxt )
    { ((NCReportBarcodeData*)d)->primaryText = ctxt; }

    inline int rotation() const
    { return ((NCReportBarcodeData*)d)->rotation; }
    inline void setRotation( int val )
    { ((NCReportBarcodeData*)d)->rotation = val; }

    inline void setColumns(int val)
    { ((NCReportBarcodeData*)d)->columns = val; }
    inline int columns() const
    { return ((NCReportBarcodeData*)d)->columns; }

    inline void setVersion(int val)
    { ((NCReportBarcodeData*)d)->version = val; }
    inline int version() const
    { return ((NCReportBarcodeData*)d)->version; }

    inline void setSecure(int val)
    { ((NCReportBarcodeData*)d)->secure = val; }
    inline int secure() const
    { return ((NCReportBarcodeData*)d)->secure; }

    inline void setMode(int val)
    { ((NCReportBarcodeData*)d)->mode = val; }
    inline int mode() const
    { return ((NCReportBarcodeData*)d)->mode; }

    bool update_barcode();
    bool isEmpty() const;
    void updateValue(NCReportOutput *output, NCReportEvaluator *evaluator);
    void rotateBarCode( QPainter* painter, const QRectF rect );
    QList<int> availableSourceTypes() const;

private:
    inline NCReportBarcodeData* data()
    { return (NCReportBarcodeData*)d; }

    //void renderSvg( QPainter *painter, const QRectF& rect, bool hires );
    void renderSvg( QPainter *painter, const QRectF& rect );
    void render( QPainter *painter, const QRectF& rect );
};

#endif
