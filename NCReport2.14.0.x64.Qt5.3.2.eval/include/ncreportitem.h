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
#ifndef NCREPORTITEM_H
#define NCREPORTITEM_H

#include "ncreportdata.h"
#include "ncreport_global.h"

#include <QGraphicsItem>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QPaintDevice;
class QGraphicsSceneMouseEvent;
class QPaintDevice;
QT_END_NAMESPACE

class NCReportXMLReader;
class NCReportXMLWriter;
class NCReportOutput;
class NCReportSection;
class NCReportDef;
class NCReportEvaluator;
class NCReportDirector;

#define SELECTION_BOXSIZE		6

/*!
This class represents the base (abstract) report item's data class
 */
class NCREPORTSHARED_EXPORT NCReportItemData : public NCReportData
{
public:
    NCReportItemData()
     : objectType(0), typeId(0x0), sourceType(0), fitRole(0),
     mPos(QPointF(0,0)), mOffset(QPointF(0,0)), mSize(QSizeF(0,0)), mRealSize(QSizeF(0,0)), mSectionSize(QSizeF(0,0)), mZoneSize(QSizeF()),
     currentSize(QSizeF(0,0)),
       autoHeight(false), templateMode(false),	 zoneID(0), adjusted(false), printWhenResult(true), upToDate(false), locked(false), breakToPage(false),
       pinToRight(false), pinToLeft(false),
       reportDef(0), director(0), section(0)
    {}

    virtual ~NCReportItemData() {}

    uint objectType;
    uint typeId;
    uint sourceType;
    uint fitRole;

    // metric position/size in mm
    QPointF mPos;
    QPointF mOffset;		// offset pos for Zone handling
    QSizeF  mSize;			// fixed size in mm
    QSizeF  mRealSize;		// real or required size in mm (for Label, Text)
    QSizeF  mSectionSize;	// size of the current section
    QSizeF  mZoneSize;		// size of the current zone (inside the section)
    QSizeF  currentSize;

    bool autoHeight;
    bool templateMode;
    int zoneID;
    bool adjusted;
    bool printWhenResult;	// for internal usage
    bool upToDate;
    bool locked;
    bool breakToPage;	// able to break to available space
    bool pinToRight;
    bool pinToLeft;
    NCReportDef *reportDef;
    NCReportDirector* director;
    NCReportSection *section;

    QString tagname;
    QString text;
    QString printWhen;
    QString description;
    QString htmlOptions;
    QString functionId;
    QRectF boundingRectOfLastPart;
    QRectF paintRect;
    //QPointF currentScenePos, lastScenePos;

};

/*!
This class represents the base (abstract) report item class
 */
class NCREPORTSHARED_EXPORT NCReportItem : public QGraphicsItem
{
public:
    NCReportItem( QGraphicsItem* parent =0 );
    virtual ~NCReportItem();

    enum ObjectType {
        NoObject  = 0x020000,
        Label     = 0x020001,
        Field     = 0x020002,
        Text      = 0x020003,
        Line      = 0x020004,
        Rectangle = 0x020005,
        Ellipse   = 0x020006,
        Image     = 0x020007,
        Barcode   = 0x020008,
        Graph     = 0x020009,
        CrossTab  = 0x02000a,
        Table     = 0x02000b,
        Chart     = 0x02000c
    };

    enum SourceType {
        Static=0x0,
        DataSource,
        File,
        FileNameFromDataSource,
        Parameter,
        Url,
        UrlFromDataSource,
        InternalDocument,
        Variable,
        SystemVariable,
        Expression,
        Template,
        FieldDisplay
    };

    enum FitRole {
        NoFit=0x0,
        FitHeightToSection,
        FitHeightToZone
    };

    enum SelectionDirection {
        TopLeft=0x0,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        LinePoint1,
        LinePoint2,
        None
    };

    /*
     *New types and categories for the further versions
     *enum SourceType { Constant=0, DataSource, Parameter, Variable, SystemVariable };
     *enum LocationType { Internal=0, File, Url };
     *enum InterpretationType { Value=0, Template, Script };
    */

    inline NCReportData::DataType dataType() const
    { return d->dataType(); }

    inline void setDataType( NCReportData::DataType dt )
    { d->setDataType(dt); }

    inline QVariant value() const
    { return d->value(); }

