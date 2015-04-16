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
#ifndef NCREPORTLABELITEM_H
#define NCREPORTLABELITEM_H

#include "ncreportitem.h"

#include <QString>
#include <QColor>
#include <QFont>
#include <QHash>

/*!
Simple text label item's data class
 */
class NCReportLabelData : public NCReportItemData
{
public:
    NCReportLabelData()
     : wordbreak(false), rotation(0), hideRepeated(false), hasTranslation(false), hasCondFormat(false)
    {}

    //Qt::Alignment alignmentH;
    //Qt::Alignment alignmentV;
    class TextStyle {
    public:
        TextStyle() : forecolor(Qt::black), backcolor(Qt::white), font(QFont()), bgMode(0), bgDesign(false), alignment(Qt::AlignLeft | Qt::AlignTop) {}
        QColor forecolor;
        QColor backcolor;
        QFont font;
        uint bgMode;
        bool bgDesign;
        Qt::Alignment alignment;
    };
    bool wordbreak; //, singleline, expandtabs;
    int rotation;
    bool hideRepeated;
    bool hasTranslation;
    bool hasCondFormat;

    TextStyle textStyle;
    TextStyle textStyle_backup;
    QString displayValue;
    //QString lastDisplayValue;
    QString lastValue;
    QHash<QString,QString> translations;
    QString conditionalFormatCode;
};

/*!
Simple text label item class
 */
class NCREPORTSHARED_EXPORT NCReportLabelItem : public NCReportItem
{
public:
    NCReportLabelItem( int type, NCReportDef* rdef, QGraphicsItem* parent =0);
    virtual ~NCReportLabelItem();

    enum BGMode { Transparent=0, FilledBackground };

    inline Qt::Alignment alignment() const
    { return ((NCReportLabelData*)d)->textStyle.alignment; }

    inline void setAlignment( Qt::Alignment al )
    { ((NCReportLabelData*)d)->textStyle.alignment = al; }

    inline bool isWordBreak() const
    { return ((NCReportLabelData*)d)->wordbreak; }

    inline void setWordBreak( bool set )
    { ((NCReportLabelData*)d)->wordbreak = set; }

    inline QColor foreColor() const
    { return ((NCReportLabelData*)d)->textStyle.forecolor; }

    inline void setForeColor( const QColor& color )
    { ((NCReportLabelData*)d)->textStyle.forecolor = color; }

    inline QColor backColor() const
    { return ((NCReportLabelData*)d)->textStyle.backcolor; }

    inline void setBackColor( const QColor& color )
    { ((NCReportLabelData*)d)->textStyle.backcolor = color; }

    inline QFont font() const
    { return ((NCReportLabelData*)d)->textStyle.font; }

    inline void setFont( const QFont& font )
    { ((NCReportLabelData*)d)->textStyle.font = font; }

    inline int rotation() const
    { return ((NCReportLabelData*)d)->rotation; }
    inline void setRotation( int val )
    { ((NCReportLabelData*)d)->rotation = val; }

    inline BGMode backgroundMode() const
    { return (BGMode)((NCReportLabelData*)d)->textStyle.bgMode; }

    inline void setBackgroundMode( BGMode mode ) const
    { ((NCReportLabelData*)d)->textStyle.bgMode = mode; }

    inline bool isBackgroundToDesign() const
    { return ((NCReportLabelData*)d)->textStyle.bgDesign; }

    inline void setBackgroundToDesign( bool set ) const
    { ((NCReportLabelData*)d)->textStyle.bgDesign = set; }

    inline bool hideIfRepeated() const
    { return ((NCReportLabelData*)d)->hideRepeated; }

    inline void setHideIfRepeated( bool set )
    { ((NCReportLabelData*)d)->hideRepeated = set; }

    void resetHideRepeated()
    { ((NCReportLabelData*)d)->lastValue = QString(); }

    inline bool hasTranslation() const
    { return ((NCReportLabelData*)d)->hasTranslation; }

    inline void setHasTranslation( bool set )
    { ((NCReportLabelData*)d)->hasTranslation = set; }

    inline QHash<QString,QString>& translations()
    { return ((NCReportLabelData*)d)->translations; }

    inline QString displayValue() const
    { return ((NCReportLabelData*)d)->displayValue; }

    void setDisplayValue( const QString& );

    inline QString conditionalFormatCode() const
    { return ((NCReportLabelData*)d)->conditionalFormatCode; }

    void setConditionalFormatCode( const QString& code )
    { ((NCReportLabelData*)d)->conditionalFormatCode = code; }

    inline bool hasConditionalFormat() const
    { return ((NCReportLabelData*)d)->hasCondFormat; }

    inline void setHasConditionalFormat( bool set )
    { ((NCReportLabelData*)d)->hasCondFormat = set; }

    QString printableText() const;
    QFont& fontRef();

    virtual bool read(NCReportXMLReader* reader);
    virtual bool write(NCReportXMLWriter* writer);
    virtual void setDefaultForEditor();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void paint( NCReportOutput* output, const QPointF& painterPosMM );
    virtual void adjustSize();
    virtual void adjustSize(NCReportOutput*output);
    virtual bool isEmpty() const;
    virtual void readSubElements(NCReportXMLReader *reader);
    virtual void writeSubElements(NCReportXMLWriter*);
    virtual QString text() const;
    virtual void setText( const QString& text, const QString& langcode = QString() );
    virtual void updateValue(NCReportOutput *output, NCReportEvaluator *evaluator);
    virtual void saveTextStyle();
    virtual void restoreTextStyle();

    virtual void printWhenTrueAction(NCReportEvaluator *evaluator);

protected:
    virtual void readProps( NCReportXMLReader* );
    virtual void writeProps( NCReportXMLWriter* );
    virtual void adjustFont( QFont & font, NCReportOutput* =0 );
    virtual void rotateText( QPainter*, const QPointF& point, const QSizeF& textSize );
    virtual QRectF screenRect() const;
    virtual QRectF outputRect(NCReportOutput* output) const;

private:
};

#endif
