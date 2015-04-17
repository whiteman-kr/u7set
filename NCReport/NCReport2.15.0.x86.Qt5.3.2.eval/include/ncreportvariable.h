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
#ifndef NCREPORTVARIABLE_H
#define NCREPORTVARIABLE_H

#include "ncreportdata.h"

class NCReportDataSource;

/*!
This class represents the report's variable
*/
class NCReportVariable : public NCReportData
{
public:
    NCReportVariable();
    ~NCReportVariable();

    enum FunctionType { Count=0, Sum, Average, Std, Min, Max, System };
    enum ResetScope { Report=0, Page, Group };
    enum CorrectionType { Off=0, Undo, Redo };
    struct VarAmount {
        VarAmount() : value(0), counter(0) {}
        double value;
        int counter;
        QList<double> valueHistory;
    };

    void setExpression( const QString& );	// var expr. (columnname, etc. )
    void setFunction( const FunctionType );
    void setFunction( const QString& );
    void setScope( const ResetScope );
    void setScope( const QString&  );
    void setInitValue( double val );
    //void setInitialValue( int val );
    const QString& expression() const;
    FunctionType function() const;
    ResetScope scope() const;
    double initValue() const;
    void update(const QVariant& val, CorrectionType ct = Off);
    void reset();
    void correction( CorrectionType ct );
    void modifyValue( double value );

/*	static QString functionToName( const NCReportVariable::FunctionType ) const;
    static NCReportVariable::FunctionType nameToFunction( const QString& fname ) const;*/
    QString functionToName() const;
    QString scopeToName() const;
    FunctionType nameToFunction( const QString& fname ) const;
    ResetScope nameToScope( const QString& sname ) const;
    NCReportDataSource *relatedDataSource() const;
    void setRelatedDataSource( NCReportDataSource* ds );
    //NCReportVariable *testPassVariable() const;

private:
    FunctionType m_functionType;
    ResetScope m_resetScope;
    NCReportDataSource* m_relatedDataSource;
    //NCReportVariable* m_testPassVariable;

    QString m_varExp;		// = Variable expression
    VarAmount m_currentValue;
    VarAmount m_lastValue;
    VarAmount m_initValue;
    QVariant m_lastUpdateValue;
};

#endif
