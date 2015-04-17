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
#ifndef NCREPORTEVALUATOR_H
#define NCREPORTEVALUATOR_H

#include <QString>
#include <QVariant>
#include "ncreportdata.h"
#include "ncreport_global.h"
#include "ncreportitem.h"


class NCReportDef;
class NCReportDataSource;
class NCReportFieldItem;
class NCReportVariable;
class NCReportGroup;
class NCReportDirector;
class NCReportItem;
class NCReportSection;
QT_BEGIN_NAMESPACE
class QScriptEngine;
QT_END_NAMESPACE
/*!
This class is responsible for evaluating fields, variables, dynmic items and script expressions.\n
NCReportEvaluator also evaluates the specified logical grouping conditions.
Used by NCReportDirector.
*/
class NCREPORTSHARED_EXPORT NCReportEvaluator
{
public:
    NCReportEvaluator();
    NCReportEvaluator( NCReportDef* reportDef);
    virtual ~NCReportEvaluator();

    //enum ExpType { DataSource=0, Parameter, Variable, Field };
    enum FunctionType { NoFunc=0, DataSourceFunction, ValueFunction };

    void setReportDef(NCReportDef* reportDef);
    NCReportDef* reportDef();
    void setDirector(NCReportDirector* director);
    NCReportDirector* director();
    void setSection(NCReportSection *section);
    NCReportSection* section() const;

    virtual bool evaluate( QString& exp );
    virtual bool evaluate( QString& exp, NCReportItem::SourceType stype );
    virtual bool evaluateEmbeddedScripts( QString& exp );
    virtual bool evaluateScan( QString& exp );
    virtual bool evaluateScript( const QString & exp, QString& result );
    virtual bool evaluateScript( const QString & exp, bool& result );
    virtual bool evaluateScript( const QString & exp, double& result );
    virtual bool evaluateScript( const QString & exp, QVariant& result );
    virtual bool evaluateField( NCReportFieldItem* field );
    virtual bool evaluateVariable( NCReportVariable* var );
    virtual bool evaluateGroup( NCReportGroup* g );
    virtual bool evaluatePrintWhen( const QString& expr );
    virtual bool evaluatePrintWhen( NCReportItem* );

    //bool evaluateDynamicItem( NCReportItem* );

    virtual bool evaluateDynamicText( NCReportItem* item, QString* target=0 );
    virtual bool evaluateDynamicImage( NCReportItem* item );
    virtual bool evaluateDynamicGraph( NCReportItem* item );
    virtual bool evaluateDynamicBarcode( NCReportItem* item );

    virtual bool updateFieldFromDataSource( const QString& columnExpr, QVariant& val, NCReportFieldItem* field );

public:
    static const QStringList availableSystemVariables();
    const QStringList availableVariables();
    static const char** availableDataSourceFunctions();
    static const char** availableValueFunctions();
    static const int* availableValueFunctionArguments();
    static void tokenizeKeyWordList( QStringList& keywordList, NCReportItem::SourceType type );
    static void tokenizeKeyWord( QString& keyword, NCReportItem::SourceType type );
    static QStringList arrayToStringList( const char* array[] );

protected:
    virtual bool getDataSourceValue(const QString& columnExpr, QVariant& val );

    bool getParameterValue( const QString& name, QVariant& val );
    bool getVariableValue( const QString& name, QVariant& val );
    bool getSysVarValue( const QString& name, QVariant& val );
    QString getFieldDisplayValue( const QString& id );

    void quoteMarkSafety( QString& exp );
    void quoteMarkReplace(QString &exp, bool encode );
    bool evaluateScriptPrivate( const QString & exp, QVariant& result );
    QString parseTokenExpression( const QString& expression, const QString& tokenBegin, const QString &tokenEnd, int &tokenBeginPos, int &tokenLength ) const;

    void evaluateFunctionValue(const QString& functionName, int fType, QVariant &value, NCReportDataSource *ds);
    FunctionType functionType( const QString& functionName ) const;
    bool isFunction(const QString &name) const;
    bool parseDataSourceExpression(const QString& expr, QString& dataSourceID , QString& columnName, QString& functionName, int &fType) const;

    //HRaba
    int getItemRoleInfo(QString& name);

    NCReportDataSource *currentDataSource();
    int currentDataColumnIndex();
    bool updateFieldAppearance( NCReportFieldItem* field );

private:
    QScriptEngine *m_engine;
    NCReportDef *m_reportDef;
    NCReportDirector *m_director;
    NCReportSection *m_section;

protected:
    NCReportDataSource *m_currentDataSource;
    int m_currentDataColumnIndex;
    int m_currentDataRole;

};

#endif