    inline QVariant & valueRef()
    { return d->valueRef(); }

    inline QString id() const
    { return d->id(); }
    inline void setId( const QString& id )
    { d->setID( id ); }

    inline void setValue( const QVariant& value )
    { d->setValue( value ); }

    inline ObjectType objectType() const
    { return (ObjectType)d->objectType; }

    void setObjectType( ObjectType t )
    { d->objectType = t; }

    inline SourceType sourceType() const
    { return (SourceType)d->sourceType; }

    void setSourceType( SourceType ist)
    { d->sourceType = ist; }

    /*!
     * \brief typeId
     * \return Type identifier number for custom graphs. Not used for internal factory types
     */
    inline int typeId() const
    { return d->typeId; }

    void setTypeId( int typeId )
    { d->typeId = typeId; }

    inline QString tagname() const
    { return d->tagname; }

    virtual QString text() const
    { return d->text; }

    virtual void setText( const QString& text, const QString& = QString() )
    { d->text = text; }

    inline QString printWhen() const
    { return d->printWhen; }

    inline void setPrintWhen( const QString& condition )
    { d->printWhen = condition; }

    inline bool printWhenResult() const
    { return d->printWhenResult; }

    inline QString description() const
    { return d->description; }

    void setDescription(const QString& descr)
    { d->description = descr ; }

    inline QString htmlOptions() const
    { return d->htmlOptions; }

    void setHtmlOptions(const QString& options)
    { d->htmlOptions = options; }

    inline bool isAutoHeight() const
    { return d->autoHeight; }

    inline void setAutoHeight( bool set )
    { d->autoHeight = set; }

    inline FitRole fitRole() const
    { return (FitRole)d->fitRole; }

    void setFitRole( FitRole role )
    { d->fitRole = role; }

    // metric position/size in mm
    inline QPointF metricPos() const
    { return d->mPos; }

    inline void setMetricPos( const QPointF& point )
    { d->mPos = point ; }

    inline void setMetricX( qreal x )
    { d->mPos.setX(x); }

    inline void setMetricY( qreal y )
    { d->mPos.setY(y); }

    /*! Returns zone position offset */
    inline QPointF metricOffset() const		// offset pos for Zone handling
    { return d->mOffset; }

    /*! Sets zone position offset */
    void setMetricOffset( const QPointF& point )
    { d->mOffset = point ; }

    inline QSizeF metricSize() const			// fixed size in mm
    { return d->mSize; }

    void setMetricSize( const QSizeF& size )
    { d->mSize = size ; }

    inline void setMetricWidth( qreal w )
    { d->mSize.setWidth(w); }

    inline void setMetricHeight( qreal h )
    { d->mSize.setHeight(h); }

    void setRealMetricSize( const QSizeF& size )
    { d->mRealSize = size ; }

    virtual QSizeF sectionMetricSize() const	// size of the current section
    { return d->mSectionSize; }

    void setSectionMetricSize( const QSizeF& size )
    { d->mSectionSize = size; }

    virtual QSizeF zoneMetricSize() const		// size of the current zone (inside the section)
    { return d->mZoneSize; }

    void setZoneMetricSize( const QSizeF& size )
    { d->mZoneSize = size; }

    inline bool isTemplateMode() const
    { return d->templateMode; }

    void setTemplateMode( bool set )
    { d->templateMode = set; }

    inline int zoneID() const
    { return d->zoneID; }

    void setZoneID( int id )
    { d->zoneID = id; }

    inline bool isAdjusted() const
    { return d->adjusted; }
    void setAdjusted( bool set )
    { d->adjusted = set; }

    inline bool isUpToDate() const
    { return d->upToDate; }
    void setUpToDate( bool set )
    { d->upToDate = set; }

    inline bool isLocked() const
    { return d->locked; }
    void setLocked( bool set )
    { d->locked = set; }

    inline NCReportDef *reportDef() const
    { return d->reportDef; }

    virtual void setReportDef( NCReportDef* rdef )
    { d->reportDef = rdef; }

    inline NCReportDirector *director() const
    { return d->director; }

    void setDirector( NCReportDirector* director )
    { d->director = director; }

    inline QRectF boundingRectOfLastPart() const
    { return d->boundingRectOfLastPart; }

    void setBoundingRectOfLastPart( const QRectF& rect )
    { d->boundingRectOfLastPart = rect; }

