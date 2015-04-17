/****************************************************************************
*
*  Copyright (C) 2002-2011 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTCROSSTABITEMCELL_H
#define NCREPORTCROSSTABITEMCELL_H

#include <QSizeF>
#include <QRectF>
#include <QFont>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QVariant>

#include "ncreportdata.h"

QT_BEGIN_NAMESPACE
class QPaintDevice;
class QStyleOptionGraphicsItem;
QT_END_NAMESPACE

class NCReportXMLReader;
class NCReportXMLWriter;
class NCReportOutput;

class NCReportCrossTabItemCell
{
public:
	enum CellType { CornerHeader=0, ColumnHeader=1, SideSummaryHeader=2,
					RowHeader=3, Data=4, SideSummaryData=5, BottomSummaryHeader=6,
					BottomSummaryData=7, CrossSummaryData=8 };

	enum BorderPos { TopBorder=0, RightBorder, BottomBorder, LeftBorder };

	class CellBorder {
	public:
		CellBorder() : pen(QPen(Qt::black)), enabled(false), lineWidth(0.1) {}
		QPen pen;
		bool enabled;
		qreal lineWidth;
	};

	NCReportCrossTabItemCell();
	~NCReportCrossTabItemCell();

	void paint( NCReportOutput* o, const QRectF& rect, const qreal padding=0 );
	void paintBorder( BorderPos pos, NCReportOutput* o, const QRectF& rect );

	bool read(NCReportXMLReader*);
	bool write(NCReportXMLWriter*);
	bool readBorder(NCReportXMLReader*);
	bool writeBorder( BorderPos, NCReportXMLWriter*);

	void setSizeMM( const QSizeF& size) { m_sizeMM = size; }
	QSizeF sizeMM() const { return m_sizeMM; }
	void setSizePixel( const QSizeF& size) { m_size = size; }
	QSizeF sizePixel() const { return m_size; }
	void setAlignment( Qt::Alignment flag ) { m_align = flag; }
	Qt::Alignment alignment () const { return m_align; }
	void setFont( const QFont& font) { m_font = font; }
	QFont& font() { return m_font; }
	void setForeColor( const QColor& color) { m_foreColor = color; }
	QColor foreColor() const { return m_foreColor; }

	void setBrush( const QBrush& brush) { m_brush = brush; }
	QBrush& brush() { return m_brush; }

	void setBorderPen( const QPen& pen, BorderPos position ) { m_borders[position].pen = pen; }
	QPen borderPen( BorderPos position ) const { return  m_borders[position].pen; }

	void setBorderEnabled( bool enable, BorderPos position ) { m_borders[position].enabled = enable; }
	bool isBorderEnabled( BorderPos position ) const { return  m_borders[position].enabled; }

	void setType( CellType type ) { m_cellType = type; }
	CellType type() const { return m_cellType; }

	QPen& pen(const BorderPos i) { return m_borders[i].pen; }
	qreal lineWidth(const BorderPos i) const { return m_borders[i].lineWidth; }
	void setLineWidth(qreal width, const BorderPos i) { m_borders[i].lineWidth = width; }
	bool enabled(const BorderPos i) const { return m_borders[i].enabled; }
	void setEnabled(bool enabled, const BorderPos i) { m_borders[i].enabled = enabled; }
	/*
	void setLeftPen( const QPen& pen) { m_borders[LeftBorder].pen = pen; }
	QPen& leftPen() { return m_borders[LeftBorder].pen; }
	void setRightPen( const QPen& pen) { m_borders[RightBorder].pen = pen; }
	QPen& rightPen() { return m_borders[RightBorder].pen; }
	void setTopPen( const QPen& pen) { m_borders[TopBorder].pen = pen; }
	QPen& topPen() { return m_borders[TopBorder].pen; }
	void setBottomPen( const QPen& pen) { m_borders[BottomBorder].pen = pen; }
	QPen& bottomPen() { return m_borders[BottomBorder].pen; }
	*/
	void setFormatString( const QString& format) { m_formatString = format; }
	QString formatString() const { return m_formatString; }

	void setNumFormat( int fieldWidth = 0, char format = 'g', int precision = -1, const QChar & fillChar = QLatin1Char( ' ' ) )
	{
		m_fieldWidth = fieldWidth;
		m_format = format;
		m_precision = precision;
		m_fillChar = fillChar;
	}
	void setBGMode( Qt::BGMode mode ) { m_backgroundMode = mode; }
	Qt::BGMode BGMode() const { return m_backgroundMode; }

	void setFieldWidth( int fieldWidth ) { m_fieldWidth=fieldWidth; }
	int fieldWidth() const { return m_fieldWidth; }

	void setFormatCode( char code )  { m_format=code; }
	char formatCode() const { return m_format; }

	void setPrecision( int prec )  { m_precision=prec; }
	int precision() const { return m_precision; }

	void setFillChar( const QChar& ch )  { m_fillChar=ch; }
	QChar fillChar() const { return m_fillChar; }

	void setBlankIfZero( bool set )  { m_blankIfZero=set; }
	bool blankIfZero() const { return m_blankIfZero; }

	void setLocalized( bool set ) { m_localized=set; }
	bool localized() const { return m_localized; }

	void setFormatted( bool set )  { m_formatNum=set; }
	bool formatted() const { return m_formatNum; }

	void setDataType( NCReportData::DataType dt ) { m_dataType=dt; }
	NCReportData::DataType dataType() const { return m_dataType; }

	void setText( const QString& text )  { m_text=text; }
	QString text() const { return m_text; }

	void setRotation( qreal value) { m_rotation = value; }
	qreal rotation() const { return m_rotation; }

	void setValue( const QVariant& value ) { m_value = value; }
	QVariant value() const { return m_value; }

	QSizeF sizeHint(const QRectF& constrainedRect, QPaintDevice* device) const;
	CellBorder& border( const BorderPos );

private:
	QSizeF m_sizeMM;
	Qt::Alignment m_align;
	QFont m_font;
	QColor m_foreColor;
	QBrush m_brush;
	QString m_formatString;
	bool m_wordbreak;
	QVariant m_value;
	int m_fieldWidth;
	char m_format;
	int m_precision;
	bool m_blankIfZero;
	bool m_localized;
	bool m_formatNum;
	QChar m_fillChar;
	CellType m_cellType;
	NCReportData::DataType m_dataType;
	Qt::BGMode m_backgroundMode;
	/*! Size in pixels */
	QSizeF m_size;
	qreal m_rotation;
	QString m_text;

	CellBorder m_borders[4];

	void rotateText( QPainter* p, const QRectF& rect );
};

#endif // NCREPORTCROSSTABITEMCELL_H
