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

#ifndef NCREPORTPIVOTTABLEMODEL_H
#define NCREPORTPIVOTTABLEMODEL_H

#include "ncreportvariable.h"

#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QPair>

class NCReportPivotTableModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit NCReportPivotTableModel(QObject *parent = 0);
	~NCReportPivotTableModel();

	class PivotTableData
	{
	public:
		PivotTableData() : value(0), counter(0) {}
		PivotTableData(double v, int c) : value(v), counter(c) {}
		double value;
		int counter;
	};

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	bool setHeaderData( int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole );

	bool addData( const QVariant& rowData, const QVariant& columnData, double value);
	void setAggregation( int function );

	double stDeviation( const PivotTableData& data ) const;
	double average( const PivotTableData& data ) const;

private:
	NCReportVariable::FunctionType m_aggregation;
	QList<QVariant> m_rows;
	QList<QVariant> m_columns;
	QMap<QModelIndex,PivotTableData> m_data;

	bool isValidIndex( const QModelIndex &index ) const;
	//void updateData( PivotTableData& pair, double value );
};




#endif // NCREPORTPIVOTTABLEMODEL_H