    NCReportSection *sectionScene() const;
    virtual QRectF boundingRect() const;

    inline NCReportSection *section() const
    { return d->section; }

    void setSection( NCReportSection* section )
    { d->section = section; }

    inline QString functionId() const
    { return d->functionId; }

    void setFunctionId(const QString& id)
    { d->functionId = id; }


    /*!
      *  Returns the item painting rectangle in pixels on the specified output device.
      * @param output The report output object
      * @param painterPosMM The Item's top left position in millimeters
      */
    virtual QRectF itemRect( NCReportOutput *output, const QPointF & painterPosMM );
    virtual void setSnapPos( const QPointF& );
    virtual void resize( const QSizeF& );
    virtual void resizeInMM( const QSizeF& );
    virtual QSizeF currentSize() const;
    SelectionDirection selectionHoverState() const;
    /*!Size hint for designer*/
    virtual QSizeF sizeHint() const;
    /*! adjust size for designer */
    virtual void adjustSize();
    /*! adjust size for report rendering */
    virtual void adjustSize( NCReportOutput* );
    /*! Returns the real/required size in mm for item. Use adjustSize() before. */
    virtual QSizeF realMetricSize() const;
    virtual void offsetPastePos();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) =0;
    virtual bool read( NCReportXMLReader* );
    virtual bool write( NCReportXMLWriter* );
    /*!
    Set default settings for editor when add as new item
    */
    virtual void setDefaultForEditor()=0;
    /*!
    Updates its settings for editor after load
    */
    virtual void updateForEditor();
    /*!
    Updates the metric settings before editor saves it's data
    */
    virtual void updateForSave();
    /*!
    Refreshes the size in the scene by metric size
    */
    virtual void updateSize();
    /*!
    Refreshes the position in the scene by metric position
    */
    virtual void updatePosition();
    /*!
    Updates the item contents.
    */
    virtual void updateValue( NCReportOutput* output, NCReportEvaluator* evaluator);
    /*!
    Explicit update of the item content. For example an image item reload in some circumnstances.
    */
    virtual void updateContent();
    /*!
    Prints/paints item in high resolution device such as QPrinter
    */
    virtual void paint( NCReportOutput* output, const QPointF& painterPosMM )=0;
    /*!
    Dummy paint method for double pass mode calculations.
    */
    virtual void dummyPaint( NCReportOutput*, const QPointF& );
    /*! Returns position in item was selected */
    QPointF selectPos() const;
    QSizeF selectSize() const;
    /*! Saves the state (position, size etc.) for undo */
    virtual void saveSelectState();
    /*! Returns if item is dynamic or not. */
    virtual bool isDynamic() const;
    /*! Returns if item is empty or not. Virtual. */
    virtual bool isEmpty() const;
    virtual void enableOffsetPosition();
    virtual void disableOffsetPosition();
    virtual QPainterPath shape() const;

    virtual bool isAbleToBreak() const;
    virtual void setAbleToBreak( bool set );

    virtual qreal breakTo( qreal heightMM, NCReportOutput* output );
    virtual bool isBroken() const;
    virtual bool hasSizeAndPosition() const;

    virtual bool pinToRight() const;
    virtual void setPinToRight( bool set );

    virtual bool pinToLeft() const;
    virtual void setPinToLeft( bool set );

    void setPaintRect( const QRectF& rect );
    QRectF paintRect() const;

    virtual QList<int> availableSourceTypes() const;

    virtual void printWhenTrueAction(NCReportEvaluator *evaluator);

protected:
    friend class NCReportEvaluator;

    NCReportItemData *d;
    SelectionDirection m_selectionHoverState;
    QPointF m_selectPos;
    QSizeF m_selectSize;
    //bool m_isMoving;

protected:
    virtual void preparePaintSelection(QPainter *painter);
    virtual void paintSelection(QPainter *painter);
    virtual QRectF selectionRect( const SelectionDirection, const QSizeF& itemsize ) const;
    virtual void updateCursor(const SelectionDirection dir);

    virtual QVariant itemChange( GraphicsItemChange change, const QVariant & value );
    virtual void itemPositionChange( const QPointF& );
    virtual void hoverMoveEvent( QGraphicsSceneHoverEvent * event );
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
};

#endif
